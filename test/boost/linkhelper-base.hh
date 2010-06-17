#ifndef TEST_BOOST_LINKHELPER_BASE_HH
#define TEST_BOOST_LINKHELPER_BASE_HH

#include <boost/mpl/list.hpp>

namespace mynamespace
{

   struct LinkHelperBase 
   {
       typedef boost::mpl::list<> FACTORY_SIGNATURE;

      virtual bool returnTrue () const = 0;

   };

}

#endif
