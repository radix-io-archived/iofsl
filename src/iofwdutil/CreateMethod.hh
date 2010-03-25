#ifndef IOFWDUTIL_CREATEMETHOD_HH
#define IOFWDUTIL_CREATEMETHOD_HH

#include <boost/mpl/size.hpp>
#include "FactoryHelper.hh"

namespace iofwdutil
{
//===========================================================================

/**
* The purpose of this class is to provide a
* static 'create' method, given a Factory compatible DERIVED
* class.
*
* Parameters are as references
* to the constructor of DERIVED.
*/
template <typename BASE, typename DERIVED,
size_t COUNT = boost::mpl::size<typename BASE::FACTORY_SIGNATURE>::type::value>
struct CreateMethod
{
};


template <typename BASE, typename DERIVED>
struct CreateMethod<BASE,DERIVED,0>
{
   static BASE * create ()
   { return new DERIVED(); }
};

template <typename BASE, typename DERIVED>
struct CreateMethod<BASE,DERIVED,1>
{
   typedef typename BASE::FACTORY_SIGNATURE ParamList;
   static BASE * create (
      typename boost::mpl::at_c<ParamList,0>::type p1
         )
   { return new DERIVED (p1); }
};

template <typename BASE, typename DERIVED>
struct CreateMethod<BASE,DERIVED,2>
{
   typedef typename BASE::FACTORY_SIGNATURE ParamList;
   static BASE * create (
      typename boost::mpl::at_c<ParamList,0>::type p1,
      typename boost::mpl::at_c<ParamList,1>::type p2
         )
   { return new DERIVED (p1,p2); }
};

template <typename BASE, typename DERIVED>
struct CreateMethod<BASE,DERIVED,3>
{
   typedef typename BASE::FACTORY_SIGNATURE ParamList;
   static BASE * create (
      typename boost::mpl::at_c<ParamList,0>::type p1,
      typename boost::mpl::at_c<ParamList,1>::type p2,
      typename boost::mpl::at_c<ParamList,2>::type p3
         )
   { return new DERIVED (p1,p2,p3); }
};


template <typename BASE, typename DERIVED>
struct CreateMethod<BASE,DERIVED,4>
{
   typedef typename BASE::FACTORY_SIGNATURE ParamList;
   static BASE * create (
      typename boost::mpl::at_c<ParamList,0>::type p1,
      typename boost::mpl::at_c<ParamList,1>::type p2,
      typename boost::mpl::at_c<ParamList,2>::type p3,
      typename boost::mpl::at_c<ParamList,3>::type p4
         )
   { return new DERIVED (p1,p2,p3,p4); }
};


//===========================================================================
}

#endif

