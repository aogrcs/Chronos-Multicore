// Include standard library headers
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include local library headers
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <debugmacros/debugmacros.h>

// Include local headers
#include "offsetGraph.h"
#include "header.h"
#include "handler.h"


// ############################################################
// #### Local data type definitions (will not be exported) ####
// ############################################################


enum ILPComputationType {
  ILP_TYPE_BCET,
  ILP_TYPE_WCET
};


// #########################################
// #### Declaration of static variables ####
// #########################################


// Whether to keep the temporary files generated during the analysis
static _Bool keepTemporaryFiles = 0;


// #########################################
// #### Definitions of static functions ####
// #########################################


/* Dumps a single node to the given file descriptor. */
static void dumpOffsetGraphNode( const offset_graph *og,
    const offset_graph_node *node, FILE *out )
{
  fprintf( out, "  %u: BCET %llu, WCET %llu\n", node->offset, node->bcet, node->wcet );

  uint j;
  fprintf( out, "    In-Edges : " );
  for ( j = 0; j < node->num_incoming_edges; j++ ) {
    const offset_graph_edge * const edge = &og->edges[node->incoming_edges[j]];
    fprintf( out, "%u (from node %u)", edge->edge_id, edge->start->offset );
    if ( j != node->num_incoming_edges - 1 ) {
      fprintf( out, ", " );
    }
  }
  fprintf( out, "\n" );

  fprintf( out, "    Out-Edges: " );
  for ( j = 0; j < node->num_outgoing_edges; j++ ) {
    const offset_graph_edge * const edge = &og->edges[node->outgoing_edges[j]];
    fprintf( out, "%u (to node %u)", edge->edge_id, edge->end->offset );
    if ( j != node->num_outgoing_edges - 1 ) {
      fprintf( out, ", " );
    }
  }
  fprintf( out, "\n" );
}


/* Prints the ILP-name of 'edge' to 'f'. */
static inline void printILPEdgeName( FILE *f, const offset_graph_edge *edge )
{
  fprintf( f, "x%u", edge->edge_id );
}
/* Prints the ILP-name of 'node' to 'f'. */
static inline void printILPNodeName( FILE *f, const offset_graph_node *node )
{
  fprintf( f, "y%u", node->offset );
}


/* Generate an ILP for lp_solve.
 *
 * The caller must free the returned string.
 * */
static char *generateOffsetGraphILP( const offset_graph *og, uint loopbound, enum ILPComputationType type )
{
  DSTART( "generateOffsetGraphILP" );

  // Get output file
  char *tmpfilename;
  MALLOC( tmpfilename, char*, 50 * sizeof( char ), "tmpfilename" );
  if ( keepTemporaryFiles ) {
    strcpy( tmpfilename, "offsetGraph.ilp" );
  } else {
    tmpnam( tmpfilename );
  }
  FILE *f = fopen( tmpfilename, "w" );
  if ( !f ) {
    prerr( "Unable to write to ILP file '%s'\n", tmpfilename );
  }
  DOUT( "Writing ILP to %s\n", tmpfilename );

  // In the ILP we map each edge to a variable 'x<edge_id>'
  // This variable represents the flow through the edge.
  // Similarly, we map each node to a variable 'y<offset>'
  // This variable represents the flow through the node.

  // Write objective function
  fprintf( f, "/*\n * Objective function\n */\n\n" );

  fprintf( f, ( type == ILP_TYPE_WCET ? "MAX: " : "MIN: " ) );

  uint i;
  _Bool firstObjectiveEntry = 1;
  for ( i = 0; i < og->num_edges; i++ ) {

    const offset_graph_edge * const edge = &og->edges[i];
    const ull factor = ( type == ILP_TYPE_BCET ? edge->bcet : edge->wcet );

    if ( factor != 0 ) {
      if ( !firstObjectiveEntry ) {
        fprintf( f, " + " );
      } else {
        firstObjectiveEntry = 0;
      }
      fprintf( f, "%llu ", factor );
      printILPEdgeName( f, edge );
    }
  }
  for ( i = 0; i < og->num_nodes; i++ ) {

    const offset_graph_node * const node = &og->nodes[i];
    const ull factor = ( type == ILP_TYPE_BCET ? node->bcet : node->wcet );

    if ( factor != 0 ) {
      if ( !firstObjectiveEntry ) {
        fprintf( f, " + " );
      } else {
        firstObjectiveEntry = 0;
      }
      fprintf( f, "%llu ", factor );
      printILPNodeName( f, node );
    }
  }

  fprintf( f, ";\n" );


  // Write out constraints section.
  fprintf( f, "\n/*\n * Constraints\n */\n\n" );

  // Write out flow conservation constraints
  fprintf( f, "/* Flow conservation constraints */\n" );
  for ( i = 0; i < og->num_nodes; i++ ) {
    offset_graph_node * const node = &og->nodes[i];

    // Incoming flow = Node flow
    if ( node->num_incoming_edges == 0 ) {
      fprintf( f, "0" );
    } else {
      uint j;
      for ( j = 0; j < node->num_incoming_edges; j++ ) {
        printILPEdgeName( f, &og->edges[node->incoming_edges[j]] );
        if ( j != node->num_incoming_edges - 1 ) {
          fprintf( f, " + " );
        }
      }
    }

    fprintf( f, " = " );
    printILPNodeName( f, node );
    fprintf( f, ";\n" );

    // Node flow = Outgoing flow
    printILPNodeName( f, node );
    fprintf( f, " = " );

    if ( node->num_outgoing_edges == 0 ) {
      fprintf( f, "0" );
    } else {
      uint j;
      for ( j = 0; j < node->num_outgoing_edges; j++ ) {
        printILPEdgeName( f, &og->edges[node->outgoing_edges[j]] );
        if ( j != node->num_outgoing_edges - 1 ) {
          fprintf( f, " + " );
        }
      }
    }

    fprintf( f, ";\n" );
  }

  // Write out demand / supply values
  fprintf( f, "\n/* Demand / supply constraints */\n" );
  const offset_graph_node * const suso = &( og->supersource );
  for ( i = 0; i < suso->num_outgoing_edges; i++ ) {
    printILPEdgeName( f, &og->edges[suso->outgoing_edges[i]] );
    if ( i != suso->num_outgoing_edges - 1 ) {
      fprintf( f, " + " );
    }
  }
  fprintf( f, " = %u;\n", loopbound );

  const offset_graph_node * const susi = &( og->supersink );
  for ( i = 0; i < susi->num_incoming_edges; i++ ) {
    printILPEdgeName( f, &og->edges[susi->incoming_edges[i]] );
    if ( i != susi->num_incoming_edges - 1 ) {
      fprintf( f, " + " );
    }
  }
  fprintf( f, " = %u;\n", loopbound );


  // Write out bounds section.
  fprintf( f, "\n/*\n * Variable bounds\n */\n\n" );

  for ( i = 0; i < og->num_edges; i++ ) {
    offset_graph_edge * const edge = &og->edges[i];
    printILPEdgeName( f, edge );
    fprintf( f, " >= 0;\n" );
    printILPEdgeName( f, edge );
    fprintf( f, " <= %u;\n", loopbound );
  }
  for ( i = 0; i < og->num_nodes; i++ ) {
    offset_graph_node * const node = &og->nodes[i];
    printILPNodeName( f, node );
    fprintf( f, " >= 0;\n" );
    printILPNodeName( f, node );
    fprintf( f, " <= %u;\n", loopbound );
  }


  // Write out declarations section.
  fprintf( f, "\n/*\n * Variable declarations\n */\n\n" );

  for ( i = 0; i < og->num_edges; i++ ) {
    offset_graph_edge * const edge = &og->edges[i];
    fprintf( f, "int " );
    printILPEdgeName( f, edge );
    fprintf( f, ";\n");
  }
  for ( i = 0; i < og->num_nodes; i++ ) {
    offset_graph_node * const node = &og->nodes[i];
    fprintf( f, "int " );
    printILPNodeName( f, node );
    fprintf( f, ";\n");
  }

  fclose( f );

  DRETURN( tmpfilename );
}


/* Solves a given ilp with lp_solve. */
ull solveILP( const offset_graph *og, const char *ilp_file )
{
  DSTART( "solveILP" );

  // Invoke external ILP solver.
  char commandline[500];
  char output_file[100];
  if ( keepTemporaryFiles ) {
    strcpy( output_file, "offsetGraph.result" );
  } else {
    tmpnam( output_file );
  }
  sprintf( commandline, "%s/lp_solve -presolve %s > %s\n", LP_SOLVE_PATH, ilp_file, output_file );
  DOUT( "Called lp_solve: %s", commandline );

  const int ret = system( commandline );

  if ( ret == -1 ) {
    prerr( "Failed to invoke lp_solve with: %s", commandline );
  } else
  if ( ret == 1 ) {
    prerr( "Timeout occurred, though no timeout option was given!" );
  } else
  if ( ret == 2 ) {
    prerr( "Problem was infeasible!" );
  } else
  if ( ret == 3 ) {
    prerr( "Problem was unbounded!" );
  } else
  if ( ret == 7 ) {
    prerr( "lp_solve found no solution!" );
  } else
  if ( ret == 255 ) {
    prerr( "ILP was erroneous!" );
  } else
  if ( ret != 0 ) {
    prerr( "Failed to solve ILP. Maybe solver not available." );
  }

  DOUT( "Solved ILP, output is in %s\n", output_file );

  // Parse result file
  FILE *result_file = fopen( output_file, "r" );
  ull result;
  const uint line_size = 500;
  char result_file_line[line_size];
  char result_string[50];

  _Bool successfully_read = // Skip first line
                            fgets( result_file_line, line_size, result_file ) &&
                            // Read the result in the second line
                            fgets( result_file_line, line_size, result_file ) &&
                            sscanf( result_file_line, "%*s %*s %*s %*s %s", result_string );
  if ( successfully_read ) {
    // Parse the number (first try direct format)
    char *end_ptr = NULL;
    result = strtoull( result_string, &end_ptr, 10 );
    successfully_read = *end_ptr == '\0';
    // In case of failure: Try to parse the number in scientific format (2.81864e+06)
    if ( !successfully_read ) {
      double time_result_float = strtod( result_string, &end_ptr );
      successfully_read = *end_ptr == '\0';
      if ( successfully_read ) {
        assert( floor( time_result_float ) == time_result_float && "Invalid result!" );
        result = (ull)time_result_float;
      }
    }
  }
  fclose( result_file );

  // Delete output file
  if ( !keepTemporaryFiles ) {
    remove( output_file );
  }

  if ( successfully_read ) {
    DOUT( "Result was: %llu\n", result );
    DRETURN( result );
  } else {
    prerr( "Could not read output file!" );
    DRETURN( result );
  }
}


// #########################################
// #### Definitions of public functions ####
// #########################################


/* Creates a new offset graph with 'number_of_nodes' nodes
 * (excluding the supersink and supersource). */
offset_graph *createOffsetGraph( uint number_of_nodes )
{
  offset_graph *result;
  CALLOC( result, offset_graph*, 1, sizeof( offset_graph ), "result" );

  result->supersource.offset = 4000000;
  result->supersink.offset = 3000000;

  // Create the nodes
  CALLOC( result->nodes, offset_graph_node*, number_of_nodes, 
      sizeof( offset_graph_node ), "result->nodes" );
  result->num_nodes = number_of_nodes;
  result->num_edges = 0;
  uint i;
  for ( i = 0; i < result->num_nodes; i++ ) {
    offset_graph_node * const node = &( result->nodes[i] );
    node->offset = i;
    node->bcet = 0;
    node->wcet = 0;

    // Initialize all incident edge lists with size 0
    node->incoming_edges = 0;
    node->outgoing_edges = 0;
    node->num_incoming_edges = 0;
    node->num_outgoing_edges = 0;
  }

  return result;
}


/* Returns the edge that was added or NULL if nothing was added. */
offset_graph_edge *addOffsetGraphEdge(
    offset_graph *og, offset_graph_node *start,
    offset_graph_node *end, ull bcet, ull wcet )
{
  DSTART( "addOffsetGraphEdge" );
  assert( og && start && end && "Invalid arguments!" );

  DOUT( "Requesting creation of edge %u --> %u with BCET %llu"
      " and WCET %llu\n", start->offset, end->offset, bcet, wcet );

  if ( getOffsetGraphEdge( og, start, end ) != NULL ) {
    DOUT( "Edge existed, returning NULL!\n" );
    DRETURN( NULL );
  } else {

    // Create new edge. Be aware that the addresses of all edges may
    // change here, thus rendering any pointer to an edge invalid.
    og->num_edges++;
    CALLOC_OR_REALLOC( og->edges, offset_graph_edge*,
        og->num_edges * sizeof( offset_graph_edge ), "og->edges" );
    offset_graph_edge *new_edge = &( og->edges[og->num_edges - 1] );

    new_edge->start   = start;
    new_edge->end     = end;
    new_edge->bcet    = bcet;
    new_edge->wcet    = wcet;
    new_edge->edge_id = og->num_edges;

    // Register with the nodes
    start->num_outgoing_edges++;
    CALLOC_OR_REALLOC( start->outgoing_edges, uint*,
        start->num_outgoing_edges * sizeof( uint ),
        "start->outgoing_edges" );
    start->outgoing_edges[start->num_outgoing_edges - 1] = og->num_edges - 1;

    end->num_incoming_edges++;
    CALLOC_OR_REALLOC( end->incoming_edges, uint*,
        end->num_incoming_edges * sizeof( uint ),
        "end->incoming_edges" );
    end->incoming_edges[end->num_incoming_edges - 1] = og->num_edges - 1;

    // Return the new edge
    DRETURN( new_edge );
  }
}


/* Returns the edge with the given start and end nodes, or NULL if it doesn't exist. */
offset_graph_edge *getOffsetGraphEdge( const offset_graph *og,
    const offset_graph_node *start, const offset_graph_node *end )
{
  assert( og && start && end && "Invalid arguments!" );

  uint i;
  for ( i = 0; i < start->num_outgoing_edges; i++ ) {
    const offset_graph_edge * const edge = &og->edges[start->outgoing_edges[i]];
    if ( edge->end->offset == end->offset ) {
      return &og->edges[start->outgoing_edges[i]];
    }
  }

  return NULL;
}

/* Gets the node which represents offset 'offset'. */
offset_graph_node *getOffsetGraphNode( offset_graph *og, uint offset )
{
  assert( og && "Invalid arguments!" );

  if ( offset >= og->num_nodes ) {
    return NULL;
  } else {
    return &( og->nodes[offset] );
  }
}


/* Prints the offset graph to th given file descriptor. */
void dumpOffsetGraph( const offset_graph *og, FILE *out )
{
  fprintf( out, "Offset graph with %u nodes and %u edges\n",
      og->num_nodes, og->num_edges );
  fprintf( out, "\n");

  uint i;
  fprintf( out, "Nodes: \n" );
  dumpOffsetGraphNode( og, &og->supersource, out );
  dumpOffsetGraphNode( og, &og->supersink, out );
  for ( i = 0; i < og->num_nodes; i++ ) {
    const offset_graph_node * const node = &og->nodes[i];
    dumpOffsetGraphNode( og, node, out );
  }

  fprintf( out, "\n");

  fprintf( out, "Edges: \n" );
  for ( i = 0; i < og->num_edges; i++ ) {
    const offset_graph_edge * const edge = &og->edges[i];

    fprintf( out, "  %u: node %u --> node %u with BCET %llu, WCET %llu \n",
        edge->edge_id, edge->start->offset, edge->end->offset, edge->bcet,
        edge->wcet );
  }
}


/* Solves a minimum cost flow problem to obtain the final BCET.
 *
 * 'loopbound_min' specifies the minimum number of iterations of the loop.
 */
ull computeOffsetGraphLoopBCET( const offset_graph *og, uint loopbound_min )
{
  assert( og && "Invalid arguments!" );

  char * const tmpfile = generateOffsetGraphILP( og, loopbound_min, ILP_TYPE_BCET );
  const ull result = solveILP( og, tmpfile );

  if ( !keepTemporaryFiles ) {
    remove( tmpfile );
  }
  free( tmpfile );

  return result;
}
/* Solves a maximum cost flow problem to obtain the final WCET.
 *
 * 'loopbound_max' specifies the maximum number of iterations of the loop.
 */
ull computeOffsetGraphLoopWCET( const offset_graph *og, uint loopbound_max )
{
  assert( og && "Invalid arguments!" );

  char * const tmpfile = generateOffsetGraphILP( og, loopbound_max, ILP_TYPE_WCET );
  const ull result = solveILP( og, tmpfile );

  if ( !keepTemporaryFiles ) {
    remove( tmpfile );
  }
  free( tmpfile );

  return result;
}


/* Deallocates an offset graph. */
void freeOffsetGraph( offset_graph *og )
{
  assert( og && "Invalid arguments!" );

  // Free the edges
  free( og->edges );
  og->edges = NULL;

  // Free the nodes
  uint i;
  for ( i = 0; i < og->num_nodes; i++ ) {
    const offset_graph_node * const node = &og->nodes[i];
    free( node->incoming_edges );
    free( node->outgoing_edges );
  }
  free( og->nodes );
  og->nodes = NULL;

  // Free the graph itself
  free( og );
}
