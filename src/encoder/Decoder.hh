#ifndef ENCODER_DECODER_HH
#define ENCODER_DECODER_HH

#include "iofwdutil/assert.hh"
#include "iofwdutil/tools.hh"
#include "Processor.hh"

namespace encoder
{
//===========================================================================

   /**
    * This class demonstrates how the Decoder interface looks like.
    * All decoders must derive from this class.
    */
   struct Decoder : public Processor
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
      void reset (const void * UNUSED(mem), size_t UNUSED(maxsize))
      { ALWAYS_ASSERT(false); }

      /**
       * Set next output location in buffer.
       */
      void rewind (size_t UNUSED(newpos) = 0)
      { ALWAYS_ASSERT(false); }

   } ;

//===========================================================================
}

#endif
