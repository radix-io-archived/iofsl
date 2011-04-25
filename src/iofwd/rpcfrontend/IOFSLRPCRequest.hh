#ifndef IOFWD_RPCFRONTEND_IOFSLRPCREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCREQUEST_HH

#include "zoidfs/util/FileSpecHelper.hh"
#include "zoidfs/util/OpHintHelper.hh"
#include "iofwdevent/CBType.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/util/zoidfs-xdr.hh"

#include "encoder/EncoderStruct.hh"

#include "rpc/RPCInfo.hh"
#include "rpc/RPCEncoder.hh"

#include "iofwdevent/ZeroCopyInputStream.hh"
#include "iofwdevent/ZeroCopyOutputStream.hh"
#include "iofwdevent/SingleCompletion.hh"

#include <boost/scoped_ptr.hpp>

#include "iofwd/Request.hh"
#include "iofwdutil/typestorage.hh"

namespace iofwd
{
   namespace rpcfrontend
   {
       class IOFSLRPCRequest
       {
           public:
               IOFSLRPCRequest(iofwdevent::ZeroCopyInputStream * in,
                       iofwdevent::ZeroCopyOutputStream * out);
               
               ~IOFSLRPCRequest();
           protected:
              
               virtual void decode(const iofwdevent::CBType & cb) = 0;
               virtual void encode() = 0;

               /* data members */

               /* streams */
               boost::scoped_ptr<iofwdevent::ZeroCopyInputStream> in_;
               boost::scoped_ptr<iofwdevent::ZeroCopyOutputStream> out_;

       };
   }
}

#endif
