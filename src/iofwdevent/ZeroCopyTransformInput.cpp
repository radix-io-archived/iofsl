#include "ZeroCopyTransformInput.hh"
namespace iofwdevent {
  /**
   * Constructor for ZeroCopyTransformInput.
   * @param[in] in        Transformed input stream which needs to be 
   *                      transformed back into its original stream.
   * @param[in] transform Transform to use on input data (in)
   */
  ZeroCopyTransformInput::ZeroCopyTransformInput  (ZeroCopyInputStream * in, 
                                                   GenericTransform * transform,
                                                   size_t bufSize) 
  {
    char * streamInternal;
    char * transformInternal;
    if (bufSize > 0)
    {
      streamInternal = new char[bufSize];
      transformInternal = new char[bufSize];
      this->streamStorage = new ZeroCopyMemoryOutput((void *)streamInternal, bufSize);
      this->transformStorage = new ZeroCopyMemoryOutput((void *) transformInternal, bufSize);
    }
    else
    {
      streamInternal = new char[1000];
      transformInternal = new char[1000];
      this->streamStorage = new ZeroCopyMemoryOutput((void *)streamInternal, 1000);
      this->transformStorage = new ZeroCopyMemoryOutput((void *) transformInternal, 1000);
    }
    this->stream = in;
    this->transform = transform;
  }

  void ZeroCopyTransformInput::transformStorageCB(CBException e, size_t * len,
                                                  CBType cb)
  {
    e.check();

    this->transformStorage->setOffset(*len + this->transformStorage->getOffset());

    if (this->transformStorage->spaceRemaining() == 0)
      this->transformStorage->reset();

    cb(*(new CBException()));
  }


  void ZeroCopyTransformInput::streamStorageCB(CBException e, size_t * len,
                                                  CBType cb)
  {
    e.check();

    this->streamStorage->setOffset(*len + this->streamStorage->getOffset());

    if (this->streamStorage->spaceRemaining() == 0)
      this->streamStorage->reset();

    cb(*(new CBException()));
  }

  void ZeroCopyTransformInput::readTransformStorage (const void ** out,
                                                     size_t * len,
                                                     const CBType & cb,
                                                     size_t suggested)
  {
    Handle ret;
    /* Create a new callback for when the stream read is compleated */
    CBType newCB;

    ZeroCopyMemoryInput * x;

    newCB = boost::bind(&ZeroCopyTransformInput::transformStorageCB,
                        boost::ref(*this), _1, len, cb);

    /* If there is any transformed input not read return unread transformed input */  
    if (this->transformStorage->getTotalLen() > 0)
    {
      x = this->convertToMemoryInput(this->transformStorage);

      ret = x->read( out, len, newCB, suggested);
    }
  }

  void ZeroCopyTransformInput::readStreamStorage (const void ** out,
                                                  size_t * len,
                                                  const CBType & cb,
                                                  size_t suggested)
  {
    Handle ret;
    /* Create a new callback for when the stream read is compleated */
    CBType newCB;

    ZeroCopyMemoryInput * x;

    newCB = boost::bind(&ZeroCopyTransformInput::streamStorageCB,
                        boost::ref(*this), _1, len, cb);

    /* If there is any transformed input not read return unread transformed input */  
    if (this->streamStorage->getTotalLen() > 0)
    {
      x = this->convertToMemoryInput(this->streamStorage);

      ret = x->read( out, len, newCB, suggested);
    }
  }

  /**
   * Read from the input stream and preform the transform from 
   * transformed input -> output.
   * @param[out] out       Pointer to location where output is going to be 
   *                       stored.
   * @param[out] len       size of out (actual)
   * @param[in]  cb        Callback to call when operation is complete 
   * @param[in]  suggested Size the client would like to see returned when 
   *                       read operation is completed. This is a best
   *                       effort and the actual size may vary from 
   *                       suggested.
   */
  Handle ZeroCopyTransformInput::read (const void ** out, size_t * len, 
                                       const CBType & cb, size_t suggested)
  {
    ZeroCopyInputWU * wu;

    /* Check to see if any non-read transformed data is availible, if so return 
       it */
    if (this->transformStorage->getTotalLen() > 0)
    {
      this->readTransformStorage(out, len, cb, suggested);
      return (Handle) 0;
    }
    
    /* if no transformed data is availible, create a callback to transform data
       after the underlying stream is compleated reading */
    wu = new ZeroCopyInputWU( out, len, cb, suggested);
    
    /* read the underlying stream */
    return this->readStream(wu);
  }

  void ZeroCopyTransformInput::nullCB (CBException e)
  {
    e.check();
  }
  void ZeroCopyTransformInput::doTransform (ZeroCopyInputWU * wu)
  {
    Handle ret;
    void * transStorage;
    size_t transSize = 0;
    size_t transOut = 0;
    bool flushFlag = false;
    int outState = 0;

    /* Create a null callback for reading of transformStorage (non blocking) */
    CBType nullCB = boost::bind(&ZeroCopyTransformInput::nullCB,
                                boost::ref(*this), _1);

    /* Get write pointer for transform storage */
    this->transformStorage->write(&transStorage, &transSize, nullCB, 
                                  wu->suggested);

    /* preform the transform */
    this->transform->transform ((const void *const) wu->streamData, wu->streamLen,
                                transStorage, transSize, &transOut,
                                &outState, flushFlag);
    
    /* Create a new callback to preform the read from the transform stream */
    CBType readCB = boost::bind(&ZeroCopyTransformInput::readTransformStorage,
                                boost::ref(*this), wu->ptr, wu->size, wu->cb, 
                                wu->suggested);
    
    /* Return any unused portions from the stream */
    ret = this->transformStorage->rewindOutput(transSize - transOut, readCB);
  }

  void ZeroCopyTransformInput::transformationState (CBException e, ZeroCopyInputWU * wu)
  {
    /* Check the exception */
    e.check();

    /* Create the function bindings for the transformation queue (this function 
       is blocking) */
    boundFunc f = boost::bind(&ZeroCopyTransformInput::doTransform,
                              boost::ref(*this), wu); 
    
    /* Add it to the process queue */
    this->workqueue.push_back(f);
  }

  /* Read the stream and write the results into this->streamStorage (wu->streamData,
    wu->streamLen) */
  Handle ZeroCopyTransformInput::readStream (ZeroCopyInputWU * wu)
  {
    /* Create a new callback for when the stream read is compleated */
    CBType newCB = boost::bind(&ZeroCopyTransformInput::transformationState,
                                boost::ref(*this), _1, wu);

    return this->stream->read((const void **) &wu->streamData, &wu->streamLen, 
                              newCB, wu->suggested);
  }

  /**
   * Return undigested input back to the stream 
   * @param[in] len        Size of data that is being returned to the stream
   * @param[in] cb         Callback when operation is complete.
   */
  Handle ZeroCopyTransformInput::rewindInput (size_t len, const CBType & cb)
  {
    this->transformStorage->setOffset(this->transformStorage->getOffset() - 
                                      len);
    cb(*(new CBException()));
    return (Handle) 0;
  }
  /**
   * Converts ZeroCopyMemoryOutput -> ZeroCopyMemoryInput. This is
   * used for changing the direction of the internal ZeroCopyMemory stream 
   * from write mode (output) to read mode (input) for the memory region 
   * stored in ZeroCopyMemoryOutput.
   */
  ZeroCopyMemoryInput * ZeroCopyTransformInput::convertToMemoryInput(
                                                     ZeroCopyMemoryOutput * x)
  {
    void * ptr = x->getMemPtr();
    size_t offset = x->getOffset();
    ZeroCopyMemoryInput * v = new ZeroCopyMemoryInput (ptr, x->getTotalLen());
    v->setOffset(offset);

    return v;
  }

}

