#include <algorithm>
#include <cstring>
#include "SHA1Simple.hh"
#include "iofwdutil/assert.hh"
#include "HashAutoRegister.hh"
#include "iofwdutil/tools.hh"


namespace iofwdutil
{
   namespace hash
   {
      //=====================================================================

      // disabled until bug is fixed. (Unit test fails)
      //HASHFUNC_AUTOREGISTER(SHA1Simple, "sha1", 0);

      //=====================================================================

      std::string SHA1Simple::getName () const
      { return "sha1"; }

      SHA1Simple::SHA1Simple ()
      {
         reset ();
      }

      void SHA1Simple::process (const void * d, size_t bytes)
      {
         SHA1Input (&ctx_, static_cast<const unsigned char *>(d), bytes);
      }

      size_t SHA1Simple::getHashSize () const
      {
         return 20;
      }

      size_t SHA1Simple::getHash (void * dest, size_t bufsize, bool finalize)
      {
         if (bufsize < 20)
            return 0;

         SHA1Context copy;
         SHA1Context * context;
         if (finalize)
         {
            context = &ctx_;
         }
         else
         {
            copy = ctx_;
            context = &copy;
         }

         SHA1Result (context);
         STATIC_ASSERT(sizeof(unsigned) == 4);
         memcpy (dest, &context->Message_Digest[0], 20);
         return 20;
      }

      void SHA1Simple::reset ()
      {
         SHA1Reset (&ctx_);
      }

      //=====================================================================
   }
}
