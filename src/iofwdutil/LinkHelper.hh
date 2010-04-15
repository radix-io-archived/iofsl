#ifndef IOFWDUTIL_LINKHELPER_HH
#define IOFWDUTIL_LINKHELPER_HH

#include "iofwdutil/FactoryAutoRegister.hh"

/**
 * This header file declares some helper macro's to simplify
 * registering with the generic factory.
 */

namespace iofwdutil
{
//===========================================================================
 


/**
 * This macro needs to be mentioned in the header file of the derived class
 */
#define GENERIC_FACTORY_CLIENT_DECLARE(key,base,derived) \
   static void _generic_factory_register ()


/**
 * This macro needs to be mentioned in the cpp file of the derived class.
 */
#define GENERIC_FACTORY_CLIENT(key,base,derived,mykey) \
   void derived::_generic_factory_register () { \
      iofwdutil::FactoryAutoRegister<key,base,derived> (mykey); }

/**
 * This macro will ensure the derived class is registered.
 */
#define GENERIC_FACTORY_CLIENT_REGISTER(key,base,derived) \
   derived::_generic_factory_register  ()


//===========================================================================
}

#endif
