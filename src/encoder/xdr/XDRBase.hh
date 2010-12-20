#ifndef ENCODER_XDRBASE_HH
#define ENCODER_XDRBASE_HH

#include <rpc/xdr.h>
#include <boost/static_assert.hpp>
#include <boost/assert.hpp>
#include "iofwdutil/always_assert.hh"

#include "encoder/EncoderWrappers.hh"

namespace encoder
{
   namespace xdr
   {
//===========================================================================

// Processor for XDR
class XDRBase
{
public:
   XDRBase (const void * mem, size_t size, bool read) :
      mem_(mem), size_(size), read_(read)
   {
      init ();
   }

   XDRBase ()
      : mem_(0), size_(0), read_(true)
   {
      init ();
   }

   void reset (void * mem, size_t size)
   {
      destroy ();
      mem_ = static_cast<const void*>(mem); size_ = size;
      init ();
   }

   void reset (const void * mem, size_t size)
   {
      destroy ();
      mem_ = static_cast<const void*>(mem); size_ = size;
      init ();
   }

   /**
    * The logic in this function needs to be fixed.
    */
   void rewind (size_t pos)
   {
      pos += initpos_;

      if (xdr_setpos (&xdr_, initpos_))
         return;

      // If rewinding fails, we can only go to the beginning of the buffer
      ALWAYS_ASSERT(!pos);

      // If setpos failed, recreate XDR mem
      destroy ();
      init ();
   }

   ~XDRBase ()
   {
      destroy ();
   }

   /* Return max size of encoded data */
   size_t getMaxSize () const
   {
      return size_;
   }

   size_t getPos () const
   {
      return xdr_getpos (&xdr_);
   }

public:
   struct ::XDR xdr_;

   void check (int t)
   {
      if (t)
         return;

      // if and xdr operation fails we can only assume it is due to a buffer
      // problem.
      bufferFailure ();
   }

protected:

   void bufferFailure ();

   void destroy ()
   {
      if (mem_)
         xdr_destroy (&xdr_);
      mem_ = 0;
      size_ = 0;
   }

   void init ()
   {
      if (!mem_)
         return ;

      xdrmem_create (&xdr_, const_cast<char *> (static_cast<const char *> (mem_)),
            (unsigned int) size_, (read_ ? XDR_DECODE : XDR_ENCODE));
      initpos_ = xdr_getpos(&xdr_);

      // This doesn't have to be true, but there might be some locations in
      // the code where we assume it to be true.
      ALWAYS_ASSERT(0 == initpos_);
   }

public:
   const void * mem_;
   size_t size_;
   bool read_;
   unsigned int initpos_;
};
//===========================================================================
//===========================================================================
        }
}

#endif
