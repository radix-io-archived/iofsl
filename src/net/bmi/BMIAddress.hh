#ifndef NET_BMI_BMIADDRESS_HH
#define NET_BMI_BMIADDRESS_HH

#include "net/Address.hh"
#include "iofwdevent/Handle.hh"
#include "iofwdevent/CBType.hh"
#include "iofwdevent/Resource-fwd.hh"

extern "C" {
#include <bmi.h>
}

namespace net
{
   namespace bmi
   {
      //=====================================================================

      class BMIAddress : public Address
      {
         public:

            BMIAddress (BMI_addr_t addr)
               : addr_ (addr)
            {
            }

            BMI_addr_t getAddr () const
            { return addr_; }

            virtual ~BMIAddress ();

         protected:
            BMI_addr_t addr_;

      };

      //=====================================================================
   }
}

#endif
