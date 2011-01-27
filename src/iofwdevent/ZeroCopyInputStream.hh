#ifndef IOFWDEVENT_ZEROCOPYINPUTSTREAM_HH
#define IOFWDEVENT_ZEROCOPYINPUTSTREAM_HH

#include "CBType.hh"
#include "Handle.hh"

namespace iofwdevent
{
   //=======================================================================

   /**
    * Stream based interface enabling zero copy in most situations.
    */
   struct ZeroCopyInputStream
   {
      /**
       * Return a pointer and size of the location where the next data block
       * can be found.
       *
       * Suggested (if non-zero) is a hint about the amount of bytes the
       * caller is expecting to receive from the stream.
       */
      virtual Handle read (const void ** ptr, size_t * size,
            const CBType & cb, size_t suggested) = 0;

      /**
       * Return unconsumed data to the stream.
       */
      virtual Handle rewindInput (size_t size,
            const CBType & cb) = 0;

      virtual ~ZeroCopyInputStream ();
   };

   //=======================================================================
}

#endif
