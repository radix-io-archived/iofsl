#ifndef NET_ADDRESS_HH
#define NET_ADDRESS_HH

#include "iofwdutil/IntrusiveHelper.hh"
#include <boost/intrusive_ptr.hpp>

namespace net
{
   //========================================================================

   /**
    * Address abstraction
    */
   class Address : public iofwdutil::IntrusiveHelper
   {
      public:

         
         virtual ~Address ();
   };

   typedef boost::intrusive_ptr<Address> AddressPtr;
   typedef boost::intrusive_ptr<const Address> ConstAddressPtr;

   INTRUSIVE_PTR_HELPER(Address);

   //========================================================================
}

#endif
