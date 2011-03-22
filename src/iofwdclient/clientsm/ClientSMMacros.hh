#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/fold_left.hpp>
#include <boost/preprocessor/seq/cat.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/pop_back.hpp>
                   
#define CLIENT_GENERATESM(CLASSNAME, INSTREAMNAME, OUTSTREAMNAME, NAME)      \
   namespace iofwdclient                                                     \
   {                                                                         \
       namespace clientsm                                                    \
       {                                                                     \
   CLASSNAME::~CLASSNAME()                                                   \
   {                                                                         \
   }                                                                         \
   void CLASSNAME::init(iofwdevent::CBException e)                           \
   {                                                                         \
       fprintf(stderr, "%s:%i\n", __func__, __LINE__);                       \
       e.check();                                                            \
       setNextMethod(&CLASSNAME::postRPCServerSM);                           \
   }                                                                         \
   void CLASSNAME::postRPCServerSM(iofwdevent::CBException e)                \
   {                                                                         \
       fprintf(stderr, "%s:%i\n", __func__, __LINE__);                       \
       e.check();                                                            \
       server_sm_.reset(new RPCServerSM< INSTREAMNAME, OUTSTREAMNAME >(smm_,    \
                  poll_, slots_[BASE_SLOT], NAME, in_, out_));    \
       slots_.wait(BASE_SLOT, &CLASSNAME::waitRPCServerSM);                  \
   }                                                                         \
   void CLASSNAME::waitRPCServerSM(iofwdevent::CBException e)                \
   {                                                                         \
       fprintf(stderr, "%s:%i\n", __func__, __LINE__);                       \
       e.check();                                                            \
       cb_(zoidfs::ZFS_COMP_DONE, e);                                        \
   }                                                                         \
   void CLASSNAME::postSMErrorState(iofwdevent::CBException e)               \
   {                                                                         \
       fprintf(stderr, "%s:%i\n", __func__, __LINE__);                       \
       e.check();                                                            \
                                                                             \
   }                                                                         \
   } }
                                                        
#define CLIENT_GENERATEHEADERSM_PARAMS(r,data,elem) CLIENT_GENERATEHEADERSM_S1(elem)
#define CLIENT_GENERATEHEADERSM_S1(elem) CLIENT_GENERATEHEADERSM_S2(elem),
#define CLIENT_GENERATEHEADERSM_S2(elem) BOOST_PP_SEQ_FOLD_LEFT(CLIENT_DOFOLD, \
                                        BOOST_PP_SEQ_HEAD(elem),             \
                                        BOOST_PP_SEQ_TAIL(elem))
#define CLIENT_DOFOLD(r,data,elem) data elem

#define CLIENT_GENERATEHEADERSM(CLASSNAME, OUTSTREAMNAME, INSTREAMNAME, PARAMETERS, INPARAMS, OUTPARAMS)          \
   namespace iofwdclient                                                     \
   {                                                                         \
      using namespace streamwrappers;                                        \
      namespace clientsm                                                     \
      {                                                                      \
         class CLASSNAME :                                                   \
            public sm::SimpleSM< iofwdclient::clientsm::CLASSNAME >          \
         {                                                                   \
            public:                                                          \
                CLASSNAME (sm::SMManager & smm,                              \
                bool poll,                                                   \
                const IOFWDClientCB & cb,                                    \
                int * ret,                                                   \
                BOOST_PP_SEQ_FOR_EACH(CLIENT_GENERATEHEADERSM_PARAMS, , PARAMETERS) \
                zoidfs::zoidfs_op_hint_t * op_hint                           \
                ):                                                           \
            sm::SimpleSM< iofwdclient::clientsm::CLASSNAME >(smm, poll),     \
            slots_(*this),                                                   \
            cb_(cb),                                                         \
            ret_(ret),                                                       \
            in_(INSTREAMNAME INPARAMS),                                     \
            out_ OUTPARAMS ,                                                 \
            server_sm_(NULL)                                                 \
            {                                                                \
               fprintf(stderr, "%s:%i\n", __func__, __LINE__);               \
            }                                                                \
               ~CLASSNAME();                                                 \
               void init(iofwdevent::CBException e);                         \
               void postRPCServerSM(iofwdevent::CBException e);              \
               void waitRPCServerSM(iofwdevent::CBException e);              \
               void postSMErrorState(iofwdevent::CBException e);             \
          protected:                                                         \
              enum {BASE_SLOT = 0, NUM_BASE_SLOTS};                          \
              sm::SimpleSlots<NUM_BASE_SLOTS,                                \
                  iofwdclient::clientsm::CLASSNAME> slots_;                  \
              const IOFWDClientCB & cb_;                                     \
              int * ret_;                                                    \
              INSTREAMNAME in_;                                              \
              OUTSTREAMNAME out_;                                            \
              sm::SMClientSharedPtr server_sm_;                              \
        };                                                                   \
    }                                                                        \
   } 
                                                                
