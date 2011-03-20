#ifndef IOFWD_EXTRASERVICE_AARPC_ATOMICAPPENDCLIENTRPC_HH
#define IOFWD_EXTRASERVICE_AARPC_ATOMICAPPENDCLIENTRPC_HH

#include "iofwd/service/ServiceManager.hh"
#include "iofwd/RPCClient.hh"
#include "iofwd/Net.hh"
#include "iofwd/IofwdLinkHelper.hh"
#include "iofwd/service/Service.hh"
#include "iofwdutil/ZException.hh"
#include "iofwdevent/SingleCompletion.hh"
#include "net/Net.hh"

#include "rpc/RPCInfo.hh"
#include "rpc/RPCEncoder.hh"

#include <iostream>
#include <string>
#include <unistd.h>

#include "zoidfs/zoidfs.h"
#include "zoidfs/util/zoidfs-xdr.hh"

#include "iofwd/extraservice/aarpc/AtomicAppendServerRPC.hh"

namespace iofwd
{
    namespace extraservice
    {
        class AtomicAppendClientRPC
        {
            public:
                AtomicAppendClientRPC() :
                    man_(iofwd::service::ServiceManager::instance()),
                    netservice_(man_.loadService<iofwd::Net>("net")),
                    rpcclient_(man_.loadService<iofwd::RPCClient>("rpcclient"))
                {
                    iofwdevent::SingleCompletion block;

                    /* get the address */
                    net::Net * net = netservice_->getNet();
                    net->lookup(iofwd::extraservice::AtomicAppendServerRPC::aarpc_master_addr_.c_str(),
                            &addr_, block);
                   
                    /* wait for the lookup to complete */ 
                    block.wait();
                }

                ~AtomicAppendClientRPC()
                {
                }

                void getNextOffset(zoidfs::zoidfs_handle_t & handle,
                        zoidfs::zoidfs_file_size_t incsize,
                        zoidfs::zoidfs_file_ofs_t & offset,
                        uint64_t & retcode)
                {
                    getNextOffsetImpl(
                            rpcclient_->rpcConnect("aarpc.getnextoffset", addr_),
                            handle, incsize, offset, retcode);
                }

                void createOffset(zoidfs::zoidfs_handle_t & handle,
                        zoidfs::zoidfs_file_ofs_t & offset,
                        uint64_t & retcode)
                {
                    createOffsetImpl(
                            rpcclient_->rpcConnect("aarpc.createoffset", addr_),
                            handle, offset, retcode);
                }

                void deleteOffset(zoidfs::zoidfs_handle_t & handle,
                        uint64_t & retcode)
                {
                    deleteOffsetImpl(
                            rpcclient_->rpcConnect("aarpc.deleteoffset", addr_),
                            handle, retcode);
                }

            protected:

            template < typename IN, typename OUT >
            void aarpcClientHelper(
                    const boost::function<void (AtomicAppendServerRPC *
                        base, const IN &, OUT &)> & rpc_func,
                    rpc::RPCClientHandle h,
                    const rpc::RPCInfo &)
            {
                iofwdevent::SingleCompletion block;
                IN rpc_arg_in;
                OUT rpc_arg_out;

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
                    const size_t sendsize =
                        rpc::getRPCEncodedSize(IN()).getMaxSize();;

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

                void getNextOffsetImpl(rpc::RPCClientHandle h,
                        zoidfs::zoidfs_handle_t & handle,
                        zoidfs::zoidfs_file_size_t incsize,
                        zoidfs::zoidfs_file_ofs_t & offset,
                        uint64_t & retcode);
                
                void createOffsetImpl(rpc::RPCClientHandle h,
                        zoidfs::zoidfs_handle_t & handle,
                        zoidfs::zoidfs_file_ofs_t offset,
                        uint64_t & retcode);
                
                void deleteOffsetImpl(rpc::RPCClientHandle h,
                        zoidfs::zoidfs_handle_t & handle,
                        uint64_t & retcode);

                iofwd::service::ServiceManager & man_;
                boost::shared_ptr<iofwd::Net> netservice_;
                boost::shared_ptr<iofwd::RPCClient> rpcclient_;
                net::AddressPtr addr_;
        };
    }
}

#endif
