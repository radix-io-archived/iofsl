#include <iostream>
#include "iofwdutil/tools.hh"
#include "iofwdutil/assert.hh"
#include "sm/SMLinks.hh"
#include <boost/mpl/list.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/single_view.hpp>
#include <boost/mpl/empty.hpp>

using namespace std; 
using namespace boost::mpl; 

struct S1 {}; 
struct S2 {}; 
struct S3 {}; 
struct S4 {}; 
struct S5 {}; 
struct S6 {}; 

SM_ALIAS_STATE(ErrorState); 

struct ExampleMachine
{
   typedef S1 INITSTATE; 
}; 

struct EM2
{
   typedef S3 INITSTATE; 
};


/*
 *       S1
 *    /  |  \
 *   S2  S3  S6
 *      /  \
 *     S4  S5 
 */

namespace sm 
{
SM_STATE_CHILDREN_BEGIN(S1)
   S2,
   S3,
   S6
SM_STATE_CHILDREN_END

SM_STATE_CHILDREN_BEGIN(S3)
   S4,
   S5
SM_STATE_CHILDREN_END


SM_MAKE_ALIAS(ExampleMachine,ErrorState,S3);
SM_MAKE_ALIAS(EM2,ErrorState,S5);

}

template <typename M>
void dumpinfo ()
{
   cout << "StateID of S1 " << sm::StateID<M,S1>::value << endl ;
   cout << "StateID of S2 " << sm::StateID<M,S2>::value << endl ;
   cout << "StateID of S3 " << sm::StateID<M,S3>::value << endl ;
   cout << "StateID of S4 " << sm::StateID<M,S4>::value << endl ;
   cout << "StateID of S5 " << sm::StateID<M,S5>::value << endl ;
   cout << "StateID of S6 " << sm::StateID<M,S6>::value << endl ;

   cout << "States in machine: " <<
      sm::MachineStates<M>::state_count << endl; 
   cout << "States in machine: " <<
      size<typename sm::MachineStates<M>::states>::value << endl; 
}


template <typename SM, typename S>
void validateID ()
{
   enum { lookup_id = sm::StateID<SM,S>::value }; 
   typedef typename sm::StateType<SM, lookup_id>::type lookup_type; 
   STATIC_ASSERT((boost::is_same<lookup_type, S>::value)); 
}

/**
 * Make sure alias A in machine SM points to state S
 */
template <typename SM, typename A, typename S>
void validateAlias ()
{
   typedef typename ::sm::StateAlias<SM,A>::type aliastype; 
   STATIC_ASSERT((boost::is_same<aliastype, S>::value)); 
}

int main (int UNUSED(argc), char ** UNUSED(args))
{
   typedef sm::AllChildren<ExampleMachine,list<S4,S5> >::children childlist_S4_S5; 
   typedef sm::AllChildren<ExampleMachine,list<S3> >::children childlist_S3; 
   typedef sm::AllChildren<ExampleMachine,list<S1> >::children childlist_S1; 

   STATIC_ASSERT(size<childlist_S4_S5>::value == 2); 
   STATIC_ASSERT(size<childlist_S3>::value == 3); 
   STATIC_ASSERT(size<childlist_S1>::value == 6); 


   cout << "States in S4,S5: " << size<childlist_S4_S5>::value << endl; 
   cout << "States in S3: " << size<childlist_S3>::value << endl; 
   cout << "States in S1: " << size<childlist_S1>::value << endl; 

   // validate state id mapping in MyMachine
   validateID<ExampleMachine,S1> (); 
   validateID<ExampleMachine,S2> (); 
   validateID<ExampleMachine,S3> (); 
   validateID<ExampleMachine,S4> (); 
   validateID<ExampleMachine,S5> (); 
   validateID<ExampleMachine,S6> (); 

   // Testing alias
   validateAlias<ExampleMachine, ErrorState, S3>(); 
   validateAlias<EM2, ErrorState, S5>(); 


   cout << endl;
   cout << " For machine1:" << endl;
   dumpinfo<ExampleMachine> (); 
   cout << endl;
   cout << " For machine2:" << endl;
   dumpinfo<EM2> (); 

   return 0; 
}
