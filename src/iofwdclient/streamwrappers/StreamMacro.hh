#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/fold_left.hpp>
#include <boost/preprocessor/seq/cat.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/punctuation/comma.hpp>
#include <boost/preprocessor/seq/pop_back.hpp>
#define CLIENT_GENSTREAMWRAP_PARAMS_DECL(r,data,elem) CLIENT_GENSTREAMWRAP_DECL_S1(elem)
#define CLIENT_GENSTREAMWRAP_DECL_S1(elem) BOOST_PP_CAT(CLIENT_GENSTREAMWRAP_S2 (elem),_);

#define CLIENT_GENSTREAMWRAP_PARAMS(r,data,elem) CLIENT_GENSTREAMWRAP_S1(elem)
#define CLIENT_GENSTREAMWRAP_S1(elem) CLIENT_GENSTREAMWRAP_S2 (elem) = NULL,
#define CLIENT_GENSTREAMWRAP_S2(elem) BOOST_PP_SEQ_FOLD_LEFT(CLIENT_DOFOLD,  \
                                        BOOST_PP_SEQ_HEAD(elem),             \
                                        BOOST_PP_SEQ_TAIL(elem))
#define CLIENT_DOFOLD(r,data,elem) data elem
#define CLIENT_GENSTREAMWRAP_PARAM_INIT(r,data,elem) CLIENT_GENSTREAMWRAP_DECPROCESS(elem)
#define CLIENT_GENSTREAMWRAP_DECPROCESS(elem) CLIENT_GENSTREAMWRAP_PNAME(elem)(CLIENT_GENSTREAMWRAP_PICKNAME(elem)),
#define CLIENT_GENSTREAMWRAP_PNAME(elem) BOOST_PP_CAT(BOOST_PP_SEQ_CAT(BOOST_PP_SEQ_TAIL(elem)),_)

#define CLIENT_GENSTREAMWRAP_DECPARAM(r,data,elem) CLIENT_GENSTREAMWRAP_PROCESS(elem)
#define CLIENT_GENSTREAMWRAP_PROCESS(elem) process(e,CLIENT_GENSTREAMWRAP_PROCESS2(elem));
#define CLIENT_GENSTREAMWRAP_PICKNAME(elem) BOOST_PP_SEQ_CAT(BOOST_PP_SEQ_TAIL(elem))
#define CLIENT_GENSTREAMWRAP_PROCESS2(elem) BOOST_PP_SEQ_FOLD_LEFT(CLIENT_WFOLD,  \
                                        BOOST_PP_SEQ_HEAD(elem),             \
                                        BOOST_PP_SEQ_TAIL(elem))
#define CLIENT_WFOLD(r,data,elem) data w.elem
#define CLIENT_GENSTREAMWRAP(CLASSNAME, INSTREAMNAME, INPARAMS, INPROCESS, OUTSTREAMNAME, OUTPARAMS, OUTPROCESS) \
   using namespace encoder;                                                  \
   using namespace encoder::xdr;                                             \
   namespace iofwdclient                                                     \
   {                                                                         \
      namespace streamwrappers                                               \
      {                                                                      \
      typedef zoidfs::zoidfs_handle_t zoidfs_handle_t;\
      typedef zoidfs::zoidfs_sattr_t zoidfs_sattr_t;\
      typedef zoidfs::zoidfs_op_hint_t zoidfs_op_hint_t;\
      typedef encoder::EncoderString<0, ZOIDFS_PATH_MAX> EncoderString;\
         class INSTREAMNAME                                                  \
         {                                                                   \
            public:                                                          \
                 INSTREAMNAME(BOOST_PP_SEQ_FOR_EACH(CLIENT_GENSTREAMWRAP_PARAMS, , INPARAMS) \
                              zoidfs::zoidfs_op_hint_t * op_hint = 0) : \
                 BOOST_PP_SEQ_FOR_EACH (CLIENT_GENSTREAMWRAP_PARAM_INIT, ,INPARAMS) \
                 op_helper_(op_hint)                                          \
        {                                                                     \
        }                                                                     \
        const encoder::OpHintHelper op_helper_;                               \
        BOOST_PP_SEQ_FOR_EACH(CLIENT_GENSTREAMWRAP_PARAMS_DECL, ,INPARAMS)    \
      };                                                                         \
      class OUTSTREAMNAME                                                        \
      {                                                                          \
          public:                                                                \
              OUTSTREAMNAME(BOOST_PP_SEQ_FOR_EACH(CLIENT_GENSTREAMWRAP_PARAMS, , OUTPARAMS) \
                               zoidfs::zoidfs_op_hint_t * op_hint = NULL) :       \
               BOOST_PP_SEQ_FOR_EACH(CLIENT_GENSTREAMWRAP_PARAM_INIT, ,OUTPARAMS) \
               op_helper_(op_hint)                                                \
           {                                                                      \
           }                                                                      \
           BOOST_PP_SEQ_FOR_EACH(CLIENT_GENSTREAMWRAP_PARAMS_DECL, ,OUTPARAMS)    \
           encoder::OpHintHelper op_helper_;                                      \
      };                                                                          \
      template <typename Enc, typename Wrapper>                                   \
      inline Enc & process (Enc & e,                                              \
           Wrapper & w,                                                           \
           typename process_filter<Wrapper, INSTREAMNAME>::type * UNUSED(d) = NULL)  \
      {                                                                           \
         BOOST_PP_SEQ_FOR_EACH(CLIENT_GENSTREAMWRAP_DECPARAM, ,INPROCESS)         \
         process(e, w.op_helper_);                                                \
         return e;                                                                \
      }                                                                           \
      template <typename Enc, typename Wrapper>                                   \
      inline Enc & process (Enc & e,                                              \
           Wrapper & w,                                                           \
           typename process_filter<Wrapper, OUTSTREAMNAME>::type * UNUSED(d) = \
           NULL)                                                                  \
      {                                                                           \
         BOOST_PP_SEQ_FOR_EACH(CLIENT_GENSTREAMWRAP_DECPARAM, ,OUTPROCESS)        \
         process(e, w.op_helper_);                                                \
         return e;                                                                \
      }                                                                           \
   }                                                                              \
   }                                                                              

