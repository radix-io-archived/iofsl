#ifndef IOFWDUTIL_ENCODEREXCEPTION_HH
#define IOFWDUTIL_ENCODEREXCEPTION_HH

#include "iofwdutil/ZException.hh"

namespace encoder
{
//===========================================================================

   /**
    * This exception is thrown by a processor
    * if an error happened during encoding
    * or decoding.
    */
   class EncoderException : public virtual iofwdutil::ZException {};

   /// When running out of buffer space while encoding or decoding
   struct BufferException : public virtual EncoderException {};

   /// When types don't match. (float into uint, a 512 len string into a 256
   /// len string.
   struct TypeException : public virtual EncoderException {};

//===========================================================================
}

#endif
