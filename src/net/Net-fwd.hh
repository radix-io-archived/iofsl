#ifndef NET_NET_FWD_HH
#define NET_NET_FWD_HH

#include <boost/intrusive_ptr.hpp>

namespace net
{
   //========================================================================

   class Address;
   typedef boost::intrusive_ptr<Address> AddressPtr;
   typedef boost::intrusive_ptr<const Address> ConstAddressPtr;

   class Net;

   class Group;
   typedef boost::intrusive_ptr<Group> GroupHandle;
   typedef boost::intrusive_ptr<const Group> ConstGroupHandle;

   class Communicator;
   typedef boost::intrusive_ptr<Communicator> CommunicatorHandle;
   typedef boost::intrusive_ptr<const Communicator> ConstCommunicatorHandle;

   //========================================================================
}

#endif
