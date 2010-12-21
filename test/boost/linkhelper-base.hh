#ifndef TEST_BOOST_LINKHELPER_BASE_HH
#define TEST_BOOST_LINKHELPER_BASE_HH

namespace mynamespace
{

   struct LinkHelperBase 
   {
       FACTORY_CONSTRUCTOR_PARAMS();

      virtual bool returnTrue () const = 0;

   };

}

#endif
