#ifndef IOFWDCLIENT_CLIENTSM_RPCCLIENTWRITE_HH
#define IOFWDCLIENT_CLIENTSM_RPCCLIENTWRITE_HH

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

template< typename INTYPE, typename OUTTYPE >
class RPCClientWrite :
    public sm::SimpleSM< iofwdclient::clientsm::RPCClientWrite <INTYPE, OUTTYPE> >
{
    public:
        RPCClientWrite(sm::SMManager & smm,
                       bool poll,
                       const iofwdevent::CBType & cb,
                       rpc::RPCClientHandle rpc_handle,
                       const INTYPE & in,
                       OUTTYPE & out) :
            sm::SimpleSM< iofwdclient::clientsm::RPCClientWrite <INTYPE,OUTTYPE> >(smm, poll),
            slots_(*this),
            e_(in),
            d_(out),
            rpc_handle_(rpc_handle)
        {
            cb_ = (iofwdevent::CBType)cb;
            fprintf(stderr, "RPCClientWrite:%s:%i\n", __func__, __LINE__);
        }

        ~RPCClientWrite()
        {
        }

        void init(iofwdevent::CBException e)
        {
            fprintf(stderr, "RPCClientWrite:%s:%i\n", __func__, __LINE__);
            e.check();

            /* Get the maximum possible send size */
            e_.net_data_size_ = rpc::getRPCEncodedSize(e_.data_).getMaxSize();
            /* Set the output stream */
            rpc_handle_->waitOutReady (slots_[BASE_SLOT]);
            slots_.wait (BASE_SLOT, 
                       &RPCClientWrite<INTYPE,OUTTYPE>::outputReady);
        }

        void outputReady (iofwdevent::CBException e)
        {
            e_.zero_copy_stream_.reset((rpc_handle_->getOut()));

            setNextMethod(&RPCClientWrite<INTYPE,OUTTYPE>::postSetupConnection);
        }

        void postSetupConnection(iofwdevent::CBException e)
        {
            fprintf(stderr, "RPCClientWrite:%s:%i\n", __func__, __LINE__);
            e.check();

            /* setup the write stream */
            e_.zero_copy_stream_->write(&e_.data_ptr_, &e_.data_size_, slots_[BASE_SLOT],
                    e_.net_data_size_);

            slots_.wait (BASE_SLOT, 
                       &RPCClientWrite<INTYPE,OUTTYPE>::waitSetupConnection);
        }

        void waitSetupConnection(iofwdevent::CBException e)
        {
            fprintf(stderr, "RPCClientWrite:%s:%i\n", __func__, __LINE__);
            e.check();
            setNextMethod(&RPCClientWrite<INTYPE,OUTTYPE>::postEncodeData);
        }

        void postEncodeData(iofwdevent::CBException e)
        {
            e.check();

            /* create the encoder */
            e_.coder_ = rpc::RPCEncoder(e_.data_ptr_, e_.data_size_);

            process(e_.coder_, e_.data_);

            e_.zero_copy_stream_->rewindOutput(e_.data_size_ - e_.coder_.getPos(), slots_[BASE_SLOT]);

            slots_.wait(BASE_SLOT,
                    &RPCClientWrite<INTYPE,OUTTYPE>::getWriteBuffer);

        }

        /* Get the write buffer for writing data */
        void getWriteBuffer (iofwdevent::CBException e)
        {
            /* setup the write stream */
            e_.zero_copy_stream_->write(&e_.data_ptr_, &e_.data_size_, slots_[BASE_SLOT],
                    RemainingWrite(e_.data_));

            slots_.wait(BASE_SLOT,&RPCClientWrite<INTYPE,OUTTYPE>::writeData);
        }

        void flushWriteBuffer (iofwdevent::CBException e)
        {
            e_.zero_copy_stream_->flush(slots_[BASE_SLOT]);
            slots_.wait(BASE_SLOT,
                    &RPCClientWrite<INTYPE,OUTTYPE>::getWriteBuffer);  
        }          
      
        /* Write data to output stream */
        void writeData (iofwdevent::CBException e)
        {
            size_t outSize = e_.data_size_;
            int ret = getWriteData (&e_.data_ptr_, &outSize, e_.data_);
            /* write finished */
            if (ret == 0)
            {
               /* rewind then continue to flush */
               e_.zero_copy_stream_->rewindOutput(e_.data_size_ - outSize, slots_[BASE_SLOT]);
               slots_.wait(BASE_SLOT,
                           &RPCClientWrite<INTYPE,OUTTYPE>::waitEncodeData); 
            }
            /* More data to be written (out of write buffer) */
            else 
            {
               setNextMethod(&RPCClientWrite<INTYPE,OUTTYPE>::flushWriteBuffer);               
            }
        }

        /* Flush state */
        void waitEncodeData(iofwdevent::CBException e)
        {
            fprintf(stderr, "RPCClientWrite:%s:%i\n", __func__, __LINE__);
            e.check();
            setNextMethod(&RPCClientWrite<INTYPE,OUTTYPE>::postFlush);
        }

        void postFlush(iofwdevent::CBException e)
        {
            fprintf(stderr, "RPCClientWrite:%s:%i\n", __func__, __LINE__);
            e.check();

            // Before we can access the output channel we need to wait until the RPC
            // code did its thing
            e_.zero_copy_stream_->flush(slots_[BASE_SLOT]);
            slots_.wait(BASE_SLOT,
                    &RPCClientWrite<INTYPE,OUTTYPE>::waitFlush);
        }

        void waitFlush(iofwdevent::CBException e)
        {
            fprintf(stderr, "RPCClientWrite:%s:%i\n", __func__, __LINE__);
            e.check();
            rpc_handle_->waitInReady (slots_[BASE_SLOT]);
            slots_.wait(BASE_SLOT,&RPCClientWrite<INTYPE,OUTTYPE>::postDecodeData);
        }

        
        void postDecodeData(iofwdevent::CBException e)
        {
            fprintf(stderr, "RPCClientWrite:%s:%i\n", __func__, __LINE__);
            e.check();      
            /* get the max size */
            d_.net_data_size_ = rpc::getRPCEncodedSize(OUTTYPE()).getMaxSize();
            d_.zero_copy_stream_.reset((rpc_handle_->getIn()));
            d_.zero_copy_stream_->read(const_cast<const void **>(&d_.data_ptr_),
                    &d_.data_size_, slots_[BASE_SLOT], d_.net_data_size_);

            slots_.wait(BASE_SLOT,
                    &RPCClientWrite<INTYPE,OUTTYPE>::waitDecodeData);
            /* Temporarily Added to check state machine progression */
//            setNextMethod (&RPCClientWrite<INTYPE,OUTTYPE>::waitDecodeData);
        }

        void waitDecodeData(iofwdevent::CBException e)
        {
            fprintf(stderr, "RPCClientWrite:%s:%i\n", __func__, __LINE__);
            e.check();

            d_.coder_ = rpc::RPCDecoder(d_.data_ptr_, d_.data_size_);

            process(d_.coder_, d_.data_);
            fprintf(stderr, "SIZE: %i, POS: %i\n", d_.coder_.getPos(), d_.data_size_);
            if(d_.coder_.getPos() != d_.data_size_)
            {
                fprintf(stderr, "%s:%i ERROR undecoded data...\n", __func__,
                        __LINE__);
            }
            //fprintf(stderr, "RPCClientWrite:%s:%p\n", __func__,cb_);
            
            cb_ (e);
        }

    protected:
        /* SM */
        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        sm::SimpleSlots<NUM_BASE_SLOTS,
            iofwdclient::clientsm::RPCClientWrite <INTYPE,OUTTYPE> > slots_;
        iofwdevent::CBType cb_;

        /* RPC */
        const std::string rpc_func_;
        boost::shared_ptr<iofwd::RPCClient> rpc_client_;
        rpc::RPCClientHandle rpc_handle_;

        /* encoder */
        RPCServerHelper<rpc::RPCEncoder, iofwdevent::ZeroCopyOutputStream,
            const INTYPE> e_;

        /* decoder */
        RPCServerHelper<rpc::RPCDecoder, iofwdevent::ZeroCopyInputStream,
            OUTTYPE> d_;
                
};

    }
}

#endif
