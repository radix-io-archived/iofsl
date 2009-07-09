#include "WriteTask.hh"
#include "zoidfs/util/ZoidFSAPI.hh"
#include "zoidfs/util/ZoidFSAsyncAPI.hh"
#include "zoidfs/zoidfs-proto.h"
#include "RequestScheduler.hh"
#include "BufferPool.hh"

#include <vector>
#include <deque>

using namespace std;

namespace iofwd
{
//===========================================================================

struct RetrievedBuffer
{
  BufferAllocCompletionID * alloc_id;
  uint64_t siz;
  uint64_t off;
  iofwdutil::completion::CompletionID * io_id;

  // for async request
  const char ** mem_starts;
  size_t * mem_sizes;
  uint64_t * file_starts;
  uint64_t * file_sizes;
};

namespace {
void releaseRetrievedBuffer(RetrievedBuffer& b)
{
   delete b.alloc_id;
   delete b.io_id;
   delete[] b.mem_starts;
   delete[] b.mem_sizes;
   delete[] b.file_starts;
   delete[] b.file_sizes;
}
}

void WriteTask::runNormalMode(const WriteRequest::ReqParam & p)
{
   std::auto_ptr<iofwdutil::completion::CompletionID> recv_id (request_.recvBuffers ());
   recv_id->wait ();

   // p.mem_sizes is uint64_t array, but ZoidFSAPI::write() takes size_t array
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

   int ret = api_->write (p.handle,
                          (size_t)p.mem_count, (const void **)p.mem_starts, tmp_mem_sizes,
                          (size_t)p.file_count, p.file_starts, p.file_sizes);
   request_.setReturnCode (ret);

   if (need_size_t_workaround)
      delete[] tmp_mem_sizes;

   std::auto_ptr<iofwdutil::completion::CompletionID> reply_id (request_.reply ());
   reply_id->wait ();
}

iofwdutil::completion::CompletionID * WriteTask::execPipelineIO(const WriteRequest::ReqParam & p,
   RetrievedBuffer * b)
{
   const char * p_buf = b->alloc_id->get_buf();
   const uint64_t p_offset = b->off;
   const uint64_t p_size = b->siz;
   const uint64_t * file_starts = p.file_starts;
   const uint64_t * file_sizes = p.file_sizes;

   uint32_t st_file, en_file;
   uint64_t st_fileofs, en_fileofs;
   {
     bool st_ok = false;
     bool en_ok = false;
     uint32_t cur_file = 0;
     uint64_t cur_ofs = 0;
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
            p_file_starts[i] = file_starts[i] + st_fileofs;
            p_file_sizes[i] = file_sizes[i] - st_fileofs;
         } else if (i == en_file) {
            p_file_starts[i] = file_starts[i];
            p_file_sizes[i] = en_fileofs;
         } else {
            p_file_starts[i] = file_starts[i];
            p_file_sizes[i] = file_sizes[i];
         }
      }
   }

   // issue async I/O
   uint64_t cur = 0;
   const char ** mem_starts = new const char*[p_file_count];
   size_t * mem_sizes = new size_t[p_file_count];
   for (size_t i = 0; i < p_file_count; i++) {
     mem_starts[i] = p_buf + cur;
     mem_sizes[i] = p_file_sizes[i];
     cur += p_file_sizes[i];
   }
   iofwdutil::completion::CompletionID * id = sched_->enqueueWrite (
      p.handle, p_file_count, (const void**)mem_starts, mem_sizes,
      p_file_starts, p_file_sizes);
   /*
   iofwdutil::completion::CompletionID * id = async_api_->async_write (
      p.handle, p_file_count, (const void**)mem_starts, mem_sizes,
      p_file_count, p_file_starts, p_file_sizes);
   */
   b->mem_starts = mem_starts;
   b->mem_sizes = mem_sizes;
   b->file_starts = p_file_starts;
   b->file_sizes = p_file_sizes;

   return id;
}

void WriteTask::runPipelineMode(const WriteRequest::ReqParam & p)
{
   // TODO: aware of system-wide memory consumption
   uint64_t pipeline_bytes = pool_->pipeline_size();
   BufferAllocCompletionID * alloc_id = NULL;

   // The life cycle of buffers is like follows:
   // from alloc -> NetworkRecv -> rx_q -> ZoidI/O -> io_q -> back to alloc
   deque<RetrievedBuffer> rx_q;
   deque<RetrievedBuffer> io_q;
  
   uint64_t cur_recv_bytes = 0;
   uint64_t total_bytes = 0;
   for (uint32_t i = 0; i < p.mem_count; i++)
      total_bytes += p.mem_sizes[i];

   // iterate until retrieving all buffers
   while (cur_recv_bytes < total_bytes) {
     uint64_t p_siz = std::min(pipeline_bytes, total_bytes - cur_recv_bytes);
     iofwdutil::completion::CompletionID * rx_id = NULL;

     // issue I/O requests for already retrieved buffers in rx_q
     while (!rx_q.empty()) {
       RetrievedBuffer b = rx_q.front();
       assert(b.alloc_id != NULL);
       rx_q.pop_front();

       b.io_id = execPipelineIO(p, &b);
       io_q.push_back(b);
     }

     // try to alloc buffer
     if (alloc_id == NULL)
       alloc_id = pool_->alloc();
     // issue recv requests for next pipeline buffer
     if (alloc_id != NULL && alloc_id->test(1)) {
       char * p_buf = alloc_id->get_buf();
       rx_id = request_.recvPipelineBuffer(p_buf, p_siz);
     }

     // check I/O requests completion in io_q
     for (deque<RetrievedBuffer>::iterator it = io_q.begin(); it != io_q.end();) {
       RetrievedBuffer& b = *it;
       iofwdutil::completion::CompletionID * io_id = b.io_id;
       if (io_id->test(1)) {
         assert(io_id != NULL);
         releaseRetrievedBuffer(b);
         it = io_q.erase(it);
       } else {
         ++it;
       }
     }

     // wait for retrieving buffer
     if (rx_id != NULL) {
       rx_id->wait();
       delete rx_id;

       RetrievedBuffer b;
       b.alloc_id = alloc_id;
       b.siz = p_siz;
       b.off = cur_recv_bytes;
       b.io_id = NULL;
       b.mem_starts = NULL;
       b.mem_sizes = NULL;
       b.file_starts = NULL;
       b.file_sizes = NULL;
       rx_q.push_back(b);

       // advance to retrieve the next pipeline
       cur_recv_bytes += p_siz;
       alloc_id = NULL;
     }
   }

   // issue remaining I/O requests
   while (!rx_q.empty()) {
     RetrievedBuffer b = rx_q.front();
     assert(b.alloc_id != NULL);
     rx_q.pop_front();
     
     b.io_id = execPipelineIO(p, &b);
     io_q.push_back(b);
   }

   // wait remaining I/O requests
   while (!io_q.empty()) {
     RetrievedBuffer b = io_q.front();
     io_q.pop_front();

     iofwdutil::completion::CompletionID * io_id = b.io_id;
     assert(io_id != NULL);
     io_id->wait();
     releaseRetrievedBuffer(b);
   }

   // reply status
   request_.setReturnCode(zoidfs::ZFS_OK);
   std::auto_ptr<iofwdutil::completion::CompletionID> reply_id (request_.reply ());
   reply_id->wait ();
}

//===========================================================================
}
