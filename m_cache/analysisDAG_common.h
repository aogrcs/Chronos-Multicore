/*! This is a header file of the Chronos timing analyzer. */

/*
 * Common functions for DAG-based analyses.
 */

#ifndef __CHRONOS_ANALYSIS_DAG_COMMON_H
#define __CHRONOS_ANALYSIS_DAG_COMMON_H

#include "header.h"

// ######### Macros #########



// ######### Datatype declarations  ###########



// ######### Function declarations  ###########

/* #### Cache analysis helper functions #### */

/* Attach best-case chmc classification to the instruction data structure */
void preprocess_chmc_BCET( procedure* proc );
/* Attach best-case chmc classification for L2 cache to the instruction
 * data structure */
void preprocess_chmc_L2_BCET( procedure* proc );

/* Attach worst-case chmc classification to the instruction data structure */
void preprocess_chmc_WCET( procedure* proc );
/* Attach worst-case chmc classification for L2 cache to the instruction
 * data structure */
void preprocess_chmc_L2_WCET( procedure* proc );

/* Return the type of the instruction access MISS/L1_HIT/L2_HIT.
 * This is computed from the shared cache analysis */
acc_type check_hit_miss(block* bb, instr* inst);

/* #### Structural analysis helper functions #### */

/* Returns the callee procedure for a procedure call instruction */
procedure* getCallee(instr* inst, procedure* proc);
/* Check whether the block specified in the header "bb"
 * is header of some loop in the procedure "proc" */
loop* check_loop(block* bb, procedure* proc);

/* #### WCET/BCET analysis helper functions #### */

/* This sets the earliest starting time of a block during BCET calculation
 * (Not context-aware) */
void set_start_time_BCET( block* bb, procedure* proc );
/* This sets the latest starting time of a block during WCET calculation.
 * (Not context-aware) */
void set_start_time_WCET( block* bb, procedure* proc );

/* Reset start and finish time of all basic blocks in this 
 * procedure */
void reset_timestamps(procedure* proc, ull start_time);
/* Reset latest start time of all tasks in the MSC before 
 * the analysis of the MSC starts */
void reset_all_task(MSC* msc);
/* Given a starting time and a particular core, this function 
 * calculates the variable memory latency for the request */
int compute_bus_delay(ull start_time, uint ncore, acc_type type);

/* #### WCRT analysis helper functions #### */

/* Given a MSC and a task inside it, this function computes
 * the earliest start time of the argument task. Finding out
 * the earliest start time is important as the bus aware BCET
 * analysis depends on the same */
void update_succ_earliest_time( MSC* msc, task_t* task );
/* Returns the latest starting of a task in the MSC */
/* Latest starting time of a task is computed as the maximum
 * of the latest finish times of all its predecessor tasks
 * imposed by the partial order of the MSC */
ull get_earliest_start_time( task_t* cur_task, uint core );

/* Given a MSC and a task inside it, this function computes
 * the latest start time of the argument task. Finding out
 * the latest start time is important as the bus aware WCET
 * analysis depends on the same */
void update_succ_task_latest_start_time( MSC* msc, task_t* task );
/* Returns the latest starting of a task in the MSC */
ull get_latest_task_start_time( task_t* cur_task, uint core );

/* #### Other helper functions #### */

/* Given a task this function returns the core number in which
 * the task is assigned to. Assignment to cores to individual
 * tasks are done statically before any analysis took place */
uint get_core(task_t* cur_task);

uint get_hex(char* hex_string);


#endif
