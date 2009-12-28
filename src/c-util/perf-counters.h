#ifndef __CUTIL_PERF_COUNTERS_H__
#define __CUTIL_PERF_COUNTERS_H__

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

#endif
