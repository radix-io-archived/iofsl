#ifndef IOFWDUTIL_LINKHELPER_HH
#define IOFWDUTIL_LINKHELPER_HH

#include <boost/function.hpp>
#include "iofwdutil/FactoryAutoRegister.hh"

/**
 * This header file declares some helper macro's to simplify
 * registering with the generic factory.
 */

namespace iofwdutil
{
//===========================================================================

   namespace linkhelper 
   {
   struct LinkHelperAutoCall
   {
      public:
         LinkHelperAutoCall (const boost::function<void()> & func)
         { func (); }
   };
   }

/**
 * This macro needs to be mentioned in the cpp file of the derived class.
 * Outside of any namespace. Don't forget to use FQN for base/derived.
 */
#define GENERIC_FACTORY_CLIENT(key,base,derived,mykey,linkkey) \
   namespace iofwdutil { namespace linkhelper { void register_key_##linkkey () { \
      iofwdutil::FactoryAutoRegister<key,base,derived> (mykey); } } }

/**
 * This macro will ensure the derived class is registered.
 * This needs to be put in a file (not in a library) included in the main
 * program _outside any namespace declaration_.
 */
#define GENERIC_FACTORY_CLIENT_REGISTER(linkkey) \
   namespace iofwdutil { namespace linkhelper { \
      void register_key_##linkkey (); } } 
   
#define GENERIC_FACTORY_CLIENT_CALL(linkkey) \
   iofwdutil::linkhelper::LinkHelperAutoCall call_##linkkey (\
            boost::function<void()>(&iofwdutil::linkhelper::register_key_##linkkey));\

//===========================================================================
}

#endif
