#ifndef IOFWDUTIL_TRANSFORM_ZLIBCOMPRESS_HH
#define IOFWDUTIL_TRANSFORM_ZLIBCOMPRESS_HH

#include "ZLibBase.hh"

namespace iofwdutil
{
   namespace transform
   {

      class ZLibCompress : public ZLibBase
      {
         public:
            virtual ~ZLibCompress ();

         protected:
            virtual int initHelper ();
            virtual int doneHelper ();
            virtual int process (int flush);
      };

   }
}
#endif
