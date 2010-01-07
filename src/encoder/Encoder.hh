#ifndef ENCODER_ENCODER_HH
#define ENCODER_ENCODER_HH

#include "iofwdutil/tools.hh"
#include "iofwdutil/assert.hh"
#include "Util.hh"
#include "Processor.hh"

namespace encoder
{
//===========================================================================

   /**
    * This class shows how the interface of an encoder looks like.
    * All encoders need to derive from this.
    */
   struct Encoder : public Processor
   {
      /**
       * Return the maximum size of the buffer we're encoding to.
       */
      size_t getMaxSize () const
      { ALWAYS_ASSERT(false); return 0; }

      /**
       * Return how many bytes have been written to the buffer.
       */
      size_t getPos () const
      { ALWAYS_ASSERT(false); return 0; }

      /**
       * Switch to a new buffer.
       */
      void reset (void * UNUSED(mem), size_t UNUSED(maxsize))
      { ALWAYS_ASSERT(false); }

      /**
       * Set next output location in buffer.
       */
      void rewind (size_t newpos = 0)
      { ALWAYS_ASSERT(newpos && false); }
   };


//===========================================================================
}


#endif
