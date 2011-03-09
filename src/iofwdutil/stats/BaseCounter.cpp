#include "iofwdutil/stats/BaseCounter.hh"

namespace iofwdutil
{
    namespace stats
    {

void configureCounter(BaseCounter * counter,
                bool enabled)
{
        /* verify pointers are not NULL */
        if(!counter)
                    return;

            counter->enabled_ = enabled;
}

std::string & getCounterKey(BaseCounter * counter)
{
    return counter->config_key_;
}

    }
}
