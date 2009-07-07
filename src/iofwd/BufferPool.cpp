#include "BufferPool.hh"

#include <cassert>

using namespace std;

namespace iofwd
{

struct WaitingBuffer
{
  char * buf;
  boost::mutex mutex;
  boost::condition_variable cond;
};

//===========================================================================

BufferAllocCompletionID::BufferAllocCompletionID(BufferPool * p_,
                                                 WaitingBuffer * b_)
  : pool_ (p_), buf_ (b_)
{
}

BufferAllocCompletionID::~BufferAllocCompletionID()
{
  if (buf_ != NULL) {
    pool_->free(buf_);
  }
  buf_ = NULL;
}

void
BufferAllocCompletionID::wait()
{
  assert(buf_ != NULL);

  WaitingBuffer * b = buf_;
  boost::unique_lock<boost::mutex> lk(b->mutex);
  while (b->buf == NULL)
    b->cond.wait(lk);
  lk.unlock();
}

bool
BufferAllocCompletionID::test(unsigned int mstimeout)
{
  assert(buf_ != NULL);

  bool ret = false;
  WaitingBuffer * b = buf_;
  boost::unique_lock<boost::mutex> lk(b->mutex);
  if (b->buf == NULL)
    b->cond.timed_wait(lk, boost::get_system_time()
                       + boost::posix_time::milliseconds(mstimeout));
  ret = (b->buf != NULL);
  lk.unlock();
  return ret;
}

char*
BufferAllocCompletionID::get_buf()
{
  assert(buf_ != NULL);
  return buf_->buf;
}

//===========================================================================

BufferPool::BufferPool(uint64_t size, uint64_t max_num)
  : size_(size), cur_size_(0), max_num_(max_num)
{
}

BufferPool::~BufferPool()
{
}

BufferAllocCompletionID*
BufferPool::alloc()
{
  WaitingBuffer * b = new WaitingBuffer();

  m_.lock();
  bool need_wait = cur_size_ >= max_num_;
  if (need_wait) {
    b->buf = NULL;
    wait_q_.push_back(b);
  } else {
    b->buf = new char[size_];
    cur_size_++;
  }
  m_.unlock();

  return new BufferAllocCompletionID(this, b);
}

void
BufferPool::free(WaitingBuffer * b)
{
  assert(b->buf != NULL);

  m_.lock();
  bool is_waited = wait_q_.size() > 0;
  if (is_waited) {
    WaitingBuffer * wait_b = wait_q_.front();
    wait_q_.pop_front();
    m_.unlock();
    
    wait_b->mutex.lock();
    wait_b->buf = b->buf;
    wait_b->cond.notify_one();
    wait_b->mutex.unlock();
  } else {
    delete[] b->buf;
    b->buf = NULL;
    
    cur_size_--;
    m_.unlock();
  }

  delete b;
}

//===========================================================================
}
