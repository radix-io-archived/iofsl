#include "XDRBase.hh"
#include "encoder/EncoderException.hh"

namespace encoder
{
   namespace xdr
   {
      //=====================================================================

      void XDRBase::bufferFailure ()
      {
         ZTHROW (BufferException () << iofwdutil::zexception_msg(
                  "XDR function failed! (most likely buffer problem!)"));
      }

      //=====================================================================
   }
}
