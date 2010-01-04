#include "iofwdutil/PerfCounters.hh"

#include <cstdio>
#include <cstdlib>
#include <cstring>

int main()
{
    void * c = NULL;
    double v = 1.2;
    double r = 0.0;

    iofwdutil::PerfCounterAdd(&c, strdup("TEST1"), PC_DOUBLE);
    iofwdutil::PerfCounterGet(&c, strdup("TEST1"), &r);
    fprintf(stderr, "counter r = %f\n", r);

    iofwdutil::PerfCounterUpdate(&c, strdup("TEST1"), &v);
    iofwdutil::PerfCounterGet(&c, strdup("TEST1"), &r);
    fprintf(stderr, "counter r = %f\n", r);

    iofwdutil::PerfCounterAdd(&c, strdup("TEST2"), PC_DOUBLE);
    iofwdutil::PerfCounterGet(&c, strdup("TEST2"), &r);
    fprintf(stderr, "counter r = %f\n", r);

    iofwdutil::PerfCounterAdd(NULL, strdup("TEST3"), PC_DOUBLE);
    iofwdutil::PerfCounterGet(NULL, strdup("TEST3"), &r);
    fprintf(stderr, "counter r = %f\n", r);

    v = 13.1;
    iofwdutil::PerfCounterUpdate(&c, strdup("TEST1"), &v);
    iofwdutil::PerfCounterGet(&c, strdup("TEST1"), &r);
    fprintf(stderr, "counter r = %f\n", r);

    iofwdutil::PerfCounterUpdate(&c, strdup("TEST2"), &v);
    iofwdutil::PerfCounterGet(&c, strdup("TEST2"), &r);
    fprintf(stderr, "counter r = %f\n", r);

    iofwdutil::PerfCounterUpdate(NULL, strdup("TEST3"), &v);
    iofwdutil::PerfCounterUpdate(NULL, strdup("TEST3"), &v);
    iofwdutil::PerfCounterUpdate(NULL, strdup("TEST3"), &v);
    iofwdutil::PerfCounterGet(NULL, strdup("TEST3"), &r);
    fprintf(stderr, "counter r = %f\n", r);

    iofwdutil::PerfCounterDelete(&c, strdup("TEST1"));

    iofwdutil::PerfCounterCleanup(c);
    iofwdutil::PerfCounterCleanup(NULL);
    
    return 0;
}
