#ifndef IOFWDUTIL_FACTORYCLIENT_HH
#define IOFWDUTIL_FACTORYCLIENT_HH

#include <boost/mpl/list.hpp>

#define FACTORY_CONSTRUCTOR_PARAMS(...) \
   typedef boost::mpl::list<__VA_ARGS__> FACTORY_SIGNATURE


#endif
