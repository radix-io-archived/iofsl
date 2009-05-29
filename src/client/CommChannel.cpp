#include "CommChannel.hh"
#include "zoidfs/util/zoidfs-xdr.hh"

using namespace iofwdutil::bmi;
using namespace iofwdutil::xdr; 

namespace client
{
//===========================================================================

   CommChannel::CommChannel (BMIContextPtr context, BMIAddr iofwdhost)
      : bmi_(context), 
      iofwdhost_(iofwdhost),buffer_send_(iofwdhost_, BMI::ALLOC_SEND),
      buffer_receive_(iofwdhost_, BMI::ALLOC_RECEIVE)
   {
   }

   CommChannel::~CommChannel ()
   {
   }

//===========================================================================

//===========================================================================
}
