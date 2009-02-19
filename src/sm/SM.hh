#ifndef SM_SM_HH
#define SM_SM_HH

#include <string>
#include <boost/mpl/if.hpp>
#include <boost/mpl/eval_if.hpp>
#include "iofwdutil/assert.hh"
#include "iofwdutil/tools.hh"

// for debug
#include <iostream>
using namespace std; 


#define SM_INITSTATE(a) \
   struct a : public ::sm::Link<a,::sm::InitBase<a> >

#define SM_SUBSTATE(statename,parentstate) \
   struct statename : public ::sm::Link<statename,parentstate>

// Needed inside the the class of each state
#define SM_STATEDEF(statename) \
     statename (MachineType & machine) : LinkType(machine) {};  \
     const std::string getStateName() const {return std::string(#statename);}\
     const std::string getFullStateName () const \
         {return /*getParent()->getStateName() +*/ std::string("::") \
            +getStateName(); }

namespace sm
{

// Forwards
template <typename T>
class StateMachine; 

//===========================================================================
//===========================================================================
//===========================================================================


template <typename T>
class StateID 
{
public:
   static const int value; 
}; 

//===========================================================================
//===========================================================================
//===========================================================================


/**
 * Default data associated with a state
 * (appears under typename StateData in the state: can be overridden 
 * by the state)
 */
struct EmptyStateData 
{
}; 

//===========================================================================
//===========================================================================
//===========================================================================

/**
 * The INITSTATE derives from this class (and by consequence all other states
 * derive from it)
 */
template <typename STATE>
class InitBase
{
public:
   typedef STATE INITSTATE; 

   typedef STATE PARENTSTATE; 

   // State depth
   enum { DEPTH = 0 }; 

   InitBase (StateMachine<INITSTATE> & mach)
      : machine_(mach)
   {
   }

protected:

   // Called when the machine enters the state
   void enter () {}

   // Called before the machine leaves the state
   void leave () {}

   // Called the first time a state is going to be entered
   void init ()  {}

   virtual ~InitBase () {} ;

protected:
   StateMachine<INITSTATE> & machine_; 


}; 

//===========================================================================
//===========================================================================
//===========================================================================

/**
 * Group data for the state machine
 */

//===========================================================================
//===========================================================================
//===========================================================================
template <typename STATE, typename PARENT>
class Link : public PARENT
{
public:
   typedef PARENT PARENTSTATE;
   typedef typename PARENT::INITSTATE INITSTATE; 

   typedef EmptyStateData StateData; 

   typedef StateMachine<INITSTATE> MachineType; 
   typedef Link<STATE,PARENT> LinkType; 

   enum { DEPTH = PARENT::DEPTH + 1 }; 

public:
   Link (MachineType & machine)
      : PARENTSTATE(machine) 
   {
      // force instantiation of StateID< >
      // ALWAYS_ASSERT(StateID<STATE>::value >= 0); 
   }

public:
   static int ID() { return StateID<STATE>::value; } 

   // The virtual version of ID()
   virtual int getID () const 
   { return ID();  } 

   virtual int getParentID () const 
   { return parentID(); }

   static int parentID () { return StateID<PARENTSTATE>::value; }

protected:

   friend class StateMachine<INITSTATE>; 

   // Make sure we don't inherit from parent
   void enter ()  {}

   void init ()   {} 

   void leave ()   {} 

   template <typename S>
   void setState ()
   {
      PARENT::machine_.setState<STATE,S> (); 
   }

   virtual void moveToTop ()
   {
      if (ID()!=INITSTATE::ID())
         PARENT::machine_.setState<STATE,INITSTATE> (); 
   }

   virtual ~Link ()
   { } 

}; 

//===========================================================================
//===========================================================================
//===========================================================================

template <typename S1, typename S2>
struct SharedParent 
{ 
   typedef typename boost::mpl::if_c<
       ((int)S1::DEPTH < (int)S2::DEPTH), 
       typename SharedParent<S1, typename S2::PARENTSTATE>::type,
       typename SharedParent<typename S1::PARENTSTATE, S2>::type
       >::type temptype; 

   
   typedef typename boost::mpl::if_c<
       ((int)S1::DEPTH == (int)S2::DEPTH), 
       typename SharedParent<typename S1::PARENTSTATE, typename S2::PARENTSTATE>::type,
       temptype
       >::type type; 
}; 

template <typename S>
struct SharedParent<S,S>
{ typedef S type; }; 


// Because both branches of the IF are instantiated, we need to have 
// the following
template <typename S1, typename S2>
struct SharedParent<InitBase<S1>, S2>
{
   typedef S2 type; 
}; 


template <typename S1, typename S2>
struct SharedParent<S1, InitBase<S2> >
{
   typedef S1 type; 
}; 

//===========================================================================
//===========================================================================
//===========================================================================

template <typename START, typename STOP>
struct WalkStatesUp
{
   typedef typename START::PARENTSTATE PARENT; 
    template <template <typename S> class F, typename P>
    static void doWalkPre (P & p)
    {
       F<START> call(p); 
       WalkStatesUp<PARENT,STOP>::template doWalkPre<F>(p);
    }
    template <template <typename S> class F, typename P>
    static void doWalkPost (P & p)
    {
       WalkStatesUp<PARENT,STOP>::template doWalkPost<F> (p); 
       F<START> call(p); 
    }
}; 

template <typename STATE>
struct WalkStatesUp<STATE,STATE>
{
   template <template <typename S> class F, typename P>
   static void doWalkPre (P & )
   { /*cerr << "WSU end" << endl;*/ }

   template <template <typename S> class F, typename P>
   static void doWalkPost (P & )
   { /*cerr << "WSD end" << endl; */}
}; 
//===========================================================================
//===========================================================================
//===========================================================================

template <typename STATE>
struct CallEnter_
{
   CallEnter_ (StateMachine<typename STATE::INITSTATE> & m)
   {
      /*cerr << "CallEnter_ on ID " << STATE::ID() <<  endl;*/ 
      m.template enterState< STATE> (); 
   }
}; 

template <typename STATE>
struct CallLeave_
{
   CallLeave_ (StateMachine<typename STATE::INITSTATE> & m)
   {
      /*cerr << "CallLeave on ID " << STATE::ID() <<  endl;*/ 
      m.template leaveState<STATE> (); 
   }
}; 


template <typename INITSTATE>
class StateMachine 
{
protected:
   class RunAfter; 

public:
   typedef INITSTATE INIT; 
   typedef INITSTATE * StatePtr; 

   StateMachine ();


   ~StateMachine ();

   /**
    * Return the number of states in this state machine
    */
   static int getStateCount ();

   RunAfter operator -> ()
   { return RunAfter(*this); } 

protected:
   class RunAfter 
   {
      RunAfter (StateMachine<INITSTATE> & me)
         : me_(me)
      {
      }

      INITSTATE * operator -> () 
      {
         ASSERT (currentState_>=0); 
         ASSERT (states_[currentState_]); 
         return states_[currentState_]; 
      }
      
      ~RunAfter ()
      { me_.process (); }

      protected:
         StateMachine<INITSTATE> & me_; 
   }; 

   template <typename S>
   void enterState ()
   { 
      /*cerr << "M::enterState ID " << S::ID() << endl; */
      currentState_ = S::ID(); 
      static_cast<S &>(*states_[S::ID()]).enter (); 
   }

   template <typename S>
   void leaveState ()
   { 
      /*cerr << "M::leaveState ID " << S::ID() << endl; */
      
      // The following does not hold when we move from 
      // from a state to its parent because we do not adjust
      // currentState while going up to the parent
      // ASSERT(currentState_ == S::ID()); 

      static_cast<S&>(*states_[S::ID()]).leave (); 
   }

   template <typename S>
   void ensureStateExists ()
   {
      if (states_[S::ID()])
         return;
      /*cerr << "Creating state " << typeid(S).name() <<
         " (ID " << S::ID() << ")" << endl; */
      states_[S::ID()] = new S(*this); 
   }

   StatePtr getCurrentState () 
   {
      return states_[currentState_]; 
   }

   StatePtr getState (int ID)
   {
      return states_[ID]; 
   }
   /**
    * Called to set the initial state
    */
   template <typename S>
   void initState  ()
   {
      ensureStateExists<S> ();
      currentState_ = S::ID(); 

      // We can avoid a virtual function call here
      static_cast<S&>(*getCurrentState()).enter (); 

      process (); 
   }

   /**
    * This function is generated for every possible state transition
    */
   template <typename CURRENT, typename NEXT>
   void makeTransition ()
   {
      /*cerr << "makeTransition from " << CURRENT::ID()
         << " to " << NEXT::ID() << endl; */
      ASSERT(CURRENT::ID() == currentState_); 

      ensureStateExists<NEXT> (); 

      typedef typename SharedParent<CURRENT,NEXT>::type CommonParent; 


      // Generate leave events from the current state up to the first shared
      // parent state
      WalkStatesUp<CURRENT,CommonParent>::template doWalkPre<CallLeave_> (*this); 

      currentState_ = CommonParent::ID(); 

      // Generate leave events from the first shared parent down to the
      // new state
      WalkStatesUp<NEXT,CommonParent>::template doWalkPost<CallEnter_> (*this); 

      ASSERT(currentState_ == NEXT::ID()); 
   }

   template <typename CURRENT,typename NEXT>
   void setState ()
   {
      /*cerr << "Machine::setState called from state " << CURRENT::ID() << 
         " to " << NEXT::ID() << endl;*/ 
      // Can only be called on valid state machine
      ALWAYS_ASSERT(currentState_ >= 0); 
      
      // Check that the state that called us is actually the active state
      ALWAYS_ASSERT(currentState_ == CURRENT::ID());

      // Make sure setState wasn't called before
      ALWAYS_ASSERT(nextState_ < 0); 

      nextState_ = NEXT::ID(); 
      transitionFunc_ = &StateMachine<INITSTATE>::template makeTransition<CURRENT,NEXT>; 
   }

   // Start the machine by entering the init state
   void start (); 

   // Actually moves to the  new state
   void changeState ()
   {
      nextState_ = -1; 
      (this->*transitionFunc_) (); 
   }


   void process ()
   {
      // See if we need to do something
      while (nextState_ >= 0)
      {
         /*cerr << "Process: nextState = " << nextState_ << endl; */
         changeState (); 
         ASSERT(getCurrentState()->getID() == currentState_); 
      /*cerr << "In state " << getCurrentState()->getID() << ", next state=" <<
         nextState_ << endl; */
      }
   }


   // Called before the state machine is destroyed.
   // Leaves all the states
   void finish ()
   {
      getCurrentState()->moveToTop (); 
      process (); 
      // now should be in top state
      ASSERT(currentState_ == INITSTATE::ID()); 

      // Leave the init state
      leaveState<INITSTATE> (); 

      // Make sure the top event didn't try to push us back into another
      // state.
      ALWAYS_ASSERT(nextState_ < 0); 

      currentState_ = -1; 
   }
protected:

   // Allow states to access our state count
   template <typename STATE>
   friend class StateID; 

   // Allow Link to access setState
   template <typename S1, typename S2>
   friend class Link; 

   // Code generation to call enter and leave functions
   template <typename S>
   friend class CallEnter_; 
   
   template <typename S>
   friend class CallLeave_; 


   static int stateCount_; 

protected:

   // States of the machine (by StateID)
   // Since all states derive from our INITSTATE, we use that as
   // base pointer
   StatePtr * states_; 

   int currentState_; 
   int nextState_; 

   void (StateMachine::*transitionFunc_) (); 
};

//===========================================================================

template <typename INITSTATE>
int StateMachine<INITSTATE>::stateCount_ = 0; 

//===========================================================================

template <typename T>
void StateMachine<T>::start ()
{
   initState<T>(); 
}

template <typename T>
int StateMachine<T>::getStateCount ()
{
   return stateCount_; 
}

template <typename T>
StateMachine<T>::StateMachine ()
   : currentState_(-1), nextState_(-1)
{
   // Allocate array for states
   states_ = new StatePtr[stateCount_];  
   for (int i=0; i<stateCount_; ++i)
      states_[i]=0; 

   // start machine
   start (); 
}

template <typename T>
StateMachine<T>::~StateMachine ()
{
   // Move to top state
   finish (); 

   // Free states
   for (int i=0; i<stateCount_; ++i)
   {
      delete (states_[i]); 
   }
   delete[] (states_); 
}



//===========================================================================
//===========================================================================
//===========================================================================

// Make sure we increment the state count for every state make a child of the
// top state (and thus of the state machine)
template <typename T>
const int StateID<T>::value = 
        StateMachine<typename T::INITSTATE>::stateCount_++; 

//===========================================================================
//===========================================================================
//===========================================================================


}

#endif
