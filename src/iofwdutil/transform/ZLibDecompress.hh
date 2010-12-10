
#ifndef IOFWDUTIL_TRANSFORM_ZLIBDECOMPRESS_HH
#define IOFWDUTIL_TRANSFORM_ZLIBDECOMPRESS_HH

#include "ZLibBase.hh"

namespace iofwdutil
{
   namespace transform
   {

      class ZLibDecompress : public ZLibBase
      {
         public:
            virtual ~ZLibDecompress ();

         protected:
            virtual int initHelper ();
            virtual int doneHelper ();
            virtual int process (int flush);
      };

   }
}
#endif
