#ifndef __IOFWDUTIL_PERF_COUNTERS_HH__
#define __IOFWDUTIL_PERF_COUNTERS_HH__

#include <map>
#include <string>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>

using namespace std;

namespace iofwdutil
{
    class PerfCountersDestroyer;

    class PerfCounterBase
    {
        public:
            PerfCounterBase() : counter_name_("UNDEF")
            {
            }
    
            PerfCounterBase(string name) : counter_name_(name)
            {
            }

            virtual PerfCounterBase & operator += (const PerfCounterBase & rhs) = 0;
    
            string counter_name_;
    };

    template<class CT>
    class PerfCounter : public PerfCounterBase
    {
        public:
            PerfCounter() : PerfCounterBase("UNDEF"), counter_value_(0)
            {
            }

            PerfCounter(string name, CT value) : PerfCounterBase(name), counter_value_(value)
            {
            }

            PerfCounterBase & operator += (const PerfCounterBase & rhs)
            {
                if(rhs.counter_name_ == counter_name_)
                {
                    counter_value_ += dynamic_cast< PerfCounter<CT> const & >(rhs).counter_value_;
                }
                return *this;
            }

            CT counter_value_;
    };

    /* singleton.. collection of all perf counters */
    class PerfCounters
    {
        public:
            static PerfCounters * Instance();
           
            friend class PerfCountersDestroyer;
 
            void UpdateCounter(PerfCounterBase & pc)
            {
            } 

        protected:
            PerfCounters();
            ~PerfCounters();
            PerfCounters(PerfCounters const &) { /* do nothing */};
            PerfCounters & operator= (PerfCounters const &) { return *Instance(); };

        private:
            static PerfCounters * perf_counters_instance_;
            static PerfCountersDestroyer perf_counters_destroyer_instance_;
    };

    class PerfCountersDestroyer
    {
        public:
            PerfCountersDestroyer() : perf_counters_(NULL)
            {
            }

            PerfCountersDestroyer(PerfCounters * pc) : perf_counters_(pc)
            {
            }

            ~PerfCountersDestroyer()
            {
                if(perf_counters_)
                {
                    delete perf_counters_;
                }
            }

            void SetPerfCounters(PerfCounters * pc)
            {
                perf_counters_ = pc;
            }

        private:
            PerfCounters * perf_counters_;
    };

}

#endif
