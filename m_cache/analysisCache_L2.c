// Include standard library headers
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Include local library headers
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <debugmacros/debugmacros.h>

// Include local headers
#include "analysisCache_L2.h"
#include "analysisCache_common.h"
#include "dump.h"


// Forward declarations of static functions

static void
calculateMust_L2(cache_line_way_t **must, int instr_addr);

static void
calculateMay_L2(cache_line_way_t **may, int instr_addr);

static void
calculatePersist_L2(cache_line_way_t **persist, int instr_addr);

static void
calculateCacheState_L2(cache_line_way_t **must, cache_line_way_t **may,
    cache_line_way_t **persist, int instr_addr);

static char
isInSet_L2_persist(int addr, cache_line_way_t **set);

static char
isInSet_L2(int addr, cache_line_way_t **set);

static cache_line_way_t **
intersectCacheState_L2(cache_line_way_t **clw_a, cache_line_way_t **clw_b);

static cache_line_way_t **
unionCacheState_L2(cache_line_way_t **clw_a, cache_line_way_t **clw_b);

static cache_line_way_t **
unionMaxCacheState_L2(cache_line_way_t **clw_a, cache_line_way_t **clw_b);

static cache_state *
allocCacheState_L2();

static void
freeCacheState_L2(cache_state *cs);

/*static void
freeCacheStateFunction_L2(procedure * proc);

static void
freeCacheStateLoop_L2(procedure *proc, loop *lp);*/

static void
freeAllFunction_L2(procedure *proc);

static void
freeAllLoop_L2(procedure *proc, loop *lp);

static char
isInCache_L2(int addr, cache_line_way_t**must);

static cache_line_way_t **
copyCacheSet_persist(cache_line_way_t **cache_set);

static cache_line_way_t **
copyCacheSet(cache_line_way_t **cache_set);

static void
freeCacheSet_L2(cache_line_way_t **cache_set);

static char
isNeverInCache_L2(int addr, cache_line_way_t**may);

static cache_state *
mapLoop_L2(procedure *pro, loop *lp);

static cache_state *
copyCacheState_L2(cache_state *cs);

static cache_state *
mapFunctionCall_L2(procedure *proc, cache_state *cs);

static void
resetFunction_L2(procedure * proc);

static void
resetLoop_L2(procedure * proc, loop * lp);



//read basic cache configuration from configFile and then
//set other cinfiguration
void
set_cache_basic_L2(char * configFile)
{

  FILE *fptr;
  int ns, na, ls, hit_latency, miss_latency, i;

  fptr = fopen(configFile, "r" );
  if (fptr == NULL) {
    fprintf(stderr, "Failed to open file: %s (analysisCacheL2.c:91)\n", configFile);
    exit (1);
  }
  
  fscanf( fptr, "%d %d %d %d %d", &ns, &na, &ls, &hit_latency, &miss_latency);
  
    cache_L2.ns = ns;
    cache_L2.na = na;
    cache_L2.ls = ls;
    cache_L2.hit_latency  = hit_latency;
    cache_L2.miss_latency = miss_latency;

   //set other configuration through the basic: ns, na, ls, cmp
    cache_L2.nsb = logBase2(cache_L2.ns);
    cache_L2.lsb = logBase2(cache_L2.ls);
    // tag bits, #tags mapping to each set
    cache_L2.ntb = MAX_TAG_BITS;
    cache_L2.nt = 1 << cache_L2.ntb;
    // tag + set bits, set + line bits, # of tag + set
    cache_L2.t_sb = cache_L2.ntb + cache_L2.nsb;
    cache_L2.s_lb = cache_L2.nsb + cache_L2.lsb;
    cache_L2.nt_s = 1 << cache_L2.t_sb;
    // cache line mask
    cache_L2.l_msk =  (1 << cache_L2.lsb) - 1;
    // set mask
    cache_L2.s_msk = 0;
    for (i = 1; i < cache_L2.ns; i <<= 1)
    cache_L2.s_msk = cache_L2.s_msk  | i;
    cache_L2.s_msk = cache_L2.s_msk << cache_L2.lsb;
    // tag mask
    cache_L2.t_msk = 0;
    for (i = 1; i < cache_L2.nt; i <<= 1)
    cache_L2.t_msk |= i;
    cache_L2.t_msk = cache_L2.t_msk << (cache_L2.lsb + cache_L2.nsb);
    // set+tag mask
    cache_L2.t_s_msk = cache_L2.t_msk | cache_L2.s_msk;
}

void
dumpCacheConfig_L2()
{
    printf("Cache Configuration as follow:\n");
    printf("nubmer of set:  %d\n", cache_L2.ns);	
    printf("nubmer of associativity:    %d\n", cache_L2.na);	
    printf("cache line size:    %d\n", cache_L2.ls);	
    printf("cache hit penalty: %d\n", cache_L2.hit_latency);
    printf("cache miss penalty: %d\n", cache_L2.miss_latency);
    printf("nubmer of set bit:  %d\n", cache_L2.nsb);	
    printf("nubmer of linesize bit: %d\n", cache_L2.lsb);	
    printf("set mask:   %u\n", cache_L2.s_msk);
    printf("tag mask:   %u\n", cache_L2.t_msk);	
    printf("tag + set mask: %u\n", cache_L2.t_s_msk);	
}


static void
calculateMust_L2(cache_line_way_t **must, int instr_addr)
{
	int i, j;
	instr_addr = TAGSET_L2(instr_addr);
	
	for(i = 0; i < cache_L2.na; i++)
	{
		if(isInWay(instr_addr, must[i]->entry, must[i]->num_entry))
		{
			if(i != 0)
			{
				must[0]->num_entry++;
				if(must[0]->num_entry == 1)
				{
					CALLOC(must[0]->entry, int*, 1, sizeof(int), "entry");
				}
				else
				{
					REALLOC(must[0]->entry, int*, (must[0]->num_entry) * sizeof(int), "enties");
				}
				must[0]->entry[(must[0]->num_entry) -1] = instr_addr;
				
				
				for(j = 0; j < must[i]->num_entry; j++)
				{
					if(must[i]->entry[j] == instr_addr)
					{
						if(j != must[i]->num_entry -1)
							must[i]->entry[j] = must[i]->entry[must[i]->num_entry -1];
						must[i]->num_entry --;
						if(must[i]->num_entry == 0)
						{
							FREE(must[i]->entry);
						}
						else
						{
							REALLOC(must[i]->entry, int*, must[i]->num_entry * sizeof(int), "entry");
						}
					}
				} // end for(j)
			}	// 	end if(i!=0)
			return;
		}
	}
	
	cache_line_way_t * tmp = must[cache_L2.na -1];
	cache_line_way_t *head = NULL;
	
	for(i = cache_L2.na - 1; i > 0 ; i--)
		must[i] = must[i -1];

	FREE(tmp);
	
	CALLOC(head, cache_line_way_t *, 1, sizeof(cache_line_way_t), "cache_line_way_t");
	CALLOC(head->entry, int*, 1, sizeof(int), "in head->entry");
	head->entry[0] = instr_addr;
	head->num_entry = 1;
	must[0] = head;
}


static void
calculateMay_L2(cache_line_way_t **may, int instr_addr)
{
	int i, j;
	instr_addr = TAGSET_L2(instr_addr);
	
	for(i = 0; i < cache_L2.na; i++)
	{
		if(isInWay(instr_addr, may[i]->entry, may[i]->num_entry))
		{
			if(i != 0)
			{
				may[0]->num_entry++;
				if(may[0]->num_entry == 1)
				{
					CALLOC(may[0]->entry, int*, may[0]->num_entry, sizeof(int), "enties");
				}
				else
				{
					REALLOC(may[0]->entry, int*, (may[0]->num_entry) * sizeof(int), "enties");
				}
				may[0]->entry[(may[0]->num_entry) -1] = instr_addr;

				
				for(j = 0; j < may[i]->num_entry; j++)
				{
					if(may[i]->entry[j] == instr_addr)
					{
						if(j != may[i]->num_entry -1)
							may[i]->entry[j] = may[i]->entry[may[i]->num_entry -1];
						may[i]->num_entry --;
						if(may[i]->num_entry == 0)
						{
							FREE(may[i]->entry);
						}
						else
						{
							REALLOC(may[i]->entry, int*, may[i]->num_entry * sizeof(int), "entry");
						}
					}
				} // end for(j)
			}	// 	end if(i!=0)
			return;
		}
	}
	
	cache_line_way_t * tmp = may[cache_L2.na -1];
	cache_line_way_t *head = NULL;
	
	for(i = cache_L2.na - 1; i > 0 ; i--)
		may[i]  = may [i -1];		
	FREE(tmp);

	CALLOC(head, cache_line_way_t *, 1, sizeof(cache_line_way_t), "cache_line_way_t");
	CALLOC(head->entry, int*, 1, sizeof(int), "in head->entry");
	head->entry[0] = instr_addr;
	head->num_entry = 1;
	may[0] = head;
}




static void
calculatePersist_L2(cache_line_way_t **persist, int instr_addr)
{
	int i, j;
	instr_addr = TAGSET_L2(instr_addr);

	/* FIXME:::: Foe persistence analysis associativity is one more than 
	 * the actual to contain all victim cache blocks */
	for(i = 0; i < cache_L2.na + 1; i++)
	{
		if(isInWay(instr_addr, persist[i]->entry, persist[i]->num_entry))
		{
			if(i != 0)
			{
				persist[0]->num_entry++;
				if(persist[0]->num_entry == 1)
				{
					CALLOC(persist[0]->entry, int*, persist[0]->num_entry, sizeof(int), "enties");
				}
				else
				{
					REALLOC(persist[0]->entry, int*, (persist[0]->num_entry) * sizeof(int), "enties");
				}
				persist[0]->entry[(persist[0]->num_entry) -1] = instr_addr;

				
				for(j = 0; j < persist[i]->num_entry; j++)
				{
					if(persist[i]->entry[j] == instr_addr)
					{
						if(j != persist[i]->num_entry -1)
							persist[i]->entry[j] = persist[i]->entry[persist[i]->num_entry -1];
						persist[i]->num_entry --;
						if(persist[i]->num_entry == 0)
						{
							FREE(persist[i]->entry);
						}
						else
						{
							REALLOC(persist[i]->entry, int*, persist[i]->num_entry * sizeof(int), "entry");
						}
					}
				} // end for(j)
			}	// 	end if(i!=0)
			return;
		}
	}
	
	//cache_line_way_t * tmp = persist[cache_L2.na -1];
	/* FIXME :::: Last block right ? */
	cache_line_way_t * tmp = persist[cache_L2.na];
	cache_line_way_t *head = NULL;
	
	for(i = cache_L2.na; i > 0 ; i--)
		persist[i]  = persist [i -1];		
	FREE(tmp);

	CALLOC(head, cache_line_way_t *, 1, sizeof(cache_line_way_t), "cache_line_way_t");
	CALLOC(head->entry, int*, 1, sizeof(int), "in head->entry");
	head->entry[0] = instr_addr;
	head->num_entry = 1;
	persist[0] = head;
}


static void
calculateCacheState_L2(cache_line_way_t **must, cache_line_way_t **may, cache_line_way_t **persist, int instr_addr)
{
	calculateMust_L2(must, instr_addr);
	calculateMay_L2(may, instr_addr);
	calculatePersist_L2(persist, instr_addr);
}

/* Needed for checking membership when updating cache
 * state of persistence cache analysis */
static char
isInSet_L2_persist(int addr, cache_line_way_t **set)
{
	int i;
	for(i = 0; i < cache_L2.na + 1; i++)
	{
		if(isInWay(addr, set[i]->entry, set[i]->num_entry) == 1)
			return i;
	}
	return -1;
}

static char
isInSet_L2(int addr, cache_line_way_t **set)
{
	int i;
	for(i = 0; i < cache_L2.na; i++)
	{
		if(isInWay(addr, set[i]->entry, set[i]->num_entry) == 1)
			return i;
	}
	return -1;
}

// from way n-> way 0, older->younger
static cache_line_way_t **
intersectCacheState_L2(cache_line_way_t **clw_a, cache_line_way_t **clw_b)
{
	int i, j, age, index, entry_b;
	//int flag = 1;
  cache_line_way_t **result;
	CALLOC(result, cache_line_way_t **, cache_L2.na, sizeof(cache_line_way_t*), "cache_line_way_t **");
	
	for(i = 0; i < cache_L2.na; i++)
	{
		CALLOC(result[i], cache_line_way_t *, 1, sizeof(cache_line_way_t), "cache_line_way_t *");
		result[i]->num_entry = 0;
		result[i]->entry = NULL;
	}


	//for each way, calculate the result of cs
	for(i = cache_L2.na - 1; i >= 0; i--)
	{
		for(j = 0; j < clw_b[i]->num_entry; j++)
		{
			entry_b = clw_b[i]->entry[j];
			age = isInSet_L2(entry_b, clw_a);
			
			//kick out entries in clw_b not in clw_a
			if(age == -1)
				continue;

			//get the older age
			if(age > i) index = age;
			else index = i;
			//add  clw_a[i].entry[j] into result[i].entry
			result[index]->num_entry ++;
			if(result[index]->num_entry == 1)
			{
				 CALLOC(result[index]->entry , int *, result[index]->num_entry, sizeof(int), "cache line way");
			}
			else
			{
				 REALLOC(result[index]->entry , int *, result[index]->num_entry*sizeof(int), "cache line way");
			}
			result[index]->entry[result[index]->num_entry - 1] = entry_b;

		} // end for

	}
	return result;
}


// from way 0-> way n, younger->older
static cache_line_way_t **
unionCacheState_L2(cache_line_way_t **clw_a, cache_line_way_t **clw_b)
{
	int i, j, age, index, entry_a, entry_b;
	//int flag = 1;
  cache_line_way_t **result;
	 CALLOC(result, cache_line_way_t **, cache_L2.na, sizeof(cache_line_way_t*), "cache_line_way_t **");
	
	for(i = 0; i < cache_L2.na; i++)
	{
		CALLOC(result[i], cache_line_way_t *, 1, sizeof(cache_line_way_t), "cache_line_way_t *");
		result[i]->num_entry = 0;
		result[i]->entry = NULL;
	}

	//for each way, calculate the result of cs
	for(i = 0; i < cache_L2.na; i++)
	{
		//for each entry in clw_a, is it in the clw_b? 
		//no, add it into result and history
		for(j = 0 ; j < clw_a[i]->num_entry; j++)
		{
			entry_a = clw_a[i]->entry[j];

			age = isInSet_L2(entry_a, clw_b);
			if(age == -1) index = i;
			else if(age < i) index = age;
			/* sudiptac ::: FIXME: Covered all cases right ? */
			else
				index = i;	 

			//add to result
			result[index]->num_entry++;
			if(result[index]->num_entry == 1)
			{
				CALLOC(result[index]->entry, int *, result[index]->num_entry, sizeof(int), "cache line way");
			}
			else
			{
				REALLOC(result[index]->entry, int *, result[index]->num_entry * sizeof(int), "cache line way");
			}
			result[index]->entry[result[index]->num_entry - 1] = entry_a;

		} //end for


		//for each entry in clw_b[i], is it in result[i]
		//no, add it into result[i]
		for(j = 0; j < clw_b[i]->num_entry; j++)
		{
			entry_b = clw_b[i]->entry[j];
			if(isInSet_L2(entry_b, clw_a) != -1)
				continue;
			else
			{
				result[i]->num_entry ++;
				if(result[i]->num_entry == 1)
				{
					CALLOC(result[i]->entry, int *, result[i]->num_entry, sizeof(int), "cache line way");
				}
				else
				{
					REALLOC(result[i]->entry, int *, result[i]->num_entry*sizeof(int), "cache line way");
				}
				result[i]->entry[result[i]->num_entry - 1] = entry_b;

			 } //end if
		} //end for

	}
	return result;
	
}




// from way n +1 -> way 0, older->younger
static cache_line_way_t **
unionMaxCacheState_L2(cache_line_way_t **clw_a, cache_line_way_t **clw_b)
{
	int i, j, age, index, entry_a, entry_b;
	//int flag = 1;
  cache_line_way_t **result;
	 CALLOC(result, cache_line_way_t **, cache_L2.na + 1, sizeof(cache_line_way_t*), "cache_line_way_t **");
	
	for(i = 0; i < cache_L2.na + 1; i++)
	{
		CALLOC(result[i], cache_line_way_t *, 1, sizeof(cache_line_way_t), "cache_line_way_t *");
		result[i]->num_entry = 0;
		result[i]->entry = NULL;
	}

	//for each way, calculate the result of cs
	for(i = cache_L2.na; i >= 0; i--)
	{
		//for each entry in clw_a, is it in the clw_b? 
		//no, add it into result 
		for(j = 0 ; j < clw_a[i]->num_entry; j++)
		{
			entry_a = clw_a[i]->entry[j];
		  
			/* FIXME:: For persistence analysis a different membership 
			 * function is needed */
			age = isInSet_L2_persist(entry_a, clw_b);
			//only in clw_a
			if(age == -1) index = i;

			//both in clw_a and clw_b
			else if(age > i) index = age;
			
			/* sudiptac ::: FIXME: Covered all cases right ? */
			else
				index = i;	 

			//add to result
			result[index]->num_entry++;
			if(result[index]->num_entry == 1)
			{
				CALLOC(result[index]->entry, int *, result[index]->num_entry, sizeof(int), "cache line way");
			}
			else
			{
				REALLOC(result[index]->entry, int *, result[index]->num_entry * sizeof(int), "cache line way");
			}
			result[index]->entry[result[index]->num_entry - 1] = entry_a;

		} //end for


		//for each entry in clw_b[i], is it in clw_a
		//no, add it into result[i]
		for(j = 0; j < clw_b[i]->num_entry; j++)
		{
			entry_b = clw_b[i]->entry[j];
			if(isInSet_L2_persist(entry_b, clw_a) != -1)
				continue;
			
			//only in clw_b
			else
			{
				result[i]->num_entry ++;
				if(result[i]->num_entry == 1)
				{
					CALLOC(result[i]->entry, int *, result[i]->num_entry, sizeof(int), "cache line way");
				}
				else
				{
					REALLOC(result[i]->entry, int *, result[i]->num_entry*sizeof(int), "cache line way");
				}
				result[i]->entry[result[i]->num_entry - 1] = entry_b;

			 } //end if
		} //end for

	}
	return result;
	
}


//allocate the memory for cache_state
static cache_state *
allocCacheState_L2()
{
  DSTART( "allocCacheState_L2" );

	int j, k;
	cache_state *result = NULL;

	CALLOC(result, cache_state *, 1, sizeof(cache_state), "cache_state_t");
	
		result->must = NULL;
		CALLOC(result->must, cache_line_way_t***, cache_L2.ns, sizeof(cache_line_way_t**), "NO set cache_line_way_t");

		result->may = NULL;
		CALLOC(result->may, cache_line_way_t***, cache_L2.ns, sizeof(cache_line_way_t**), "NO set cache_line_way_t");

		result->persist= NULL;
		CALLOC(result->persist, cache_line_way_t***, cache_L2.ns, sizeof(cache_line_way_t**), "NO set cache_line_way_t");

		for(j = 0; j < cache_L2.ns; j++)
		{
			DOUT("\nalloc CS memory for j = %d \n", j );
				CALLOC(result->must[j], cache_line_way_t**, cache_L2.na, sizeof(cache_line_way_t*), "NO assoc cache_line_way_t");

			CALLOC(result->may[j], cache_line_way_t**, cache_L2.na, sizeof(cache_line_way_t*), "NO assoc cache_line_way_t");

			CALLOC(result->persist[j], cache_line_way_t**, cache_L2.na + 1, sizeof(cache_line_way_t*), "NO assoc cache_line_way_t");

			for( k = 0; k < cache_L2.na; k++)
			{
				DOUT("\nalloc CS memory for k = %d \n", k );
				CALLOC(result->must[j][k], cache_line_way_t*, 1, sizeof(cache_line_way_t), "one cache_line_way_t");
				result->must[j][k]->num_entry = 0;
				result->must[j][k]->entry = NULL;

				CALLOC(result->may[j][k], cache_line_way_t*, 1, sizeof(cache_line_way_t), "one cache_line_way_t");
				result->may[j][k]->num_entry = 0;
				result->may[j][k]->entry = NULL;

				CALLOC(result->persist[j][k], cache_line_way_t*, 1, sizeof(cache_line_way_t), "one cache_line_way_t");
				result->persist[j][k]->num_entry = 0;
				result->persist[j][k]->entry = NULL;
			}
			CALLOC(result->persist[j][cache_L2.na], cache_line_way_t*, 1, sizeof(cache_line_way_t), "one cache_line_way_t");
			result->persist[j][cache_L2.na]->num_entry = 0;
			result->persist[j][cache_L2.na]->entry = NULL;

			
		}

	DRETURN( result );
}






static void
freeCacheState_L2(cache_state *cs)
{
	int i, j;
	
	for(i = 0; i < cache_L2.ns; i++ )
	{
		for(j = 0; j < cache_L2.na; j++)
		{
			if(cs->must[i][j]->entry != NULL)
				FREE(cs->must[i][j]->entry);

			if(cs->may[i][j]->entry != NULL)
				FREE(cs->may[i][j]->entry);

			if(cs->persist[i][j]->entry != NULL)
				FREE(cs->persist[i][j]->entry);

			FREE(cs->must[i][j]);
			FREE(cs->may[i][j]);
			FREE(cs->persist[i][j]);
	
		}
		
		if(cs->persist[i][cache_L2.na]->entry != NULL)
			FREE(cs->persist[i][cache_L2.na]->entry);
		FREE(cs->persist[i][cache_L2.na]);
		
		FREE(cs->must[i]);
		FREE(cs->may[i]);
		FREE(cs->persist[i]);
	}

	FREE(cs->must);
	FREE(cs->may);
	FREE(cs->persist);
	FREE(cs);
}

/*static void
freeCacheStateFunction_L2(procedure * proc)
{
	procedure *p = proc;
	block *bb;
	int i;

	int  num_blk = p->num_topo;
	


	for(i = num_blk -1 ; i >= 0 ; i--)
	{

		bb = p ->topo[i];

		bb = p->bblist[bb->bbid];

		bb->num_cache_state_L2= 0;
		if(bb->bb_cache_state_L2!= NULL)
		{
			freeCacheState_L2(bb->bb_cache_state_L2);	
			bb->bb_cache_state_L2 = NULL;
		}
	}

	
}


static void
freeCacheStateLoop_L2(procedure *proc, loop *lp)
{
	procedure *p = proc;
	loop *loop_ptr = lp;
	block *bb;
	int i;

	int  num_blk = loop_ptr->num_topo;

	for(i = num_blk -1 ; i >= 0 ; i--)
	{
		bb = loop_ptr->topo[i];

		bb = p->bblist[bb->bbid];

		if(bb->is_loophead && i == num_blk -1) continue;

		bb->num_cache_state_L2 = 0;
		if(bb->bb_cache_state_L2!= NULL)
		{
			freeCacheState_L2(bb->bb_cache_state_L2);
			bb->bb_cache_state_L2 = NULL;
		}
	}
}*/



static void
freeAllFunction_L2(procedure *proc)
{
	procedure *p = proc;
	block *bb;
	int i;
	int  num_blk = p->num_topo;	
		
	for(i = num_blk -1 ; i >= 0 ; i--)
	{	
		bb = p ->topo[i];
		bb = p->bblist[ bb->bbid ];

		if(bb->is_loophead)
		  freeAllLoop_L2(p, p->loops[bb->loopid]);
		else if(bb->callpid != -1)
		{
			bb->num_cache_state_L2 = 0;
			if(bb->bb_cache_state_L2 != NULL)
			{
				FREE(bb->bb_cache_state_L2);
				bb->bb_cache_state_L2 = NULL;
			}
			freeAllFunction_L2(bb->proc_ptr);
		}
		else
		{
			bb->num_cache_state_L2 = 0;
			if(bb->bb_cache_state_L2 != NULL)
			{
				FREE(bb->bb_cache_state_L2);
				bb->bb_cache_state_L2 = NULL;
			}
		}
	}

}

static void
freeAllLoop_L2(procedure *proc, loop *lp)
{
	procedure *p = proc;
	loop *lp_ptr = lp;
	block *bb;
	int i;
	
	int  num_blk = lp_ptr->num_topo;	
		
	for(i = num_blk -1 ; i >= 0 ; i--)
	{	
		bb = lp_ptr ->topo[i];
		bb = p->bblist[ bb->bbid ];

		if(bb->is_loophead && i!= num_blk -1)
		  freeAllLoop_L2(p, p->loops[bb->loopid]);
		else if(bb->callpid != -1)
		{
			bb->num_cache_state_L2 = 0;
			if(bb->bb_cache_state_L2 != NULL)
			{
				FREE(bb->bb_cache_state_L2);
				bb->bb_cache_state_L2 = NULL;
			}
			freeAllFunction_L2(bb->proc_ptr);
		}
		else
		{
			bb->num_cache_state_L2 = 0;
			if(bb->bb_cache_state_L2 != NULL)
			{
				FREE(bb->bb_cache_state_L2);
				bb->bb_cache_state_L2 = NULL;
			}
		}
	}

}

void
freeAll_L2()
{
	freeAllFunction_L2(main_copy);
}


static char
isInCache_L2(int addr, cache_line_way_t**must)
{
	int i;
	addr = TAGSET_L2(addr);
	
	for(i = 0; i < cache_L2.na; i++)
		if(isInWay(addr, must[i]->entry, must[i]->num_entry))
			return i;
	return -1;	
}

/* For persistence analysis the function would be different */

static cache_line_way_t **
copyCacheSet_persist(cache_line_way_t **cache_set)
{
	int i, j;
	cache_line_way_t **src = cache_set, **copy;
	 CALLOC(copy, cache_line_way_t**, cache_L2.na + 1, sizeof(cache_line_way_t*), "cache_line_way_t*");
	for(i = 0; i < cache_L2.na + 1; i++)
	{
		CALLOC(copy[i], cache_line_way_t*, 1, sizeof(cache_line_way_t), "cache_line_way_t");

		copy[i]->num_entry = src[i]->num_entry;
		if(copy[i]->num_entry)
		{
			 CALLOC(copy[i]->entry, int*, copy[i]->num_entry, sizeof(int), "cache_line_way_t");
			for(j = 0; j < copy[i]->num_entry; j++)
				copy[i]->entry[j] = src[i]->entry[j];
		}
	}
	return copy;
}

static cache_line_way_t **
copyCacheSet(cache_line_way_t **cache_set)
{
	int i, j;
	cache_line_way_t **src = cache_set, **copy;
	 CALLOC(copy, cache_line_way_t**, cache_L2.na, sizeof(cache_line_way_t*), "cache_line_way_t*");
	for(i = 0; i < cache_L2.na; i++)
	{
    CALLOC(copy[i], cache_line_way_t*, 1, sizeof(cache_line_way_t), "cache_line_way_t");

		copy[i]->num_entry = src[i]->num_entry;
		if(copy[i]->num_entry)
		{
			 CALLOC(copy[i]->entry, int*, copy[i]->num_entry, sizeof(int), "cache_line_way_t");
			for(j = 0; j < copy[i]->num_entry; j++)
				copy[i]->entry[j] = src[i]->entry[j];
		}
	}
	return copy;
}


static void
freeCacheSet_L2(cache_line_way_t **cache_set)
{
	int i;
	for(i = 0; i < cache_L2.na; i++)
	{
		if(cache_set[i]->entry != NULL)
			FREE(cache_set[i]->entry);
		FREE(cache_set[i]);
	}

	FREE(cache_set);

}

static char
isNeverInCache_L2(int addr, cache_line_way_t**may)
{
	int i;
	addr = TAGSET_L2(addr);
	
	for(i = 0; i < cache_L2.na; i++)
		if(isInWay(addr, may[i]->entry, may[i]->num_entry))
			return 0;
	return 1;
	
}



static cache_state *
mapLoop_L2(procedure *pro, loop *lp)
{
  DSTART( "mapLoop_L2" );

	int i, j, k, n, set_no, cnt, tmp, addr, addr_next, copies, age; 
	int lp_level, tag, tag_next;
	
	procedure *p = pro;

	block *bb, *incoming_bb;
	cache_state *cs_ptr;
	cache_line_way_t **cache_set_must, **cache_set_may, **cache_set_persist, **clw;

	int  num_blk = lp->num_topo;

	//cs_ptr = copyCacheState_L2(cs);
	
	CHMC *current_chmc;

	for(i = 0; i < MAX_NEST_LOOP; i++)
		if(loop_level_arr[i] == INVALID)
		{
			lp_level = i -1;
			break;
		}
		
	cnt = 0;
	for(k = 0; k <= lp_level; k++)
		if(loop_level_arr [k] == NEXT_ITERATION)
			cnt += (1<<(lp_level - k));


	for(i = num_blk -1 ; i >= 0 ; i--)
	{

		bb = lp ->topo[i];
		
		bb = p->bblist[bb->bbid];

		bb->num_instr = bb->size / INSN_SIZE;

		if(bb->is_loophead && i != num_blk -1)
		{

			loop_level_arr[lp_level + 1] = FIRST_ITERATION;
			p->loops[bb->loopid]->num_fm_L2 = 0;

			mapLoop_L2(p, p->loops[bb->loopid]);

			//cs_ptr = bb->bb_cache_state_L2;
			 
			loop_level_arr[lp_level + 1] = NEXT_ITERATION;

			mapLoop_L2(p, p->loops[bb->loopid]);

			//cs_ptr = copyCacheState_L2(bb->bb_cache_state_L2);

			loop_level_arr[lp_level + 1] = INVALID;

			//freeCacheStateLoop_L2(p, p->loops[bb->loopid]);
			continue;
		}



		if(bb->is_loophead == 0)
		{
			incoming_bb = p->bblist[bb->incoming[0]];

			if(!incoming_bb->bb_cache_state_L2) 
				continue;	 

			cs_ptr = copyCacheState_L2(incoming_bb->bb_cache_state_L2);

			if(bb->num_incoming > 1)
			{
				DOUT("\ndo operations if more than one incoming edge\n");
				DACTION( dumpCacheState(cs_ptr); );
				
				for(j = 1; j < bb->num_incoming; j++)
				{
					incoming_bb = p->bblist[bb->incoming[j]];
					
					if(incoming_bb->bb_cache_state_L2 == NULL) continue;
					
					for(k = 0; k < cache_L2.ns; k++)
					{
						clw = cs_ptr->must[k];							
						cs_ptr->must[k] = intersectCacheState_L2(cs_ptr->must[k], incoming_bb->bb_cache_state_L2->must[k]);

						freeCacheSet_L2(clw);

						clw = cs_ptr->may[k];							
						cs_ptr->may[k] = unionCacheState_L2(cs_ptr->may[k], incoming_bb->bb_cache_state_L2->may[k]);

						freeCacheSet_L2(clw);

						clw = cs_ptr->persist[k];							
						cs_ptr->persist[k] = unionMaxCacheState_L2(cs_ptr->persist[k], incoming_bb->bb_cache_state_L2->persist[k]);

						freeCacheSet_L2(clw);

					}
				} //end for(all incoming)

			}

			
		}

		else
		{
			if(loop_level_arr[lp_level] == FIRST_ITERATION)
			{
				incoming_bb = p->bblist[bb->incoming[0]];

				//FREE(cs_ptr);
				if(!incoming_bb->bb_cache_state_L2)
					 continue;
			
				cs_ptr = copyCacheState_L2(incoming_bb->bb_cache_state_L2);

				if(bb->num_incoming > 1)
				{
					DOUT("\ndo operations if more than one incoming edge\n");
					DACTION( dumpCacheState(cs_ptr); );
					
					for(j = 1; j < bb->num_incoming; j++)
					{
						incoming_bb = p->bblist[bb->incoming[j]];

						if(incoming_bb->bb_cache_state_L2 == NULL) continue;
	
						
						for(k = 0; k < cache_L2.ns; k++)
						{
							clw = cs_ptr->must[k];							
							cs_ptr->must[k] = intersectCacheState_L2(cs_ptr->must[k], incoming_bb->bb_cache_state_L2->must[k]);

							freeCacheSet_L2(clw);

							clw = cs_ptr->may[k];							
							cs_ptr->may[k] = unionCacheState_L2(cs_ptr->may[k], incoming_bb->bb_cache_state_L2->may[k]);

							freeCacheSet_L2(clw);

							clw = cs_ptr->persist[k];							
							cs_ptr->persist[k] = unionMaxCacheState_L2(cs_ptr->persist[k], incoming_bb->bb_cache_state_L2->persist[k]);

							freeCacheSet_L2(clw);

						}
					} //end for(all incoming)

				}

			}
			else if(loop_level_arr[lp_level] == NEXT_ITERATION)
			{
				if(! (p->bblist[lp->topo[0]->bbid]->bb_cache_state_L2))
					 continue;

				cs_ptr = copyCacheState_L2(p->bblist[lp->topo[0]->bbid]->bb_cache_state_L2);
				//cache_state *cs_tmp = cs_ptr;
				//freeCacheState(cs_tmp);


			}
			else
			{
				DOUT("\nCFG error!\n");
				exit(1);
			}
		}

		if(bb->num_cache_state_L2 == 0)
		{
			//CALLOC(bb->bb_cache_state_L2, cache_state *, 1, sizeof(cache_state), "cache_state");

			bb->num_cache_state_L2 = 1;
		}

		if(bb->num_chmc_L2 == 0)
		{
			copies = 2<<(lp_level);

			bb->num_chmc_L2 = copies;

			CALLOC(bb->chmc_L2, CHMC**, copies, sizeof(CHMC*), "CHMC");

			for(tmp = 0; tmp < copies; tmp++)
			{
				CALLOC(bb->chmc_L2[tmp], CHMC*, 1, sizeof(CHMC), "CHMC");
			}

		}

		bb->bb_cache_state_L2 = cs_ptr;
		
		current_chmc = bb->chmc_L2[cnt];

		current_chmc->hit = 0;
		current_chmc->hit_copy = 0;

		current_chmc->miss = 0;
		
		current_chmc->unknow = 0;
		current_chmc->unknow_copy = 0;

		current_chmc->wcost = 0;
		current_chmc->bcost = 0;
		current_chmc->wcost_copy = 0;
		current_chmc->bcost_copy = 0;
	
		
		//current_chmc->hit_addr = NULL;
		//current_chmc->miss_addr = NULL;
		//current_chmc->unknow_addr = NULL;
		//current_chmc->hitmiss_addr= NULL;
		//current_chmc->hit_change_miss= NULL;

		//int start_addr = bb->startaddr;
		//bb->num_cache_fetch_L2 = bb->size / cache_L2.ls;
		
		//int start_addr_fetch = (start_addr >> cache_L2.lsb) << cache_L2.lsb;
		//tmp = start_addr - start_addr_fetch;

		
		//if(tmp) bb->num_cache_fetch_L2++;

		current_chmc->hitmiss = bb->num_instr;
		//if(current_chmc->hitmiss > 0) 
			CALLOC(current_chmc->hitmiss_addr, char*, current_chmc->hitmiss, sizeof(char),"hitmiss_addr");

		addr = bb->startaddr;
		
		for(n = 0; n < bb->num_instr ; n++)
		{
			set_no = SET_L2(addr);

			if(isInWay(addr, bb->chmc[cnt]->hit_addr, bb->chmc[cnt]->hit))
			{
				current_chmc->hitmiss_addr[n] = HIT_UPPER;
				addr = addr + INSN_SIZE;
				continue;
			}

			if(main_copy->hit_cache_set_L2[set_no] != USED)
			{	
				numConflictTask[set_no]++;
				numConflictMSC[set_no]++;
				//not hit in L1?
				main_copy->hit_cache_set_L2[set_no] = USED;
			}

			main_copy->hit_addr[set_no].num_entry++;
			if(main_copy->hit_addr[set_no].num_entry == 1)
			{
				CALLOC(main_copy->hit_addr[set_no].entry, int*, 1, sizeof(int), "entry");
				main_copy->hit_addr[set_no].entry[0] = TAGSET_L2(addr); 
			}
			else
			{
				REALLOC(main_copy->hit_addr[set_no].entry, int*, main_copy->hit_addr[set_no].num_entry * sizeof(int), "entry");
				main_copy->hit_addr[set_no].entry[main_copy->hit_addr[set_no].num_entry -1] = TAGSET_L2(addr); 
			}


			age = isInCache_L2(addr, bb->bb_cache_state_L2->must[set_no]);
			if(age != -1)
			{
				current_chmc->hitmiss_addr[ n ] = ALWAYS_HIT;

				main_copy->hit_cache_set_L2[set_no] = USED;
				
				if(current_chmc->hit == 0)
				{
					current_chmc->hit++;
					CALLOC(current_chmc->hit_addr, int*, 1, sizeof(int), "hit_addr");

					CALLOC(current_chmc->hit_change_miss, char*, 1, sizeof(char), "hit_change_miss");
					CALLOC(current_chmc->age, char*, 1, sizeof(char), "age");

					current_chmc->hit_addr[current_chmc->hit-1] = addr;
					current_chmc->hit_change_miss[current_chmc->hit-1] = HIT;
					current_chmc->age[current_chmc->hit-1] = age;
					
				}
				else
				{
					current_chmc->hit++;	

					REALLOC(current_chmc->hit_addr, int*, current_chmc->hit * sizeof(int), "hit_addr");				
					REALLOC(current_chmc->hit_change_miss, char*, current_chmc->hit * sizeof(char), "hit_change_miss");				
					REALLOC(current_chmc->age, char*, current_chmc->hit * sizeof(char), "age");				

					current_chmc->hit_addr[current_chmc->hit-1] = addr;
					current_chmc->hit_change_miss[current_chmc->hit-1] = HIT;
					current_chmc->age[current_chmc->hit-1] = age;

				}
				
				addr_next = addr + INSN_SIZE;
				tag = SET_L2(addr);
				tag_next = SET_L2(addr_next);
				while(tag == tag_next && n < bb->num_instr - 1)
				{
					n++;
					current_chmc->hitmiss_addr[n] = ALWAYS_HIT;
					if(!isInWay(addr_next, bb->chmc[cnt]->hit_addr, bb->chmc[cnt]->hit))
					{
						if(current_chmc->hit == 0)
					       {
						current_chmc->hit++;
						CALLOC(current_chmc->hit_addr, int*, 1, sizeof(int), "hit_addr");

						CALLOC(current_chmc->hit_change_miss, char*, 1, sizeof(char), "hit_change_miss");
						CALLOC(current_chmc->age, char*, 1, sizeof(char), "age");

						current_chmc->hit_addr[current_chmc->hit-1] = addr_next;
						current_chmc->hit_change_miss[current_chmc->hit-1] = HIT;
						current_chmc->age[current_chmc->hit-1] = age;
					
					     	}
					  	else
					  	{
						current_chmc->hit++;		
						
						REALLOC(current_chmc->hit_addr, int*, current_chmc->hit * sizeof(int), "hit_addr");				
						REALLOC(current_chmc->hit_change_miss, char*, current_chmc->hit * sizeof(char), "hit_change_miss");				
						REALLOC(current_chmc->age, char*, current_chmc->hit * sizeof(char), "age");				

						current_chmc->hit_addr[current_chmc->hit-1] = addr_next;
						current_chmc->hit_change_miss[current_chmc->hit-1] = HIT;
						current_chmc->age[current_chmc->hit-1] = age;

						}
					}	
					
                                        addr_next += INSN_SIZE;
	                                tag_next = SET_L2(addr_next);
             						  

				}


				
			}
			else if(isNeverInCache_L2(addr, bb->bb_cache_state_L2->may[set_no]))
			{
				current_chmc->hitmiss_addr[ n ] = ALWAYS_MISS;

				main_copy->hit_cache_set_L2[set_no] = USED;
					
				if(current_chmc->miss == 0)
				{
					current_chmc->miss++;
					
					CALLOC(current_chmc->miss_addr, int*, 1, sizeof(int), "miss_addr");
					current_chmc->miss_addr[current_chmc->miss-1] = addr;
				}
				else
				{
					current_chmc->miss++;		
					
					REALLOC(current_chmc->miss_addr, int*, current_chmc->miss * sizeof(int), "miss_addr");				
					current_chmc->miss_addr[current_chmc->miss-1] = addr;
				}
				
				addr_next = addr + INSN_SIZE;
				tag = SET_L2(addr);
				tag_next = SET_L2(addr_next);
				while(tag == tag_next && n < bb->num_instr - 1)
				{
					n++;
					current_chmc->hitmiss_addr[n] = ALWAYS_HIT; 
					
					if(!isInWay(addr_next, bb->chmc[cnt]->hit_addr, bb->chmc[cnt]->hit))
					{
						if(current_chmc->hit == 0)
					       {
						current_chmc->hit++;
						CALLOC(current_chmc->hit_addr, int*, 1, sizeof(int), "hit_addr");

						CALLOC(current_chmc->hit_change_miss, char*, 1, sizeof(char), "hit_change_miss");
						CALLOC(current_chmc->age, char*, 1, sizeof(char), "age");

						current_chmc->hit_addr[current_chmc->hit-1] = addr_next;
						current_chmc->hit_change_miss[current_chmc->hit-1] = HIT;
						current_chmc->age[current_chmc->hit-1] = cache_L2.na - 1;
						
					     	}
					  	else
					  	{
						current_chmc->hit++;		
						
						REALLOC(current_chmc->hit_addr, int*, current_chmc->hit * sizeof(int), "hit_addr");				
						REALLOC(current_chmc->hit_change_miss, char*, current_chmc->hit * sizeof(char), "hit_change_miss");				
						REALLOC(current_chmc->age, char*, current_chmc->hit * sizeof(char), "age");				

						current_chmc->hit_addr[current_chmc->hit-1] = addr_next;
						current_chmc->hit_change_miss[current_chmc->hit-1] = HIT;
						current_chmc->age[current_chmc->hit-1] = cache_L2.na - 1;

						}
					}	
					
                                        addr_next += INSN_SIZE;
	                                tag_next = SET_L2(addr_next);
             						  

				}


			}

			else if(isInCache_L2(addr, bb->bb_cache_state_L2->persist[set_no]) != -1)
			{
				current_chmc->hitmiss_addr[ n ] = FIRST_MISS;
				lp->num_fm_L2 ++;

				main_copy->hit_cache_set_L2[set_no] = USED;
				
				if(current_chmc->hit == 0)
				{
					current_chmc->hit++;
					CALLOC(current_chmc->hit_addr, int*, 1, sizeof(int), "hit_addr");

					CALLOC(current_chmc->hit_change_miss, char*, 1, sizeof(char), "hit_change_miss");
					CALLOC(current_chmc->age, char*, 1, sizeof(char), "age");

					current_chmc->hit_addr[current_chmc->hit-1] = addr;
					current_chmc->hit_change_miss[current_chmc->hit-1] = HIT;
					current_chmc->age[current_chmc->hit-1] = age;
					
				}
				else
				{
					current_chmc->hit++;	

					REALLOC(current_chmc->hit_addr, int*, current_chmc->hit * sizeof(int), "hit_addr");				
					REALLOC(current_chmc->hit_change_miss, char*, current_chmc->hit * sizeof(char), "hit_change_miss");				
					REALLOC(current_chmc->age, char*, current_chmc->hit * sizeof(char), "age");				

					current_chmc->hit_addr[current_chmc->hit-1] = addr;
					current_chmc->hit_change_miss[current_chmc->hit-1] = HIT;
					current_chmc->age[current_chmc->hit-1] = age;

				}
				
				addr_next = addr + INSN_SIZE;
				tag = SET_L2(addr);
				tag_next = SET_L2(addr_next);
				while(tag == tag_next && n < bb->num_instr - 1)
				{
					n++;
					
					current_chmc->hitmiss_addr[n] = ALWAYS_HIT;
					
					if(!isInWay(addr_next, bb->chmc[cnt]->hit_addr, bb->chmc[cnt]->hit))
					{
						if(current_chmc->hit == 0)
					       {
						current_chmc->hit++;
						CALLOC(current_chmc->hit_addr, int*, 1, sizeof(int), "hit_addr");

						CALLOC(current_chmc->hit_change_miss, char*, 1, sizeof(char), "hit_change_miss");
						CALLOC(current_chmc->age, char*, 1, sizeof(char), "age");

						current_chmc->hit_addr[current_chmc->hit-1] = addr_next;
						current_chmc->hit_change_miss[current_chmc->hit-1] = HIT;
						current_chmc->age[current_chmc->hit-1] = age;
					
					     	}
					  	else
					  	{
						current_chmc->hit++;		
						
						REALLOC(current_chmc->hit_addr, int*, current_chmc->hit * sizeof(int), "hit_addr");				
						REALLOC(current_chmc->hit_change_miss, char*, current_chmc->hit * sizeof(char), "hit_change_miss");				
						REALLOC(current_chmc->age, char*, current_chmc->hit * sizeof(char), "age");				

						current_chmc->hit_addr[current_chmc->hit-1] = addr_next;
						current_chmc->hit_change_miss[current_chmc->hit-1] = HIT;
						current_chmc->age[current_chmc->hit-1] = age;

						}
					}	
					
                                        addr_next += INSN_SIZE;
	                                tag_next = SET_L2(addr_next);
             						  

				}
			}

			else
			{
				current_chmc->hitmiss_addr[ n ] = UNKNOW;

				main_copy->hit_cache_set_L2[set_no] = USED;
				
				if(current_chmc->unknow == 0)
				{
					current_chmc->unknow++;
					CALLOC(current_chmc->unknow_addr, int*, 1, sizeof(int), "unknow_addr");
					current_chmc->unknow_addr[current_chmc->unknow-1] = addr;
				}
				else
				{
					current_chmc->unknow++;				
					REALLOC(current_chmc->unknow_addr, int*, current_chmc->unknow * sizeof(int), "unknow_addr");				
					current_chmc->unknow_addr[current_chmc->unknow-1] = addr;
				}
				
				addr_next = addr + INSN_SIZE;
				tag = SET_L2(addr);
				tag_next = SET_L2(addr_next);
				while(tag == tag_next && n < bb->num_instr - 1)
				{
					n++;
					current_chmc->hitmiss_addr[n] = ALWAYS_HIT; 
					
					if(!isInWay(addr_next, bb->chmc[cnt]->hit_addr, bb->chmc[cnt]->hit))
					{
						if(current_chmc->hit == 0)
					       {
						current_chmc->hit++;
						CALLOC(current_chmc->hit_addr, int*, 1, sizeof(int), "hit_addr");

						CALLOC(current_chmc->hit_change_miss, char*, 1, sizeof(char), "hit_change_miss");
						CALLOC(current_chmc->age, char*, 1, sizeof(char), "age");

						current_chmc->hit_addr[current_chmc->hit-1] = addr_next;
						current_chmc->hit_change_miss[current_chmc->hit-1] = HIT;
						current_chmc->age[current_chmc->hit-1] = cache_L2.na - 1;
				
					     	}
					  	else
					  	{
						current_chmc->hit++;		
						
						REALLOC(current_chmc->hit_addr, int*, current_chmc->hit * sizeof(int), "hit_addr");				
						REALLOC(current_chmc->hit_change_miss, char*, current_chmc->hit * sizeof(char), "hit_change_miss");				
						REALLOC(current_chmc->age, char*, current_chmc->hit * sizeof(char), "age");				

						current_chmc->hit_addr[current_chmc->hit-1] = addr_next;
						current_chmc->hit_change_miss[current_chmc->hit-1] = HIT;
						current_chmc->age[current_chmc->hit-1] = cache_L2.na - 1;

						}
					}	
					
                                        addr_next += INSN_SIZE;
	                                tag_next = SET_L2(addr_next);
             						  

				}

				
			}
			addr = addr_next;

		}

		for(n = 0; n < bb->num_instr; n++)
		{
			//L1 hit
			if(bb->chmc[cnt]->hitmiss_addr[ n ] == ALWAYS_HIT)
			{
				current_chmc->wcost += IC_HIT;
				current_chmc->bcost += IC_HIT;
			}

			//L1 fm
			else if(bb->chmc[cnt]->hitmiss_addr[ n ] == FIRST_MISS)
			{
				if(loop_level_arr[lp_level] == FIRST_ITERATION)
				{
					if(current_chmc->hitmiss_addr[ n ] == ALWAYS_HIT)
					{
						current_chmc->wcost += IC_HIT_L2;
						current_chmc->bcost += IC_HIT_L2;
					}
					
					else if(current_chmc->hitmiss_addr[ n ] == ALWAYS_MISS ||current_chmc->hitmiss_addr[ n ] == FIRST_MISS )
					{
						current_chmc->wcost += IC_MISS_L2;
						current_chmc->bcost += IC_MISS_L2;
					}
					
					else
					{
						current_chmc->wcost += IC_MISS_L2;
						current_chmc->bcost += IC_HIT_L2;
					}
				}
				
				else if(loop_level_arr[lp_level] == NEXT_ITERATION)
				{
					current_chmc->wcost += IC_HIT;
					current_chmc->bcost += IC_HIT;
				}
					
			}
			
			//L1 miss
			else if(bb->chmc[cnt]->hitmiss_addr[ n ] == ALWAYS_MISS)
			{
				if(current_chmc->hitmiss_addr[ n ] == ALWAYS_HIT)
				{
					current_chmc->wcost += IC_HIT_L2;
					current_chmc->bcost += IC_HIT_L2;
				}

				else if(current_chmc->hitmiss_addr[ n ] == FIRST_MISS)
				{
					if(loop_level_arr[lp_level] == FIRST_ITERATION)
					{
						current_chmc->wcost += IC_MISS_L2;
						current_chmc->bcost += IC_MISS_L2;
					}
					else if(loop_level_arr[lp_level] == NEXT_ITERATION)
					{
						current_chmc->wcost += IC_HIT_L2;
						current_chmc->bcost += IC_HIT_L2;
					}
				}
				
				else if(current_chmc->hitmiss_addr[ n ] == ALWAYS_MISS)
				{
					current_chmc->wcost += IC_MISS_L2;
					current_chmc->bcost += IC_MISS_L2;

				}
				else
				{
					current_chmc->wcost += IC_MISS_L2;
					current_chmc->bcost += IC_HIT_L2;
				}
			}
			//L1 unknow
			else
			{
				if(current_chmc->hitmiss_addr[ n ] == ALWAYS_HIT)
				{
					current_chmc->wcost += IC_HIT_L2;
					current_chmc->bcost += IC_HIT;
				}

				else if(current_chmc->hitmiss_addr[ n ] == FIRST_MISS)
				{
					if(loop_level_arr[lp_level] == FIRST_ITERATION)
					{
						current_chmc->wcost += IC_MISS_L2;
						current_chmc->bcost += IC_HIT;
					}
					else if(loop_level_arr[lp_level] == NEXT_ITERATION)
					{
						current_chmc->wcost += IC_HIT;
						current_chmc->bcost += IC_HIT_L2;
					}
				}

				else if(current_chmc->hitmiss_addr[ n ] == ALWAYS_MISS)
				{
					current_chmc->wcost += IC_MISS_L2;
					current_chmc->bcost += IC_HIT;

				}
				else
				{
					current_chmc->wcost += IC_MISS_L2;
					current_chmc->bcost += IC_HIT;
				}	//end L2

			}	//end L1

		}


		current_chmc->hit_copy = current_chmc->hit;
		current_chmc->unknow_copy = current_chmc->unknow;

		current_chmc->wcost_copy = current_chmc->wcost;
		current_chmc->bcost_copy = current_chmc->bcost;


/*
	{
		current_chmc->wcost = bb->chmc[cnt]->hit * IC_HIT;
		current_chmc->wcost += current_chmc->hit * IC_HIT_L2 + (current_chmc->miss + current_chmc->unknow) * IC_MISS_L2;


		current_chmc->bcost  = (bb->chmc[cnt]->hit + bb->chmc[cnt]->unknow) * IC_HIT;

		if(bb->chmc[cnt]->miss > (current_chmc->hit + current_chmc->unknow))
		{
			current_chmc->bcost  +=  (current_chmc->hit + current_chmc->unknow)* IC_HIT_L2; 
			current_chmc->bcost  +=  (bb->chmc[cnt]->miss - (current_chmc->hit + current_chmc->unknow)) * IC_MISS_L2;
		}
		else
		{
			current_chmc->bcost  +=  bb->chmc[cnt]->miss * IC_HIT_L2;
		}
	}
*/
	for(k = 0; k < current_chmc->hitmiss; k++)
		DOUT("L2: %d ", current_chmc->hitmiss_addr[k]);
	DOUT("cnt = %d, bb->size = %d, bb->startaddr = %d\n", cnt, bb->size, bb->startaddr);
	DOUT("L1:\nnum of fetch = %d, hit = %d, miss= %d, unknow = %d\n", bb->chmc[cnt]->hitmiss, bb->chmc[cnt]->hit, bb->chmc[cnt]->miss, bb->chmc[cnt]->unknow);
	DOUT("L2:\nnum of fetch = %d, hit = %d, miss= %d, unknow = %d\n", current_chmc->hitmiss, current_chmc->hit, current_chmc->miss, current_chmc->unknow);
	DOUT("\nwcost = %d, bcost = %d\n", current_chmc->wcost, current_chmc->bcost);

		//compute output cache state of this bb
		//check the bb if it is a function call
		if(bb->callpid != -1)
		{
			addr = bb->startaddr;
			
			for(j = 0; j < bb->num_instr; j++)
			{
				set_no = SET_L2(addr);

				if(isInWay(addr, bb->chmc[cnt]->hit_addr, bb->chmc[cnt]->hit))
					continue;
				else 	if(isInWay(addr, current_chmc->unknow_addr, current_chmc->unknow))
				{
					cache_set_must = copyCacheSet(bb->bb_cache_state_L2->must[set_no]);
					cache_set_may = copyCacheSet(bb->bb_cache_state_L2->may[set_no]);
					cache_set_persist = copyCacheSet_persist(bb->bb_cache_state_L2->persist[set_no]);
					
					calculateCacheState_L2(bb->bb_cache_state_L2->must[set_no], bb->bb_cache_state_L2->may[set_no], bb->bb_cache_state_L2->persist[set_no], addr);

					/* for(tmp = 0; tmp < cache_L2.na; tmp ++)
					{*/

						bb->bb_cache_state_L2->must[set_no] = intersectCacheState_L2(cache_set_must, bb->bb_cache_state_L2->must[set_no]);
						bb->bb_cache_state_L2->may[set_no] = unionCacheState_L2(cache_set_may, bb->bb_cache_state_L2->may[set_no]);
						bb->bb_cache_state_L2->persist[set_no] = unionMaxCacheState_L2(cache_set_persist, bb->bb_cache_state_L2->persist[set_no]);

					/* } */

					freeCacheSet_L2(cache_set_must);
					freeCacheSet_L2(cache_set_may);
					freeCacheSet_L2(cache_set_persist);
				}
				else
				{
					calculateCacheState_L2(bb->bb_cache_state_L2->must[set_no], bb->bb_cache_state_L2->may[set_no], bb->bb_cache_state_L2->persist[set_no], addr);
				}
				addr = addr + INSN_SIZE;

			}

			cs_ptr = bb->bb_cache_state_L2;

			bb->bb_cache_state_L2 = copyCacheState_L2(mapFunctionCall_L2(bb->proc_ptr, cs_ptr));

			freeCacheState_L2(cs_ptr);
			//cs_ptr = copyCacheState_L2(bb->bb_cache_state_L2);

			//freeCacheStateFunction_L2(bb->proc_ptr);

		}	
		else
		{
			addr = bb->startaddr;
			
			for(j = 0; j < bb->num_instr; j ++)
			{
				set_no = SET_L2(addr);

				if(isInWay(addr, current_chmc->hit_addr, current_chmc->hit))
					continue;
				else 	if(isInWay(addr, current_chmc->unknow_addr, current_chmc->unknow))
				{
					cache_set_must = copyCacheSet(bb->bb_cache_state_L2->must[set_no]);
					cache_set_may = copyCacheSet(bb->bb_cache_state_L2->may[set_no]);
					cache_set_persist = copyCacheSet_persist(bb->bb_cache_state_L2->persist[set_no]);
					
					calculateCacheState_L2(bb->bb_cache_state_L2->must[set_no], bb->bb_cache_state_L2->may[set_no], bb->bb_cache_state_L2->persist[set_no], addr);

				/*	for(tmp = 0; tmp < cache_L2.na; tmp ++)
					{ */

						bb->bb_cache_state_L2->must[set_no] = intersectCacheState_L2(cache_set_must, bb->bb_cache_state_L2->must[set_no]);
						bb->bb_cache_state_L2->may[set_no] = unionCacheState_L2(cache_set_may, bb->bb_cache_state_L2->may[set_no]);
						bb->bb_cache_state_L2->persist[set_no] = unionMaxCacheState_L2(cache_set_persist, bb->bb_cache_state_L2->persist[set_no]);

				/*	} */
					freeCacheSet_L2(cache_set_must);
					freeCacheSet_L2(cache_set_may);
					freeCacheSet_L2(cache_set_persist);
					
				}
				else
				{
					calculateCacheState_L2(bb->bb_cache_state_L2->must[set_no], bb->bb_cache_state_L2->may[set_no], bb->bb_cache_state_L2->persist[set_no], addr);
				}
				addr = addr + INSN_SIZE;
				
			}

			//cs_ptr = copyCacheState_L2(bb->bb_cache_state_L2);
			
		}// normal bb
		
	}
	
	DRETURN( p->bblist[lp->topo[0]->bbid]->bb_cache_state_L2 );
}



static cache_state *
copyCacheState_L2(cache_state *cs)
{
	int j, k, num_entry;
	cache_state *copy = NULL;

	CALLOC(copy, cache_state*, 1, sizeof(cache_state), "cache_state");
	copy->must = NULL;
	copy->may = NULL;
	copy->persist = NULL;


	  CALLOC(copy->must, cache_line_way_t***, cache_L2.ns, sizeof(cache_line_way_t**), "NO set cache_line_way_t");

		CALLOC(copy->may, cache_line_way_t***, cache_L2.ns, sizeof(cache_line_way_t**), "NO set cache_line_way_t");

		CALLOC(copy->persist, cache_line_way_t***, cache_L2.ns, sizeof(cache_line_way_t**), "NO set cache_line_way_t");

		for(j = 0; j < cache_L2.ns; j++)
		{
				CALLOC(copy->must[j], cache_line_way_t**, cache_L2.na, sizeof(cache_line_way_t*), "NO assoc cache_line_way_t");

			CALLOC(copy->may[j], cache_line_way_t**, cache_L2.na, sizeof(cache_line_way_t*), "NO assoc cache_line_way_t");

			CALLOC(copy->persist[j], cache_line_way_t**, cache_L2.na + 1, sizeof(cache_line_way_t*), "NO assoc cache_line_way_t");

			for( k = 0; k < cache_L2.na; k++)
			{
				CALLOC(copy->must[j][k], cache_line_way_t*, 1, sizeof(cache_line_way_t), "one cache_line_way_t");
				CALLOC(copy->may[j][k], cache_line_way_t*, 1, sizeof(cache_line_way_t), "one cache_line_way_t");
				CALLOC(copy->persist[j][k], cache_line_way_t*, 1, sizeof(cache_line_way_t), "one cache_line_way_t");

				copy->must[j][k]->num_entry = cs->must[j][k]->num_entry;
				if(copy->must[j][k]->num_entry)
				{
					CALLOC(copy->must[j][k]->entry, int*, copy->must[j][k]->num_entry, sizeof(int), "entries");
					
					for(num_entry = 0; num_entry < copy->must[j][k]->num_entry; num_entry++)
						copy->must[j][k]->entry[num_entry] =  cs->must[j][k]->entry[num_entry];
				}

				copy->may[j][k]->num_entry = cs->may[j][k]->num_entry;
				if(copy->may[j][k]->num_entry)
				{
					CALLOC(copy->may[j][k]->entry, int*, copy->may[j][k]->num_entry, sizeof(int), "entries");

					for(num_entry = 0; num_entry < copy->may[j][k]->num_entry; num_entry++)
						copy->may[j][k]->entry[num_entry] =  cs->may[j][k]->entry[num_entry];
				}

				copy->persist[j][k]->num_entry = cs->persist[j][k]->num_entry;
				if(copy->persist[j][k]->num_entry)
				{
					CALLOC(copy->persist[j][k]->entry, int*, copy->persist[j][k]->num_entry, sizeof(int), "entries");

					for(num_entry = 0; num_entry < copy->persist[j][k]->num_entry; num_entry++)
						copy->persist[j][k]->entry[num_entry] =  cs->persist[j][k]->entry[num_entry];
				}

			}


			CALLOC(copy->persist[j][cache_L2.na], cache_line_way_t*, 1, sizeof(cache_line_way_t), "one cache_line_way_t may");
			copy->persist[j][cache_L2.na]->num_entry = cs->persist[j][cache_L2.na]->num_entry;
			if(copy->persist[j][cache_L2.na]->num_entry)
			{
				CALLOC(copy->persist[j][cache_L2.na]->entry, int*, copy->persist[j][cache_L2.na]->num_entry, sizeof(int), "entries");

				for(num_entry = 0; num_entry < copy->persist[j][cache_L2.na]->num_entry; num_entry++)
					copy->persist[j][cache_L2.na]->entry[num_entry] =  cs->persist[j][cache_L2.na]->entry[num_entry];
			}

		}
//	}
	return copy;
}





static cache_state *
mapFunctionCall_L2(procedure *proc, cache_state *cs)
{
  DSTART( "mapFunctionCall_L2" );

	int i, j, k, n, set_no, cnt, addr, addr_next, copies, tmp, tag, tag_next; 
	int lp_level, age;

	//dumpCacheState(cs);
	//exit(1);
	procedure *p = proc;
	block *bb, *incoming_bb;
	cache_state *cs_ptr;
	cache_line_way_t **cache_set_must, **cache_set_may, **clw;
	CHMC *current_chmc;
	
	DOUT("\nIn mapFunctionCall, p[%d]\n", p->pid);
	
	cs_ptr = copyCacheState_L2(cs);
	
	int  num_blk = p->num_topo;	


	for(i = 0; i < MAX_NEST_LOOP; i++)
		if(loop_level_arr[i] == INVALID)
		{
			lp_level = i -1;
			break;
		}

	cnt = 0;		
	
	for(k = 0; k <= lp_level; k++)
		if(loop_level_arr [k] == NEXT_ITERATION)
			cnt += (1<<(lp_level - k));
		
	for(i = num_blk -1 ; i >= 0 ; i--)
	{	
		bb = p ->topo[i];

		bb = p->bblist[ bb->bbid ];

		bb->num_instr = bb->size / INSN_SIZE;

		if(bb->is_loophead)
		{

			loop_level_arr[lp_level +1] = FIRST_ITERATION;
			p->loops[bb->loopid]->num_fm_L2 = 0;
			
			mapLoop_L2(p, p->loops[bb->loopid]);


			loop_level_arr[lp_level +1] = NEXT_ITERATION;

			mapLoop_L2(p, p->loops[bb->loopid]);

			//cs_ptr = copyCacheState_L2(bb->bb_cache_state_L2);

			loop_level_arr[lp_level +1] = INVALID;

			//freeCacheStateLoop_L2(p, p->loops[bb->loopid]);
			continue;
		}

		
		if(bb->num_incoming > 0)
		{
			incoming_bb = p->bblist[bb->incoming[0]];
			
			//printBlock(incoming_bb);
			if(!incoming_bb->bb_cache_state_L2) continue;
			
			cs_ptr = copyCacheState_L2(incoming_bb->bb_cache_state_L2);


			incoming_bb->num_outgoing--;
			if(incoming_bb->num_outgoing < 1 && incoming_bb->num_cache_state_L2 == 1)
			{
				incoming_bb->num_cache_state_L2 = 0;
				//freeCacheState_L2(incoming_bb->bb_cache_state_L2);
				//incoming_bb->bb_cache_state_L2 = NULL;
			}
			
			
			if(bb->num_incoming > 1)
			{
			  DOUT("\ndo operations if more than one incoming edge\n");

				//dumpCacheState(cs_ptr);
				//printBlock(incoming_bb);

				for(j = 1; j < bb->num_incoming; j++)
				{
					incoming_bb = p->bblist[bb->incoming[j]];

					if(incoming_bb->bb_cache_state_L2 == NULL) continue;
					
					for(k = 0; k < cache_L2.ns; k++)
					{
						clw = cs_ptr->must[k];							
						cs_ptr->must[k] = intersectCacheState_L2(cs_ptr->must[k], incoming_bb->bb_cache_state_L2->must[k]);

						freeCacheSet_L2(clw);

						clw = cs_ptr->may[k];							
						cs_ptr->may[k] = unionCacheState_L2(cs_ptr->may[k], incoming_bb->bb_cache_state_L2->may[k]);

						freeCacheSet_L2(clw);

						clw = cs_ptr->persist[k];							
						cs_ptr->persist[k] = unionMaxCacheState_L2(cs_ptr->persist[k], incoming_bb->bb_cache_state_L2->persist[k]);

						freeCacheSet_L2(clw);

					}

					incoming_bb->num_outgoing--;
					if(incoming_bb->num_outgoing < 1 && incoming_bb->num_cache_state_L2 == 1)
					{
						incoming_bb->num_cache_state_L2 = 0;
						//freeCacheState_L2(incoming_bb->bb_cache_state_L2);
						//incoming_bb->bb_cache_state_L2 = NULL;
					}
	
				}	//end for all incoming
				
				DACTION( dumpCacheState_L2(cs_ptr); );
			}
		}



		if(bb->num_cache_state_L2== 0)
		{
			//CALLOC(bb->bb_cache_state_L2, cache_state *, 1, sizeof(cache_state), "cache_state");

			bb->num_cache_state_L2 = 1;
		}

		if(bb->num_chmc_L2 == 0)
		{
			if(lp_level == -1)
				copies = 1;
			else
				copies = 2<<(lp_level);

			bb->num_chmc_L2 = copies;

			CALLOC(bb->chmc_L2, CHMC**, copies, sizeof(CHMC*), "CHMC");
	
			for(tmp = 0; tmp < copies; tmp++)
			{
				CALLOC(bb->chmc_L2[tmp], CHMC*, 1, sizeof(CHMC), "CHMC");
			}

		}


		bb->bb_cache_state_L2 = cs_ptr;
		
		current_chmc = bb->chmc_L2[cnt];

		current_chmc->hit = 0;
		current_chmc->hit_copy = 0;

		current_chmc->miss = 0;
		
		current_chmc->unknow = 0;
		current_chmc->unknow_copy = 0;

		current_chmc->wcost = 0;
		current_chmc->bcost = 0;
		current_chmc->wcost_copy = 0;
		current_chmc->bcost_copy = 0;
	
		
		//current_chmc->hit_addr = NULL;
		//current_chmc->miss_addr = NULL;
		//current_chmc->unknow_addr = NULL;
		//current_chmc->hitmiss_addr= NULL;
		//current_chmc->hit_change_miss= NULL;
		
		//compute categorization of each instruction line
		//int start_addr = bb->startaddr;
		//bb->num_cache_fetch_L2 = bb->size / cache_L2.ls;

		//int start_addr_fetch = (start_addr >> cache_L2.lsb) << cache_L2.lsb;
		//tmp = start_addr - start_addr_fetch;

		
		//tmp is not 0 if start address is not multiple of sizeof(cache line) in bytes
		//if(tmp) bb->num_cache_fetch_L2++;

		current_chmc->hitmiss = bb->num_instr;
		//if(current_chmc->hitmiss > 0) 
			CALLOC(current_chmc->hitmiss_addr, char*, current_chmc->hitmiss, sizeof(char),"hitmiss_addr");

		addr = bb->startaddr;


		DOUT("bbid = %d, num_instr = %d, hit = %d, miss = %d, unknow = %d\n", bb->bbid,
		    bb->num_instr, bb->chmc[cnt]->hit, bb->chmc[cnt]->miss, bb->chmc[cnt]->unknow);

		for(n = 0; n < bb->num_instr; n++)
		{
			//if(bb->is_loophead!= 0)
				//break;
			set_no = SET_L2(addr);

			//hit in L1?
			if(isInWay(addr, bb->chmc[cnt]->hit_addr, bb->chmc[cnt]->hit))
			{
				current_chmc->hitmiss_addr[n] = HIT_UPPER;
				addr = addr +  INSN_SIZE;
				continue;
			}

			if(main_copy->hit_cache_set_L2[set_no] != USED)
			{	
				numConflictTask[set_no]++;
				numConflictMSC[set_no]++;
				//not hit in L1?
				main_copy->hit_cache_set_L2[set_no] = USED;
			}
			
			main_copy->hit_addr[set_no].num_entry++;
			if(main_copy->hit_addr[set_no].num_entry == 1)
			{
				CALLOC(main_copy->hit_addr[set_no].entry, int*, 1, sizeof(int), "entry");
				main_copy->hit_addr[set_no].entry[0] = TAGSET_L2(addr); 
			}
			else
			{
				REALLOC(main_copy->hit_addr[set_no].entry, int*, main_copy->hit_addr[set_no].num_entry * sizeof(int), "entry");
				main_copy->hit_addr[set_no].entry[main_copy->hit_addr[set_no].num_entry -1] = TAGSET_L2(addr); 
			}

			age = isInCache_L2(addr, bb->bb_cache_state_L2->must[set_no]);
			if(age !=-1)
			{
				current_chmc->hitmiss_addr[n] = ALWAYS_HIT;

				//current_chmc->hitmiss++;
				if(current_chmc->hit == 0)
				{
					current_chmc->hit++;
					CALLOC(current_chmc->hit_addr, int*, 1, sizeof(int), "hit_addr");

					CALLOC(current_chmc->hit_change_miss, char*, 1, sizeof(char), "hit_change_miss");

					CALLOC(current_chmc->age, char*, 1, sizeof(char), "age");

					current_chmc->hit_addr[current_chmc->hit-1] = addr;
					current_chmc->hit_change_miss[current_chmc->hit-1] = HIT;
					current_chmc->age[current_chmc->hit-1] = age;
				}
				else
				{
					current_chmc->hit++;		
					
					REALLOC(current_chmc->hit_addr, int*, current_chmc->hit * sizeof(int), "hit_addr");				
					REALLOC(current_chmc->hit_change_miss, char*, current_chmc->hit * sizeof(char), "hit_change_miss");				
					REALLOC(current_chmc->age, char*, current_chmc->hit * sizeof(char), "hit_change_miss");				

					current_chmc->hit_addr[current_chmc->hit-1] = addr;
					current_chmc->hit_change_miss[current_chmc->hit-1] = HIT;
					current_chmc->age[current_chmc->hit-1] = age;


				}
				//current_chmc_copy->hit = current_chmc->hit;

				addr_next = addr + INSN_SIZE;
				tag = SET_L2(addr);
				tag_next = SET_L2(addr_next);
				while(tag == tag_next && n < bb->num_instr - 1)
				{
					n++;
					current_chmc->hitmiss_addr[n] = ALWAYS_HIT;

					if(!isInWay(addr_next, bb->chmc[cnt]->hit_addr, bb->chmc[cnt]->hit))
					{
						if(current_chmc->hit == 0)
					       {
						current_chmc->hit++;
						CALLOC(current_chmc->hit_addr, int*, 1, sizeof(int), "hit_addr");

						CALLOC(current_chmc->hit_change_miss, char*, 1, sizeof(char), "hit_change_miss");

						CALLOC(current_chmc->age, char*, 1, sizeof(char), "age");

						current_chmc->hit_addr[current_chmc->hit-1] = addr_next;
						current_chmc->hit_change_miss[current_chmc->hit-1] = HIT;
						current_chmc->age[current_chmc->hit-1] = cache_L2.na -1;
						
					     	}
					  	else
					  	{
						current_chmc->hit++;		
						
						REALLOC(current_chmc->hit_addr, int*, current_chmc->hit * sizeof(int), "hit_addr");				
						REALLOC(current_chmc->hit_change_miss, char*, current_chmc->hit * sizeof(char), "hit_change_miss");				
						REALLOC(current_chmc->age, char*, current_chmc->hit * sizeof(char), "age");				

						current_chmc->hit_addr[current_chmc->hit-1] = addr_next;
						current_chmc->hit_change_miss[current_chmc->hit-1] = HIT;
						current_chmc->age[current_chmc->hit-1] = cache_L2.na -1;

						}
					}	
					
                                        addr_next += INSN_SIZE;
	                                tag_next = SET_L2(addr_next);
             						  

				}

			}
			else if(isNeverInCache_L2(addr, bb->bb_cache_state_L2->may[set_no]))
			{
				current_chmc->hitmiss_addr[n] = ALWAYS_MISS;

				if(current_chmc->miss == 0)
				{
					current_chmc->miss++;
					
					CALLOC(current_chmc->miss_addr, int*, 1, sizeof(int), "miss_addr");
					current_chmc->miss_addr[current_chmc->miss-1] = addr;
				}
				else
				{
					current_chmc->miss++;	
					
					REALLOC(current_chmc->miss_addr, int*, current_chmc->miss * sizeof(int), "miss_addr");				
					current_chmc->miss_addr[current_chmc->miss-1] = addr;
				}
				//current_chmc_copy->miss = current_chmc->miss;

				addr_next = addr + INSN_SIZE;
				tag = SET_L2(addr);
				tag_next = SET_L2(addr_next);
				while(tag == tag_next && n < bb->num_instr - 1)
				{
					n++;
					current_chmc->hitmiss_addr[n] = ALWAYS_HIT; 
					
					if(!isInWay(addr_next, bb->chmc[cnt]->hit_addr, bb->chmc[cnt]->hit))
					{
						if(current_chmc->hit == 0)
					       {
						current_chmc->hit++;
						CALLOC(current_chmc->hit_addr, int*, 1, sizeof(int), "hit_addr");

						CALLOC(current_chmc->hit_change_miss, char*, 1, sizeof(char), "hit_change_miss");

						CALLOC(current_chmc->age, char*, 1, sizeof(char), "age");

						current_chmc->hit_addr[current_chmc->hit-1] = addr_next;
						current_chmc->hit_change_miss[current_chmc->hit-1] = HIT;
						current_chmc->age[current_chmc->hit-1] = cache_L2.na -1;
						
					     	}
					  	else
					  	{
						current_chmc->hit++;		
						
						REALLOC(current_chmc->hit_addr, int*, current_chmc->hit * sizeof(int), "hit_addr");				
						REALLOC(current_chmc->hit_change_miss, char*, current_chmc->hit * sizeof(char), "hit_change_miss");				
						REALLOC(current_chmc->age, char*, current_chmc->hit * sizeof(char), "age");				

						current_chmc->hit_addr[current_chmc->hit-1] = addr_next;
						current_chmc->hit_change_miss[current_chmc->hit-1] = HIT;
						current_chmc->age[current_chmc->hit-1] = cache_L2.na -1;

						}
					}	
					
                                        addr_next += INSN_SIZE;
	                                tag_next = SET_L2(addr_next);
             						  

				}

				
			}

			else if(isInCache_L2(addr, bb->bb_cache_state_L2->persist[set_no])!= -1)
			{
				current_chmc->hitmiss_addr[n] = FIRST_MISS;

				if(current_chmc->hit == 0)
				{
					current_chmc->hit++;
					CALLOC(current_chmc->hit_addr, int*, 1, sizeof(int), "hit_addr");

					CALLOC(current_chmc->hit_change_miss, char*, 1, sizeof(char), "hit_change_miss");

					CALLOC(current_chmc->age, char*, 1, sizeof(char), "age");

					current_chmc->hit_addr[current_chmc->hit-1] = addr;
					current_chmc->hit_change_miss[current_chmc->hit-1] = HIT;
					current_chmc->age[current_chmc->hit-1] = age;
				}
				else
				{
					current_chmc->hit++;		
					
					REALLOC(current_chmc->hit_addr, int*, current_chmc->hit * sizeof(int), "hit_addr");				
					REALLOC(current_chmc->hit_change_miss, char*, current_chmc->hit * sizeof(char), "hit_change_miss");				
					REALLOC(current_chmc->age, char*, current_chmc->hit * sizeof(char), "hit_change_miss");				

					current_chmc->hit_addr[current_chmc->hit-1] = addr;
					current_chmc->hit_change_miss[current_chmc->hit-1] = HIT;
					current_chmc->age[current_chmc->hit-1] = age;


				}
				//current_chmc_copy->hit = current_chmc->hit;

				addr_next = addr + INSN_SIZE;
				tag = SET_L2(addr);
				tag_next = SET_L2(addr_next);
				while(tag == tag_next && n < bb->num_instr - 1)
				{
					n++;
					current_chmc->hitmiss_addr[n] = ALWAYS_HIT; 

					if(!isInWay(addr_next, bb->chmc[cnt]->hit_addr, bb->chmc[cnt]->hit))
					{
						if(current_chmc->hit == 0)
					       {
						current_chmc->hit++;
						CALLOC(current_chmc->hit_addr, int*, 1, sizeof(int), "hit_addr");

						CALLOC(current_chmc->hit_change_miss, char*, 1, sizeof(char), "hit_change_miss");

						CALLOC(current_chmc->age, char*, 1, sizeof(char), "age");

						current_chmc->hit_addr[current_chmc->hit-1] = addr_next;
						current_chmc->hit_change_miss[current_chmc->hit-1] = HIT;
						current_chmc->age[current_chmc->hit-1] = cache_L2.na -1;
						
					     	}
					  	else
					  	{
						current_chmc->hit++;		
						
						REALLOC(current_chmc->hit_addr, int*, current_chmc->hit * sizeof(int), "hit_addr");				
						REALLOC(current_chmc->hit_change_miss, char*, current_chmc->hit * sizeof(char), "hit_change_miss");				
						REALLOC(current_chmc->age, char*, current_chmc->hit * sizeof(char), "age");				

						current_chmc->hit_addr[current_chmc->hit-1] = addr_next;
						current_chmc->hit_change_miss[current_chmc->hit-1] = HIT;
						current_chmc->age[current_chmc->hit-1] = cache_L2.na -1;

						}
					}	
					
                                        addr_next += INSN_SIZE;
	                                tag_next = SET_L2(addr_next);
             						  

				}

			}
			else
			{
				current_chmc->hitmiss_addr[n] = UNKNOW;
			        
				if(current_chmc->unknow == 0)
				{
					current_chmc->unknow++;
					CALLOC(current_chmc->unknow_addr, int*, 1, sizeof(int), "unknow_addr");
					current_chmc->unknow_addr[current_chmc->unknow-1] = addr;
				}
				else
				{
					current_chmc->unknow++;				
					REALLOC(current_chmc->unknow_addr, int*, current_chmc->unknow * sizeof(int), "unknow_addr");				
					current_chmc->unknow_addr[current_chmc->unknow-1] = addr;
				}

				addr_next = addr + INSN_SIZE;
				tag = SET_L2(addr);
				tag_next = SET_L2(addr_next);
				while(tag == tag_next && n < bb->num_instr - 1)
				{
					n++;
					current_chmc->hitmiss_addr[n] = ALWAYS_HIT; 
					if(!isInWay(addr_next, bb->chmc[cnt]->hit_addr, bb->chmc[cnt]->hit))
					{
						if(current_chmc->hit == 0)
					       {
						current_chmc->hit++;
						CALLOC(current_chmc->hit_addr, int*, 1, sizeof(int), "hit_addr");

						CALLOC(current_chmc->hit_change_miss, char*, 1, sizeof(char), "hit_change_miss");
						CALLOC(current_chmc->age, char*, 1, sizeof(char), "age");

						current_chmc->hit_addr[current_chmc->hit-1] = addr_next;
						current_chmc->hit_change_miss[current_chmc->hit-1] = HIT;
						current_chmc->age[current_chmc->hit-1] = cache_L2.na -1;
						
					     	}
					  	else
					  	{
						current_chmc->hit++;		
						
						REALLOC(current_chmc->hit_addr, int*, current_chmc->hit * sizeof(int), "hit_addr");				
						REALLOC(current_chmc->hit_change_miss, char*, current_chmc->hit * sizeof(char), "hit_change_miss");				
						REALLOC(current_chmc->age, char*, current_chmc->hit * sizeof(char), "age");				

						current_chmc->hit_addr[current_chmc->hit-1] = addr_next;
						current_chmc->hit_change_miss[current_chmc->hit-1] = HIT;
						current_chmc->age[current_chmc->hit-1] = cache_L2.na -1;

						}
					}	
					
                                        addr_next += INSN_SIZE;
	                                tag_next = SET_L2(addr_next);
             						  

				}

				
			} //end else
			
			addr = addr_next;
		}

	  DOUT("bbid = %d, num_instr = %d, hit = %d, miss = %d, unknow = %d\n", bb->bbid,
	      bb->num_instr, bb->chmc[cnt]->hit, bb->chmc[cnt]->miss, bb->chmc[cnt]->unknow);
    assert((bb->chmc[cnt]->miss + bb->chmc[cnt]->unknow) ==
           (current_chmc->hit + current_chmc->miss + current_chmc->unknow));
		
		for(n = 0; n < bb->num_instr; n++)
		{
			//L1 hit
			if(bb->chmc[cnt]->hitmiss_addr[ n ] == ALWAYS_HIT)
			{
				current_chmc->wcost += IC_HIT;
				current_chmc->bcost += IC_HIT;
			}

			//L1 fm
			else if(bb->chmc[cnt]->hitmiss_addr[ n ] == FIRST_MISS)
			{
			  if ( lp_level >= 0 ) {
          if(loop_level_arr[lp_level] == FIRST_ITERATION)
          {
            if(current_chmc->hitmiss_addr[ n ] == ALWAYS_HIT)
            {
              current_chmc->wcost += IC_HIT_L2;
              current_chmc->bcost += IC_HIT_L2;
            }

            else if(current_chmc->hitmiss_addr[ n ] == ALWAYS_MISS ||current_chmc->hitmiss_addr[ n ] == FIRST_MISS )
            {
              current_chmc->wcost += IC_MISS_L2;
              current_chmc->bcost += IC_MISS_L2;
            }

            else
            {
              current_chmc->wcost += IC_MISS_L2;
              current_chmc->bcost += IC_HIT_L2;
            }
          }

          else if(loop_level_arr[lp_level] == NEXT_ITERATION)
          {
            current_chmc->wcost += IC_HIT;
            current_chmc->bcost += IC_HIT;
          }
			  }
					
			}
			
			//L1 miss
			else if(bb->chmc[cnt]->hitmiss_addr[ n ] == ALWAYS_MISS)
			{
				if(current_chmc->hitmiss_addr[ n ] == ALWAYS_HIT)
				{
					current_chmc->wcost += IC_HIT_L2;
					current_chmc->bcost += IC_HIT_L2;
				}

				else if(current_chmc->hitmiss_addr[ n ] == FIRST_MISS)
				{
				  if ( lp_level >= 0 ) {
            if(loop_level_arr[lp_level] == FIRST_ITERATION)
            {
              current_chmc->wcost += IC_MISS_L2;
              current_chmc->bcost += IC_MISS_L2;
            }
            else if(loop_level_arr[lp_level] == NEXT_ITERATION)
            {
              current_chmc->wcost += IC_HIT_L2;
              current_chmc->bcost += IC_HIT_L2;
            }
				  }
				}
				
				else if(current_chmc->hitmiss_addr[ n ] == ALWAYS_MISS)
				{
					current_chmc->wcost += IC_MISS_L2;
					current_chmc->bcost += IC_MISS_L2;

				}
				else
				{
					current_chmc->wcost += IC_MISS_L2;
					current_chmc->bcost += IC_HIT_L2;
				}
			}
			//L1 unknow
			else
			{
				if(current_chmc->hitmiss_addr[ n ] == ALWAYS_HIT)
				{
					current_chmc->wcost += IC_HIT_L2;
					current_chmc->bcost += IC_HIT;
				}

				else if(current_chmc->hitmiss_addr[ n ] == FIRST_MISS)
				{
				  if ( lp_level >= 0 ) {
            if(loop_level_arr[lp_level] == FIRST_ITERATION)
            {
              current_chmc->wcost += IC_MISS_L2;
              current_chmc->bcost += IC_HIT;
            }
            else if(loop_level_arr[lp_level] == NEXT_ITERATION)
            {
              current_chmc->wcost += IC_HIT;
              current_chmc->bcost += IC_HIT_L2;
            }
				  }
				}

				else if(current_chmc->hitmiss_addr[ n ] == ALWAYS_MISS)
				{
					current_chmc->wcost += IC_MISS_L2;
					current_chmc->bcost += IC_HIT;

				}
				else
				{
					current_chmc->wcost += IC_MISS_L2;
					current_chmc->bcost += IC_HIT;
				}	//end L2

			}	//end L1

		}

	
		current_chmc->hit_copy = current_chmc->hit;
		current_chmc->unknow_copy = current_chmc->unknow;

		current_chmc->wcost_copy = current_chmc->wcost;
		current_chmc->bcost_copy = current_chmc->bcost;

		/*
		current_chmc->wcost = bb->chmc[cnt]->hit * IC_HIT;
		current_chmc->wcost += current_chmc->hit * IC_HIT_L2 + (current_chmc->miss + current_chmc->unknow) * IC_MISS_L2;


		current_chmc->bcost  = (bb->chmc[cnt]->hit + bb->chmc[cnt]->unknow) * IC_HIT;

		if(bb->chmc[cnt]->miss > (current_chmc->hit + current_chmc->unknow))
		{
			current_chmc->bcost  +=  (current_chmc->hit + current_chmc->unknow)* IC_HIT_L2; 
			current_chmc->bcost  +=  (bb->chmc[cnt]->miss - (current_chmc->hit + current_chmc->unknow)) * IC_MISS_L2;
		}
		else
		{
			current_chmc->bcost  +=  bb->chmc[cnt]->miss * IC_HIT_L2;
		}
		*/

		for(k = 0; k < current_chmc->hitmiss; k++)
		  DOUT("L2: %d ", current_chmc->hitmiss_addr[k]);
		DOUT("cnt = %d, bb->size = %d, bb->startaddr = %d\n", cnt, bb->size, bb->startaddr);

		DOUT("L1:\nnum of fetch = %d, hit = %d, miss= %d, unknow = %d\n", bb->chmc[cnt]->hitmiss, bb->chmc[cnt]->hit, bb->chmc[cnt]->miss, bb->chmc[cnt]->unknow);
		DOUT("L2:\nnum of fetch = %d, hit = %d, miss= %d, unknow = %d\n", current_chmc->hitmiss, current_chmc->hit, current_chmc->miss, current_chmc->unknow);
		DOUT("\nwcost = %d, bcost = %d\n", current_chmc->wcost, current_chmc->bcost);
		
		if(bb->callpid != -1)
		{
			addr = bb->startaddr;
			
			for(j = 0; j < bb->num_instr; j ++)
			{
			
				set_no = SET_L2(addr);

				if(isInWay(addr, bb->chmc[cnt]->hit_addr, bb->chmc[cnt]->hit))
				{
					continue;
				}
				else if(isInWay(addr, current_chmc->unknow_addr, current_chmc->unknow))
				{
					cache_set_must = copyCacheSet(bb->bb_cache_state_L2->must[set_no]);
					cache_set_may = copyCacheSet(bb->bb_cache_state_L2->may[set_no]);
					
					calculateCacheState_L2(bb->bb_cache_state_L2->must[set_no], bb->bb_cache_state_L2->may[set_no], bb->bb_cache_state_L2->persist[set_no], addr);

					/*for(tmp = 0; tmp < cache_L2.na; tmp ++)
					{*/

						bb->bb_cache_state_L2->must[set_no] = intersectCacheState_L2(cache_set_must, bb->bb_cache_state_L2->must[set_no]);
						bb->bb_cache_state_L2->may[set_no] = unionCacheState_L2(cache_set_may, bb->bb_cache_state_L2->may[set_no]);

					/*}*/
					freeCacheSet_L2(cache_set_must);
					freeCacheSet_L2(cache_set_may);
				}
				else
				{
					calculateCacheState_L2(bb->bb_cache_state_L2->must[set_no], bb->bb_cache_state_L2->may[set_no], bb->bb_cache_state_L2->persist[set_no], addr);
				}

				addr = addr + INSN_SIZE;

			}

			cs_ptr = bb->bb_cache_state_L2;
			bb->bb_cache_state_L2 = copyCacheState_L2(mapFunctionCall_L2(bb->proc_ptr, cs_ptr));
			//cs_ptr = copyCacheState_L2(bb->bb_cache_state_L2);
			freeCacheState_L2(cs_ptr);
			cs_ptr = NULL;
			//freeCacheStateFunction_L2(bb->proc_ptr);
		}

		else
		{
			addr = bb->startaddr;

			for(j = 0; j < bb->num_instr; j ++)
			{
				set_no = SET_L2(addr);

				//hit in L1
				if(isInWay(addr, bb->chmc[cnt]->hit_addr, bb->chmc[cnt]->hit))
				{
					continue;
				}
				//unknow in L1, consider access and not access L2, both cases
				else 	if(isInWay(addr, current_chmc->unknow_addr, current_chmc->unknow))
				{
					cache_set_must = copyCacheSet(bb->bb_cache_state_L2->must[set_no]);
					cache_set_may = copyCacheSet(bb->bb_cache_state_L2->may[set_no]);
					
					calculateCacheState_L2(bb->bb_cache_state_L2->must[set_no], bb->bb_cache_state_L2->may[set_no], bb->bb_cache_state_L2->persist[set_no], addr);

					/*for(tmp = 0; tmp < cache_L2.na; tmp ++)
					{*/

						bb->bb_cache_state_L2->must[set_no] = intersectCacheState_L2(cache_set_must, bb->bb_cache_state_L2->must[set_no]);
						bb->bb_cache_state_L2->may[set_no] = unionCacheState_L2(cache_set_may, bb->bb_cache_state_L2->may[set_no]);

					/*}*/
					freeCacheSet_L2(cache_set_must);
					freeCacheSet_L2(cache_set_may);
				}
				//miss in L1
				else
				{
					calculateCacheState_L2(bb->bb_cache_state_L2->must[set_no], bb->bb_cache_state_L2->may[set_no], bb->bb_cache_state_L2->persist[set_no], addr);
				}
				addr = addr  +INSN_SIZE;

			}
			
			//FREE(cs_ptr);
			//cs_ptr = copyCacheState_L2(bb->bb_cache_state_L2);
		}// end if else

	}

	for(i = 0; i < p->num_bb; i ++)
		p->bblist[i]->num_outgoing = p->bblist[i]->num_outgoing_copy;

	DRETURN( p ->bblist[ p->topo[0]->bbid]->bb_cache_state_L2 );
}


static void
resetFunction_L2(procedure * proc)
{
	int i, j, cnt, lp_level, num_blk;

	procedure *p = proc;
	block *bb;

	for(i = 0; i < MAX_NEST_LOOP; i++)
		if(loop_level_arr[i] == INVALID)
		{
			lp_level = i -1;
			break;
		}

	cnt = 0;		
	for(i = 0; i <= lp_level; i++)
		if(loop_level_arr [i] == NEXT_ITERATION)
			cnt += (1<<(lp_level - i));


	num_blk = p->num_topo;	
	
	for(i = num_blk -1 ; i >= 0 ; i--)
	{	
		//bb is in topo list, to get the bbid of this bb
		bb = p ->topo[i];

		//bb is in bblist
		bb = p->bblist[ bb->bbid ];

		if(bb->is_loophead)
		{

			loop_level_arr[lp_level +1] = FIRST_ITERATION;
			resetLoop_L2(p, p->loops[bb->loopid]);

			loop_level_arr[lp_level +1] = NEXT_ITERATION;
			resetLoop_L2(p, p->loops[bb->loopid]);

			loop_level_arr[lp_level +1] = INVALID;

			continue;
		}


		bb->chmc_L2[cnt]->hit =  bb->chmc_L2[cnt]->hit_copy;
		bb->chmc_L2[cnt]->unknow =  bb->chmc_L2[cnt]->unknow_copy;

		bb->chmc_L2[cnt]->wcost = bb->chmc_L2[cnt]->wcost_copy;
		bb->chmc_L2[cnt]->bcost = bb->chmc_L2[cnt]->bcost_copy;
		
		for(j = 0; j < bb->chmc_L2[cnt]->hit; j ++)
			bb->chmc_L2[cnt]->hit_change_miss[j] = HIT;

		//check the bb if it is a function call
		
		if(bb->callpid != -1)
		{
			resetFunction_L2(bb->proc_ptr);
		}
		
	}
			
}

static void
resetLoop_L2(procedure * proc, loop * lp)
{

	int i, j, cnt, lp_level, num_blk;

	procedure *p = proc;
	block *bb;
	loop *lp_ptr = lp;

	num_blk = lp_ptr->num_topo;

	for(i = 0; i < MAX_NEST_LOOP; i++)
		if(loop_level_arr[i] == INVALID)
		{
			lp_level = i -1;
			break;
		}

	cnt = 0;		
	for(i = 0; i <= lp_level; i++)
		if(loop_level_arr [i] == NEXT_ITERATION)
			cnt += (1<<(lp_level - i));

	for(i = num_blk -1 ; i >= 0 ; i--)
	{	
		//bb is in topo list, to get the bbid of this bb
		bb = lp_ptr->topo[i];

		//bb is in bblist
		bb = p->bblist[ bb->bbid ];

		if(bb->is_loophead && i!= num_blk -1)
		{

			loop_level_arr[lp_level +1] = FIRST_ITERATION;

			resetLoop_L2(p, p->loops[bb->loopid]);

			loop_level_arr[lp_level +1] = NEXT_ITERATION;
			resetLoop_L2(p, p->loops[bb->loopid]);
			
			loop_level_arr[lp_level +1] = INVALID;

			continue;
		}

		//check the bb if it is a function call
		
		bb->chmc_L2[cnt]->hit =  bb->chmc_L2[cnt]->hit_copy;
		bb->chmc_L2[cnt]->unknow =  bb->chmc_L2[cnt]->unknow_copy;

		bb->chmc_L2[cnt]->wcost = bb->chmc_L2[cnt]->wcost_copy;
		bb->chmc_L2[cnt]->bcost = bb->chmc_L2[cnt]->bcost_copy;

		for(j = 0; j < bb->chmc_L2[cnt]->hit; j ++)
			bb->chmc_L2[cnt]->hit_change_miss[j] = HIT;
		
		if(bb->callpid != -1)
		{
			resetFunction_L2(bb->proc_ptr);
			
		}
		
	}
			
}// end if else




void
resetHitMiss_L2(MSC *msc)
{
	int i;
	
	for(i = 0; i < MAX_NEST_LOOP; i++)
		loop_level_arr[i] = INVALID;
	for(i = 0; i < msc->num_task; i++)
	{
		resetFunction_L2(msc->taskList[i].main_copy);
	}
}



//do level one cache analysis
void
cacheAnalysis_L2()
{
  DSTART( "cacheAnalysis_L2" );

	int i;
	for(i = 0; i < MAX_NEST_LOOP; i++)
		loop_level_arr[i] = INVALID;

	//set initial cache state for main precedure
	cache_state *start;

	start = allocCacheState_L2();

	CALLOC(main_copy->hit_cache_set_L2, char*, cache_L2.ns, sizeof(char), "hit_cache_set_L2");
	CALLOC(main_copy->hit_addr, cache_line_way_t*, cache_L2.ns, sizeof(cache_line_way_t), "cache_line_way_t");
	for(i = 0; i < cache_L2.ns; i++)
		main_copy->hit_cache_set_L2[i] = NOT_USED;

	
	start = mapFunctionCall_L2(main_copy, start);

	DOUT("\nprocedure %d\n\n", main_copy->pid);
	DACTION(
	    for(i = 0; i < cache_L2.ns; i++)
	      DOUT("%d ", main_copy->hit_cache_set_L2[i]);
      DOUT("\n");
  );

	DEND();
}
