#ifndef __IOFWD_TASKPOOL_HH__
#define __IOFWD_TASKPOOL_HH__

#include <queue>
#include <stack>
#include <boost/thread/mutex.hpp>
#include "zoidfs/zoidfs-proto.h"
#include "iofwdutil/completion/BMIResource.hh"
#include "iofwd/WriteTask.hh"
#include "iofwd/ReadTask.hh"
#include "iofwdutil/ConfigFile.hh"

namespace iofwd
{

/* a preallocated object pool for IOFWD tasks */
class TaskPool
{
    public:

        /* for each task type, allocate maxTask  instances */
        TaskPool(const iofwdutil::ConfigFile & c, boost::function<void (Task *)> reschedule) : max_task_(0), reschedule_(reschedule)
        {
            /* get the number of tasks from the config file */
            max_task_ = c.getKeyAsDefault("numtasks", 0);

            /* populate the task pool data structures... thread safe access */
            for(int i = 0 ; i < zoidfs::ZOIDFS_PROTO_MAX ; i++)
            {
                /* for now, only read and write tasks are handled... */
                if(i == zoidfs::ZOIDFS_PROTO_WRITE || i == zoidfs::ZOIDFS_PROTO_READ)
                {
                    boost::mutex::scoped_lock lock(task_pool_locks_[i]);
                    for(int j = 0 ; j < max_task_ ; j++)
                    {
                        task_pool_[i].push(createTask(i));
                    }
                }
            }
        }

        /* deallocate all tasks in the pool... thread safe */
        ~TaskPool()
        {
            for(int i = 0 ; i < zoidfs::ZOIDFS_PROTO_MAX ; i++)
            {
                /* for now, only read and write tasks are handled... */
                if(i == zoidfs::ZOIDFS_PROTO_WRITE || i == zoidfs::ZOIDFS_PROTO_READ)
                {
                    boost::mutex::scoped_lock lock(task_pool_locks_[i]);
                    while(task_pool_[i].size() > 0)
                    {
                       delete task_pool_[i].top();
                       task_pool_[i].pop();
                    }
                }
            }
        }

        /* get and rm a task from the pool queue... thread safe */
        Task * get(int tt)
        {
            /* don't get invalid types to the pool */
            if(tt < 0 && tt >= zoidfs::ZOIDFS_PROTO_MAX)
            {
                return NULL;
            }

            boost::mutex::scoped_lock lock(task_pool_locks_[tt]);
            Task * t = NULL; /* if there are no preallocated tasks, return NULL */

            /* if there are tasks available, return one from the stack */
            if(task_pool_[tt].size() > 0)
            {
                t = task_pool_[tt].top();
                task_pool_[tt].pop();
            }
            return t;
        }

        /* add a task to a queue... thread safe */
        void put(int tt, Task * t)
        {
            /* don't add NULL values to the pool */
            if(!t)
            {
                return;
            }

            /* don't add invalid types to the pool */
            if(tt < 0 && tt >= zoidfs::ZOIDFS_PROTO_MAX)
            {
                return;
            }

            boost::mutex::scoped_lock lock(task_pool_locks_[tt]);
            //t->reset();
            task_pool_[tt].push(t);
        }

    protected:
        /* allocate a new task */
        Task * createTask(int tt);

        /*
            task pool data structure is an array of stl stacks. Array is indexed
             by int enum. Task storage is a stack.
        */
        std::stack< Task * > task_pool_[zoidfs::ZOIDFS_PROTO_MAX];

        /*
            locks for the task pool. Indexed by int. Allow
            thread safe access to stack for a particular int and
            concurrent access to the pool for diff task types
         */
        boost::mutex task_pool_locks_[zoidfs::ZOIDFS_PROTO_MAX];

        int max_task_;

        boost::function<void (Task *)> reschedule_;

        iofwdutil::completion::BMIResource bmi_;
};

}

#endif
