#include "iofwdutil/PerfCounters.hh"

using namespace iofwdutil;

int main()
{
    PerfCounter<int> t1("INT", -123);
    PerfCounter<double> t2("DOUBLE", 1234.5);
    PerfCounter<unsigned int>  t3("UINT", 567);

    PerfCounters::Instance();
    /*PerfCounters::Instance()->UpdateCounter(*t1);
    PerfCounters::Instance()->UpdateCounter(*t2);
    PerfCounters::Instance()->UpdateCounter(*t3);*/

    return 0;
}
