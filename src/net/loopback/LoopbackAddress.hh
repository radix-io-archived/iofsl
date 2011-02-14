#ifndef NET_LOOPBACK_LOOPBACKADDRESS_HH
#define NET_LOOPBACK_LOOPBACKADDRESS_HH

#include "net/Address.hh"

namespace net
{
   namespace loopback
   {
      //=====================================================================

      class LoopbackAddress : public Address
      {
         public:

            virtual ~LoopbackAddress ();
      };

      //=====================================================================
   }
}

#endif
