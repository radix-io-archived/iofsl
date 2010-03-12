#include "BMIBufferPool.hh"

#include <cassert>

using namespace std;

namespace iofwd
{

//===========================================================================
BMIBufferWrapper::BMIBufferWrapper() : pool_(NULL), buf_(NULL)
{
}

BMIBufferWrapper::~BMIBufferWrapper()
{
    if(buf_)
    {
        pool_->free(buf_);
    }
    pool_ = NULL;
}

iofwdutil::bmi::BMIBuffer * BMIBufferWrapper::get_buf()
{
    assert(buf_ != NULL);
    return buf_->buf;
}

BMIBufferAllocCompletionID::BMIBufferAllocCompletionID(BMIBufferPool * p_,
                                                 WaitingBMIBuffer * b_)
  : pool_ (p_), buf_ (b_)
{
}

BMIBufferAllocCompletionID::~BMIBufferAllocCompletionID()
{
  if (buf_ != NULL) {
    pool_->free(buf_);
  }
  buf_ = NULL;
}

void
BMIBufferAllocCompletionID::wait()
{
  assert(buf_ != NULL);

  WaitingBMIBuffer * b = buf_;
  boost::unique_lock<boost::mutex> lk(b->mutex);
  while (b->buf->get() == NULL)
    b->cond.wait(lk);
  lk.unlock();
}

bool
BMIBufferAllocCompletionID::test(unsigned int mstimeout)
{
  assert(buf_ != NULL);

  bool ret = false;
  WaitingBMIBuffer * b = buf_;
  boost::unique_lock<boost::mutex> lk(b->mutex);
  if (b->buf->get() == NULL)
    b->cond.timed_wait(lk, boost::get_system_time()
                       + boost::posix_time::milliseconds(mstimeout));
  ret = (b->buf->get() != NULL);
  lk.unlock();
  return ret;
}

iofwdutil::bmi::BMIBuffer * BMIBufferAllocCompletionID::get_buf()
{
  assert(buf_ != NULL);
  return buf_->buf;
}

//===========================================================================

BMIBufferPool::BMIBufferPool(const iofwdutil::ConfigFile & c)
  : size_(0), cur_size_(0), max_num_(0)
{
    size_ = c.getKeyAsDefault("buffersize", 0);
    max_num_ = c.getKeyAsDefault("maxnumbuffers", 0);
}

BMIBufferPool::~BMIBufferPool()
{
}

BMIBufferAllocCompletionID*
BMIBufferPool::alloc(iofwdutil::bmi::BMIAddr addr, iofwdutil::bmi::BMI::AllocType a)
{
  WaitingBMIBuffer * b = new WaitingBMIBuffer();

  m_.lock();
  bool need_wait = cur_size_ >= max_num_;
  if (need_wait) {
    b->buf = new iofwdutil::bmi::BMIBuffer(addr, a);
    b->buf->resize(0);
    wait_q_.push_back(b);
  } else {
    b->buf = new iofwdutil::bmi::BMIBuffer(addr, a);
    b->buf->resize(size_);
    cur_size_++;
  }
  m_.unlock();

  return new BMIBufferAllocCompletionID(this, b);
}

void BMIBufferPool::allocCB(iofwdevent::CBType cb, iofwdutil::bmi::BMIAddr addr, iofwdutil::bmi::BMI::AllocType a, BMIBufferWrapper * bwrap)
{
  WaitingBMIBuffer * b = new WaitingBMIBuffer();

  /* try to get a BMIBuffer */
  m_.lock();
  bool need_wait = cur_size_ >= max_num_;
  if (need_wait) {
    b->buf = new iofwdutil::bmi::BMIBuffer(addr, a);
    b->buf->resize(0);
    wait_q_.push_back(b);
  } else {
    b->buf = new iofwdutil::bmi::BMIBuffer(addr, a);
    b->buf->resize(size_);
    cur_size_++;
  }
  m_.unlock();

  /* setup the buffer data */
  bwrap->buf_ = b;
  bwrap->pool_ = this;

  /* issue the callback */
  cb(iofwdevent::COMPLETED);
}

void
BMIBufferPool::free(WaitingBMIBuffer * b)
{
  assert(b->buf != NULL);

  m_.lock();
  bool is_waited = wait_q_.size() > 0;
  if (is_waited) {
    WaitingBMIBuffer * wait_b = wait_q_.front();
    wait_q_.pop_front();
    m_.unlock();

    wait_b->mutex.lock();
    delete b->buf;
    wait_b->buf->resize(size_);
    wait_b->cond.notify_one();
    wait_b->mutex.unlock();
  } else {
    delete b->buf;
    b->buf = NULL;

    cur_size_--;
    m_.unlock();
  }

  delete b;
}

//===========================================================================
}
