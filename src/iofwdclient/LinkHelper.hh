#ifndef IOFWDUTIL_LINKHELPER_HH
#define IOFWDUTIL_LINKHELPER_HH

#include <boost/preprocessor/seq.hpp>
#include <boost/function.hpp>
#include "iofwdutil/FactoryAutoRegister.hh"

/**
 * This header file declares some helper macro's to simplify
 * registering with the generic factory.
 */

namespace iofwdutil
{
//===========================================================================

/**
 * This macro needs to be mentioned in the cpp file of the derived class.
 * Outside of any namespace. Don't forget to use FQN for base/derived.
 */
#define GENERIC_FACTORY_CLIENT(key,base,derived,mykey,linkkey) \
   void register_key_##linkkey () { \
      iofwdutil::FactoryAutoRegister<key,base,void,derived> (mykey); }

/**
 * This macro needs to be mentioned in the cpp file of the derived class.
 * Outside of any namespace. Don't forget to use FQN for base/derived.
 */
#define GENERIC_FACTORY_CLIENT_TAG(key,base,tag,derived,mykey,linkkey) \
   void register_key_##linkkey () { \
      iofwdutil::FactoryAutoRegister<key,base,tag,derived> (mykey); }

/**
 * This macro will ensure the derived class is registered.
 * This needs to be put in a file (not in a library) included in the main
 * program _outside any namespace declaration_.
 *
 * The macro takes a boost preprocessor sequence (i.e.  (a) (b) ... (c) )
 */
#define GENERIC_FACTORY_LINKHELPER(moduleparam) \
      BOOST_PP_SEQ_FOR_EACH(GENERIC_FACTORY_GENCALL, ~, moduleparam)

#define GENERIC_FACTORY_GENCALL(a,b,c) \
   void BOOST_PP_CAT(register_key_,c) (); \
   BOOST_PP_CAT(register_key_,c) ();

//===========================================================================
}

#endif
