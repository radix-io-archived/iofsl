#ifndef IOFWDCLIENT_CLIENTSM_BMISERVERSM_HH
#define IOFWDCLIENT_CLIENTSM_BMISERVERSM_HH

#include "sm/SMManager.hh"
#include "sm/SMClient.hh"
#include "sm/SimpleSM.hh"
#include "sm/SimpleSlots.hh"

#include "iofwdutil/tools.hh"

#include "iofwdclient/IOFWDClientCB.hh"
#include "iofwdclient/clientsm/BMIServerHelper.hh"

#include "zoidfs/zoidfs.h"

#include <cstdio>

using namespace iofwdclient;
using namespace encoder;
using namespace encoder::xdr;

namespace iofwdclient
{
    namespace clientsm
    {

template< typename InStream, typename OutStream >
class BMIServerSM :
    public sm::SimpleSM< iofwdclient::clientsm::BMIServerSM <InStream, OutStream> >
{
    public:
        BMIServerSM(sm::SMManager & smm,
                bool poll,
                const iofwdevent::CBType & cb,
                const InStream & in,
                OutStream & out) :
            sm::SimpleSM< iofwdclient::clientsm::BMIServerSM <InStream,OutStream> >(smm, poll),
            slots_(*this),
            cb_(cb),
            e_(in),
            d_(out)
        {
            fprintf(stderr, "%s:%i\n", __func__, __LINE__);
        }

        ~BMIServerSM()
        {
        }

        void init(iofwdevent::CBException e)
        {
            e.check();
            setNextMethod(&BMIServerSM<InStream,OutStream>::postSendResquest);
        }

        void postSendRequest(iofwdevent::CBException e)
        {
            e.check();

            slots_.wait(BASE_SLOT,
                    &BMIServerSM<InStream,OutStream>::waitSendRequest);
        }

        void waitSendRequest(iofwdevent::CBException e)
        {
            e.check();
            setNextMethod(&BMIServerSM<InStream,OutStream>::postRecvResponse);
        }

        void postRecvResponse(iofwdevent::CBException e)
        {
            e.check();

            slots_.wait(BASE_SLOT,
                    &BMIServerSM<InStream,OutStream>::waitRecvResponse);
        }

        void waitRecvResponse(iofwdevent::CBException e)
        {
            e.check();
        }

    protected:
        /* SM */
        enum {BASE_SLOT = 0, NUM_BASE_SLOTS};
        sm::SimpleSlots<NUM_BASE_SLOTS,
            iofwdclient::clientsm::BMIServerSM <InStream,OutStream> > slots_;
        const iofwdevent::CBType & cb_;

        /* BMI */

        /* encoder */
        BMIServerHelper<rpc::BMIEncoder, const InStream> e_;

        /* decoder */
        BMIServerHelper<rpc::BMIDecoder, OutStream> d_;
};

    }
}

#endif
