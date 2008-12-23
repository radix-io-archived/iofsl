/*
 * (C) 2001 Clemson University and The University of Chicago
 *
 * Changes by Acxiom Corporation to add PINT_check_mode() helper function
 * as a replacement for check_mode() in permission checking, also added
 * PINT_check_group() for supplimental group support 
 * Copyright © Acxiom Corporation, 2005.
 *
 * See COPYING in top-level directory.
 */

/* This file includes definitions of common internal utility functions */
#include <string.h>
#include <assert.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#define __PINT_REQPROTO_ENCODE_FUNCS_C
#include "gen-locks.h"
#include "pint-util.h"

void PINT_time_mark(PINT_time_marker *out_marker)
{
    struct rusage usage;

    gettimeofday(&out_marker->wtime, NULL);
    getrusage(RUSAGE_SELF, &usage);
    out_marker->utime = usage.ru_utime;
    out_marker->stime = usage.ru_stime;
}

void PINT_time_diff(PINT_time_marker mark1, 
                    PINT_time_marker mark2,
                    double *out_wtime_sec,
                    double *out_utime_sec,
                    double *out_stime_sec)
{
    *out_wtime_sec = 
        ((double)mark2.wtime.tv_sec +
         (double)(mark2.wtime.tv_usec) / 1000000) -
        ((double)mark1.wtime.tv_sec +
         (double)(mark1.wtime.tv_usec) / 1000000);

    *out_stime_sec = 
        ((double)mark2.stime.tv_sec +
         (double)(mark2.stime.tv_usec) / 1000000) -
        ((double)mark1.stime.tv_sec +
         (double)(mark1.stime.tv_usec) / 1000000);

    *out_utime_sec = 
        ((double)mark2.utime.tv_sec +
         (double)(mark2.utime.tv_usec) / 1000000) -
        ((double)mark1.utime.tv_sec +
         (double)(mark1.utime.tv_usec) / 1000000);
}
void PINT_util_get_current_timeval(struct timeval *tv)
{
    gettimeofday(tv, NULL);
}

int PINT_util_get_timeval_diff(struct timeval *tv_start, struct timeval *tv_end)
{
    return (tv_end->tv_sec * 1e6 + tv_end->tv_usec) -
        (tv_start->tv_sec * 1e6 + tv_start->tv_usec);
}


struct timespec PINT_util_get_abs_timespec(int microsecs)
{
    struct timeval now, add, result;
    struct timespec tv;

    gettimeofday(&now, NULL);
    add.tv_sec = (microsecs / 1e6);
    add.tv_usec = (microsecs % 1000000);
    timeradd(&now, &add, &result);
    tv.tv_sec = result.tv_sec;
    tv.tv_nsec = result.tv_usec * 1e3;
    return tv;
}

