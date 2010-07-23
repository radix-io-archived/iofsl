#include "ZLibCompress.hh"

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================


      int ZLibCompress::initHelper ()
      {
         // @TODO: add option string to GenericTransform interface;
         // use option string here to get compression level
         return deflateInit (&stream_, Z_DEFAULT_COMPRESSION);
      }

      int ZLibCompress::doneHelper ()
      {
         return deflateEnd (&stream_);
      }

      int ZLibCompress::process (int flush)
      {
         return deflate (&stream_, flush ? Z_FINISH : Z_NO_FLUSH);
      }

      ZLibCompress::~ZLibCompress ()
      {
         if (isInitialized ())
            done ();
      }


      //=====================================================================
   }
}
