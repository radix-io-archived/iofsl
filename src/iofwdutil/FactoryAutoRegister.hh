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
template <typename KEY, typename BASE, typename TAG, typename DERIVED>
struct FactoryAutoRegister
{
   FactoryAutoRegister (const KEY & key)
   {
      Factory<KEY,BASE,TAG>::instance ().add (key,
            &CreateMethod<BASE,DERIVED>::create);
   }
};

#define F_TOKENPASTE(x, y) x ## y
#define F_TOKENPASTE2(x, y) F_TOKENPASTE(x, y)
#define F_UNIQUE(a) F_TOKENPASTE2(a, __LINE__)


#define FACTORYAUTOREGISTER(key,base,derived,name) \
   namespace { \
      static iofwdutil::FactoryAutoRegister<key,base,void,derived> \
      F_UNIQUE(autoreg_) (name); \
   }

#define FACTORYAUTOREGISTER_TAG(key,base,tag,derived,name) \
   namespace { \
      static iofwdutil::FactoryAutoRegister<key,base,tag,derived> \
      F_UNIQUE(autoreg_) (name); \
   }

}
#endif
