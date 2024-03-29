#ifndef IOFWD_TASKSM_SHAREDIOCB_HH
#define IOFWD_TASKSM_SHAREDIOCB_HH

#include <vector>
#include "zoidfs/zoidfs.h"
#include "iofwd/tasksm/IOCBWrapper.hh"
#include "iofwdutil/tools.hh"

namespace iofwd
{
    namespace tasksm
    {
/* callback that maps multiple read / write statemachine requests to a single merged request */

/*
 * Instances of this class own the dyn memory passed to the ZoidFSAPI and are responsible for
 *  cleanup up / freeing the allocated memory
 */
class SharedIOCB
{
    public:
        /* constructor... anybody can allocate this */
        SharedIOCB(int * ret, char ** mem_starts, size_t * mem_sizes,
              const zoidfs::zoidfs_file_ofs_t * file_starts, 
              const zoidfs::zoidfs_file_size_t * file_sizes) :
            ret_(ret), mem_starts_(mem_starts), mem_sizes_(mem_sizes), file_starts_(file_starts), file_sizes_(file_sizes)
        {
        }

        /* this is the callback registered with the ZoidFS API */
        virtual void issueIOCallbacks(iofwdevent::CBException status) = 0;

    protected:
        /* only instance of this class can delete / free itself... */
        virtual ~SharedIOCB();

        /* variables... ownership of this dyn memory is transfered to this class */
        int * ret_;
        char ** mem_starts_;
        size_t * mem_sizes_;
        const zoidfs::zoidfs_file_ofs_t * file_starts_;
        const zoidfs::zoidfs_file_size_t * file_sizes_;
};

/* use this wrapper when multiple callbacks must be invoked
 * - Request merger issues 1 request from N individual requests
 * - Upon completion of merged IO request, MultiSHaredIOCB wrapper
 *   will issue callbacks for the N original requests
 */
class MultiSharedIOCB : public SharedIOCB
{
    public:
        MultiSharedIOCB(int * ret, char ** mem_starts, size_t * mem_sizes,
              const zoidfs::zoidfs_file_ofs_t * file_starts,
              const zoidfs::zoidfs_file_size_t * file_sizes)
            : SharedIOCB(ret, mem_starts, mem_sizes, file_starts, file_sizes)
        {
        }

        /* load the Task callbacks */
        void loadCBs(std::vector< iofwd::tasksm::IOCBWrapper * > & cbvec)
        {
            cbs_.insert(cbs_.end(), cbvec.begin(), cbvec.end());
        }

        virtual void issueIOCallbacks(iofwdevent::CBException status)
        {
           status.check ();
            for(unsigned int i = 0 ; i < cbs_.size() ; i++)
            {
                cbs_[i]->invoke(*ret_, status);
            }

            /* delete this instance */
            // @TODO: make the callback call a static member function, and
            // in which 'delete this' is safe...
            delete this;
        }

    protected:
        virtual ~MultiSharedIOCB();

        /* all Task callbacks associated with this ZoidFS */
        std::vector<iofwd::tasksm::IOCBWrapper *> cbs_;
};

/* use this wrapper when a single callback must be invoked
 * - Request merger issues 1 request from 1 individual IO request
 * - Upon completion of the single IO request, SingleSharedIOCB wrapper
 *   will issue callbacks for the 1 original request
 *
 * This is a special case of the MultiSharedIOCB and does not allocate
 *  the std::vector...
 */
class SingleSharedIOCB : public SharedIOCB
{
    public:
        SingleSharedIOCB(int * ret, char ** mem_starts, size_t * mem_sizes,
              const zoidfs::zoidfs_file_ofs_t * file_starts,
              const zoidfs::zoidfs_file_size_t * file_sizes)
            : SharedIOCB(ret, mem_starts, mem_sizes, file_starts, file_sizes)
        {
        }

        void loadCB(iofwd::tasksm::IOCBWrapper * cb)
        {
            cb_ = cb;
        }

        virtual void issueIOCallbacks(iofwdevent::CBException status)
        {
           status.check ();
            cb_->invoke(*ret_, status);

            /* delete this instance */
            delete this;
        }

    protected:
        virtual ~SingleSharedIOCB();

        /* the task callback */
        iofwd::tasksm::IOCBWrapper * cb_;
};
    }
}
#endif
