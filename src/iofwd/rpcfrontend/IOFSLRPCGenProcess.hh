#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/fold_left.hpp>
#include <boost/preprocessor/seq/cat.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/punctuation/comma.hpp>
#include <boost/preprocessor/seq/pop_back.hpp>

#define RPC_ENCODER(r,data,elem) RPC_ENCPROCESS(elem)
#define RPC_ENCPROCESS(elem) enc_struct.RPC_PICKNAME(elem) = RPC_GETNAME(elem);
#define RPC_GETNAME(elem)    BOOST_PP_SEQ_FOLD_LEFT(EMSG_DOFOLD,             \
                                        BOOST_PP_SEQ_HEAD(elem),             \
                                        BOOST_PP_SEQ_TAIL(elem))
#define RPC_PICKNAME(elem)  BOOST_PP_SEQ_CAT(BOOST_PP_SEQ_TAIL(elem))

#define RPC_DECPARAM(r,data,elem) RPC_DECPROCESS(elem)
#define RPC_DECPROCESS(elem) param_.RPC_PICKNAME(elem) = &dec_struct.RPC_PICKNAME(elem);

#define RPC_GENPROCESS(NAME, DECODELIST, ENCLIST)                            \
      void NAME::decode()                                                    \
      {                                                                      \
         process(dec_, dec_struct);                                          \
         zoidfs::hints::zoidfs_hint_create(&op_hint_);                       \
         decodeOpHint(&op_hint_);                                            \
      }                                                                      \
      void NAME::encode()                                                    \
      {                                                                      \
          int returnCode = getReturnCode();                                  \
          BOOST_PP_SEQ_FOR_EACH(RPC_ENCODER, , ENCLIST)                      \
          process (enc_, enc_struct);                                        \
      }                                                                      

