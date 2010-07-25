#ifndef IOFWDUTIL_TRANSFORM_BZLIB_HH
#define IOFWDUTIL_TRANSFORM_BZLIB_HH

#include "GenericTransform.hh"

extern "C"
{
#include <bzlib.h>
}


namespace iofwdutil
{
   namespace transform
   {
      //=====================================================================

      class BZLib : public GenericTransform
      {
         public:
            BZLib (bool compress);

            virtual ~BZLib ();

            virtual void reset ();

            virtual void transform(const void *const inBuf, size_t inSize,
                  void *outBuf, size_t outSize, size_t *outBytes,
                  int *outState, bool flushFlag);

            virtual state getTransformState () const;

         protected:
            int init ();

            int done ();

            int process (bool flush);

            int check (int ret) const;

         protected:
            bool compress_;

            bz_stream stream_;
            bool stream_init_;

            state curstate_;
      };

      //=====================================================================
   }
}
#endif
