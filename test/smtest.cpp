#include <iostream>
#include "iofwdutil/tools.hh"
#include "iofwdutil/assert.hh"
#include <boost/type_traits/is_same.hpp>
#include "sm/SMLinks.hh"

using namespace std;

namespace testsm
{

SM_INITSTATE(test)
{
   SM_STATEDEF(test)

   public: 
    void enter ();
    void leave ();
};

SM_SUBSTATE(test2, test)
{
   SM_STATEDEF(test2)
   public: 
    void enter ();
    void leave ();
}; 

SM_SUBSTATE(test3, test2)
{
   SM_STATEDEF(test3)
   public: 
    void enter ();
    void leave ();
}; 

SM_SUBSTATE(test4, test2)
{
   SM_STATEDEF(test4)
   public: 
    void enter ();
    void leave ();
}; 


void test::enter() 
{ 
   cout << "Entering " << getStateName() << " (stateid " <<
      ID() << ") " << endl; 
   setState<test2>(); 
}

void test2::enter() 
{ 
   cout << "Entering " << getStateName() << " (stateid " <<
      ID() << ") " << endl; 
   setState<test3>(); 
}

void test3::enter() 
{ 
   cout << "Entering " << getStateName() << " (stateid " <<
      ID() << ") " << endl; 
   setState<test4>(); 
}

void test4::enter() 
{ 
   cout << "Entering " << getStateName() << " (stateid " <<
      ID() << ") " << endl; 
}

void test::leave() { cout << "Leaving " << getStateName() << endl; }
void test2::leave() { cout << "Leaving " << getStateName() << endl; }
void test3::leave() { cout << "Leaving " << getStateName() << endl; }
void test4::leave() { cout << "Leaving " << getStateName() << endl; }


}

#define NEED_SHARED_DEPTH(a,b,c) \
   STATIC_ASSERT((::sm::SharedParent<a,b>::type::DEPTH == c));

int main (int UNUSED(argc), char ** UNUSED(args))
{
   typedef sm::StateMachine<testsm::test> MyMachine; 

   STATIC_ASSERT(sizeof(TP_IF<char[10],char[20],(2>3)>::value)==20);
   STATIC_ASSERT(sizeof(TP_IF<char[10],char[20],(2<3)>::value)==10);
   STATIC_ASSERT(sizeof(TP_IF<char[10],char[20],((int)testsm::test3::DEPTH > 
               (int) testsm::test4::DEPTH)>::value) == 20); 
   STATIC_ASSERT(sizeof(TP_IF<char[10],char[20],((int)testsm::test3::DEPTH < 
               (int) testsm::test4::DEPTH)>::value) == 20); 


   STATIC_ASSERT((int)testsm::test3::DEPTH == (int)testsm::test4::DEPTH); 
   STATIC_ASSERT((int)testsm::test::DEPTH < (int)testsm::test2::DEPTH); 
   STATIC_ASSERT((int)testsm::test::DEPTH < (int)testsm::test3::DEPTH); 
   STATIC_ASSERT((int)testsm::test::DEPTH < (int)testsm::test4::DEPTH); 
   STATIC_ASSERT((int)testsm::test2::DEPTH < (int)testsm::test3::DEPTH); 


   NEED_SHARED_DEPTH(testsm::test, testsm::test, 1); 
   NEED_SHARED_DEPTH(testsm::test, testsm::test2, 1); 
   NEED_SHARED_DEPTH(testsm::test, testsm::test3, 1); 
   NEED_SHARED_DEPTH(testsm::test, testsm::test4, 1); 

   NEED_SHARED_DEPTH(testsm::test2, testsm::test, 1); 
   NEED_SHARED_DEPTH(testsm::test3, testsm::test, 1); 
   NEED_SHARED_DEPTH(testsm::test4, testsm::test, 1); 

   NEED_SHARED_DEPTH(testsm::test2, testsm::test3, 2); 
   NEED_SHARED_DEPTH(testsm::test3, testsm::test2, 2); 
   
   NEED_SHARED_DEPTH(testsm::test3, testsm::test4, 2); 
   NEED_SHARED_DEPTH(testsm::test4, testsm::test3, 2);  

   cout << "Depth of test: " << testsm::test::DEPTH << endl; 
   cout << "Depth of test2: " << testsm::test2::DEPTH << endl; 
   cout << "Depth of test3: " << testsm::test3::DEPTH << endl; 
   cout << "Depth of test4: " << testsm::test4::DEPTH << endl; 

   cout << "Shared depth of 3 and 2: " << 
      sm::SharedParent<testsm::test3,testsm::test2>::type::DEPTH << endl; 
   cout << "Shared depth of 1 and 4: " << 
      sm::SharedParent<testsm::test,testsm::test4>::type::DEPTH << endl; 
   cout << "Shared depth of 4 and 1: " << 
      sm::SharedParent<testsm::test4,testsm::test>::type::DEPTH << endl; 

   cout << "Number of states in state machine: " << MyMachine::getStateCount ()
      << endl; 
 
   cout << endl; 
   {
      //const int initparam = 3; 
   MyMachine machine;

   //machine.start (); 
   //machine->event1(); 
   }

   return EXIT_SUCCESS; 
}
