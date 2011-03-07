#include "iofwdutil/stats/CounterTable.hh"

#include "iofwdutil/stats/CounterHelper.hh"

namespace iofwdutil
{
    namespace stats
    {
        CounterTable::CounterTable()
        {
        }

        CounterTable::~CounterTable()
        {
            dumpCounters();
            clearCounters();
        }

        void CounterTable::clearCounters()
        {
            lock();

            /* cleanup counters */
            std::map<std::string, BaseCounter *>::iterator it;

            /* for each counter, delete the counter mem */
            for(it = counters_.begin() ; it != counters_.end() ; it++)
            {
                it->second->reset();
                delete it->second;
            }

            counters_.clear();

            unlock();
        }

        void CounterTable::dumpCounters()
        {
            lock();

            /* cleanup counters */
            std::map<std::string, BaseCounter *>::iterator it;

            /* for each counter, delete the counter mem */
            for(it = counters_.begin() ; it != counters_.end() ; it++)
            {
                it->second->print();
            }

            unlock();
        }
    }
}
