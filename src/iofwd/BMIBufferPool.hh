#ifndef IOFWD_BMI_BUFFER_POOL_HH
#define IOFWD_BMI_BUFFER_POOL_HH

#include <deque>
#include <boost/thread.hpp>
#include <iofwdutil/completion/CompletionID.hh>
#include "iofwdutil/bmi/BMIBuffer.hh"
#include "iofwdutil/bmi/BMIAddr.hh"
#include "iofwdutil/bmi/BMI.hh"
#include "iofwdutil/ConfigFile.hh"
#include "iofwdevent/CBType.hh"

namespace iofwd
{

class BMIBufferPool;

struct WaitingBMIBuffer
{
  iofwdutil::bmi::BMIBuffer * buf;
  boost::mutex mutex;
  boost::condition_variable cond;
};

class BMIBufferAllocCompletionID : public iofwdutil::completion::CompletionID
{
public:
  BMIBufferAllocCompletionID(BMIBufferPool *pool_, WaitingBMIBuffer *buf_);
  virtual ~BMIBufferAllocCompletionID();

  virtual void wait ();
  virtual bool test (unsigned int mstimeout);

  iofwdutil::bmi::BMIBuffer * get_buf();

protected:
  BMIBufferPool * pool_;
  WaitingBMIBuffer * buf_;
};

class BMIBufferWrapper
{
    public:
        BMIBufferWrapper();
        ~BMIBufferWrapper();
        iofwdutil::bmi::BMIBuffer * get_buf();

        BMIBufferPool * pool_;
        WaitingBMIBuffer * buf_;
};

class BMIBufferPool
{
public:
  BMIBufferPool(const iofwdutil::ConfigFile & c);
  ~BMIBufferPool();

  uint64_t pipeline_size() const { return size_; }
  BMIBufferAllocCompletionID * alloc(iofwdutil::bmi::BMIAddr addr, iofwdutil::bmi::BMI::AllocType a);
  void allocCB(iofwdevent::CBType cb, iofwdutil::bmi::BMIAddr addr, iofwdutil::bmi::BMI::AllocType a, BMIBufferWrapper * bwrap);

protected:
  friend class BMIBufferAllocCompletionID;
  friend class BMIBufferWrapper;
  void free(WaitingBMIBuffer * buf);

private:
  uint64_t size_;
  uint64_t cur_size_;
  uint64_t max_num_;
  std::deque<WaitingBMIBuffer*> wait_q_;

  boost::mutex m_;
};

}

#endif
