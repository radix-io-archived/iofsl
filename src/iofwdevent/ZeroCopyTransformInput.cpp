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
    if (bufSize == 0)
      bufSize = 4194304;
    /* Stores transformed data */
    streamStorage = new ZeroCopyMemoryInput (NULL,0);
    buff = (void *)new char[bufSize];
    buffSize = bufSize;
    stream = in;
    this->transform = transform;
    inStreamData = NULL;
    inStreamSize = 0;
    flushFlag = false;
  }

  ZeroCopyTransformInput::~ZeroCopyTransformInput ()
  {
//      delete[] (char*)buff;
//      delete streamStorage;
//      delete stream;
//      delete transform;
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
    if (streamStorage->spaceRemaining() > 0)
    {
      readTransformStorage(out, len, cb, suggested);
      return (Handle) 0;
    }

    /* If there is still unconsumed input, go directly to transformation 
       state */
    if (inStreamData != NULL || flushFlag == true)
    {
      doTransform (out, len, cb, suggested);
      return (Handle) 0;
    }
  
    /* If nether of the above cases hold, read the underlying stream */
    return readStream( out, len, cb, suggested);
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

    /* If there is any transformed input not read return unread transformed input */  
    if (streamStorage->spaceRemaining() > 0)
    {
      ret = streamStorage->read( out, len, cb, suggested);
    }
    else
    {
      *len = 0;
      cb(CBException());
    }
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
                        newCB, suggested);
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

    inStreamData =  *writeLoc;
    inStreamSize = *writeSize;
    if (inStreamSize == 0)
      flushFlag = true;

    delete[] writeLoc;
    delete[] writeSize;

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
      size_t data_size = 0;
      int outState = 0;

      transform->transform ((const void *const) inStreamData, inStreamSize,
                            buff, buffSize, &data_size,
                            &outState, flushFlag);

      streamStorage->reset(buff, data_size);

      if (outState == SUPPLY_INBUF)
      {
         inStreamData = NULL;
         inStreamSize = 0;
      }
//      else if (outState == CONSUME_OUTBUF)
//      {
//      }

      readTransformStorage(out, len, cb, suggested);
   }

  /**
   * Return undigested input back to the stream 
   * @param[in] len        Size of data that is being returned to the stream
   * @param[in] cb         Callback when operation is complete.
   */
  Handle ZeroCopyTransformInput::rewindInput (size_t len, const CBType & cb)
  {
    return streamStorage->rewindInput(len,cb);
  }
}

