#include "ZeroCopyTransformInput.hh"
#include <iostream>
using namespace std;
namespace iofwdevent {
  /**
   * Constructor for ZeroCopyTransformInput.
   * @param[in] in        Transformed input stream which needs to be 
   *                      transformed back into its original stream.
   * @param[in] transform Transform to use on input data (in)
   * @param[in] bufSize   Buffer size for the internal streams (0 = Default)
   */
  ZeroCopyTransformInput::ZeroCopyTransformInput  (ZeroCopyInputStream * in, 
                                                   GenericTransform * transform,
                                                   size_t bufSize) 
  {
    char * transformInternal;
    tempInputStream = new ZeroCopyMemoryInput(NULL,0);
    streamStorage = new ZeroCopyMemoryInput(NULL,0);
    if (bufSize > 0)
    {
      transformInternal = new char[bufSize];
      this->transformStorage = new ZeroCopyMemoryOutput((void *) transformInternal, bufSize);
    }
    else
    {
      transformInternal = new char[1000];
      this->transformStorage = new ZeroCopyMemoryOutput((void *) transformInternal, 1000);
    }
    this->stream = in;
    this->transform = transform;
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
    /* Check to see if any non-read transformed data is availible, if so 
       return it */
    if (transformStorage->getOffset() > 0)
    {
      readTransformStorage(out, len, cb, suggested);
      return (Handle) 0;
    }

    /* If there is still unconsumed input, go directly to transformation 
       state */
    if (streamStorage->getTotalLen() > 0)
    {
      doTransform (out, len, cb, suggested);
      return (Handle) 0;
    }
  
    /* If nether of the above cases hold, read the underlying stream */
    return this->readStream( out, len, cb, suggested);
  }
  
 /**
   * Reads the internal Transform Storage stream.
   * @param[out] out       Pointer to location where output is going to be 
   *                       stored.
   * @param[out] len       size of out (actual)
   * @param[in]  cb        Callback to call when operation is complete 
   * @param[in]  suggested Size the client would like to see returned when 
   *                       read operation is completed. This is a best
   *                       effort and the actual size may vary from 
   *                       suggested.
   */
  void ZeroCopyTransformInput::readTransformStorage (const void ** out,
                                                     size_t * len,
                                                     const CBType & cb,
                                                     size_t suggested)
  {
    Handle ret;
    /* Create a new callback for when the stream read is compleated */
    CBType newCB;

    newCB = boost::bind(&ZeroCopyTransformInput::transformStorageCB,
                        boost::ref(*this), _1, cb);

    /* If there is any transformed input not read return unread transformed input */  
    if (this->transformStorage->getOffset() > 0)
    {
      tempInputStream->convertToInput(this->transformStorage);

      ret = tempInputStream->read( out, len, newCB, suggested);
    }
  }
  
  /**
   * Callback return from Transform Storage read
   * @param[in] e    Callback exception class
   * @param[in] len  Length of the read 
   * @param[in] cb   Callback to use to signal that task is completed 
   */
  void ZeroCopyTransformInput::transformStorageCB(CBException e, CBType cb)
  {
    e.check();

    transformStorage->convertToOutput(tempInputStream);

    if (this->transformStorage->spaceRemaining() == 0)
      this->transformStorage->reset();

    cb(*(new CBException()));
  }

  /**
   * Read the stream and write the results into streamStorage 
   * @param[out] out       Pointer to location where output is going to be 
   *                       stored.
   * @param[out] len       size of out (actual)
   * @param[in]  cb        Callback to call when operation is complete 
   * @param[in]  suggested Size the client would like to see returned when 
   *                       read operation is completed. This is a best
   *                       effort and the actual size may vary from 
   *                       suggested.
   */
  Handle ZeroCopyTransformInput::readStream  (const void ** out,
                                              size_t * len,
                                              const CBType & cb,
                                              size_t suggested)
  {
    Handle x;  
    void ** writeLoc = new void * [1];
    size_t * writeSize = new size_t[1];

    /* Create a new callback for when the stream read is compleated */
    CBType newCB = boost::bind(&ZeroCopyTransformInput::transformationState,
                                boost::ref(*this), _1, out, len, cb, suggested,
                                writeSize, writeLoc);

    x = stream->read((const void **) writeLoc, writeSize,
                        newCB, (size_t)(*writeSize));
    return x;
  }

  void ZeroCopyTransformInput::nullCB (CBException e)
  {
    e.check();
  }

  /**
   * Prepares the streamStorage stream for transformation in doTransform
   * @param[out] out       Pointer to location where output is going to be 
   *                       stored.
   * @param[out] len       size of out (actual)
   * @param[in]  cb        Callback to call when operation is complete 
   * @param[in]  suggested Size the client would like to see returned when 
   *                       read operation is completed. This is a best
   *                       effort and the actual size may vary from 
   *                       suggested.
   * @param[in]  writeLoc  Location of where the data read from stream is 
   *                       being stored 
   * @param[in]  writeSize Size of the data read from the stream 
   */
  void ZeroCopyTransformInput::transformationState (CBException e,
                                                    const void ** out,
                                                    size_t * len,
                                                    const CBType & cb,
                                                    size_t suggested,
                                                    size_t * writeSize,
                                                    void ** writeLoc)
  {
    e.check();
    CBType nullCB = boost::bind(&ZeroCopyTransformInput::nullCB, 
                                boost::ref(*this), _1);
      
    streamStorage->reset(*writeLoc,*writeSize);

    doTransform ( out, len, cb, suggested);
  }

  /**
   * Preform the transform between input and output 
   * @param[out] out       Pointer to location where output is going to be 
   *                       stored.
   * @param[out] len       size of out (actual)
   * @param[in]  cb        Callback to call when operation is complete 
   * @param[in]  suggested Size the client would like to see returned when 
   *                       read operation is completed. This is a best
   *                       effort and the actual size may vary from 
   *                       suggested.
   */
  void ZeroCopyTransformInput::doTransform (const void ** out,
                                            size_t * len,
                                            const CBType & cb,
                                            size_t suggested)
  {
    Handle ret;
    void ** transStorage = new void * [1];
    size_t transSize = 0;
    size_t transOut = 0;
    bool flushFlag = false;
    int outState = 0;
    void * streamData = streamStorage->getMemPtr();
    size_t streamLen  = streamStorage->getTotalLen();
    if (streamLen == 0)
    {
    	flushFlag = true;
    }
    /* Create a null callback for reading of transformStorage (non blocking) */
    CBType nullCB = boost::bind(&ZeroCopyTransformInput::nullCB,
                                boost::ref(*this), _1);

    /* Get write pointer for transform storage */
    transformStorage->write(transStorage, &transSize, nullCB, suggested);


    /* preform the transform */
    transform->transform ((const void *const) streamData, streamLen,
                          *transStorage, transSize, &transOut,
                          &outState, flushFlag);

    ret = transformStorage->rewindOutput(transSize - transOut, nullCB);

    transformStorage->flush(nullCB);

    /* If we are out of data to trasnform, read more of the stream */
    if (outState == SUPPLY_INBUF)
    {
      streamStorage->reset(NULL, 0);
      ret = readStream( out, len, cb, suggested);
    }
    /* if the output is full return */
    else if (outState == CONSUME_OUTBUF)
    {
      readTransformStorage(out, len, cb, suggested);
    }
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
}

