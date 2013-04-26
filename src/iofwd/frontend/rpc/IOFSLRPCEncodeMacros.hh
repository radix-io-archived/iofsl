#include <iostream>
#include <typeinfo>
#include <boost/preprocessor/list/for_each.hpp>
#include <boost/preprocessor/tuple/to_list.hpp>

# define IOFSL_ENC_TYPES                                                     \
   BOOST_PP_TUPLE_TO_LIST(                                                   \
      13,                                                                    \
      (                                                                      \
         bool,                                                               \
         char, signed char, unsigned char,                                   \
         unsigned short, short,                                              \
         int, unsigned,                                                      \
         long, unsigned long,                                                \
         float,                                                              \
         double, long double                                                 \
         zoidfs::zoidfs_op_hint_t *,                                         \
         zoidfs::zoidfs_handle_t *                                           \
      )                                                                      \
   )                                                                         

#define RUNPROC(N, P)                                                        \
   {                                                                         \
      XDRSizeProcessor t = N;                                                \
      BOOST_PP_LIST_FOR_EACH (PREFORMPROCESS, P, IOFSL_ENC_TYPES);           \
   }
   
# define PREFORMPROCESS(R, P, T)                                             \
   if (typeid(P) == typeid(T))                                               \
      process(t, P);
