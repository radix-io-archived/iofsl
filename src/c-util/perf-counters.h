#ifndef CUTIL_PERF_COUNTERS_H
#define CUTIL_PERF_COUNTERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>

typedef enum
{
    PC_DOUBLE = 0,
    PC_UINT8_T,
    PC_UINT16_T,
    PC_UINT32_T,
    PC_UINT64_T,
    PC_SIZE_T,
    PC_UNDEF_T,
    PC_MAX_T
} iofwd_pc_dt_t;

struct iofwd_pc
{
    size_t pc_key;
    char * pc_name;
    void * pc_data;
    iofwd_pc_dt_t pc_dt;
};

typedef struct iofwd_pc iofwd_pc_t;

/* function prototypes */

/* counter create and destroy */
int perf_counters_counter_add(void ** pc_tree, char * pc_key, iofwd_pc_dt_t pc_dt);
int perf_counters_counter_delete(void ** pc_tree, char * pc_key);
int perf_counters_cleanup(void * pc_tree);

/* counter updates */
int perf_counters_counter_update(void ** pc_tree, char * pc_key, void * pc_data);
int perf_counters_counter_reset(void ** pc_tree, char * pc_key);
int perf_counters_counter_get(void ** pc_tree, char * pc_key, void * pc_data);

#ifdef __cplusplus
}
#endif

#endif
