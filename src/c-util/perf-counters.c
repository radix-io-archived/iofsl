#define _GNU_SOURCE

#include <search.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "c-util/perf-counters.h"

/* library static vaiables */
static void * iofwd_global_pc_tree = NULL;

/* derived from the one-at-a-time has
 http://burtleburtle.net/bob/hash/doobs.html */
size_t perf_counters_key_generate(char * pc_key)
{
    size_t hash = 0;
    size_t i = 0;

    for (i = 0; i < strlen(pc_key) ; i++)
    {
        hash += pc_key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

int perf_counters_key_compare(const void * a, const void * b)
{
    if(((iofwd_pc_t *)a)->pc_key > ((iofwd_pc_t *)b)->pc_key)
    {
        return 1;
    }
    if(((iofwd_pc_t *)a)->pc_key < ((iofwd_pc_t *)b)->pc_key)
    {
        return -1;
    }
    return 0;
}

void * perf_counters_key_find(void * pc_tree, char * pc_key)
{
    size_t k = perf_counters_key_generate(pc_key);
    iofwd_pc_t sk = {k,NULL,NULL,PC_DOUBLE};

    /* if a local pc tree was given, search it */
    if(pc_tree)
    {
        return tfind(&sk, &pc_tree, perf_counters_key_compare); 
    }
    /*else, search the global pc tree for the key */
    else
    {
        return tfind(&sk, &iofwd_global_pc_tree, perf_counters_key_compare); 
    }
    return NULL;
}

/* update the perf counter based on the data type specified in the counter */
int perf_counters_update(iofwd_pc_t * tpc, iofwd_pc_t * pc)
{
    switch(pc->pc_dt)
    {
        case PC_DOUBLE:
        {
            *(double *)tpc->pc_data += *(double *)pc->pc_data;
            break;
        }
        case PC_UINT8_T:
        {
            *(uint8_t *)tpc->pc_data += *(uint8_t *)pc->pc_data;
            break;
        }
        case PC_UINT16_T:
        {
            *(uint16_t *)tpc->pc_data += *(uint16_t *)pc->pc_data;
            break;
        }
        case PC_UINT32_T:
        {
            *(uint32_t *)tpc->pc_data += *(uint32_t *)pc->pc_data;
            break;
        }
        case PC_UINT64_T:
        {
            *(uint64_t *)tpc->pc_data += *(uint64_t *)pc->pc_data;
            break;
        }
        case PC_SIZE_T:
        {
            *(size_t *)tpc->pc_data += *(size_t *)pc->pc_data;
            break;
        }
        default:
            break;
    };
    return 0;
}

/* zero the perf counters */
int perf_counters_zero(iofwd_pc_t * pc)
{
    switch(pc->pc_dt)
    {
        case PC_DOUBLE:
        {
            *(double *)pc->pc_data = 0.0;
            break;
        }
        case PC_UINT8_T:
        {
            *(uint8_t *)pc->pc_data = 0; 
            break;
        }
        case PC_UINT16_T:
        {
            *(uint16_t *)pc->pc_data = 0;
            break;
        }
        case PC_UINT32_T:
        {
            *(uint32_t *)pc->pc_data = 0;
            break;
        }
        case PC_UINT64_T:
        {
            *(uint64_t *)pc->pc_data = 0;
            break;
        }
        case PC_SIZE_T:
        {
            *(size_t *)pc->pc_data = 0;
            break;
        }
        default:
            break;
    };
    return 0;
}

/* create a counter */
iofwd_pc_t * perf_counters_counter_create(char * pc_name, iofwd_pc_dt_t pc_dt)
{
    iofwd_pc_t * k = (iofwd_pc_t *)malloc(sizeof(iofwd_pc_t));

    k->pc_name = pc_name;
    k->pc_dt = pc_dt;
    k->pc_key = perf_counters_key_generate(pc_name);

    switch(k->pc_dt)
    {
        case PC_DOUBLE:
        {
            k->pc_data = (double *)malloc(sizeof(double));
            *(double *)k->pc_data = 0.0;
            break;
        }
        case PC_UINT8_T:
        {
            k->pc_data = (uint8_t *)malloc(sizeof(uint8_t));
            *(uint8_t *)k->pc_data = 0;
            break;
        }
        case PC_UINT16_T:
        {
            k->pc_data = (uint16_t *)malloc(sizeof(uint16_t));
            *(uint16_t *)k->pc_data = 0;
            break;
        }
        case PC_UINT32_T:
        {
            k->pc_data = (uint32_t *)malloc(sizeof(uint32_t));
            *(uint32_t *)k->pc_data = 0;
            break;
        }
        case PC_UINT64_T:
        {
            k->pc_data = (uint64_t *)malloc(sizeof(uint64_t));
            *(uint64_t *)k->pc_data = 0;
            break;
        }
        case PC_SIZE_T:
        {
            k->pc_data = (size_t *)malloc(sizeof(size_t));
            *(size_t *)k->pc_data = 0;
            break;
        }
        default:
            break;
    };
 
    return k; 
}

/* insert a counter into the counter tree */
void * perf_counters_counter_insert(void * pc_tree, iofwd_pc_t * pc)
{
    iofwd_pc_t * tpc = NULL;

    /* if a local pc tree was given, search it */
    if(pc_tree)
    {
        tpc = tsearch(pc, &pc_tree, perf_counters_key_compare); 
    }
    /*else, search the global pc tree for the key */
    else
    {
        tpc = tsearch(pc, &iofwd_global_pc_tree, perf_counters_key_compare); 
    }

    /* a replica of the counter was found... update the existing counter */
    if(tpc != pc)
    {
       perf_counters_update(tpc, pc); 
    }

    return tpc;
}

/* reset a counter value to 0 */
int perf_counters_reset(void * pc_tree, char * pc_key)
{
    size_t k = perf_counters_key_generate(pc_key);
    iofwd_pc_t sk = {k,NULL,NULL,PC_DOUBLE};
    iofwd_pc_t * tdata = NULL;

    /* if a local pc tree was given, search it */
    if(pc_tree)
    {
        tdata = tfind(&sk, &pc_tree, perf_counters_key_compare); 
    }
    /*else, search the global pc tree for the key */
    else
    {
        tdata = tfind(&sk, &iofwd_global_pc_tree, perf_counters_key_compare); 
    }

    /* if the counter was found, zero out the data */
    if(tdata)
    {
       perf_counters_zero(tdata); 
    }
    return 0;
}

/* free a perf counter... free the data field and then the counter */
void perf_counters_counter_free(void * c)
{
    iofwd_pc_t * pc = (iofwd_pc_t *)c;

    free(pc->pc_data);
    free(pc);

    return;
}

/* delete a counter from the tree */
int perf_counters_counter_delete(void * pc_tree, char * pc_key)
{
    size_t k = perf_counters_key_generate(pc_key);
    iofwd_pc_t sk = {k,NULL,NULL,PC_DOUBLE};
    iofwd_pc_t * tdata = NULL;

    /* if a local pc tree was given, search it */
    if(pc_tree)
    {
        tdata = tdelete(&sk, &pc_tree, perf_counters_key_compare); 
    }
    /*else, search the global pc tree for the key */
    else
    {
        tdata = tdelete(&sk, &iofwd_global_pc_tree, perf_counters_key_compare); 
    }

    /* if the node was found, delete it */
    if(tdata)
    {
        perf_counters_counter_free(tdata);
    }

    return 0;
}

/* destroy an entire counter tree */
int perf_counters_destroy_tree(void * pc_tree)
{
    if(pc_tree)
    {
        tdestroy(pc_tree, perf_counters_counter_free);
    }
    else
    {
        tdestroy(iofwd_global_pc_tree, perf_counters_counter_free);
    }
    
    return 0;
}
