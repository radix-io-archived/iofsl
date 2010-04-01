#include "WriteTask.hh"
#include "zoidfs/util/ZoidFSAPI.hh"
#include "zoidfs/util/ZoidFSAsyncAPI.hh"
#include "zoidfs/zoidfs-proto.h"
#include "RequestScheduler.hh"
#include "BMIBufferPool.hh"

#include <vector>
#include <deque>

#include <cstdio>

using namespace std;

namespace iofwd
{
//===========================================================================

void WriteTask::runNormalMode(const WriteRequest::ReqParam & p)
{
   // issue recvBuffers w/ callback
   block_.reset();
   request_.recvBuffers((block_));
   block_.wait();

   block_.reset();
   sched_->enqueueWriteCB(block_, p.handle, (size_t)p.mem_count, (const void**)p.mem_starts, p.mem_sizes, p.file_starts, p.file_sizes, p.op_hint);
   block_.wait();

   int ret = zoidfs::ZFS_OK;
   request_.setReturnCode(ret);

   // issue reply w/ callback
   block_.reset();
   request_.reply((block_));
   block_.wait();
}

void WriteTask::execPipelineIO(const WriteRequest::ReqParam & UNUSED(p))
{
}

void WriteTask::runPipelineMode(const WriteRequest::ReqParam & UNUSED(p))
{
   // reply status
   request_.setReturnCode(zoidfs::ZFSERR_NOTIMPL);
   block_.reset();
   request_.reply((block_));
   block_.wait();
}

//===========================================================================
}
