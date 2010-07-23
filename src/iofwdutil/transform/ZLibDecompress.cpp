
#include "ZLibDecompress.hh"

namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================


      int ZLibDecompress::initHelper ()
      {
         // @TODO: add option string to GenericTransform interface;
         // use option string here to get compression level
         return inflateInit (&stream_);
      }

      int ZLibDecompress::doneHelper ()
      {
         return inflateEnd (&stream_);
      }

      int ZLibDecompress::process (int )
      {
         return inflate (&stream_, Z_NO_FLUSH);
      }

      ZLibDecompress::~ZLibDecompress ()
      {
         if (isInitialized ())
            done ();
      }


      //=====================================================================
   }
}
