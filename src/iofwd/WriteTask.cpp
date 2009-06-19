#include "WriteTask.hh"
#include "zoidfs/util/ZoidFSAPI.hh"
#include "zoidfs/zoidfs-proto.h"

#include <vector>
#include <deque>

using namespace std;

namespace iofwd
{
//===========================================================================

namespace {

class BufferPool
{
public:
  BufferPool(size_t size, size_t num)
    : size_(size), num_(num)
  {
    for (size_t i = 0; i < num_; i++)
      free_bufs_.push_back(new char[size_]);
  }
  ~BufferPool() {
    assert(free_bufs_.size() == num_);
    for (size_t i = 0; i < free_bufs_.size(); i++)
      delete[] free_bufs_[i];
  }
  bool hasFreeBuf() const {
    return !free_bufs_.empty();
  }
  char* getBuf() {
    if (free_bufs_.empty()) return NULL;
    char * ret = free_bufs_[free_bufs_.size()-1];
    free_bufs_.pop_back();
    return ret;
  }
  void putBuf(char *b) {
    free_bufs_.push_back(b);
  }
private:
  size_t size_;
  size_t num_;
  vector<char*> free_bufs_;
};

struct RetrievedBuffer
{
  char *buf;
  uint64_t siz;
  uint64_t off;
  iofwdutil::completion::CompletionID * io_id;
};

}

void WriteTask::runNormalMode(const WriteRequest::ReqParam & p)
a{
   std::auto_ptr<iofwdutil::completion::CompletionID> recv_id (request_.recvBuffers ());
   recv_id->wait ();

   // p.mem_sizes is uint32_t array, but ZoidFSAPI::write() takes size_t array
   // for its arguments. Therefore, in (sizeof(size_t) != sizeof(uint32_t))
   // environment (64bit), p.mem_sizes is not valid for size_t array.
   // We allocate temporary buffer to fix this problem.
   size_t * tmp_mem_sizes = (size_t*)p.mem_sizes;
   bool need_size_t_workaround = (sizeof(size_t) != sizeof(uint32_t));
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

void WriteTask::runPipelineMode(const WriteRequest::ReqParam & p)
{
   // TODO: aware of system-wide memory consumption
   uint64_t pipeline_bytes = zoidfs::ZOIDFS_BUFFER_MAX * 2;
   BufferPool alloc(pipeline_bytes, 8);

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
     uint64_t p_siz = 0;
     char * p_buf = NULL;
     iofwdutil::completion::CompletionID * rx_id = NULL;

     // issue I/O requests for already retrieved buffers in rx_q
     while (!rx_q.empty()) {
       RetrievedBuffer b = rx_q.front();
       assert(b.buf != NULL);
       rx_q.pop_front();

       iofwdutil::completion::CompletionID * io_id = NULL; // TODO: issue write
       b.io_id = io_id;
       io_q.push_back(b);
     }

     // issue recv requests for next pipeline buffer
     if (alloc.hasFreeBuf()) {
       p_siz = std::min(pipeline_bytes, total_bytes - cur_recv_bytes);
       p_buf = alloc.getBuf();
       rx_id = request_.recvPipelineBuffer(p_buf, p_siz);
     }

     // check I/O requests completion in io_q
     for (deque<RetrievedBuffer>::iterator it = io_q.begin(); it != io_q.end();) {
       RetrievedBuffer& b = *it;
       iofwdutil::completion::CompletionID * io_id = b.io_id;
       int done = 1; // TODO: io_id->test();
       if (done) {
         if (io_id) delete io_id; // TODO: no null check
         alloc.putBuf(b.buf);
         it = io_q.erase(it);
       } else {
         ++it;
       }
     }

     // wait for retrieving buffer
     if (rx_id != NULL) {
       rx_id->wait();
       RetrievedBuffer b;
       b.buf = p_buf;
       b.siz = p_siz;
       b.off = cur_recv_bytes;
       b.io_id = NULL;
       rx_q.push_back(b);
       delete rx_id;

       // advance to retrieve the next pipeline
       cur_recv_bytes += p_siz;
     }
   }

   // issue remaining I/O requests
   while (!rx_q.empty()) {
     RetrievedBuffer b = rx_q.front();
     assert(b.buf != NULL);
     rx_q.pop_front();
     
     iofwdutil::completion::CompletionID * io_id = NULL; // TODO: issue write
     b.io_id = io_id;
     io_q.push_back(b);
   }

   // wait remaining I/O requests
   while (!io_q.empty()) {
     RetrievedBuffer& b = io_q.front();
     io_q.pop_front();

     iofwdutil::completion::CompletionID * io_id = b.io_id;
     if (io_id)
       io_id->wait();
     alloc.putBuf(b.buf);
   }

   // reply status
   request_.setReturnCode(zoidfs::ZFS_OK);
   std::auto_ptr<iofwdutil::completion::CompletionID> reply_id (request_.reply ());
   reply_id->wait ();
}

//===========================================================================
}
