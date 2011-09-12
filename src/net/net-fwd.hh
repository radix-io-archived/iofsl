#ifndef NET_NET_FWD_HH
#define NET_NET_FWD_HH

#include <boost/intrusive_ptr.hpp>

namespace net
{
   //========================================================================

   class Communicator;

   typedef boost::intrusive_ptr<Communicator> CommunicatorHandle;
   typedef boost::intrusive_ptr<const Communicator> ConstCommunicatorHandle;


   //========================================================================
}

#endif
