#ifndef SRC_IOFWDUTIL_STATS_COUNTER_HH
#define SRC_IOFWDUTIL_STATS_COUNTER_HH

#include <string>
#include "iofwdutil/stats/CounterTable.hh"

namespace iofwdutil
{
    namespace stats
    {

template<typename T>
class Counter
{
    public:
        /* construct a new counter */
        Counter(std::string name, std::string fmt) : name_(name), fmt_(fmt)
        {
            CounterTable::instance().allocate(name_, sizeof(T), fmt);
        }

        /* add v to the counter */
        virtual void add(T v)
        {
            iofwdutil::stats::CounterTable::instance().lock(name_);
            T * vref = static_cast<T *>(iofwdutil::stats::CounterTable::instance().get(name_));
            if(vref)
            {
                *vref += v;
            }
            iofwdutil::stats::CounterTable::instance().unlock(name_);
        }

        /* dec v from the counter */
        virtual void dec(T v)
        {
            iofwdutil::stats::CounterTable::instance().lock(name_);
            T * vref = static_cast<T *>(iofwdutil::stats::CounterTable::instance().get(name_));
            if(vref)
            {
                *vref -= v;
            }
            iofwdutil::stats::CounterTable::instance().unlock(name_);
        }

        /* get the current value of the counter */
        virtual T & get()
        {
            T * vref = static_cast<T *>(iofwdutil::stats::CounterTable::instance().get(name_));

            /* error if this is undefined */
            assert(vref);

            iofwdutil::stats::CounterTable::instance().unlock(name_);
            return *vref;
        }

        /* set the value of the counter to v */
        virtual void set(T v)
        {
            iofwdutil::stats::CounterTable::instance().lock(name_);
            T * vref = static_cast<T *>(iofwdutil::stats::CounterTable::instance().get(name_));
            if(vref)
            {
                *vref = v;
            }
            iofwdutil::stats::CounterTable::instance().unlock(name_);
        }

    protected:

        /* name of the counter */
        std::string name_;

        /* format of the counter */
        std::string fmt_;
};

    } /* stats */
} /* iofwdutil */
#endif
