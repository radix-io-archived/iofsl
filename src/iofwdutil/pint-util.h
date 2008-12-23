/*
 * (C) 2001 Clemson University and The University of Chicago
 *
 * See COPYING in top-level directory.
 */

/* This header includes prototypes for common internal utility functions */

#ifndef __PINT_UTIL_H
#define __PINT_UTIL_H


struct PINT_time_marker_s
{
    struct timeval wtime; /* real time */
    struct timeval utime; /* user time */
    struct timeval stime; /* system time */
};
typedef struct PINT_time_marker_s PINT_time_marker;

void PINT_time_mark(PINT_time_marker* out_marker);
void PINT_time_diff(PINT_time_marker mark1, 
    PINT_time_marker mark2,
    double* out_wtime_sec,
    double* out_utime_sec,
    double* out_stime_sec);
void PINT_util_get_current_timeval(struct timeval *tv);
int PINT_util_get_timeval_diff(struct timeval *tv_start, struct timeval *tv_end);

struct timespec PINT_util_get_abs_timespec(int microsecs);


#endif /* __PINT_UTIL_H */

/*
 * Local variables:
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 *
 * vim: ts=8 sts=4 sw=4 expandtab
 */
