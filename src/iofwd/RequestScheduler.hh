#ifndef IOFWD_REQUESTSCHEDULER_HH
#define IOFWD_REQUESTSCHEDULER_HH

#include <vector>
#include <set>

#include <boost/thread.hpp>
#include <boost/smart_ptr.hpp>

#include "zoidfs/zoidfs.h"
#include "zoidfs/util/ZoidFSDefAsync.hh"
#include "iofwdutil/zlog/ZLogSource.hh"
#include "Range.hh"
#include "iofwdutil/ConfigFile.hh"
#include "iofwdevent/CBType.hh"
#include "iofwdutil/tools.hh"
#include "sm/SimpleSlots.hh"

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
  RequestScheduler(zoidfs::ZoidFSAsyncAPI * async_api, zoidfs::util::ZoidFSDefAsync * async_cb_api, const iofwdutil::ConfigFile & c, int mode);
  virtual ~RequestScheduler();

  iofwdutil::completion::CompletionID * enqueueWrite(
     zoidfs::zoidfs_handle_t * handle, size_t count,
     const void ** mem_starts, size_t * mem_sizes,
     uint64_t * file_starts, uint64_t * file_sizes, zoidfs::zoidfs_op_hint_t * op_hint);

  void enqueueWriteCB(
     iofwdevent::CBType cb, zoidfs::zoidfs_handle_t * handle, size_t count,
     const void ** mem_starts, size_t * mem_sizes,
     uint64_t * file_starts, uint64_t * file_sizes, zoidfs::zoidfs_op_hint_t * op_hint);

  iofwdutil::completion::CompletionID * enqueueRead(
     zoidfs::zoidfs_handle_t * handle, size_t count,
     void ** mem_starts, size_t * mem_sizes,
     uint64_t * file_starts, uint64_t * file_sizes, zoidfs::zoidfs_op_hint_t * op_hint);

  void enqueueReadCB(
     iofwdevent::CBType cb, zoidfs::zoidfs_handle_t * handle, size_t count,
     void ** mem_starts, size_t * mem_sizes,
     uint64_t * file_starts, uint64_t * file_sizes, zoidfs::zoidfs_op_hint_t * op_hint);

protected:
  void run();
  void issue(std::vector<ChildRange *>& rs);
  void notifyConsumer();
  void issueWait(int status);

private:
  iofwdutil::zlog::ZLogSource & log_;

  boost::scoped_ptr<boost::thread> consumethread_;
  boost::mutex lock_;
  boost::condition_variable ready_;
  bool exiting_;

  zoidfs::ZoidFSAsyncAPI * async_api_;
  zoidfs::util::ZoidFSDefAsync * async_cb_api_;
  boost::scoped_ptr<RangeScheduler> range_sched_;

  enum{EVMODE_TASK = 0, EVMODE_SM};
  int mode_;
};

}

#endif
