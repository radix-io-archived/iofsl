#ifndef IOFWDUTIL_STATS_COUNTERTABLE_HH
#define IOFWDUTIL_STATS_COUNTERTABLE_HH

#include <string>
#include <map>
#include <vector>
#include "iofwdutil/Singleton.hh"
#include "iofwdutil/IOFWDLog.hh"
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>

namespace iofwdutil
{
    namespace stats
    {

class CounterTable : public iofwdutil::Singleton< CounterTable >
{
    protected:
        class CounterData
        {
            public:
                std::string fmt_;
                void * mem_;
                boost::mutex lock_;
        };

    public:
        CounterTable();

        ~CounterTable();

        void dumpAllCounters();

        void lock(std::string name)
        {
            std::map<std::string, CounterData *>::iterator mem;

            /* get the counter */
            mem = counters_.find(name);
            if(mem != counters_.end())
            {
                (mem->second)->lock_.lock();
            }
        }

        void unlock(std::string name)
        {
            std::map<std::string, CounterData *>::iterator mem;

            /* get the counter */
            mem = counters_.find(name);
            if(mem != counters_.end())
            {
                (mem->second)->lock_.unlock();
            }
        }

        /* get the counter */
        void * get(std::string name)
        {
            std::map<std::string, CounterData *>::iterator mem;

            /* make sure our local storage was allocated */
            checkStorage();

            /* get the counter */
            mem = counters_.find(name);

            if(mem != counters_.end())
            {
                return (mem->second)->mem_;
            }
            return NULL;
        }

        /* allocate a counter */
        void * allocate(std::string name, size_t csize, std::string fmt)
        {
            void * mem = NULL;
            std::map<std::string, CounterData * >::iterator it;

            /* make sure our local storage was allocated */
            checkStorage();

            /* get the counter */
            it = counters_.find(name);

            if(it == counters_.end())
            {
                CounterData * data = NULL;
 
                /* allocate some mem for the counter */
                mem = static_cast<void *>(new char[csize]()); /* zero init */
                data = new CounterData();
                data->mem_ = mem;
                data->fmt_ = std::string(fmt);

                /* hash the counter name to the counter mem */
                (counters_)[name] = data;
            }
            else
            {
                mem = (it->second)->mem_; 
            }
            return mem;
        }

        void dumpCounters();

    protected:

        void checkStorage()
        {
#if 0
            boost::mutex::scoped_lock lock(mutex_);
            if(counters_.get() == NULL)
            {
                std::map<std::string, CounterData *> * v = new std::map<std::string, CounterData *>();
                counters_.reset(v);
                tl_counter_vec_.push_back(v); 
            }
#endif
        }

        boost::mutex mutex_;

        iofwdutil::IOFWDLogSource & log_;
        std::map<std::string, CounterData * >  global_counters_;

        //boost::thread_specific_ptr< std::map< std::string, CounterData * > >  counters_;
        std::map< std::string, CounterData * > counters_;
};

    }
}
#endif
