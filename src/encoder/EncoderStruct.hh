#ifndef SRC_ENCODER_ENCODERSTRUCT_HH
#define SRC_ENCODER_ENCODERSTRUCT_HH

#include "encoder/Util.hh"
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/fold_left.hpp>
#include <boost/preprocessor/seq/cat.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/punctuation/comma.hpp>
#include <boost/preprocessor/seq/pop_back.hpp>

#define ENCODERSTRUCT(NAME, LIST)                                            \
   struct NAME {                                                             \
      BOOST_PP_SEQ_FOR_EACH(EMSG_ENCODEHELPER, , LIST)                       \
   };                                                                        \
   template <typename PROC, typename WRAPPER>                                \
   inline Enc & process (PROC & e,                                           \
                         WRAPPER & w,                                        \
                         typename process_filter<WRAPPER, NAME>::type * UNUSED(d) = \
                         NULL)                                               \
   {                                                                         \
      BOOST_PP_SEQ_FOR_EACH(EMSG_ENCODERHELPER2, , LIST)                     \
    return e;                                                                \
   }

#define EMSG_ENCODEHELPER(r, data, elem) EMSG_PROCESS1(elem)
#define EMSG_DOFOLD(r,data,elem) data elem
#define EMSG_PROCESS1(elem) BOOST_PP_SEQ_FOLD_LEFT(EMSG_DOFOLD,              \
                                        BOOST_PP_SEQ_HEAD(elem),             \
                                        BOOST_PP_SEQ_TAIL(elem));



#define EMSG_ENCODERHELPER2(r,data,elem) EMSG_PROCESS2(elem)
#define EMSG_PROCESS2(elem) process (e,w.EMSG_PICKNAME(elem));
#define EMSG_PICKNAME(elem) BOOST_PP_SEQ_CAT(BOOST_PP_SEQ_TAIL(elem))


#endif
