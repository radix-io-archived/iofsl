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
    type = 'T';
    char * internalMemBuf;
    if (len > 0)
    {
      internalMemBuf = new char[len];
    }
    else
    {
      internalMemBuf = new char[4194304];
      len = 4194304;
    }
    internalBuf = new ZeroCopyMemoryOutput ((void *)internalMemBuf, len); 
    outStream = out;
    this->transform = transform;
    flushFlag = false;
    dataWritten = false;
    intMemPtr = NULL;
    intMemSize = 0;
  }

  ZeroCopyTransformOutput::~ZeroCopyTransformOutput ()
  {
      delete internalBuf;
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
    /* After a flush condition has been triggered with transform, there is no 
       going back. Flush condition is triggered when flush is called with no 
       additional write call */
    assert ( flushFlag != true );

    dataWritten = true;
    /* Create a null callback for reading of transformStorage (non blocking) */
    CBType nullCB = boost::bind(&ZeroCopyTransformOutput::nullCB,
                                boost::ref(*this), _1);

    /* Check the Internal Memory Buffer. If there is any free space return 
       the free space in the buffer. */    
    if (internalBuf->spaceRemaining() > 0 && intMemPtr == NULL)
      return internalBuf->write(ptr, size, cb, suggested);
    else if (intMemPtr != NULL)
       /* If there is no space remaining, flush internal buffer and proceed 
          to getting the memory location for the output stream (this->outStream) 
       */
       internalBuf->flush(nullCB);

    return getWriteLoc (ptr, size, cb, suggested);
  }

  /** 
   * Makes write call to output stream (this is a blocking call) this function 
   * sets up a call back to transformState when this operation is complete.
   *
   * @param ptr           Pointer to the transformed output data
   * @param size          Pointer to the size of data in ptr 
   * @param cb            Callback
   * @param suggested     Suggested size of the write 
   */
  Handle ZeroCopyTransformOutput::getWriteLoc (void ** ptr, size_t * size, 
                                          const CBType & cb, size_t suggested)
  {
    /* Create Pointers to store write ptr location for outpur stream */
    void ** writeLoc = new void * [1];
    size_t * writeSize = new size_t[1];
    
    /* Set up callback for transformation state */
    CBType transCB = boost::bind(&ZeroCopyTransformOutput::transformState,
                                 boost::ref(*this), _1, ptr, size, cb, 
                                 suggested, writeLoc, writeSize);

    /* Run blocking write call to output stream */
    return outStream->write(writeLoc, writeSize, transCB, suggested);
  }
  
  /** 
   * Transforms user write then write the resulting data to the output 
   * stream.
   *
   * @param ptr           Pointer to the transformed output data
   * @param size          Pointer to the size of data in ptr 
   * @param cb            Callback
   * @param suggested     Suggested size of the write 
   * @param writeLoc      Location of the output stream which can be written
   * @param writeSize     Size availible for writing to the output stream.
   */
  void ZeroCopyTransformOutput::transformState (CBException e,
                                                void ** ptr, 
                                                size_t * size, 
                                                const CBType & cb, 
                                                size_t suggested,
                                                void ** writeLoc,
                                                size_t * writeSize)
  {
    e.check();
    int outState = 0;
    Handle x;
    /* Preform the transform */  
    outState = doTransform (writeLoc, writeSize, false);

    /* If we are out of input buffer, reset internal buffer and return the 
       write pointer to the user */
    if (outState == SUPPLY_INBUF)
    {
      intMemSize = 0;
      intMemPtr = NULL;
      internalBuf->reset();
      x = internalBuf->write(ptr, size, cb, suggested);
    } 
    else if (outState == CONSUME_OUTBUF)
    /* If we are out of output buffer, repeat this run */
    {
      SingleCompletion block;
      block.reset();
      outStream->flush(block);
      block.wait();
     getWriteLoc(ptr, size, cb, suggested);
    }
    
    /* We should never get here, transformation should not be signaled as done
       at this location */
    assert (outState != 0);
  }


//  int ZeroCopyTransformOutput::flushStream ()
//  {
//    CBType transCB = boost::bind(&ZeroCopyTransformOutput::flushTransform,
//                                 boost::ref(*this), cb, writeLoc, writeSize);

  /**
   * Preform the transform on input data to output data 
   * 
   * @param writeLoc      Location to write transformed data
   * @param writeSize     Size of the write location 
   * @param flushFlag     Flush flag for the stream 
   */
  int ZeroCopyTransformOutput::doTransform ( void ** writeLoc, 
                                              size_t * writeSize,
                                              bool flushFlag)
  {
    SingleCompletion block;
    Handle ret;
    size_t transOut = 0;
    int outState = 0;

    /* Get the memory locations for the internal buffer */
    if (intMemPtr == NULL)
    {
      intMemSize = internalBuf->getBufferUsed();
      intMemPtr  = internalBuf->getMemPtr();
    }

    /* Create a null callback for reading of transformStorage (non blocking) */

    /* preform the transform */
    transform->transform ((const void *const) intMemPtr, intMemSize,
                          *writeLoc, *writeSize, &transOut,
                          &outState, flushFlag);

    /* return unused output stream space */
    if (transOut <= *writeSize)
    {   
       block.reset();
       ret = outStream->rewindOutput(*writeSize - transOut, block);
       block.wait();
    }
    delete[] writeLoc;
    delete[] writeSize;

    return outState;
  }

  /**
   * Flush the internal buffers, if any 
   *  
   * @param cb        Callback for function
   */
  Handle ZeroCopyTransformOutput::flush (const CBType & cb)
  {
    void ** writeLoc = new void * [1];
    size_t * writeSize = new size_t[1];
    if (intMemPtr == NULL)
    {
      /* Create a null callback for flushing the internal memory stream */
      CBType nullCB = boost::bind(&ZeroCopyTransformOutput::nullCB,
                                   boost::ref(*this), _1);

       /* Flush the internal buffer */
       internalBuf->flush(nullCB);
    }
    size_t streamLen = internalBuf->getBufferUsed();
    /* Set up callback for transformation state */
    CBType transCB = boost::bind(&ZeroCopyTransformOutput::flushTransform,
                                 boost::ref(*this), cb, writeLoc, writeSize);
  
    return outStream->write(writeLoc, writeSize, transCB, streamLen);

  }

  /**
   * Handles getting write pointer from output stream 
   * 
   * @param cb       User Callback
   */
  void ZeroCopyTransformOutput::flushTransform (const CBType & cb,
                                                void ** writeLoc, 
                                                size_t * writeSize)
  {
    int outState = 0;

    if (dataWritten == true)
       outState = doTransform ( writeLoc, writeSize, flushFlag);
    else
       outState = TRANSFORM_DONE;

    if (outState == CONSUME_OUTBUF || outState == TRANSFORM_DONE)
      outStream->flush(cb);
    else if (flushFlag == true)
      flush(cb);
    else
    {
      intMemSize = 0;
      intMemPtr = NULL;
      internalBuf->reset();
      cb(CBException());
    }
  }

  /* Close out the stream */
  void ZeroCopyTransformOutput::close(const CBType & cb)
  {
    flushFlag = true;
    flush(cb);
  }

  /**
   * Rewinds the write stream 
   *
   * @param size    Size of the rewind 
   * @param cb      Callback
   */
  Handle ZeroCopyTransformOutput::rewindOutput (size_t size, 
                                                const CBType & cb)
  {
    return internalBuf->rewindOutput(size, cb);
  }


}
