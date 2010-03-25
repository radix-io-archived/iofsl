#ifndef IOFWDUTIL_FACTORYHELPER_HH
#define IOFWDUTIL_FACTORYHELPER_HH

#include <boost/mpl/size.hpp>
#include <boost/mpl/at.hpp>

namespace iofwdutil
{

   /**
    * Turns typelist into BASE (*) (typelist types) function signature.
    */
template <typename BASE,
size_t COUNT = boost::mpl::size<typename BASE::FACTORY_SIGNATURE>::type::value>
struct FactoryHelper
{
   typedef typename BASE::FACTORY_SIGNATURE ParamList;
};

template <typename BASE>
struct FactoryHelper<BASE, 0>
{
      typedef BASE * (*CONSTFUNC) ();
};


template <typename BASE>
struct FactoryHelper<BASE, 1>
{
   typedef typename BASE::FACTORY_SIGNATURE ParamList;
      typedef BASE * (*CONSTFUNC) (
            typename boost::mpl::at_c<ParamList,0>::type
            );
};

template <typename BASE>
struct FactoryHelper<BASE, 2>
{
   typedef typename BASE::FACTORY_SIGNATURE ParamList;
      typedef BASE * (*CONSTFUNC) (
            typename boost::mpl::at_c<ParamList,0>::type,
            typename boost::mpl::at_c<ParamList,1>::type
            );
};


template <typename BASE>
struct FactoryHelper<BASE, 3>
{
   typedef typename BASE::FACTORY_SIGNATURE ParamList;
      typedef BASE * (*CONSTFUNC) (
            typename boost::mpl::at_c<ParamList,0>::type,
            typename boost::mpl::at_c<ParamList,1>::type,
            typename boost::mpl::at_c<ParamList,2>::type
            );
};

template <typename BASE>
struct FactoryHelper<BASE, 4>
{
   typedef typename BASE::FACTORY_SIGNATURE ParamList;
      typedef BASE * (*CONSTFUNC) (
            typename boost::mpl::at_c<ParamList,0>::type,
            typename boost::mpl::at_c<ParamList,1>::type,
            typename boost::mpl::at_c<ParamList,2>::type,
            typename boost::mpl::at_c<ParamList,3>::type
            );
};


}

#endif
