#include "ZeroCopyMemoryOutput.hh"

#include "iofwdutil/assert.hh"

namespace iofwdevent
{
   //========================================================================

   void ZeroCopyMemoryOutput::cancel (Handle )
   {
   }

   ZeroCopyMemoryOutput::ZeroCopyMemoryOutput ()
      : mem_ (0),
        memSize_ (0),
        pos_ (0)
   {
      type = 'M';
   }

   /**
    * Constructor for ZeroCopyMemoryOutput.
    * @param[in] in        Memory region to serve as the output location for the
    *                      stream (this memory must be allocated externally!)
    * @param[in] len       Length of the memory region void * out.
    */
   ZeroCopyMemoryOutput::ZeroCopyMemoryOutput (void * output, size_t len)
      : mem_ (static_cast<char*>(output)),
        memSize_ (len),
        pos_ (0)
   {
   }

   void ZeroCopyMemoryOutput::reset (void * ptr, size_t size)
   {
      mem_ = static_cast<char*>(ptr);
      memSize_ = size;
      reset ();
   }

   void ZeroCopyMemoryOutput::reset()
   {
      pos_ = 0;
   }

   void * ZeroCopyMemoryOutput::getMemPtr()
   {
      return mem_;
   }

   size_t ZeroCopyMemoryOutput::getBufferSize () const
   {
      return memSize_;
   }

   size_t ZeroCopyMemoryOutput::getBufferUsed () const
   {
      return getBufferSize () - spaceRemaining ();
   }

   /**
    * Write to the stream returning a pointer to the region of memory where 
    * the data can be written to. 
    * @param[out] ptr      Location where a pointer containing a region of memory
    *                      can be written to is stored. This pointer is where you
    *                      can safely write data. 
    * @param[out] size     Number of bytes that can be written to ptr.
    * @param[in]  cb       Callback function to be used when operation is completed
    * @param[in] suggested Client specified suggested write size. This write size 
    *                      is best effort and the actual write size may be more or
    *                      less then the number specified by this parameter 
    * @return              Returns a handle relating to this particular write. 
    *                      (not used with ZeroCopyMemoryOutput due to no blocking
    *                      in this class)
    */
   Handle ZeroCopyMemoryOutput::write (void ** ptr, size_t * size,
         const CBType & cb, size_t suggested)
   {
      ALWAYS_ASSERT(mem_);

      // When suggested is 0, take the whole buffer
      const size_t have = std::min (spaceRemaining (), suggested);

      *size = have;
      *ptr = mem_ + pos_;
      pos_ += have;

      /* Call the callback */
      if (cb)
         cb (CBException());

      /* Do not return a handle with this call (non-blocking) */
      return Handle ();
   }

   /**
    * Allows for the stream to be "rewinded" and unused portions of a write to be
    * returned to ZeroCopyMemoryOutput. 
    * @param[in] size      How much to rewind.
    * @param[in] cb        Callback to use when the operation is completed (called 
    *                      immedietly).
    * @return              Returns a handle relating to this particular rewind. 
    *                      (not used with ZeroCopyMemoryOutput due to no blocking
    *                      in this class)
    */
   Handle ZeroCopyMemoryOutput::rewindOutput (size_t size, const CBType & cb)
   {
      ALWAYS_ASSERT (pos_ >= size);
      pos_ -= size;

      cb (CBException ());

      return Handle ();
   }

   /**
    * Returns the amount of buffer remaining for the write
    * @return              Value containing remaining space left in buffer 
    *                      (this->mem)
    */
   size_t ZeroCopyMemoryOutput::spaceRemaining () const
   {
      ASSERT (memSize_ >= pos_);
      return memSize_ - pos_;
   }

   /**
    * Flush the internal buffer;
    *
    * Nothing needs to be done here.
    *
    * @param[in] cb        Callback to use when the operation is completed (called 
    *                      immedietly).
    * @return              Returns a handle relating to this particular flush. 
    *                      (not used with ZeroCopyMemoryOutput due to no 
    *                      blocking in this class)
    */
   Handle ZeroCopyMemoryOutput::flush (const CBType & cb)
   {
      cb (CBException ());
      return Handle ();
   }

   //========================================================================
}
