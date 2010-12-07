#include <boost/cstdint.hpp>
#include "HashFunc.hh"
#include "HashAutoRegister.hh"

namespace iofwdutil
{
   namespace hash
   {
//===========================================================================


      class Adler32 : public HashFunc
      {
         public:
            HASHFUNC_DECLARE(Adler32);

            Adler32 ();

            virtual void reset ();
            virtual std::string getName () const;
            virtual void process (const void * d, size_t bytes);
            virtual size_t getHash (void * dest, size_t bufsize,
                  bool finalize);
            virtual size_t getHashSize () const;

            virtual ~Adler32 ();

        protected:
            boost::uint32_t hash_;
      };

//===========================================================================
   }
}
