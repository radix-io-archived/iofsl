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

struct ReadBuffer
{
  BMIBufferAllocCompletionID * alloc_id;
  uint64_t siz;
  uint64_t off;
  iofwdutil::completion::CompletionID * io_id;
  iofwdutil::completion::CompletionID * tx_id;

  // for async request
  char ** mem_starts;
  size_t * mem_sizes;
  uint64_t * file_starts;
  uint64_t * file_sizes;
};

namespace {
void releaseReadBuffer(ReadBuffer& b)
{
  delete b.alloc_id;
  delete b.tx_id;
  delete[] b.mem_starts;
  delete[] b.mem_sizes;
  delete[] b.file_starts;
  delete[] b.file_sizes;
}
}

void ReadTask::runNormalMode(const ReadRequest::ReqParam & p)
{
   // p.mem_sizes is uint64_t array, but ZoidFSAPI::read() takes size_t array
   // for its arguments. Therefore, in (sizeof(size_t) != sizeof(uint64_t))
   // environment (32bit), p.mem_sizes is not valid for size_t array.
   // We allocate temporary buffer to fix this problem.
   size_t * tmp_mem_sizes = (size_t*)p.mem_sizes;
   bool need_size_t_workaround = (sizeof(size_t) != sizeof(uint64_t));
   if (need_size_t_workaround) {
      tmp_mem_sizes = new size_t[p.mem_count];
      for (uint32_t i = 0; i < p.mem_count; i++)
         tmp_mem_sizes[i] = p.mem_sizes[i];
   }

   std::auto_ptr<iofwdutil::completion::CompletionID> io_id (sched_->enqueueRead (
      p.handle, (size_t)p.mem_count,
      (void**)p.mem_starts, tmp_mem_sizes,
      p.file_starts, p.file_sizes, p.op_hint));
   io_id->wait ();
   int ret = zoidfs::ZFS_OK;
   request_.setReturnCode (ret);

   if (need_size_t_workaround)
      delete[] tmp_mem_sizes;

   std::auto_ptr<iofwdutil::completion::CompletionID> send_id (request_.sendBuffers ());
   send_id->wait ();

   std::auto_ptr<iofwdutil::completion::CompletionID> reply_id (request_.reply ());
   reply_id->wait ();
}

iofwdutil::completion::CompletionID * ReadTask::execPipelineIO(const ReadRequest::ReqParam & p,
   ReadBuffer * b)
{
   char * p_buf = (char *)b->alloc_id->get_buf()->get();
   const uint64_t p_offset = b->off;
   const uint64_t p_size = b->siz;
   const uint64_t * file_starts = p.file_starts;
   const uint64_t * file_sizes = p.file_sizes;

   uint32_t st_file = 0, en_file = 0;
   uint64_t st_fileofs = 0, en_fileofs = 0;
   {
     bool st_ok = false;
     bool en_ok = false;
     uint32_t cur_file = 0;
     uint64_t cur_ofs = p_offset;
     while (!(st_ok && en_ok)) {
       const uint64_t st = p_offset;
       const uint64_t en = p_offset + p_size;
       assert(cur_file < p.file_count);
       if (cur_ofs <= st && st < cur_ofs + file_sizes[cur_file]) {
         st_file = cur_file;
         st_fileofs = st - cur_ofs;
         st_ok = true;
       }
       if (cur_ofs < en && en <= cur_ofs + file_sizes[cur_file]) {
         en_file = cur_file;
         en_fileofs = en - cur_ofs;
         en_ok = true;
         assert(st_ok);
         break;
       }
       cur_ofs += file_sizes[cur_file];
       cur_file++;
     }
     assert(st_file <= en_file);
   }

   size_t p_file_count = en_file + 1 - st_file;
   uint64_t * p_file_starts = new uint64_t[p_file_count];
   uint64_t * p_file_sizes = new uint64_t[p_file_count];
   if (st_file == en_file) {
      p_file_starts[0] = file_starts[st_file] + st_fileofs;
      assert(en_fileofs > st_fileofs);
      p_file_sizes[0] = en_fileofs - st_fileofs;
   } else {
      for (uint32_t i = st_file; i <= en_file; i++) {
         if (i == st_file) {
            p_file_starts[i - st_file] = file_starts[i] + st_fileofs;
            p_file_sizes[i - st_file] = file_sizes[i] - st_fileofs;
         } else if (i == en_file) {
            p_file_starts[i - st_file] = file_starts[i];
            p_file_sizes[i - st_file] = en_fileofs;
         } else {
            p_file_starts[i - st_file] = file_starts[i];
            p_file_sizes[i - st_file] = file_sizes[i];
         }
      }
   }

   // issue async I/O
   uint64_t cur = 0;
   char ** mem_starts = new char*[p_file_count];
   size_t * mem_sizes = new size_t[p_file_count];
   for (size_t i = 0; i < p_file_count; i++) {
     mem_starts[i] = p_buf + cur;
     mem_sizes[i] = p_file_sizes[i];
     cur += p_file_sizes[i];
   }
   iofwdutil::completion::CompletionID * id = sched_->enqueueRead (
      p.handle, p_file_count, (void**)mem_starts, mem_sizes,
      p_file_starts, p_file_sizes, p.op_hint);
   /*
   iofwdutil::completion::CompletionID * id = async_api_->async_read (
      p.handle, p_file_count, (void**)mem_starts, mem_sizes,
      p_file_count, p_file_starts, p_file_sizes);
   */
   b->mem_starts = mem_starts;
   b->mem_sizes = mem_sizes;
   b->file_starts = p_file_starts;
   b->file_sizes = p_file_sizes;

   return id;
}

void ReadTask::runPipelineMode(const ReadRequest::ReqParam & p)
{
   // TODO: aware of system-wide memory consumption
   uint64_t pipeline_bytes = bpool_->pipeline_size();
   BMIBufferAllocCompletionID * alloc_id = NULL;

   // The life cycle of buffers is like follows:
   // from alloc -> ZoidI/O -> io_q -> NetworkSend -> tx_q -> back to alloc
   deque<ReadBuffer> io_q;
   deque<ReadBuffer> tx_q;

   uint64_t cur_read_bytes = 0;
   uint64_t total_bytes = 0;
   for (uint32_t i = 0; i < p.mem_count; i++)
      total_bytes += p.mem_sizes[i];

   // iterate until read all regions
   while (cur_read_bytes < total_bytes) {
      bool is_issue_read = false;
      size_t cur_pipeline_bytes = std::min(pipeline_bytes, total_bytes - cur_read_bytes);

      // issue send requests one-by-one for already read buffers in tx_q
      // to ensure the message order, tx_q must be empty here
      // TODO: support out-of-order transfer
      // test for first read buffer
      if (!io_q.empty() && tx_q.empty()) {
         ReadBuffer b = io_q.front();
         iofwdutil::completion::CompletionID * io_id = b.io_id;
         if (io_id->test(10)) {
            delete io_id;
            io_q.pop_front();

            b.tx_id = request_.sendPipelineBuffer(b.alloc_id->get_buf(), b.siz);
            tx_q.push_back(b);
         }        
      }

      // try to alloc buffer
      if (alloc_id == NULL)
         alloc_id = bpool_->alloc(request_.getRequestAddr(), iofwdutil::bmi::BMI::ALLOC_SEND);
      // issue I/O requests for next pipeline buffer
      if (alloc_id != NULL && alloc_id->test(10)) {
         ReadBuffer b;
         b.alloc_id = alloc_id;
         b.off = cur_read_bytes;
         b.siz = cur_pipeline_bytes;
         b.tx_id = NULL;
         b.mem_starts = NULL;
         b.mem_sizes = NULL;
         b.file_starts = NULL;
         b.file_sizes = NULL;
         b.io_id = execPipelineIO(p, &b);
         io_q.push_back(b);
         is_issue_read = true;
      }

      // check send requests completion in tx_q
      assert(tx_q.size() <= 1);
      for (deque<ReadBuffer>::iterator it = tx_q.begin(); it != tx_q.end();) {
         ReadBuffer& b = *it;
         iofwdutil::completion::CompletionID * tx_id = b.tx_id;
         if (tx_id->test(10)) {
            assert(tx_id != NULL);
            releaseReadBuffer(b);
            it = tx_q.erase(it);
            break;
         } else {
            ++it;
         }
      }
      
      // advance to read the next pipeline
      if (is_issue_read) {
         cur_read_bytes += cur_pipeline_bytes;
         alloc_id = NULL;
      }
   }

   // wait remaining send requests, not to violate message order
   while (!tx_q.empty()) {
      ReadBuffer b = tx_q.front();
      tx_q.pop_front();

      iofwdutil::completion::CompletionID * tx_id = b.tx_id;
      assert(tx_id != NULL);
      tx_id->wait();
      releaseReadBuffer(b);
   }

   // send remaining I/O requests
   {
     while (!io_q.empty()) {
       ReadBuffer b = io_q.front();
       assert(b.alloc_id != NULL);
       io_q.pop_front();

       iofwdutil::completion::CompletionID * io_id = b.io_id;
       io_id->wait();
       delete io_id;

       b.tx_id = request_.sendPipelineBuffer(b.alloc_id->get_buf(), b.siz);
       assert(b.tx_id != NULL);
       b.tx_id->wait();
       releaseReadBuffer(b);
     }
   }

   // reply status
   request_.setReturnCode(zoidfs::ZFS_OK);
   std::auto_ptr<iofwdutil::completion::CompletionID> reply_id (request_.reply ());
   reply_id->wait ();
}

//===========================================================================
}
