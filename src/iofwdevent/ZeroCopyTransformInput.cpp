#include "ZeroCopyTransformInput.hh"

#include "iofwdevent/NOPCompletion.hh"

#include <cstdio>
namespace iofwdevent
{
   //========================================================================

   /**
    * Constructor for ZeroCopyTransformInput.
    * @param[in] in        Transformed input stream which needs to be 
    *                      transformed back into its original stream.
    * @param[in] transform Transform to use on input data (in)
    * @param[in] bufSize   Buffer size for the internal streams (0 = Default)
    *
    * Ownership of in & transform changes to this class.
    */
   ZeroCopyTransformInput::ZeroCopyTransformInput  (ZeroCopyInputStream * in,
         iofwdutil::transform::GenericTransform * transform, size_t bufSize)
      : stream_ (in),
        transform_ (transform),
        buffSize_ (bufSize ? bufSize : 4*1024*1024),
        buff_ (new char[buffSize_]),
        streamStorage_ (buff_.get(), 0),
        inStreamData_ (0),
        inStreamSize_ (0),
        flushFlag_ (false)
   {
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
      if ( streamStorage_.spaceRemaining() > 0)
      {
         readTransformStorage(out, len, cb, suggested);
         return Handle();
      }

      /* If there is still unconsumed input, go directly to transformation 
         state */
      if (inStreamData_ != NULL || flushFlag_ == true)
      {
         doTransform (out, len, cb, suggested);
         return Handle();
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
      /* Create a new callback for when the stream read is compleated */

      /* If there is any transformed input not read return unread transformed input */  
      if (streamStorage_.spaceRemaining() > 0)
      {
         streamStorage_.read( out, len, cb, suggested);
      }
      else
      {
         read (out, len, cb, suggested);
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
      /* Create a new callback for when the stream read is compleated */
      CBType newCB = boost::bind(&ZeroCopyTransformInput::transformationState,
            boost::ref(*this), _1, out, len, cb, suggested);
//      fprintf(stderr, "Reading Stream\n");
      return stream_->read(&writeLoc_, &writeSize_, newCB, suggested);
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
    * writeLoc_  Location of where the data read from stream is 
    *                      being stored 
    * writeSize_ Size of the data read from the stream 
    *
    * This function is called by the underlying input stream when new data is
    * available.
    */
   void ZeroCopyTransformInput::transformationState (CBException e,
         const void ** out,
         size_t * len,
         const CBType & cb,
         size_t suggested)
   {
      e.check();

      inStreamData_ =  writeLoc_;
      inStreamSize_ = writeSize_;

      if (inStreamSize_ == 0)
         flushFlag_ = true;

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
//      fprintf (stderr, "Performing Transform\n");
      size_t data_size = 0;
      int outState = 0;

      transform_->transform (inStreamData_, inStreamSize_,
            buff_.get(), buffSize_, &data_size,
            &outState, flushFlag_);

      streamStorage_.reset(buff_.get(), data_size);

      if (outState == iofwdutil::transform::SUPPLY_INBUF)
      {
         inStreamData_ = NULL;
         inStreamSize_ = 0;
      }

      readTransformStorage(out, len, cb, suggested);
   }

   /**
    * Return undigested input back to the stream 
    * @param[in] len        Size of data that is being returned to the stream
    * @param[in] cb         Callback when operation is complete.
    */
   Handle ZeroCopyTransformInput::rewindInput (size_t len, const CBType & cb)
   {
      return streamStorage_.rewindInput (len,cb);
   }

   void ZeroCopyTransformInput::close (const CBType & cb)
   {
      cb (CBException ());
   }

   //========================================================================
}

