#include "ZLibDecompress.hh"
#include "iofwdutil/LinkHelper.hh"

GENERIC_FACTORY_CLIENT_TAG(std::string,
      iofwdutil::transform::GenericTransform,
      iofwdutil::transform::GTDecode,
      iofwdutil::transform::ZLibDecompress,
      "ZLIB",
      zlibdecode);


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
