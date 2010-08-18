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
   class EncoderException : public iofwdutil::ZException
   {
   };


//===========================================================================
}

#endif
