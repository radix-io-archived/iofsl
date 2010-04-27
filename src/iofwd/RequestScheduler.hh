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
#include "zoidfs/util/ZoidFSAsyncPT.hh"

namespace iofwd
{

class WriteRequest;
class ReadRequest;
class RangeScheduler;

class RequestScheduler : public zoidfs::util::ZoidFSAsyncPT
{
public:
  RequestScheduler(zoidfs::util::ZoidFSAsync * api, const iofwdutil::ConfigFile & c, int mode);
  virtual ~RequestScheduler();

  void write(
     const iofwdevent::CBType & cb, int * ret, const zoidfs::zoidfs_handle_t * handle, size_t count,
     const void ** mem_starts, const size_t * mem_sizes, size_t file_count,
     const zoidfs::zoidfs_file_ofs_t file_starts[], zoidfs::zoidfs_file_size_t file_sizes[], zoidfs::zoidfs_op_hint_t * op_hint);

  void read(
     const iofwdevent::CBType & cb, int * ret, const zoidfs::zoidfs_handle_t * handle, size_t count,
     void * mem_starts[], const size_t mem_sizes[], size_t file_count,
     const zoidfs::zoidfs_file_ofs_t file_starts[], zoidfs::zoidfs_file_size_t file_sizes[], zoidfs::zoidfs_op_hint_t * op_hint);

protected:

    /* ThreadPool helper for the RequestScheduler */
    class ReqSchedHelper
    {
        public:
            ReqSchedHelper(RequestScheduler * rs) : rs_(rs)
            {
            }

            ~ReqSchedHelper()
            {
            }

            void run()
            {
                rs_->run(false);
            }

        protected:
            RequestScheduler * rs_;
    };

  void run(bool waitForWork);
  void issue(std::vector<ChildRange *>& rs);
  void notifyConsumer();
  void issueWait(int status);

private:
  iofwdutil::zlog::ZLogSource & log_;

  boost::scoped_ptr<boost::thread> consumethread_;
  boost::mutex lock_;
  boost::condition_variable ready_;
  bool exiting_;

  zoidfs::util::ZoidFSAsync * api_;
  boost::scoped_ptr<RangeScheduler> range_sched_;

  enum{EVMODE_TASK = 0, EVMODE_SM};
  int mode_;
  bool schedActive_;
};

}

#endif
