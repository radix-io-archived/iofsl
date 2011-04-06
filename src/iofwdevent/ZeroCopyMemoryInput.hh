#ifndef SRC_IOFWDEVENT_ZEROCOPYMEMORYINPUT
#define SRC_IOFWDEVENT_ZEROCOPYMEMORYINPUT

#include "ZeroCopyInputStream.hh"

namespace iofwdevent
{
   //========================================================================

   /**
    * @class ZeroCopyMemoryInput
    * 
    * @brief Zero copy input stream implementation for creating a stream from a
    *        region in memory.
    *
    * This class allows for a ZeroCopyInputStream to be created using a region
    * of memory. This allows for this region of memory to be accessed using
    * the ZeroCopyInputStream interfacing style.
    *
    */
   class ZeroCopyMemoryInput : public ZeroCopyInputStream
   {
      public:

         /// Construct uninitialized memory stream
         ZeroCopyMemoryInput ();

         /* Constructor for ZeroCopyMemoryInput. */
         ZeroCopyMemoryInput  (const void *  buf, size_t membuf);

         /* Cancel operation (not used since this class does not block) */
         void cancel (Handle h);

         /* Read from the input stream */
         Handle read (const void ** buf, size_t * s, const CBType & cb,
               size_t suggested);

         /* Rewind the input stream */
         Handle rewindInput (size_t amount, const CBType & cb);

         /* Resets the ZeroCopyMemoryInput stream and sets it to a new memory 
            location */
         void reset (const void * newbuf, size_t newsize);

         /// Reset the stream back to the beginning
         void reset ();

         /* Amount of space remaining that has not be read in this->mem */
         size_t spaceRemaining () const;

         /* get the memory pointer for this location */
         const void * getMemPtr () const;

         /* Get the current offset for the stream (pos) */
         size_t getBufferUsed () const;

         /* Get the total lenght of the underlying stream */
         size_t getBufferSize () const;

      protected:
         const char * mem_;    /*< Stores memory location data */
         size_t memSize_;      /*< Stores the size of the memory location 
                                 (readable size) */
         size_t pos_;          /*< Stores current pointer position inside mem */
   };

   //========================================================================
}
#endif

