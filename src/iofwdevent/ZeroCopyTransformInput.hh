#include "ZeroCopyInputStream.hh"
#include "ZeroCopyMemoryInput.hh"

#include "iofwdutil/transform/GenericTransform.hh"

#include <boost/scoped_ptr.hpp>

#include <boost/scoped_array.hpp>

namespace iofwdevent 
{
   /**
    * Creates an input zero copy stream from a memory region 
    *
    * @TODO: Need to define semantics: does this class own the transform and
    * the input stream? If so, should be using scoped_ptr to avoid memory
    * leaks.
    *
    * If not: should be documented
    */
   /**
    * @class ZeroCopyTransformInput
    * 
    * @brief Zero copy input stream implementation for creating a stream
    *        from a transformed input (ex: compressed input).
    *
    * This class allows for a ZeroCopyInputStream to be created using a compressed
    * ZeroCopyMemoryInput stream. 
    *
    */
   class ZeroCopyTransformInput : public ZeroCopyInputStream
   {
      public:
         /**
          * Constructor for ZeroCopyTransformInput.
          * @param[in] in        Transformed input stream which needs to be 
          *                      transformed back into its original stream.
          */
         ZeroCopyTransformInput  (ZeroCopyInputStream * in,
               iofwdutil::transform::GenericTransform * transform,
               size_t bufSize = 4*1024*1024);

         /**
          * Cancel the transformation operation specified by Handle x.
          * @param[in] x         Handle of operation to stop
          */
         void cancel (Handle );

         /**
          * Read from the input stream and preform the transform from 
          * transformed input -> output.
          * @param[out] out       Pointer to location where output is going to
          *                       be stored.
          * @param[out] len       size of out (actual)
          * @param[in]  cb        Callback to call when operation is complete 
          * @param[in]  suggested Size the client would like to see returned when 
          *                       read operation is completed. This is a best
          *                       effort and the actual size may vary from 
          *                       suggested.
          */
         Handle read (const void **, size_t * , const CBType &, size_t );

         /**
          * Return undigested input back to the stream 
          * @param[in] len        Size of data that is being returned to the stream
          * @param[in] cb         Callback when operation is complete.
          */
         Handle rewindInput (size_t , const CBType & );

         void close (const CBType & cb);

      protected:

         /* Read the transform storage stream */
         void readTransformStorage (const void **, size_t *, const CBType &, 
               size_t);

         /* Perform the transformation */
         void doTransform (const void **, size_t *, const CBType &, 
               size_t);


         Handle readStream (const void **, size_t *, const CBType &,  size_t);

         void transformationState (CBException, const void ** , size_t * ,
               const CBType & , size_t);

      protected:
         boost::scoped_ptr<ZeroCopyInputStream>  stream_; /*< Stores input stream from which to transform */
         boost::scoped_ptr<iofwdutil::transform::GenericTransform> transform_; /*< Stores transform information */
         const size_t buffSize_;     /*< the internal buffer size (maximum) */
         boost::scoped_array<char> buff_;         /*< used by stream storeage as its internal buffer */
         ZeroCopyMemoryInput streamStorage_;

         const void * inStreamData_; /*< Memory pointer to internal stream read (stream) */
         size_t inStreamSize_; /*< Input size */
         bool flushFlag_;

         const void * writeLoc_;
         size_t writeSize_;
   };

   //========================================================================
}


