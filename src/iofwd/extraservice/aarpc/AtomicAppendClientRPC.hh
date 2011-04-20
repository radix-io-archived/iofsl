#ifndef IOFWD_EXTRASERVICE_AARPC_ATOMICAPPENDCLIENTRPC_HH
#define IOFWD_EXTRASERVICE_AARPC_ATOMICAPPENDCLIENTRPC_HH

#include "iofwd/service/ServiceManager.hh"
#include "iofwd/RPCClient.hh"
#include "iofwd/Net.hh"
#include "iofwd/IofwdLinkHelper.hh"
#include "iofwd/service/Service.hh"
#include "iofwd/ExtraService.hh"
#include "iofwdutil/ZException.hh"
#include "iofwdevent/SingleCompletion.hh"

#include "rpc/RPCInfo.hh"
#include "rpc/RPCEncoder.hh"

#include "net/Net.hh"
#include "net/Communicator.hh"

#include "iofwdutil/tools.hh"

#include <string>
#include <unistd.h>

#include "zoidfs/zoidfs.h"
#include "zoidfs/util/zoidfs-xdr.hh"

#include "iofwd/extraservice/aarpc/AtomicAppendServerRPC.hh"
#include "iofwd/extraservice/aarpc/AtomicAppendUtil.hh"

#include "iofwdutil/IOFSLKey.hh"
#include "iofwdutil/IOFSLKeyValueStorage.hh"

namespace iofwd
{
    namespace extraservice
    {
        class AtomicAppendClientRPC : public ExtraService
        {
            public:
                AtomicAppendClientRPC(service::ServiceManager & m);
                ~AtomicAppendClientRPC();

                virtual void configureNested(const iofwdutil::ConfigFile &
                        UNUSED(f)) 
                {
                }

                void getNextOffset(zoidfs::zoidfs_handle_t & handle,
                        zoidfs::zoidfs_file_size_t incsize,
                        zoidfs::zoidfs_file_ofs_t & offset,
                        uint64_t & retcode)
                {
                    AARPCGetNextOffsetIn in;
                    AARPCGetNextOffsetOut out;
                    int64_t server_rank = AtomicAppendFileHandleHash(&handle) %
                        comm_size_;
                    int local_rpc = server_rank == netservice_->getServerRank() ? 1 :
                        0;

                    in.handle = handle;
                    in.inc = incsize;

                    if(master_mode_ && !local_rpc)
                    {
                        aarpcClientHelper(
                                rpcclient_->rpcConnect("aarpc.getnextoffset",
                                    addr_),
                                in, out);
                        offset = out.offset;
                        retcode = out.retcode;
                    }
                    else if(distributed_mode_ && !local_rpc)
                    {
                        aarpcClientHelper(
                                rpcclient_->rpcConnect("aarpc.getnextoffset",
                                    (*comm_)[server_rank]),
                                in, out);
                        offset = out.offset;
                        retcode = out.retcode;
                    }
                    else
                    {
                        AARPCOffsetLocalInfo offset_init_args(&handle);
                        iofwdutil::IOFSLKey key = iofwdutil::IOFSLKey();

                        /* build the key */
                        key.setFileHandle(&handle);
                        key.setDataKey(std::string("NEXTAPPENDOFFSET"));

                        /* fetch and inc the offset for the key */
                        iofwdutil::IOFSLKeyValueStorage::instance().rpcFetchAndInc(key, 
                            incsize, &offset, offset_init_func_, &offset_init_args);
                        retcode = 0;
                    }
                }

                void createOffset(zoidfs::zoidfs_handle_t & handle,
                        zoidfs::zoidfs_file_ofs_t & offset,
                        uint64_t & retcode)
                {
                    AARPCCreateOffsetIn in;
                    AARPCCreateOffsetOut out;
                    int64_t server_rank = AtomicAppendFileHandleHash(&handle) %
                        comm_size_;
                    int local_rpc = server_rank == netservice_->getServerRank() ? 1 :
                        0;

                    in.handle = handle;

                    if(master_mode_ && !local_rpc)
                    {
                        aarpcClientHelper(
                                rpcclient_->rpcConnect("aarpc.getnextoffset",
                                    addr_),
                                in, out);
                        retcode = out.retcode;
                        offset = out.offset;
                    }
                    else if(distributed_mode_ && !local_rpc)
                    {
                        aarpcClientHelper(
                                rpcclient_->rpcConnect("aarpc.createoffset",
                                    (*comm_)[server_rank]),
                                in, out);
                        retcode = out.retcode;
                        offset = out.offset;
                    }
                    else
                    {
                        iofwdutil::IOFSLKey key = iofwdutil::IOFSLKey();

                        /* build the key */
                        key.setFileHandle(&handle);
                        key.setDataKey(std::string("NEXTAPPENDOFFSET"));

                        /* create the key / value pair */
                        iofwdutil::IOFSLKeyValueStorage::instance().rpcInitKeyValue<
                            zoidfs::zoidfs_file_size_t >(key, offset);
                        
                        retcode = 0;
                    }

                }

                void deleteOffset(zoidfs::zoidfs_handle_t & handle,
                        uint64_t & retcode)
                {
                    AARPCDeleteOffsetIn in;
                    AARPCDeleteOffsetOut out;
                    int64_t server_rank = AtomicAppendFileHandleHash(&handle) %
                        comm_size_;
                    int local_rpc = server_rank == netservice_->getServerRank() ? 1 :
                        0;

                    in.handle = handle;

                    if(master_mode_ && !local_rpc)
                    {
                        aarpcClientHelper(
                                rpcclient_->rpcConnect("aarpc.getnextoffset",
                                    addr_),
                                in, out);
                        retcode = out.retcode;
                    }
                    else if(distributed_mode_ && !local_rpc)
                    {
                        aarpcClientHelper(
                                rpcclient_->rpcConnect("aarpc.deleteoffset",
                                    (*comm_)[server_rank]),
                                in, out);
                        retcode = out.retcode;
                    }
                    else
                    {
                        iofwdutil::IOFSLKey key = iofwdutil::IOFSLKey();

                        /* build the key */
                        key.setFileHandle(&handle);
                        key.setDataKey(std::string("NEXTAPPENDOFFSET"));

                        /* delete the key / value pair */
                        iofwdutil::IOFSLKeyValueStorage::instance().rpcFetchAndDrop< 
                            zoidfs::zoidfs_file_size_t >(key);
                        retcode = 0;
                    }

                }

            protected:

            template < typename IN, typename OUT >
            void aarpcClientHelper(
                    rpc::RPCClientHandle h,
                    const IN & rpc_arg_in,
                    OUT & rpc_arg_out)
            {
                iofwdevent::SingleCompletion block;
                boost::mutex::scoped_lock l(chmutex_);

                {
                    // wait until outgoing RPC ready
                    block.reset();
                    h->waitOutReady(block);
                    block.wait();
                    
                    // Now we can use the outgoing stream
                    boost::scoped_ptr<iofwdevent::ZeroCopyOutputStream> out (h->getOut());

                    // encode input arguments for remote RPC

                    void * write_ptr;
                    size_t write_size;
                    const size_t sendsize = rpc::getRPCEncodedSize(IN()).getMaxSize();;

                    block.reset();
                    out->write(&write_ptr, &write_size, block, sendsize);
                    block.wait();

                    rpc::RPCEncoder enc(write_ptr, write_size);
                    process(enc, rpc_arg_in);

                    block.reset();
                    out->rewindOutput(write_size - enc.getPos(), block);
                    block.wait();

                    block.reset();
                    out->flush(block);
                    block.wait();
                }

                // Read response
                {
                    block.reset();
                    h->waitInReady(block);
                    block.wait();

                    boost::scoped_ptr<iofwdevent::ZeroCopyInputStream> in (h->getIn ());

                    const void * read_ptr;
                    size_t read_size;

                    const size_t recvsize =
                        rpc::getRPCEncodedSize(OUT()).getMaxSize();;

                    block.reset();
                    in->read(&read_ptr, &read_size, block, recvsize);
                    block.wait();

                    rpc::RPCDecoder dec(read_ptr, read_size);
                    process(dec, rpc_arg_out);

                    if(dec.getPos() != read_size)
                        std::cout << "Extra bytes at end of RPC response?" << std::endl;
                }
            }

            class AARPCOffsetLocalInfo
            {
                public:
                    AARPCOffsetLocalInfo(const zoidfs::zoidfs_handle_t * h) :
                        handle(h)
                    {
                    }

                    const zoidfs::zoidfs_handle_t * handle;
            };

            static zoidfs::zoidfs_file_size_t aarpcOffsetLocalInitializer(
                    AARPCOffsetLocalInfo * args)
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

                boost::mutex chmutex_;
                iofwd::service::ServiceManager & man_;
                boost::shared_ptr<iofwd::Net> netservice_;
                boost::shared_ptr<iofwd::RPCClient> rpcclient_;
                net::Net * net_;
                net::ConstCommunicatorHandle comm_;
                net::AddressPtr addr_;
                size_t comm_size_;
                bool master_mode_;
                bool distributed_mode_;
            
                boost::function< zoidfs::zoidfs_file_size_t(AARPCOffsetLocalInfo *) >
                    offset_init_func_;
        };
    }
}

#endif
