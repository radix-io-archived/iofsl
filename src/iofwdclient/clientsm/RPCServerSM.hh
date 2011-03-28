#ifndef IOFWDCLIENT_CLIENTSM_RPCSERVERSM_HH
#define IOFWDCLIENT_CLIENTSM_RPCSERVERSM_HH

#include "sm/SMManager.hh"
#include "sm/SMClient.hh"
#include "sm/SimpleSM.hh"
#include "sm/SimpleSlots.hh"

#include "iofwdutil/tools.hh"

#include "iofwdclient/IOFWDClientCB.hh"
#include "iofwdclient/clientsm/RPCServerHelper.hh"

#include "iofwdclient/streamwrappers/ZoidFSStreamWrappers.hh"

#include "iofwd/RPCClient.hh"
#include "iofwd/service/ServiceManager.hh"
#include "iofwdevent/SingleCompletion.hh"
#include "iofwdevent/ZeroCopyInputStream.hh"
#include "iofwdevent/ZeroCopyOutputStream.hh"

#include "zoidfs/zoidfs.h"
#include "zoidfs/zoidfs-rpc.h"

#include "rpc/RPCEncoder.hh"
#include "iofwd/Net.hh"

#include "net/Address.hh"
#include "net/Net.hh"

#include <cstdio>

using namespace iofwdclient;
using namespace iofwdclient::streamwrappers;
using namespace encoder;
using namespace encoder::xdr;

namespace iofwdclient
{
    namespace clientsm
    {

template< typename InStream, typename OutStream >
class RPCServerSM :
    public sm::SimpleSM< iofwdclient::clientsm::RPCServerSM <InStream, OutStream> >
{
    public:
        RPCServerSM(sm::SMManager & smm,
                bool poll,
                const iofwdevent::CBType & cb,
                std::string rpc_func,
                const InStream & in,
                OutStream & out,
                net::AddressPtr addr) :
            sm::SimpleSM< iofwdclient::clientsm::RPCServerSM <InStream,OutStream> >(smm, poll),
            slots_(*this),
            cb_(cb),
            rpc_func_(rpc_func),
            e_(in),
            d_(out),
            rpc_client_(iofwd::service::ServiceManager::instance().loadService<iofwd::RPCClient>("rpcclient")),
            addr_(addr)
        {
//            iofwd::service::ServiceManager & man = iofwd::service::ServiceManager::instance ();
//            man.registerService ("rpcclient", iofwd::RPCClient);
//            rpc_client_ = man.loadService<iofwd::RPCClient>("rpcclient");
//            rpc_client_(iofwd::service::ServiceManager::instance().loadService<iofwd::RPCClient>("rpcclient")),
            fprintf(stderr, "RPCServerSM:%s:%s\n",in.full_path_, in.component_name_);
            fprintf(stderr, "%s:%i\n", __func__, __LINE__);
        }

        ~RPCServerSM()
        {
        }

        void init(iofwdevent::CBException e)
        {
            fprintf(stderr, "RPCServerSM:%s:%i\n", __func__, __LINE__);
            e.check();
            setNextMethod(&RPCServerSM<InStream,OutStream>::postSetupConnection);
        }

        void postSetupConnection(iofwdevent::CBException e)
        {
            fprintf(stderr, "RPCServerSM:%s:%i\n", __func__, __LINE__);
            e.check();

            /* get the max size */
            e_.net_data_size_ = rpc::getRPCEncodedSize(InStream()).getMaxSize();

            /* TODO how the heck do we get / set the address? */
            /* get a handle for this RPC */


            rpc_handle_ = rpc_client_->rpcConnect(rpc_func_.c_str(), addr_);
            e_.zero_copy_stream_.reset((rpc_handle_->getOut()));


            iofwdevent::SingleCompletion block;
            /* setup the write stream */
            block.reset();
            e_.zero_copy_stream_->write(&e_.data_ptr_, &e_.data_size_, block,
                    e_.net_data_size_);
            block.wait();
            setNextMethod (&RPCServerSM<InStream,OutStream>::waitSetupConnection);
        }

        void waitSetupConnection(iofwdevent::CBException e)
        {
            fprintf(stderr, "RPCServerSM:%s:%i\n", __func__, __LINE__);
            e.check();
            setNextMethod(&RPCServerSM<InStream,OutStream>::postEncodeData);
        }

        void postEncodeData(iofwdevent::CBException e)
        {
            fprintf(stderr, "RPCServerSM:%s:%i\n", __func__, __LINE__);
            e.check();

            /* create the encoder */
            e_.coder_ = rpc::RPCEncoder(e_.data_ptr_, e_.data_size_);

            process(e_.coder_, e_.data_);

            iofwdevent::SingleCompletion block;
            block.reset();
            e_.zero_copy_stream_->rewindOutput(e_.net_data_size_ - e_.coder_.getPos(), block);
            block.wait();
            
            setNextMethod(&RPCServerSM<InStream,OutStream>::waitEncodeData);
//            slots_.wait(BASE_SLOT,
//                    &RPCServerSM<InStream,OutStream>::waitEncodeData);
        }

        void waitEncodeData(iofwdevent::CBException e)
        {
            fprintf(stderr, "RPCServerSM:%s:%i\n", __func__, __LINE__);
            e.check();
            setNextMethod(&RPCServerSM<InStream,OutStream>::postFlush);
        }

        void postFlush(iofwdevent::CBException e)
        {
            fprintf(stderr, "RPCServerSM:%s:%i\n", __func__, __LINE__);
            e.check();

            // Before we can access the output channel we need to wait until the RPC
            // code did its thing
            iofwdevent::SingleCompletion block;
            block.reset();
            e_.zero_copy_stream_->flush(block);
            block.wait();
            setNextMethod ( &RPCServerSM<InStream,OutStream>::waitFlush);
//            slots_.wait(BASE_SLOT,
//                    &RPCServerSM<InStream,OutStream>::waitFlush);
        }

        void waitFlush(iofwdevent::CBException e)
        {
            fprintf(stderr, "RPCServerSM:%s:%i\n", __func__, __LINE__);
            e.check();
            setNextMethod(&RPCServerSM<InStream,OutStream>::postDecodeData);
        }

        void postDecodeData(iofwdevent::CBException e)
        {
            fprintf(stderr, "RPCServerSM:%s:%i\n", __func__, __LINE__);
            e.check();      
            d_.zero_copy_stream_.reset((rpc_handle_->getIn()));
            /* get the max size */
            d_.net_data_size_ = rpc::getRPCEncodedSize(OutStream()).getMaxSize();
   
            iofwdevent::SingleCompletion block;
            block.reset();
            d_.zero_copy_stream_->read(const_cast<const void **>(&d_.data_ptr_),
                    &d_.data_size_, block, d_.net_data_size_);
            block.wait();
            setNextMethod (&RPCServerSM<InStream,OutStream>::waitDecodeData);
//            slots_.wait(BASE_SLOT,
//                    &RPCServerSM<InStream,OutStream>::waitDecodeData);

        }

        void waitDecodeData(iofwdevent::CBException e)
        {
            fprintf(stderr, "RPCServerSM:%s:%i\n", __func__, __LINE__);
            e.check();

            d_.coder_ = rpc::RPCDecoder(d_.data_ptr_, d_.data_size_);

            process(d_.coder_, d_.data_);
            fprintf(stderr, "SIZE: %i, POS: %i\n", d_.coder_.getPos(), d_.data_size_);
            if(d_.coder_.getPos() != d_.data_size_)
            {
                fprintf(stderr, "%s:%i ERROR undecoded data...\n", __func__,
                        __LINE__);
            }
            cb_ ( * (new iofwdevent::CBException()));
        }

    protected:
        /* SM */
        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        sm::SimpleSlots<NUM_BASE_SLOTS,
            iofwdclient::clientsm::RPCServerSM <InStream,OutStream> > slots_;
        const iofwdevent::CBType & cb_;

        /* RPC */
        const std::string rpc_func_;
        boost::shared_ptr<iofwd::RPCClient> rpc_client_;
        rpc::RPCClientHandle rpc_handle_;
        net::AddressPtr addr_;

        /* encoder */
        RPCServerHelper<rpc::RPCEncoder, iofwdevent::ZeroCopyOutputStream,
            const InStream> e_;

        /* decoder */
        RPCServerHelper<rpc::RPCDecoder, iofwdevent::ZeroCopyInputStream,
            OutStream> d_;
};

    }
}

#endif
