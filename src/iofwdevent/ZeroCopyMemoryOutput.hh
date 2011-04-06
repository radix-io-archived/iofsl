#ifndef IOFWDEVENT_ZEROCOPYMEMORYOUTPUT
#define IOFWDEVENT_ZEROCOPYMEMORYOUTPUT

#include "ZeroCopyOutputStream.hh"

namespace iofwdevent
{
   //========================================================================

   /**
    * @class ZeroCopyMemoryOutput
    *
    * @brief Zero copy output stream implementation for creating a stream from
    *              a region in memory.
    *
    * This class allows for a ZeroCopyOutputStream to be created using a region
    * of memory. This allows for this region of memory to be accessed using
    * the ZeroCopyOutputStream interfacing style.
    *
    */
   class ZeroCopyMemoryOutput : public ZeroCopyOutputStream
   {
      public:

         ZeroCopyMemoryOutput ();

         /* Constructor for InputMemoryZeroCopy. */
         ZeroCopyMemoryOutput  (void * buf, size_t bufsize);

         /* Cancel operation (not used since this class does not block) */
         void cancel (Handle h);

         /* Returns pointer to memory area where a write can take place */
         Handle write (void ** ptr, size_t * memsize, const CBType & cb, 
               size_t suggested);

         /* Rewinds the write stream */
         Handle rewindOutput (size_t amount, const CBType & cb);

         /* Flush internal buffers, if any */
         Handle flush (const CBType & cb);

         // --- extra methods ---

         /* Amount of space remaining in this->mem */
         size_t spaceRemaining () const;

         /* Return the initial starting pointer for memory (used in conversion 
            between ZeroCopyMemoryOutput -> ZeroCopyMemoryInput ) */
         void * getMemPtr ();

         /* Gets the total length of the memory location mem */
         size_t getBufferSize () const;

         /// Return the number of bytes used
         size_t getBufferUsed () const;

         /* reset the stream position back to 0 */
         void reset ();

         /* Reset the stream position to zero and to a new buffer and size
          */
         void reset (void * buf, size_t size);
         void close (const iofwdevent::CBType & cb) {}
      protected:
         char * mem_;         /*< Stores memory location data */
         size_t memSize_;     /*< Stores the size of the memory location */
         size_t pos_;         /*< Stores current pointer position inside mem */
   };

   //========================================================================
}
#endif

