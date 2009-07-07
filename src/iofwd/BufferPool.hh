#ifndef IOFWD_BUFFER_POOL_HH
#define IOFWD_BUFFER_POOL_HH

#include <boost/thread.hpp>
#include <iofwdutil/completion/CompletionID.hh>

namespace iofwd
{

class BufferPool;
class WaitingBuffer;

class BufferAllocCompletionID : public iofwdutil::completion::CompletionID
{
public:
  BufferAllocCompletionID(BufferPool *pool_, WaitingBuffer *buf_);
  virtual ~BufferAllocCompletionID();
  
  virtual void wait ();
  virtual bool test (unsigned int mstimeout);

  char * get_buf();

protected:
  BufferPool * pool_;
  WaitingBuffer * buf_;
};

class BufferPool
{
public:
  BufferPool(uint64_t size, uint64_t max_num);
  ~BufferPool();

  uint64_t pipeline_size() const { return size_; }
  BufferAllocCompletionID * alloc();

protected:
  friend class BufferAllocCompletionID;
  void free(WaitingBuffer * buf);
  
private:
  uint64_t size_;
  uint64_t cur_size_;
  uint64_t max_num_;
  std::deque<WaitingBuffer*> wait_q_;
  
  boost::mutex m_;
};

}

#endif
