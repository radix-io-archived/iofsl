#ifndef IOFWDUTIL_STATS_COUNTERHELPER_HH
#define IOFWDUTIL_STATS_COUNTERHELPER_HH

#include <string>
#include "iofwdutil/stats/CounterTable.hh"
#include "iofwdutil/tools.hh"

namespace iofwdutil
{
    namespace stats
    {

template <typename T>
class CounterHelper
{
    public:
        static T * get(const std::string & name,
                bool UNUSED(run)=true)
        {
            /* lock the table */
            iofwdutil::stats::CounterTable::instance().lock();

            /* find the counter */
            T * c = dynamic_cast<T
                *>(iofwdutil::stats::CounterTable::instance().find(name));

            /* if no counter was found, allocate a new one and store it in the
             * table */
            if(c == NULL)
            {
                c = new T(name);
                iofwdutil::stats::CounterTable::instance().store(name, c);
            }

            /* unlock the table */
            iofwdutil::stats::CounterTable::instance().unlock();

            return c;
        }

        static void release(std::string & name)
        {
            /* lock the table */
            iofwdutil::stats::CounterTable::instance().lock();

            /* find the counter */
            T * c = dynamic_cast<T
                *>(iofwdutil::stats::CounterTable::instance().find(name));

            /* unlock the table */
            iofwdutil::stats::CounterTable::instance().unlock();
        }

        static void free(std::string & name)
        {
            /* lock the table */
            iofwdutil::stats::CounterTable::instance().lock();

            /* find the counter */
            T * c = dynamic_cast<T
                *>(iofwdutil::stats::CounterTable::instance().find(name));

            /* if wefound the counter, delete it */
            if(c)
            {
                /* delete the counter */
                delete c;
            }

            /* unlock the table */
            iofwdutil::stats::CounterTable::instance().unlock();
        }
};

    }
}
#endif
