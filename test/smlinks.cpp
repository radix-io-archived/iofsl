#include <iostream>
#include "iofwdutil/tools.hh"
#include "iofwdutil/assert.hh"
#include "sm/SM.hh"
#include <boost/mpl/size.hpp>

using namespace std; 
using namespace boost::mpl; 

SM_STATE(S1) 
{
   SM_STATEDEF;

   void enter ()
   {
      cout << "Entering S1\n"; 
   }

   void leave ()
   {
      cout << "Leaving S1\n"; 
   }

}; 

SM_STATE(S2) 
{
   SM_STATEDEF;
   void enter ()
   {
      cout << "Entering S2\n"; 
   }
   void leave ()
   {
      cout << "Leaving S2\n"; 
   }

}; 

SM_STATE(S3) 
{
   SM_STATEDEF;
   void enter ()
   {
      cout << "Entering S3\n"; 
   }
   void leave ()
   {
      cout << "Leaving S3\n"; 
   }

};

SM_STATE(S4) 
{
   SM_STATEDEF;
   void enter ()
   {
      cout << "Entering S4\n"; 
   }
   void leave ()
   {
      cout << "Leaving S4\n"; 
   }

}; 

SM_STATE(S5) 
{
   SM_STATEDEF;
   void enter ()
   {
      cout << "Entering S5\n"; 
   }

   void leave ()
   {
      cout << "Leaving S5\n"; 
   }

}; 

SM_STATE(S6) 
{
   SM_STATEDEF;
   void enter ()
   {
      cout << "Entering S6\n"; 
   }
   void leave ()
   {
      cout << "Leaving S6\n"; 
   }

}; 

SM_ALIAS_STATE(ErrorState); 

SM_MACHINETYPE(ExampleMachine, S1);

SM_MACHINETYPE(EM2, S3); 


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
   SM_STATE_CHILD(S2),
   SM_STATE_CHILD(S3),
   SM_STATE_CHILD(S6)
SM_STATE_CHILDREN_END

SM_STATE_CHILDREN_BEGIN(S3)
   SM_STATE_CHILD(S4),
   SM_STATE_CHILD(S5)
SM_STATE_CHILDREN_END


SM_MAKE_ALIAS(ExampleMachine,ErrorState,S3);
SM_MAKE_ALIAS(EM2,ErrorState,S5);

}

template <typename M>
void dumpinfo ()
{
   cout << "StateID of S1 " << sm::StateID<S1<M> >::value << endl ;
   cout << "StateID of S2 " << sm::StateID<S2<M> >::value << endl ;
   cout << "StateID of S3 " << sm::StateID<S3<M> >::value << endl ;
   cout << "StateID of S4 " << sm::StateID<S4<M> >::value << endl ;
   cout << "StateID of S5 " << sm::StateID<S5<M> >::value << endl ;
   cout << "StateID of S6 " << sm::StateID<S6<M> >::value << endl ;

   cout << "Depth of S1 " << sm::StateDepth<S1<M> >::value << endl; 
   cout << "Depth of S2 " << sm::StateDepth<S2<M> >::value << endl; 
   cout << "Depth of S3 " << sm::StateDepth<S3<M> >::value << endl; 
   cout << "Depth of S4 " << sm::StateDepth<S4<M> >::value << endl; 
   cout << "Depth of S5 " << sm::StateDepth<S5<M> >::value << endl; 

   cout << "States in machine: " <<
      sm::MachineStates<M>::state_count << endl; 
   cout << "States in machine: " <<
      size<typename sm::MachineStates<M>::states>::value << endl; 
}


template <typename SM, template <typename> class S>
void validateID ()
{
   enum { lookup_id = sm::StateID<S<SM> >::value }; 
   typedef typename sm::StateType<SM, lookup_id>::type lookup_type; 
   STATIC_ASSERT((boost::is_same<lookup_type, S<SM> >::value)); 
}

/**
 * Make sure alias A in machine SM points to state S
 */
template <typename SM, typename A, template <typename> class S>
void validateAlias ()
{
   typedef typename ::sm::StateAlias<SM,A>::type aliastype; 
   STATIC_ASSERT((boost::is_same<aliastype, S<SM> >::value)); 
}

/**
 * Validate that P is a parent state of C in SM
 */
template <typename SM, template <typename> class P, template <typename> class C>
void validateDirectParent ()
{
 /*  typedef typename sm::StateDirectParent<SM,C>::type parent; 
   cout << "Parent state of ID " << sm::StateID<SM,C>::value 
      << " is " << typeid(parent).name() << endl; */
      
   STATIC_ASSERT((boost::is_same<
             typename sm::StateDirectParent<SM, C>::type,
             P<SM>
            >::value));  
}

template <typename SMDEF, template <typename> class S, int DEPTH>
void validateDepth ()
{
   cout << "Depth of " << S<SMDEF>::getStateName() 
      << " in machine " << SMDEF::getMachineName () <<
      ": " << sm::StateDepth< S<SMDEF> >::value << endl; 
   STATIC_ASSERT((sm::StateDepth< S<SMDEF> >::value == DEPTH)); 
}


template <typename EM, template <typename> class S1, 
         template <typename> class S2, 
         template <typename> class P>
void validateSharedParent ()
{
   typedef typename sm::SharedParent< S1<EM>, S2<EM> >::type SP; 

   STATIC_ASSERT((boost::is_same <SP, P<EM> >::value)); 

   cout << "Shared parent of " << S1<EM>::getStateName () 
      << " and " << S2<EM>::getStateName () 
      << " is " << SP::getStateName () << endl; 
}


int main (int UNUSED(argc), char ** UNUSED(args))
{
   typedef sm::AllChildren<ExampleMachine,list<S4<ExampleMachine>,S5<ExampleMachine> > >::children childlist_S4_S5; 
   typedef sm::AllChildren<ExampleMachine,list<S3<ExampleMachine> > >::children childlist_S3; 
   typedef sm::AllChildren<ExampleMachine,list<S1<ExampleMachine> > >::children childlist_S1; 

   STATIC_ASSERT(size<childlist_S4_S5>::value == 2); 
   STATIC_ASSERT(size<childlist_S3>::value == 3); 
   STATIC_ASSERT(size<childlist_S1>::value == 6); 

   cout << "\n\
       S1\n\
    /  |  \\\n\
   S2  S3  S6\n\
      /  \\\n\
     S4  S5\n";

   cout << "States in submachine S4,S5: " << size<childlist_S4_S5>::value << endl; 
   cout << "States in submacihne S3: " << size<childlist_S3>::value << endl; 
   cout << "States in submachine S1: " << size<childlist_S1>::value << endl; 


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
   
   // Validate parent links
   // Check if SMDEF is parent of INITSTATE
   STATIC_ASSERT((boost::is_same<
             sm::StateDirectParent<ExampleMachine, S1>::type,
             ExampleMachine
            >::value));  
   validateDirectParent<ExampleMachine, S1, S2>(); 
   validateDirectParent<ExampleMachine, S1, S3>(); 
   validateDirectParent<ExampleMachine, S1, S6>(); 
   validateDirectParent<ExampleMachine, S3, S4>(); 
   validateDirectParent<ExampleMachine, S3, S5>(); 


   validateDepth<ExampleMachine, S1, 0>(); 
   validateDepth<ExampleMachine, S2, 1>(); 
   validateDepth<ExampleMachine, S3, 1>(); 
   validateDepth<ExampleMachine, S4, 2>(); 
   validateDepth<ExampleMachine, S5, 2>(); 
   
   validateDepth<EM2, S3, 0>(); 
   validateDepth<EM2, S4, 1>(); 
   validateDepth<EM2, S5, 1>(); 

   validateSharedParent<ExampleMachine, S1, S1, S1>(); 
   validateSharedParent<ExampleMachine, S1, S2, S1>(); 
   validateSharedParent<ExampleMachine, S1, S3, S1>(); 
   validateSharedParent<ExampleMachine, S1, S4, S1>(); 
   validateSharedParent<ExampleMachine, S1, S5, S1>(); 

   validateSharedParent<ExampleMachine, S2, S1, S1>(); 
   validateSharedParent<ExampleMachine, S3, S1, S1>(); 
   validateSharedParent<ExampleMachine, S4, S1, S1>(); 
   validateSharedParent<ExampleMachine, S5, S1, S1>(); 
   validateSharedParent<ExampleMachine, S6, S1, S1>(); 

   validateSharedParent<ExampleMachine, S2, S3, S1>(); 
   validateSharedParent<ExampleMachine, S2, S4, S1>(); 
   validateSharedParent<ExampleMachine, S2, S5, S1>(); 
   validateSharedParent<ExampleMachine, S2, S6, S1>(); 

   validateSharedParent<ExampleMachine, S3, S3, S3>(); 
   validateSharedParent<ExampleMachine, S3, S4, S3>(); 
   validateSharedParent<ExampleMachine, S3, S5, S3>(); 
   validateSharedParent<ExampleMachine, S3, S6, S1>(); 
   
   validateSharedParent<ExampleMachine, S4, S4, S4>(); 
   validateSharedParent<ExampleMachine, S4, S5, S3>(); 
   validateSharedParent<ExampleMachine, S4, S6, S1>(); 
   
   validateSharedParent<ExampleMachine, S5, S5, S5>(); 
   validateSharedParent<ExampleMachine, S5, S6, S1>(); 

   validateSharedParent<EM2, S3, S3, S3>(); 
   validateSharedParent<EM2, S3, S4, S3>(); 
   validateSharedParent<EM2, S3, S5, S3>(); 
   
   validateSharedParent<EM2, S4, S4, S4>(); 
   validateSharedParent<EM2, S4, S5, S3>(); 
   
   validateSharedParent<EM2, S5, S5, S5>(); 

   cout << endl;
   cout << " For machine1:" << endl;
   dumpinfo<ExampleMachine> (); 
   cout << endl;
   cout << " For machine2:" << endl;
   dumpinfo<EM2> (); 


   cout << endl; 
   cout << "===================================================\n"; 
   cout << endl; 


   sm::StateMachine<ExampleMachine> machine;

   cout << "States in machine: " << machine.getStateCount () << endl; 


   return 0; 
}
