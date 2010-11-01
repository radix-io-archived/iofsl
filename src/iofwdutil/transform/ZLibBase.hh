#ifndef IOFWDUTIL_ZLIBBASE_HH
#define IOFWDUTIL_ZLIBBASE_HH

#include "iofwdutil/assert.hh"

extern "C"
{
#include <zlib.h>
}

#include "iofwdutil/transform/GenericTransform.hh"

namespace iofwdutil
{

  namespace transform
  {
    //=======================================================================

    class ZLibBase : public GenericTransform
    {

      public:
        ZLibBase();

        virtual void reset ();

        virtual void transform(const void *const inBuf, size_t inSize,
                      void *outBuf, size_t outSize, size_t *outBytes,
                      int *outState, bool flushFlag);

        virtual state getTransformState () const;

        virtual ~ZLibBase();

      protected:

        bool isInitialized () const
        { return isInit_; }

        int init ()
        { ASSERT(!isInit_); isInit_ = true; return initHelper (); }

        int done ()
        { ASSERT(isInit_); isInit_ = false; return doneHelper (); }

        virtual int initHelper ()  = 0;

        virtual int doneHelper () = 0;

        virtual int process (int flush)  = 0;

        /// Check zlib function return code
        int check (int ret) const;

      protected:
        z_stream stream_;

        size_t out_used_;

        state curstate_;

        bool isInit_;
        bool first_call_;
        bool first_process_;
    };

    //=======================================================================
  }

}

#endif
