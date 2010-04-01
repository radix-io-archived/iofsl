#include "ReadTask.hh"
#include "zoidfs/util/ZoidFSAPI.hh"
#include "zoidfs/util/ZoidFSAsyncAPI.hh"
#include "zoidfs/zoidfs-proto.h"
#include "RequestScheduler.hh"
#include "iofwd/BMIBufferPool.hh"

#include <vector>
#include <deque>

using namespace std;

namespace iofwd
{
//===========================================================================

void ReadTask::runNormalMode(const ReadRequest::ReqParam & p)
{

   block_.reset();
   sched_->enqueueReadCB(block_, p.handle, (size_t)p.mem_count, (void**)p.mem_starts, p.mem_sizes, p.file_starts, p.file_sizes, p.op_hint);
   block_.wait();

   request_.setReturnCode(zoidfs::ZFS_OK); /* TODO: pass back the actual return value */

   // send buffers w/ callback
   block_.reset();
   request_.sendBuffers((block_));
   block_.wait();

   // send reply w/ callback
   block_.reset();
   request_.reply((block_));
   block_.wait();
}

void ReadTask::execPipelineIO(const ReadRequest::ReqParam & UNUSED(p))
{
}

void ReadTask::runPipelineMode(const ReadRequest::ReqParam & UNUSED(p))
{
   // send reply w/ callback

   request_.setReturnCode(zoidfs::ZFSERR_NOTIMPL);
   block_.reset();
   request_.reply((block_));
   block_.wait();
}

//===========================================================================
}
