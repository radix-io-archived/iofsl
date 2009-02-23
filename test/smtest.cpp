#include <iostream>
#include "iofwdutil/tools.hh"
#include "iofwdutil/assert.hh"
#include "sm/SM.hh"

using namespace std;


// Can have a different namespace for every state machine
// (so that state names do not need to be unique)
namespace testsm
{

SM_STATE(test1)
{
   SM_STATEDEF(test1);

   public: 
    void enter ();
    void leave ();
};

SM_STATE(test2)
{
   SM_STATEDEF(test2);

   public: 
    void enter ();
    void leave ();
}; 

SM_STATE(test3)
{
   SM_STATEDEF(test3);

   public: 
    void enter ();
    void leave ();
}; 

SM_STATE(test4)
{
   SM_STATEDEF(test4);
   public: 
    void enter ();
    void leave ();
}; 


template <typename SMDEF>
void test1<SMDEF>::enter() 
{ 
   cout << "Entering " << this->getStateName() << " (stateid " <<
      this->ID() << ") " << endl; 
   this->template setState<test2>(); 
}

template <typename SMDEF>
void test2<SMDEF>::enter() 
{ 
   cout << "Entering " << this->getStateName() << " (stateid " <<
      this->ID() << ") " << endl; 
   this->template setState<test3>(); 
}

template <typename SMDEF>
void test3<SMDEF>::enter() 
{ 
   cout << "Entering " << this->getStateName() << " (stateid " <<
      this->ID() << ") " << endl; 
   this->template setState<test4>(); 
}

template <typename SMDEF>
void test4<SMDEF>::enter() 
{ 
   cout << "Entering " << this->getStateName() << " (stateid " <<
      this->ID() << ") " << endl; 
}

template <typename SMDEF>
void test1<SMDEF>::leave() { cout << "Leaving " << this->getStateName() << endl; }

template <typename SMDEF>
void test2<SMDEF>::leave() { cout << "Leaving " << this->getStateName() << endl; }

template <typename SMDEF>
void test3<SMDEF>::leave() { cout << "Leaving " << this->getStateName() << endl; }

template <typename SMDEF>
void test4<SMDEF>::leave() { cout << "Leaving " << this->getStateName() << endl; }


// Machinetype can go anywhere
SM_MACHINETYPE(test,test1);

SM_MACHINETYPE(subtest,test3);
}


// State machine links need to be defined within the sm namespace
namespace sm
{
SM_STATE_CHILDREN_BEGIN(testsm::test1)
   SM_STATE_CHILD(testsm::test2),
   SM_STATE_CHILD(testsm::test3)
SM_STATE_CHILDREN_END

SM_STATE_CHILDREN_BEGIN(testsm::test3)
   SM_STATE_CHILD(testsm::test4)
SM_STATE_CHILDREN_END

}


int main (int UNUSED(argc), char ** UNUSED(args))
{
   typedef sm::StateMachine<testsm::test> MyMachine; 
   typedef sm::StateMachine<testsm::subtest> SubMachine; 

   cout << "Number of states in state machine: " << MyMachine::getStateCount ()
      << endl; 
 
   cout << endl; 

   {
      MyMachine machine;

      cout << "Calling run...\n"; 
      machine.run (); 
   }

   cout << "Running submachine\n";
   {
      SubMachine machine;
      machine.run (); 
   }

   return EXIT_SUCCESS; 
}
