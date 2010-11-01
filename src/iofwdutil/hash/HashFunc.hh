#ifndef IOFWDUTIL_HASHFUNC_HH
#define IOFWDUTIL_HASHFUNC_HH

#include <string>

namespace iofwdutil
{
   //========================================================================

   namespace hash
   {

   class HashFunc
   {
   public:

      /**
       * Reset the internal hash function state.
       */
      virtual void reset () = 0;

      /// Return the name of the hash function
      virtual std::string getName () const = 0;

      virtual void process (const void * d, size_t bytes) = 0;

      /**
       * Return hash in buffer; Returns number of bytes written
       * or 0 if the buffer is too small.
       * 
       * (Optimization):
       * If finalize is true, no more data will be given to this instance
       * until reset has been called.
       *
       * If finalize is false, the hash will be calculated but more data can
       * still be added to this hash calculation.
       */
      virtual size_t getHash (void * dest, size_t bufsize, bool finalize =
            true) = 0;

      /**
       * Returns number of bytes in the output of this hash func.
       */
      virtual size_t getHashSize () const = 0;

      virtual ~HashFunc ();
   };

   //========================================================================
   }
}

#endif
