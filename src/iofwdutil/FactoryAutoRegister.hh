#ifndef IOFWDUTIL_FACTORYAUTOREGISTER_HH
#define IOFWDUTIL_FACTORYAUTOREGISTER_HH

#include "Factory.hh"
#include "CreateMethod.hh"

namespace iofwdutil
{

/**
 * Automatic registration for factory.
 *
 */
template <typename KEY, typename BASE, typename DERIVED>
struct FactoryAutoRegister
{
   FactoryAutoRegister (const KEY & key)
   {
      Factory<KEY,BASE>::instance ().add (key,
            &CreateMethod<BASE,DERIVED>::create);
   }
};


#define FACTORYAUTOREGISTER(KEYT,BASE,DERIVED,KEYVAL)\
   static ::iofwdutil::FactoryAutoRegister<KEYT,BASE,DERIVED> fac##DERIVED (KEYVAL); \
   void * LinkHelper_##DERIVED () { return &fac##DERIVED; }

#define FACTORYLINKHELPER(KEYT,BASE,DERIVED) \
   namespace { /* TODO */ }

}
#endif
