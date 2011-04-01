#ifndef IOFWD_RPCFRONTEND_RPCSIMPLEREQUEST_HH
#define IOFWD_RPCFRONTEND_RPCSIMPLEREQUEST_HH

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
      template< typename IN, typename OUT >
      class RPCSimpleRequest : 
          public IOFSLRPCRequest
      {
        protected:
          IN inStruct;
          OUT outStruct;

        public:
          void decode()
          {
            encoder::xdr::XDRSizeProcessor * size_ = new 
                                            encoder::xdr::XDRSizeProcessor();

            iofwdevent::SingleCompletion block;

            /* sanity */
            block.reset();      

            /* prepare to read from the in stream */
  
            // @TODO: Not using actual size until size function is fixed 
            /* process (size_, inStruct); */
            insize_ = 15000;
            
            /* Read Stream */
            in_->read(reinterpret_cast<const void **>(&read_ptr_),
                    &read_size_, block, insize_);
            block.wait();
      
            /* Start RPCDecoder */            
            dec_ = rpc::RPCDecoder(read_ptr_, read_size_);

            /* Process the struct */
            process (dec_, inStruct);
          }
    
          void encode()
          {
            encoder::xdr::XDRSizeProcessor * size_ = new 
                                            encoder::xdr::XDRSizeProcessor();

            iofwdevent::SingleCompletion block;
          
            /* sanity */ 
            block.reset();


            // @TODO: Not using actual size until size function is fixed          
            /* Get size of encode */ 
            //process (size_, outStruct);
            outsize_ = 15000;  /* (e->size()).getMaxSize(); */
            
            /* Get Write Location */
            out_->write(reinterpret_cast<void**>(&write_ptr_),
                    &write_size_, block, outsize_);
            block.wait();

            /* Build encoder struct */
            enc_ = rpc::RPCEncoder(write_ptr_, write_size_);
      
            /* Process */
            process (enc_, outStruct);

            /* sanity */
            block.reset();

            /* rewind */
            out_->rewindOutput(write_size_ - enc_.getPos(), block);
            block.wait();

            /* flush the reponse */
            block.reset();
            out_->flush(block);
            block.wait();
          }
      };

   }
}
