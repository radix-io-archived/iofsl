#include "ZeroCopyMemoryInput.hh"

#include "iofwdutil/assert.hh"

namespace iofwdevent
{
   //========================================================================

   ZeroCopyMemoryInput::ZeroCopyMemoryInput ()
      : mem_ (0),
        memSize_ (0),
        pos_ (0)
   {
   }

   /**
    * Constructor for ZeroCopyMemoryInput.
    * @param[in] in        Region of memory (with data) to convert into an 
    *                      input stream
    * @param[in] len       Length of the memory region void * in.
    */
   ZeroCopyMemoryInput::ZeroCopyMemoryInput (const void * in, size_t len)
      : mem_ (static_cast<const char *>(in)),
        memSize_ (len),
        pos_ (0)
   {
   }

   size_t ZeroCopyMemoryInput::getBufferSize () const
   {
      return memSize_;
   }

   const void * ZeroCopyMemoryInput::getMemPtr() const
   {
      return mem_;
   }

   size_t ZeroCopyMemoryInput::getBufferUsed () const
   {
      return pos_;
   }

   /**
    * Read's from the stream returning a pointer to the region of memory where 
    * the data is stored. 
    * @param[out] ptr      void ** pointer containing the address to the data 
    *                      that was read
    * @param[out] size     Number of bytes that were read.
    * @param[in]  cb        Callback function to be used when operation is 
    *                      completed
    * @param[in]  suggested Client specified suggested read size. This read size 
    *                      is best effort and the actual read may be more or less
    *                      then the value specified in suggested.
    * @return              Returns a handle relating to this particular read. 
    *                      (not used with ZeroCopyMemoryInput due to no blocking 
    *                      in this class)         
    */
   Handle ZeroCopyMemoryInput::read (const void ** ptr, size_t * size, 
         const CBType & cb, size_t suggested)
   {
      const size_t have = std::min (suggested, spaceRemaining ());

      *size = have;
      *ptr = mem_ + pos_;
      pos_ += have;

      /* Call the callback */
      cb (CBException());

      /* Do not return a handle with this call (non-blocking) */
      return Handle ();
   }

   /**
    * Returns the amount of buffer remaining that has not been read 
    * @return              Value containing remaining space left in buffer 
    *                      (this->mem)
    */
   size_t ZeroCopyMemoryInput::spaceRemaining () const
   {
      return memSize_ - pos_;
   }


   /**
    * Resets the ZeroCopyMemoryInput stream and sets it to a new memory 
    * location
    * @param[in] in        Region of memory (with data) to convert into an 
    *                      input stream
    * @param[in] len       Length of the memory region void * in.
    * @return              Status of the operation ( > 0 = success)
    */
   void ZeroCopyMemoryInput::reset (const void * in, size_t len)
   {
      mem_ = static_cast<const char *>(in);
      pos_ = 0;
      memSize_ = len;
   }


   /**
    * Allows for the stream to be "rewinded" and unused portions of a read to be
    * returned to ZeroCopyMemoryInput. This prevents data being skipped on 
    * subsequent reads.
    * @param[in] size      How much to rewind.
    * @param[in] cb        Callback to use when the operation is completed (called 
    *                      immedietly).
    * @return              Returns a handle relating to this particular rewind. 
    *                      (not used with ZeroCopyMemoryInput due to no blocking 
    *                      in this class)         
    */
   Handle ZeroCopyMemoryInput::rewindInput (size_t amount, const CBType & cb)
   {
      ALWAYS_ASSERT(pos_ >= amount);
      pos_ -= amount;

      cb (CBException ());

      return Handle ();
   }

   //========================================================================
}
