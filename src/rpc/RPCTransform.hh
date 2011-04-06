#ifndef RPC_RPCTRANSFORM_HH
#define RPC_RPCTRANSFORM_HH

#include "iofwdutil/transform/GenericTransform.hh"
#include "iofwdevent/ZeroCopyInputStream.hh"
#include "iofwdevent/ZeroCopyOutputStream.hh"

namespace rpc
{
   //========================================================================

   iofwdutil::transform::GenericTransform * getRPCTransform (uint32_t );

   iofwdevent::ZeroCopyOutputStream * getOutputTransform
      (iofwdevent::ZeroCopyOutputStream *, uint32_t flags);

   iofwdevent::ZeroCopyInputStream * getInputTransform
      (iofwdevent::ZeroCopyInputStream *, uint32_t flags);


   //========================================================================
}

#endif
