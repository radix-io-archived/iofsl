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

void * perf_counters_key_find(void ** pc_tree, char * pc_key)
{
    size_t k = perf_counters_key_generate(pc_key);
    iofwd_pc_t * sk = (iofwd_pc_t *)calloc(1, sizeof(iofwd_pc_t));
    iofwd_pc_t ** fk = NULL;

    sk->pc_key = k;
    sk->pc_dt = PC_UNDEF_T;
    sk->pc_name = pc_key; 
    sk->pc_data = NULL; 

    /* if a local pc tree was given, search it */
    if(pc_tree)
    {
        fk = (iofwd_pc_t **)tfind(sk, pc_tree, perf_counters_key_compare);
        if(!fk || !*fk)
        {
            free(sk);
            return NULL;
        }

        if(*fk == sk)
        {
            return sk;
        }
        else
        {
            free(sk);
            return *fk;
        }
    }
    /*else, search the global pc tree for the key */
    else
    {
        fk = (iofwd_pc_t **)tfind(sk, &iofwd_global_pc_tree, perf_counters_key_compare);
        if(!fk || !*fk)
        {
            free(sk);
            return NULL;
        }

        if(*fk == sk)
        {
            return sk;
        }
        else
        {
            free(sk);
            return *fk;
        }
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

int perf_counters_update_data(iofwd_pc_t * tpc, void * pc_data)
{
    switch(tpc->pc_dt)
    {
        case PC_DOUBLE:
        {
            *(double *)tpc->pc_data += *(double *)pc_data;
            break;
        }
        case PC_UINT8_T:
        {
            *(uint8_t *)tpc->pc_data += *(uint8_t *)pc_data;
            break;
        }
        case PC_UINT16_T:
        {
            *(uint16_t *)tpc->pc_data += *(uint16_t *)pc_data;
            break;
        }
        case PC_UINT32_T:
        {
            *(uint32_t *)tpc->pc_data += *(uint32_t *)pc_data;
            break;
        }
        case PC_UINT64_T:
        {
            *(uint64_t *)tpc->pc_data += *(uint64_t *)pc_data;
            break;
        }
        case PC_SIZE_T:
        {
            *(size_t *)tpc->pc_data += *(size_t *)pc_data;
            break;
        }
        default:
            break;
    };
    return 0;
}

/* zero the perf counters */
int perf_counters_get(iofwd_pc_t * pc, void * pc_data)
{
    switch(pc->pc_dt)
    {
        case PC_DOUBLE:
        {
            *(double *)pc_data = *(double *)pc->pc_data;
            break;
        }
        case PC_UINT8_T:
        {
            *(uint8_t *)pc_data = *(uint8_t *)pc->pc_data;
            break;
        }
        case PC_UINT16_T:
        {
            *(uint16_t *)pc_data = *(uint16_t *)pc->pc_data;
            break;
        }
        case PC_UINT32_T:
        {
            *(uint32_t *)pc_data = *(uint32_t *)pc->pc_data;
            break;
        }
        case PC_UINT64_T:
        {
            *(uint64_t *)pc_data = *(uint64_t *)pc->pc_data;
            break;
        }
        case PC_SIZE_T:
        {
            *(size_t *)pc_data = *(size_t *)pc->pc_data;
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
    iofwd_pc_t * k = (iofwd_pc_t *)calloc(1, sizeof(iofwd_pc_t));

    k->pc_name = pc_name;
    k->pc_dt = pc_dt;
    k->pc_key = perf_counters_key_generate(pc_name);

    switch(k->pc_dt)
    {
        case PC_DOUBLE:
        {
            k->pc_data = (double *)calloc(1, sizeof(double));
            *(double *)k->pc_data = 0.0;
            break;
        }
        case PC_UINT8_T:
        {
            k->pc_data = (uint8_t *)calloc(1, sizeof(uint8_t));
            *(uint8_t *)k->pc_data = 0;
            break;
        }
        case PC_UINT16_T:
        {
            k->pc_data = (uint16_t *)calloc(1, sizeof(uint16_t));
            *(uint16_t *)k->pc_data = 0;
            break;
        }
        case PC_UINT32_T:
        {
            k->pc_data = (uint32_t *)calloc(1, sizeof(uint32_t));
            *(uint32_t *)k->pc_data = 0;
            break;
        }
        case PC_UINT64_T:
        {
            k->pc_data = (uint64_t *)calloc(1, sizeof(uint64_t));
            *(uint64_t *)k->pc_data = 0;
            break;
        }
        case PC_SIZE_T:
        {
            k->pc_data = (size_t *)calloc(1, sizeof(size_t));
            *(size_t *)k->pc_data = 0;
            break;
        }
        default:
            break;
    };

    return k;
}

/* insert a counter into the counter tree */
void * perf_counters_counter_insert(void ** pc_tree, iofwd_pc_t * pc)
{
    /* if a local pc tree was given, search it */
    if(pc_tree)
    {
        tsearch(pc, pc_tree, perf_counters_key_compare);
    }
    /*else, search the global pc tree for the key */
    else
    {
        tsearch(pc, &iofwd_global_pc_tree, perf_counters_key_compare);
    }

    return pc;
}

int perf_counters_counter_add(void ** pc_tree, char * pc_key, iofwd_pc_dt_t pc_dt)
{
    if(!perf_counters_key_find(pc_tree, pc_key))
    {
        iofwd_pc_t * pc = perf_counters_counter_create(pc_key, pc_dt); 
        perf_counters_counter_insert(pc_tree, pc);
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

int perf_counters_delete(void ** pc_tree, iofwd_pc_t * tpc)
{
    if(pc_tree)
    {
        tdelete(tpc, pc_tree, perf_counters_key_compare);
        perf_counters_counter_free(tpc);
    }
    else
    {
        tdelete(tpc, &iofwd_global_pc_tree, perf_counters_key_compare);
    }
    return 0;
}

/* delete a counter from the tree */
int perf_counters_counter_delete(void ** pc_tree, char * pc_key)
{
    iofwd_pc_t * tpc = perf_counters_key_find(pc_tree, pc_key);
    if(tpc)
    {
        perf_counters_delete(pc_tree, tpc);
        return 0;
    }
    return 1;
}

/* destroy an entire counter tree */
int perf_counters_cleanup(void * pc_tree)
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

int perf_counters_counter_update(void ** pc_tree, char * pc_key, void * pc_data)
{
    iofwd_pc_t * tpc = perf_counters_key_find(pc_tree, pc_key);
    if(tpc)
    {
        perf_counters_update_data(tpc, pc_data);
        return 0;
    }
    return 1;
}

int perf_counters_counter_reset(void ** pc_tree, char * pc_key)
{
    iofwd_pc_t * tpc = perf_counters_key_find(pc_tree, pc_key);
    if(!tpc)
    {
        perf_counters_zero(tpc);
        return 0;
    }
    return 1;
}

int perf_counters_counter_get(void ** pc_tree, char * pc_key, void * pc_data)
{
    iofwd_pc_t * tpc = perf_counters_key_find(pc_tree, pc_key);
    if(tpc)
    {
        perf_counters_get(tpc, pc_data);
        return 0;
    }
    return 1;
}
