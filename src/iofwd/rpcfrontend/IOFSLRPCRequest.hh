#ifndef IOFWD_RPCFRONTEND_IOFSLRPCREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCREQUEST_HH

#include "zoidfs/util/FileSpecHelper.hh"
#include "zoidfs/util/OpHintHelper.hh"

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/util/zoidfs-xdr.hh"

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
               
               virtual ~IOFSLRPCRequest();

               void decodeRPCInput()
               {
                   /* setup */
                   initRPCDecoder();
                   
                   /* decode */
                   decode();

                   /* cleanup */
                   finalizeRPCDecoder();
               }
           
               void encodeRPCOutput()
               {
                   /* setup */
                   initRPCEncoder();
                   
                   /* decode */
                   encode();

                   /* cleanup */
                   finalizeRPCEncoder();
               }
           
           protected:
               
               typedef struct
               {
                   zoidfs::zoidfs_handle_t parent_handle;
                   char full_path[ZOIDFS_PATH_MAX];
                   char component_name[ZOIDFS_NAME_MAX];
               } FileInfo;

               void decodeFileSpec(FileInfo & info)
               {
                   process(dec_, encoder::FileSpecHelper(&info.parent_handle,
                               info.component_name, info.full_path));
               }

               void decodeOpHint(zoidfs::zoidfs_op_hint_t * op_hint)
               {
                   process(dec_, encoder::OpHintHelper(op_hint));
               }

               virtual size_t rpcEncodedInputDataSize() = 0;
               virtual size_t rpcEncodedOutputDataSize() = 0;

               void initRPCDecoder()
               {
                   iofwdevent::SingleCompletion block;
                   
                   /* sanity */
                   block.reset();
                   
                   /* prepare to read from the in stream */
                   insize_ = rpcEncodedInputDataSize();
                   in_->read(reinterpret_cast<const void **>(&read_ptr_),
                           &read_size_, block, insize_);
                   block.wait();
                   
                   dec_ = rpc::RPCDecoder(read_ptr_, read_size_);
               }

               virtual void decode() = 0;

               void finalizeRPCDecoder()
               {
                   /* do nothing */
               }

               void initRPCEncoder()
               {
                   iofwdevent::SingleCompletion block;
                  
                   /* sanity */ 
                   block.reset();
                  
                   /* prepare to write to the out stream */ 
                   outsize_ = rpcEncodedOutputDataSize();
                   out_->write(reinterpret_cast<void**>(&write_ptr_),
                           &write_size_, block, outsize_);
                   block.wait();

                   enc_ = rpc::RPCEncoder(write_ptr_, write_size_);
               }

               virtual void encode() = 0;

               void finalizeRPCEncoder()
               {
                   iofwdevent::SingleCompletion block;

                   /* sanity */
                   block.reset();

                   /* rewind */
                   out_->rewindOutput(write_size_ - 
                           enc_.getPos(), block);
                   block.wait();
                   
                   /* flush the reponse */
                   block.reset();
                   out_->flush(block);
                   block.wait();
               }

               /* data members */

               /* streams */
               boost::scoped_ptr<iofwdevent::ZeroCopyInputStream> in_;
               boost::scoped_ptr<iofwdevent::ZeroCopyOutputStream> out_;

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
       };
   }
}

#endif
