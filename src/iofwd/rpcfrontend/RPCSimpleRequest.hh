#ifndef IOFWD_RPCFRONTEND_RPCSIMPLEREQUEST_HH
#define IOFWD_RPCFRONTEND_RPCSIMPLEREQUEST_HH
#include <cstdio>

#include "zoidfs/util/FileSpecHelper.hh"
#include "zoidfs/util/OpHintHelper.hh"
#include "iofwdevent/CBType.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/util/zoidfs-xdr.hh"

#include "encoder/EncoderStruct.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"

#include "rpc/RPCInfo.hh"
#include "rpc/RPCEncoder.hh"

#include "iofwdevent/ZeroCopyInputStream.hh"
#include "iofwdevent/ZeroCopyOutputStream.hh"
#include "iofwdevent/SingleCompletion.hh"

#include <boost/scoped_ptr.hpp>
#include <boost/bind.hpp>

#include "iofwd/Request.hh"
#include "iofwdutil/typestorage.hh"

namespace iofwd
{
   namespace rpcfrontend
   {
      template< typename IN, typename OUT >
      class RPCSimpleRequest : 
          public IOFSLRPCRequest
      {
        protected:
          IN inStruct;
          OUT outStruct;
          /* pointers and sizes of mem stream data */
          const char * read_ptr_;
          size_t read_size_;
          char * write_ptr_;
          size_t write_size_;
          size_t insize_;
          size_t outsize_;

          /* RPC encoder / decoder */
          rpc::RPCDecoder dec_;
          rpc::RPCEncoder enc_;
          zoidfs::zoidfs_op_hint_t op_hint_;
        public:
          RPCSimpleRequest (iofwdevent::ZeroCopyInputStream * in,
                            iofwdevent::ZeroCopyOutputStream * out) :
               IOFSLRPCRequest (in,out)
          {
          }

          void decode(const iofwdevent::CBType & cb)
          {
            iofwdevent::CBType decodeComplete = boost::bind(&RPCSimpleRequest<IN,OUT>::processDecode, this, _1, cb);
            /* prepare to read from the in stream */
  
            insize_ = 15000;
            
            /* Read Stream */
            in_->read(reinterpret_cast<const void **>(&read_ptr_),
                    &read_size_, decodeComplete, insize_);
          }

          void processDecode (iofwdevent::CBException e,
                              const iofwdevent::CBType & cb)
          {
            e.check();
            if (read_size_ == 0)
              decode(cb);
            /* Start RPCDecoder */            
            dec_ = rpc::RPCDecoder(read_ptr_, read_size_);

            /* Process the struct */
            process (dec_, inStruct);            
            cb(iofwdevent::CBException());
          }
          void encode()
          {
            ASSERT ("WILL BE REMOVED" == 0); 
          }

          void encode(iofwdevent::CBType cb)
          {
            iofwdevent::CBType next = boost::bind (&RPCSimpleRequest<IN,OUT>::processEncode,
                                                   this, _1, cb);

            outsize_ = 15000;  /* (e->size()).getMaxSize(); */            
            /* Get Write Location */
            out_->write(reinterpret_cast<void**>(&write_ptr_),
                    &write_size_, next, outsize_);
          }
          void processEncode(iofwdevent::CBException e, iofwdevent::CBType cb)
          {
            e.check();
            iofwdevent::CBType next = boost::bind (&RPCSimpleRequest<IN,OUT>::flushEncode, this,
                                                   _1, cb);

            /* Build encoder struct */
            enc_ = rpc::RPCEncoder(write_ptr_, write_size_);
      
            /* Process */
            process (enc_, outStruct);

            /* rewind */
            out_->rewindOutput(write_size_ - enc_.getPos(), next);
          }
          void flushEncode(iofwdevent::CBException e, iofwdevent::CBType cb)
          {
            e.check();
            if (out_->type == 'T')
               out_->close(cb);
            else
               out_->flush(cb);
          }
      };

   }
}
#endif
