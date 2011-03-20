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
                AtomicAppendClientRPC();
                ~AtomicAppendClientRPC();

                void getNextOffset(zoidfs::zoidfs_handle_t & handle,
                        zoidfs::zoidfs_file_size_t incsize,
                        zoidfs::zoidfs_file_ofs_t & offset,
                        uint64_t & retcode)
                {
                    AARPCGetNextOffsetIn in;
                    AARPCGetNextOffsetOut out;

                    in.handle = handle;
                    in.inc = incsize;

                    aarpcClientHelper(
                            rpcclient_->rpcConnect("aarpc.getnextoffset", addr_),
                            in, out);

                    offset = out.offset;
                    retcode = out.retcode;
                }

                void createOffset(zoidfs::zoidfs_handle_t & handle,
                        zoidfs::zoidfs_file_ofs_t & offset,
                        uint64_t & retcode)
                {
                    AARPCCreateOffsetIn in;
                    AARPCCreateOffsetOut out;

                    in.handle = handle;

                    aarpcClientHelper(
                            rpcclient_->rpcConnect("aarpc.createoffset", addr_),
                            in, out);

                    retcode = out.retcode;
                    offset = out.offset;
                }

                void deleteOffset(zoidfs::zoidfs_handle_t & handle,
                        uint64_t & retcode)
                {
                    AARPCDeleteOffsetIn in;
                    AARPCDeleteOffsetOut out;

                    in.handle = handle;

                    aarpcClientHelper(
                            rpcclient_->rpcConnect("aarpc.deleteoffset", addr_),
                            in, out);

                    retcode = out.retcode;
                }

            protected:

            template < typename IN, typename OUT >
            void aarpcClientHelper(
                    rpc::RPCClientHandle h,
                    const IN & rpc_arg_in,
                    OUT & rpc_arg_out)
            {
                iofwdevent::SingleCompletion block;

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

                iofwd::service::ServiceManager & man_;
                boost::shared_ptr<iofwd::Net> netservice_;
                boost::shared_ptr<iofwd::RPCClient> rpcclient_;
                net::AddressPtr addr_;
        };
    }
}

#endif
