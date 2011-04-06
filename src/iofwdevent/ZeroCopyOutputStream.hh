#ifndef IOFWDEVENT_ZEROCOPYOUTPUTSTREAM_HH
#define IOFWDEVENT_ZEROCOPYOUTPUTSTREAM_HH

#include "CBType.hh"
#include "Handle.hh"

namespace iofwdevent
{
   //========================================================================

   /**
    * Efficiant output stream
    */
   struct ZeroCopyOutputStream
   {
      char type;
      /**
       * Return pointer to a region of size size where output data can be
       * stored.
       * ptr and size are only valid when the callback is called.
       *
       * suggested (if nonzero) is a hint to the implementation about the
       * ideal block size / amount of data that will be written.
       */
      virtual Handle write (void ** ptr, size_t * size, const CBType & cb,
            size_t suggested) = 0;

      /**
       * Used to indicate size bytes were left unused in the current output
       * buffer.
       */
      virtual Handle rewindOutput (size_t size, const CBType & cb) = 0;

      /**
       * Flush internal buffers, if any
       *
       * @TODO: add rewind functionality in here
       *    size = 0 -> flush everything size != 0, flush size bytes
       */
      virtual Handle flush (const CBType & cb) = 0;

      /**
       * NOTE: Any output not flushed *might* be discarded when the stream is
       * destructed.
       */
      virtual ~ZeroCopyOutputStream ();

      /**
       * TODO: fold this into flush.
       */
      virtual void close (const CBType & cb) = 0;
   };

   //========================================================================
}
#endif
