#include "ZeroCopyTransformOutput.hh"
namespace iofwdevent {

  /**
   * Constructor for ZeroCopyTransformOutput
   * @param[in] out       Output stream for data written to 
   *                      ZeroCopyTransformOutput stream.
   * @param[in] transform Transform class for the transformation 
   *                      to apply to the stream.
   */
  ZeroCopyTransformOutput::ZeroCopyTransformOutput(ZeroCopyOutputStream * out,
                                                 GenericTransform * transform,
                                                 size_t len)
  {
    this->internalMemPtr = new char[len];
    this->internalMem = new ZeroCopyMemoryOutput ((void *)internalMemPtr, len); 
    this->mem = out;
    this->transform = transform;
  }

  /**
   * Returns a ptr to the transformed output data.
   * 
   * @param ptr           Pointer to the transformed output data
   * @param size          Pointer to the size of data in ptr 
   * @param cb            Callback
   * @param suggested     Suggested size of the write 
   */
  Handle ZeroCopyTransformOutput::write ( void ** ptr, size_t * size, 
                                          const CBType & cb, size_t suggested)
  {
    /* If there is any space left in the internal memory, use the remainder 
       (or up to suggested size). */
    if (this->internalMem->spaceRemaining() > 0)
    {
      return this->internalMem->write(ptr, size, cb, suggested);
    }
    /* Else this is a blocking write */
    else
      return this->blockingWrite(ptr, size, cb, suggested);
  }

  /**
   * Internal function that actually performs the write request. Assume that
   * this function will block (it wont always but assume that it can/will)
   */
  Handle ZeroCopyTransformOutput::blockingWrite (void ** ptr, size_t * size,
                                                 const CBType & cb, 
                                                 size_t suggested)
  {
    /* Create the work unit for this callback */
    ZeroCopyTransformWriteWU * wu = new ZeroCopyTransformWriteWU (ptr, size, 
                                        cb, suggested);

    /* Bind internal callback (not client callback) */
    CBType writeCB = boost::bind(&ZeroCopyTransformOutput::blockingWriteCB,
                                 boost::ref(*this), _1, wu);

    /* Bind the function for workqueue */
    boundFunc f = boost::bind(&ZeroCopyTransformOutput::flush, 
                           boost::ref(*this), writeCB);

    /* Lock the callback queue */
    boost::mutex::scoped_lock lock(this->lock_);

    /* Push the callback onto the queue */
    this->writeCBQueue.push_back(f);
    
    return (Handle)0;
  }

  void ZeroCopyTransformOutput::blockingWriteCB(CBException e, 
                                                ZeroCopyTransformWriteWU * wu)
  {
    e.check();

    /* If there is any space left in the internal memory, use the remainder 
       (or up to suggested size). */
    if (this->internalMem->spaceRemaining() > 0)
    {
      this->internalMem->write(wu->ptr_, wu->size_, wu->cb_, wu->suggested_);
    }
    else
      assert(1==0);
    delete wu;
  }

  /* Flush internal buffers, if any */
  Handle ZeroCopyTransformOutput::flush (const CBType & cb)
  {
    CBType flushCB;
    Handle x;
    size_t curSize = this->internalMem->getTotalLen();

    /* If there is nothing to flush */
    if (curSize == 0)
    {
      cb(*(new CBException()));
      return (Handle) 0;
    }

    /* Convert the internalMem ZeroCopyOutput data type to ZeroCopyInput so
       that it can be read */
    ZeroCopyMemoryInput * internal = this->convertToMemoryInput
                                                          (this->internalMem);

    /* Create the workunit for the flush */
    ZeroCopyTransformFlushWU * wu = new ZeroCopyTransformFlushWU (cb);

    /* Create the callback */
    flushCB = boost::bind(&ZeroCopyTransformOutput::internalReadCB,
                          boost::ref(*this), _1, wu);

    /* Make the call to the internal read */
    x = internal->read((const void **) &wu->inData_, &wu->inLen_, flushCB, curSize);

    /* This is an assumption that the size of the data read is equal to the
       size of the stream currently (this assumption may need to change) */
    assert(wu->inLen_ == curSize);

    return (Handle) 0;
  }

  /* No Callback */
  void ZeroCopyTransformOutput::nullCB (CBException e)
  { 
    e.check();
  }

  /* Finale Flush Stage */
  void ZeroCopyTransformOutput::doTransform (ZeroCopyTransformFlushWU * flushWu)
  {
    void ** outData_ = new void *[1];
    size_t outLen_;
    size_t outputSize_;
    int outState_;
    bool flushFlag_ = true;

    /* Get the write location */
    CBType nullCB = boost::bind(&ZeroCopyTransformOutput::nullCB, 
                                  boost::ref(*this), _1 );
  
    /* Get location to write to */
    this->mem->write(outData_, &outLen_, nullCB, flushWu->inLen_);

    /* Preform the transform */
    this->transform->transform ((const void* const) flushWu->inData_, 
                                flushWu->inLen_, outData_, 
                                outLen_, &outputSize_, 
                                &outState_, flushFlag_);

    /* If all the output from write was not used, return unused portion */
    if (outputSize_ != outLen_)
      this->mem->rewindOutput(outLen_ - outputSize_, nullCB);

    this->internalMem->reset();

    flushWu->cb_(*(new CBException()));
    
  }
  /* Flush stage one completion */
  void ZeroCopyTransformOutput::internalReadCB(CBException e, ZeroCopyTransformFlushWU * wu)
  {
    e.check();
    /* Lock the work unit queue (may need a differnt lock for this) */
    boost::mutex::scoped_lock lock(this->lock_);    

    /* Bind the function for workqueue */
    boundFunc f = boost::bind(&ZeroCopyTransformOutput::doTransform, 
                           boost::ref(*this), wu);

    this->writeCBQueue.push_back(f);
  }

  /* Rewinds the write stream */
  Handle ZeroCopyTransformOutput::rewindOutput (size_t size, 
                                                const CBType & cb)
  {
    return this->internalMem->rewindOutput(size, cb);
  }

  /**
   * Converts ZeroCopyMemoryOutput -> ZeroCopyMemoryInput. This is
   * used for changing the direction of the internal ZeroCopyMemory stream 
   * from write mode (output) to read mode (input) for the memory region 
   * stored in ZeroCopyMemoryOutput.
   */
  ZeroCopyMemoryInput * ZeroCopyTransformOutput::convertToMemoryInput(
                                              ZeroCopyMemoryOutput * x)
  {
    void * ptr = x->getMemPtr();
    ZeroCopyMemoryInput * v = new ZeroCopyMemoryInput ( ptr, x->getTotalLen());
    return v;
  }

}
