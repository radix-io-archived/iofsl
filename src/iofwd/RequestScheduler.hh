#ifndef IOFWD_REQUESTSCHEDULER_HH
#define IOFWD_REQUESTSCHEDULER_HH

#include <vector>
#include <set>

#include <boost/thread.hpp>
#include <boost/smart_ptr.hpp>

#include "zoidfs/zoidfs.h"

namespace iofwdutil {
  namespace completion {
    class CompletionID;
    class CompositeCompletionID;
  }
}
namespace zoidfs {
  class ZoidFSAsyncAPI;
}

namespace iofwd
{

class WriteRequest;
class ReadRequest;
class RangeScheduler;

class RequestScheduler
{
public:
  RequestScheduler(zoidfs::ZoidFSAsyncAPI * async_api);
  virtual ~RequestScheduler();

  iofwdutil::completion::CompletionID * enqueueWrite(
     zoidfs::zoidfs_handle_t * handle, size_t count,
     const void ** mem_starts, size_t * mem_sizes,
     uint64_t * file_starts, uint64_t * file_sizes);

  iofwdutil::completion::CompletionID * enqueueRead(
     zoidfs::zoidfs_handle_t * handle, size_t count,
     void ** mem_starts, size_t * mem_sizes,
     uint64_t * file_starts, uint64_t * file_sizes);

  void notifyConsumer();

protected:
  void run();
  
private:
  boost::scoped_ptr<boost::thread> consumethread_;
  boost::mutex lock_;
  boost::condition_variable ready_;
  bool exiting;

  zoidfs::ZoidFSAsyncAPI * async_api_;
  boost::scoped_ptr<RangeScheduler> range_sched_;
};

}

#endif
