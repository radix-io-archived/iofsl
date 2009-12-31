#include "c-util/perf-counters.h"

#include <stdio.h>
#include <stdlib.h>

int main()
{
    void * c = NULL;
    double v = 1.2;
    double r = 0.0;

    perf_counters_counter_add(&c, "TEST1", PC_DOUBLE);
    perf_counters_counter_get(&c, "TEST1", &r);
    fprintf(stderr, "counter r = %f\n", r);

    perf_counters_counter_update(&c, "TEST1", &v);
    perf_counters_counter_get(&c, "TEST1", &r);
    fprintf(stderr, "counter r = %f\n", r);

    perf_counters_counter_add(&c, "TEST2", PC_DOUBLE);
    perf_counters_counter_get(&c, "TEST2", &r);
    fprintf(stderr, "counter r = %f\n", r);

    v = 13.1;
    perf_counters_counter_update(&c, "TEST1", &v);
    perf_counters_counter_get(&c, "TEST1", &r);
    fprintf(stderr, "counter r = %f\n", r);

    perf_counters_counter_update(&c, "TEST2", &v);
    perf_counters_counter_get(&c, "TEST2", &r);
    fprintf(stderr, "counter r = %f\n", r);

    perf_counters_counter_delete(&c, "TEST1");

    perf_counters_cleanup(c);
    
    return 0;
}
