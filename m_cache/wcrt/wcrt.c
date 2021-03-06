#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEF_GLOBALS
#include "header.h"
#undef DEF_GLOBALS

#include "wcrt.h"
#include "handler.h"
#include "util.h"
#include "parse.h"
#include "dump.h"
#include "topo.h"
#include "timing.h"
#include "timingMSG.h"
#include "alloc.h"
#include "slacks.h"


// ###############################################################################
// "Main" function of the WCRT analyzer submodule
// ###############################################################################


/*! Carries out the WCRT analysis, with the given input files. */
int wcrt_analysis( char* filename_cf, char *filename_pd, char *filename_dg )
{
  int i, j;
  //time_t wcet, bcet;

  cfname = filename_cf;
  pdname = filename_pd;
  dgname = filename_dg;
  
  DEBUG = 0;

  // If the input files had the form <path>/myinput.xy then we will dump the
  // debug output to <path>/myinput.1.WCRT etc, so this string saves the common
  // part (<path>/myinput) to facilitate the generation of output file names.
  sprintf( resultFileBaseName, "%s", cfname );
  
  allocweight = DEFAULT_ALLOCWEIGHT;
  //limitsoln = DEFAULT_LIMITSOLN;

  // parse input
  readConfig();
  readTasks();
  readMSG();
  readEdgeBounds();
  topoTask();
  topoGraph();
  /* For debugging */
  dumpTaskInfo();

  /* record of critical path */
  CALLOC( isCritical, char*, numTasks, sizeof(char), "isCritical" );

  // interference graph for non-greedy method
  // reset is not done here; should be done before first analysis of each chart
  MALLOC( interfere, char**, numTasks * sizeof(char*), "interfere" );
  for( i = 0; i < numTasks; i++ ) {
    CALLOC( interfere[i], char*, numTasks, sizeof(char), "interfere[i]" );
  }

  for(i = 0; i < numCharts; i++) {
	  generateWeiConflict(&(msg[i]));
  }
  writeWeiConflict();
  printf("Done writing Wei conflict....\n");
  fflush(stdout);

  readCost();

  printf("Writing our conflict now....\n");
  fflush(stdout);

  // for timing analysis
  /* Allocate memory for timing analysis parameters */
  CALLOC( earliestReq, time_t*, numTasks, sizeof(time_t), "earliestReq" );
  CALLOC( latestReq, time_t*,   numTasks, sizeof(time_t), "latestReq"   );
  CALLOC( latestReq_copy, time_t*,   numTasks, sizeof(time_t), "latestReq"   );
  CALLOC( earliestFin, time_t*, numTasks, sizeof(time_t), "earliestFin" );
  CALLOC( latestFin, time_t*,   numTasks, sizeof(time_t), "latestFin"   );
  CALLOC( peers, char **, numTasks, sizeof(char *), "peers");

  for(i = 0; i < numTasks; i++) {
		CALLOC(peers[i], char *, numTasks, sizeof(char), "peers[i]");
		
		for(j = 0; j < numTasks; j++)
		  peers[i][j] = 0;
  }
  concat = SYNCH;
  timingEstimate();

  /* dumpTaskInfo(); */

  /* Initialize all-timing info file */		  
  if(!timefp) {	  
    char filename[200];
    strcpy( filename, resultFileBaseName );
    strcat( filename, ".task_timing.db" );
	  timefp = fopen(filename, "w");
    if( !timefp ) {
      fprintf( stderr, "Failed to open file %s (main.c:138).\n", filename );
      exit(1);
    }
	}
  
  /* printf("Interfere as follow:	\n"); */
  /* Set intereference for each MSC */
  for(i = 0; i < numCharts; i++) {
    fprintf(timefp, "MSC ID %d	\n\n", i);
 	  setInterference(&(msg[i]));
 	  /* resetInterference( &(msg[i]) );
		   dumpInterference(&(msg[i]));
		*/
  }
  /* Close the timing file */
  fclose(timefp);
  writeInterference();
  
  printf("Done writing our conflict....\n");
  fflush(stdout);

  /* free memory
   freeAll(); */

  return 0;
}
