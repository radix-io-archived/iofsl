#ifndef IOFWDUTIL_HASH_HASHAUTOREGISTER_HH
#define IOFWDUTIL_HASH_HASHAUTOREGISTER_HH

#include <string>

#include "HashFactory.hh"

namespace iofwdutil
{
   namespace hash
   {

      template <typename T>
      class HashAutoRegister
      {
         protected:
            static HashFunc * createFunc ()
            { return new T (); }

         public:
            HashAutoRegister (const std::string & name, int prio)
            {
               HashFactory::instance().registerHash(name, prio,
                     &HashAutoRegister<T>::createFunc);
            }
      };

      /// Some macro's to help static linking
/* 
#define HASHFUNC_AUTOREGISTER(a,b,c) HashAutoRegister<a> a::_autoreg (b,c)
#define HASHFUNC_DECLARE(a) static HashAutoRegister<a> _autoreg;
#define HASHFUNC_REFERENCE(a) void * UNUSED(a##f) = (void*) &a::_autoreg;
*/
#define HASHFUNC_REFERENCE(a) a::_autoref()
#define HASHFUNC_DECLARE(a) static void _autoref()
#define HASHFUNC_AUTOREGISTER(a,b,c) void a::_autoref () { \
   HashAutoRegister<a> (b, c); }

   }
}
#endif

