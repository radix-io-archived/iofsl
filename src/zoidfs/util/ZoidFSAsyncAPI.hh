#ifndef ZOIDFS_ZOIDFSASYNCAPI_HH
#define ZOIDFS_ZOIDFSASYNCAPI_HH

#include "zoidfs-wrapped.hh"
#include "LogAPI.hh"
#include "iofwdutil/workqueue/PoolWorkQueue.hh"

namespace iofwdutil
{
   namespace completion
   {
      class CompletionID;
   }
   namespace workqueue
   {
      class WorkQueue;
   }
}

namespace zoidfs
{
//===========================================================================

class ZoidFSAPI;

/**
 * @TODO:
 *    this is wrong. There should be a ZoidFSAsyncAPI, implementing ALL the
 *    functions asynchronous and a default implementation, which uses the
 *    workqueue and calls a ZoidFSAPI interface using blocking threads.
 */
class ZoidFSAsyncAPI
{
public:
   ZoidFSAsyncAPI(ZoidFSAPI * api = NULL, iofwdutil::workqueue::WorkQueue *q = NULL)
      : api_(api), q_(q) {
      if (api_ == NULL)
         api_ = &fallback_;
      if (q_ == NULL)
         /* TODO make the thread params a config option */
         q_ = new iofwdutil::workqueue::PoolWorkQueue (8, 100);
   }
   ~ZoidFSAsyncAPI();

   iofwdutil::completion::CompletionID * async_write(
      const zoidfs_handle_t * handle,
      size_t mem_count,
      const void * mem_starts[],
      const size_t mem_sizes[],
      size_t file_count,
      const uint64_t file_starts[],
      uint64_t file_sizes[],
      zoidfs_op_hint_t * op_hint);

   iofwdutil::completion::CompletionID * async_read(
      const zoidfs_handle_t * handle,
      size_t mem_count,
      void * mem_starts[],
      const size_t mem_sizes[],
      size_t file_count,
      const uint64_t file_starts[],
      uint64_t file_sizes[],
      zoidfs_op_hint_t * op_hint);

protected:
   LogAPI fallback_; 
   ZoidFSAPI * api_;
   iofwdutil::workqueue::WorkQueue * q_;
};

//===========================================================================
}

#endif
