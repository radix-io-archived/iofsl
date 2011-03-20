#ifndef EXTRASERVICE_AARPC_ATOMICAPPENDSERVERRPC_HH
#define EXTRASERVICE_AARPC_ATOMICAPPENDSERVERRPC_HH

#include "iofwd/Net.hh"
#include "iofwd/ExtraService.hh"
#include "iofwd/extraservice/aarpc/AtomicAppendRPCTypes.hh"

#include "rpc/RPCHandler.hh"
#include "rpc/RPCInfo.hh"
#include "rpc/RPCEncoder.hh"

#include "net/Net.hh"
#include "net/Communicator.hh"

#include "iofwdutil/IOFWDLog-fwd.hh"

#include "iofwdevent/SingleCompletion.hh"

#include "zoidfs/zoidfs.h"
#include "zoidfs/util/zoidfs-wrapped.hh"

namespace iofwd
{
   class Log;
   class RPCServer;

   namespace extraservice
   {
      class AtomicAppendServerRPC : public ExtraService
      {
         public:
            AtomicAppendServerRPC(service::ServiceManager & m);

            virtual void configureNested(const iofwdutil::ConfigFile & f)
            {
                aarpc_master_addr_ =
                    f.getKeyAsDefault<std::string>("master",""); 
            }

            virtual ~AtomicAppendServerRPC();

            static std::string aarpc_master_addr_;

         protected:

            /* rpc handlers */

            void createOffset(const AARPCCreateOffsetIn & in,
                    AARPCCreateOffsetOut & out);
            
            void deleteOffset(const AARPCDeleteOffsetIn & in,
                    AARPCDeleteOffsetOut & out);
            
            void getNextOffset(const AARPCGetNextOffsetIn & in,
                    AARPCGetNextOffsetOut & out);

            /* key value storage helpers */

            uint64_t getNextOffsetFromStorage(const zoidfs::zoidfs_file_size_t inc,
                    const zoidfs::zoidfs_handle_t & handle,
                    zoidfs::zoidfs_file_size_t & atomic_append_base_offset);
            
            uint64_t createOffsetInStorage(const zoidfs::zoidfs_handle_t & handle,
                    zoidfs::zoidfs_file_size_t & atomic_append_base_offset);

            uint64_t deleteOffsetInStorage(const zoidfs::zoidfs_handle_t & handle);
            
            template <typename IN, typename OUT>
                void aarpcServerHelper(
                        const boost::function<void (AtomicAppendServerRPC *
                            base, const IN &, OUT &)> & rpc_func,
                        iofwdevent::ZeroCopyInputStream * in,
                        iofwdevent::ZeroCopyOutputStream * out,
                        const rpc::RPCInfo & )
                {
                    /* streams */
                    boost::scoped_ptr<iofwdevent::ZeroCopyInputStream> instream(in);
                    boost::scoped_ptr<iofwdevent::ZeroCopyOutputStream> outstream(out);

                    IN rpc_func_arg_in;
                    OUT rpc_func_arg_out;

                    /* read data tracking */
                    const char * read_ptr;
                    size_t read_size = 0;
          
                      /* write data tracking */
                    char * write_ptr;
                    size_t write_size = 0;

                    /* max request size */
                    const size_t insize = rpc::getRPCEncodedSize(IN()).getMaxSize();

                    /* max response size */
                    const size_t outsize = rpc::getRPCEncodedSize(OUT()).getMaxSize();

                    /* comp block */
                    iofwdevent::SingleCompletion block;
                
                    /* prepare to read from the stream */
                    in->read(reinterpret_cast<const void **>(&read_ptr),
                            &read_size, block, insize);
                    block.wait();
          
                    /* decode the XDR in the RPC request */
                    rpc::RPCDecoder dec(read_ptr, read_size);
                    process(dec, rpc_func_arg_in);
                    
                    /* get the atomic offset */
                    rpc_func(this, rpc_func_arg_in, rpc_func_arg_out);
              
                    /* prepare to send the response back to the caller */
                    block.reset();
                    out->write(reinterpret_cast<void**>(&write_ptr), &write_size,
                            block, outsize);
                    block.wait();
                    
                    /* encode the offset into XDR for the RPC response */
                    rpc::RPCEncoder enc(write_ptr, write_size);
                    process(enc, rpc_func_arg_out);

                    /* rewind */
                    block.reset();
                    out->rewindOutput(write_size - enc.getPos(), block);
                    block.wait();
                    
                    /* flush the reponse */
                    block.reset();
                    out->flush(block);
                    block.wait(); 
                }

            class AARPCOffsetInfo
            {
                public:
                    AARPCOffsetInfo(const zoidfs::zoidfs_handle_t * h) :
                        handle(h)
                    {
                    }

                    const zoidfs::zoidfs_handle_t * handle;
            };

            static zoidfs::zoidfs_file_size_t aarpcOffsetInitializer(
                    AARPCOffsetInfo * args)
            {
                int ret = 0;
                zoidfs::zoidfs_file_size_t size = 0;
                zoidfs::zoidfs_attr_t attr;

                attr.mask = ZOIDFS_ATTR_SIZE;
                ret = zoidfs::zoidfs_getattr(args->handle, &attr, NULL);
                
                if(ret == zoidfs::ZFS_OK && attr.mask == ZOIDFS_ATTR_SIZE)
                {
                    size = attr.size;
                }
                else
                {
                    size = 0;
                }

                return size;
            }

            boost::shared_ptr<Log> log_service_;
            boost::shared_ptr<RPCServer> rpcserver_;
            iofwdutil::IOFWDLogSource & log_;
            boost::shared_ptr<iofwd::Net> netservice_;
            boost::function< zoidfs::zoidfs_file_size_t(AARPCOffsetInfo *) > offset_init_func_;
      };
   }
}

#endif
