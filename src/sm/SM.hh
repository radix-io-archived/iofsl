// NOTE: data storage: see in place construction boost library (in place
// factories)


#ifndef SM_SMLINKS_HH
#define SM_SMLINKS_HH

#include <typeinfo>

#include <boost/mpl/list.hpp>
#include <boost/mpl/joint_view.hpp>
#include <boost/mpl/single_view.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/push_front.hpp>
#include <boost/mpl/order.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/lambda.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/quote.hpp>
#include <boost/mpl/count.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/size.hpp>

#include <boost/array.hpp>

#include <string>

#include "iofwdutil/demangle.hh"
#include "iofwdutil/assert.hh"

// for debugging
#include <iostream>


/**
 * TODO:
 *    evaluate this and boost::FiniteChart based on
 *
 *       * are states templates or not (ease of use)
 *       * exception handling (and resource leaks)
 *       * can state machines reuse collections of states / be nested
 *       * how easy is it to deliver events to the state machine.
 *          (also: deal with non-blocking resources / resources that 
 *            complete immediately)
 */

using namespace std; 


namespace sm
{
//===========================================================================  


// Forward
template <typename INITSTATE>
class StateMachine; 

//===========================================================================
//====== Machine Definition =================================================
//===========================================================================

/**
 * SM_MACHINETYPE creates a struct with the following members:
 *   - a template INITSTATE_TYPE<...> which gives access to the template type 
 *     of the INITSTATE
 *   - INITSTATE, the initial state templatized with the machinetype.
 */

#define SM_MACHINETYPE(machinename,initstate) \
  struct machinename \
  { \
     template <typename T> \
     struct INITSTATE_TYPE { typedef initstate<T> type; }; \
     typedef initstate<machinename> INITSTATE; \
     typedef machinename SMDEF; \
     static const char * getMachineName () { return #machinename; }\
  }

//===========================================================================
//========== State Children =================================================
//===========================================================================
// By default, a node has no children
template <typename SMDEF, typename N>
struct StateChildList { typedef boost::mpl::list<>::type children; }; 


//
// NOTE: this macro needs to be used within the 'sm' namespace
// 
#define SM_STATE_CHILDREN_BEGIN(S) \
    template <typename SMDEF> \
   struct StateChildList<SMDEF,S<SMDEF> > { typedef boost::mpl::list<

#define SM_STATE_CHILD(a) a<SMDEF> 
#define SM_STATE_CHILDREN_END \
      > children; }; 

// ISO C does not support variadic macro's
//#define SM_STATE_CHILDREN(P,C...) 
//   SM_STATE_CHILDREN_BEGIN(P) C SM_STATE_CHILDREN_END


// Get children of state without specifying SMDEF
template <typename S>
struct IStateChildList
{
   typedef typename StateChildList<typename S::SMDEF, S>::children 
       children; 
};

//===========================================================================  
//===========================================================================  
//===========================================================================  
#define SM_STATE(statename) \
   template <typename SMDEF_> \
   struct statename : public ::sm::StateFunc<SMDEF_, statename<SMDEF_> >

#define SM_STATEDEF(statename) \
protected:\
   friend class ::sm::StateMachine<SMDEF_>; \
   statename (::sm::StateMachine<SMDEF_> & sm) :\
      ::sm::StateFunc<SMDEF_,statename<SMDEF_> >(sm) {} \
   typedef ::sm::StateMachine<SMDEF_> STATEMACHINE; \
   typedef typename SMDEF_::INITSTATE INITSTATE; \
 public:\
   typedef SMDEF_                     SMDEF



//===========================================================================
//===========================================================================
//===========================================================================

typedef boost::mpl::pop_front<boost::mpl::list<int>::type>::type ENDITR;


/*
 * Merge 2 lists
 */
template <typename L1, typename L2>
struct merge_lists
{
   typedef typename boost::mpl::pop_front<L1>::type L1_remainder; 
   typedef typename boost::mpl::front<L1>::type L1_front; 

   typedef 
      typename boost::mpl::push_front<typename merge_lists<L1_remainder, L2>::type, 
                L1_front>::type type; 
}; 

template <>
struct merge_lists<ENDITR,ENDITR>
{
   typedef boost::mpl::list<>::type type; 
}; 

template <typename L2>
struct merge_lists<ENDITR, L2>
{
   typedef L2 type; 
}; 

template <typename L1>
struct merge_lists<L1, ENDITR>
{
   typedef L1 type; 
}; 


//===========================================================================
//===========================================================================
//===========================================================================

// Given a list of states, count all of the states and substates in the list
//
// Separate L into  
//    - list_head
//    - list_remainder
// 
// Set our_children to the childlist of list_head
//
// * Add list_head to the result list obtained by 
//    getting AllChildren on list_remainder + our_children
// 
template <typename SMDEF, typename L>
struct AllChildren
{
   typedef typename boost::mpl::front<L>::type list_head;
   typedef typename boost::mpl::pop_front<L>::type list_remainder; 

   // The list of our childern
   typedef typename StateChildList<SMDEF, list_head>::children our_children; 

   // our_children + list_remainder
   typedef typename merge_lists<our_children,list_remainder>::type combined_list; 

   // The list of children of the remainder of the argument list
   typedef typename AllChildren<SMDEF,combined_list>::children children_list_remainder;

   typedef typename boost::mpl::push_front<children_list_remainder,
           list_head>::type children;

}; 

template <typename SMDEF>
struct AllChildren<SMDEF,ENDITR>
{
   typedef boost::mpl::list<>::type children; 
}; 

//===========================================================================
//===========================================================================
//===========================================================================


// Contains a set of all the states in the machine
template <typename SMDEF>
struct MachineStates 
{
   // THe set of all states of a state machine
   typedef typename AllChildren<
      SMDEF,typename boost::mpl::list<typename SMDEF::INITSTATE>::type >::children states; 

   enum { state_count = boost::mpl::size<states>::type::value }; 
}; 

//===========================================================================
//===========================================================================
//===========================================================================


/**
 * Checks if C is a direct child of the state passed to apply
 * Note: apply is passed the state instantiated for SMDEF
 *
 * NOTE: This does not check if SMDEF::INITSTATE equals C<SMDEF>
 */
template <typename S>
struct IsDirectParentPred
{
   
   template <typename P>
   struct apply 
   {
      typedef typename IStateChildList<P>::children searchlist; 
      typedef S searchitem; 
      enum { value = boost::mpl::count<searchlist,searchitem>::value  }; 
      typedef typename boost::mpl::bool_<value>::type type; 
   }; 
};
/**
 * Determine direct parent of a state in SMDEF by going through the state list 
 * and finding which state has the specified state in its child list.
 *
 * If S<SMDEF> == SMDEF::INITSTATE returns SMDEF
 */

// Helper to make sure we don't try to deref end()
template <typename T>
struct tryderef
{
   typedef typename boost::mpl::deref<T>::type type; 
}; 

template <>
struct tryderef<boost::mpl::end<boost::mpl::list<> >::type>
{
   typedef void type; 
};

template <typename SMDEF, template <typename> class S>
struct StateDirectParent
{
   typedef typename MachineStates<SMDEF>::states searchlist; 
   typedef typename boost::mpl::find_if<
               searchlist,
               IsDirectParentPred<S<SMDEF> > 
              >::type parentiter; 
   typedef typename tryderef<parentiter>::type realparent;

   typedef typename boost::mpl::if_<
                boost::is_same< S<SMDEF>, typename SMDEF::INITSTATE>,
                SMDEF,
                realparent
               >::type type; 

}; 

template <typename S>
struct IStateDirectParent
{
   typedef typename MachineStates<typename S::SMDEF>::states searchlist; 
   typedef typename boost::mpl::find_if<
               searchlist,
               IsDirectParentPred<S> 
              >::type parentiter; 
   typedef typename tryderef<parentiter>::type realparent;

   typedef typename boost::mpl::if_<
                boost::is_same< S, typename S::SMDEF::INITSTATE>,
                typename S::SMDEF,
                realparent
               >::type type; 

}; 

// template <typename SM, 

/*template <typename SM, typename STATE>
struct ParentLink
{
   // By default, unless specified otherwise, a state is a child of the
   // statemachine.
   typedef SM PARENT; 
}; 


// MAKE_PARENT adds a parent link to a node
#define SM_MAKE_PARENT(P,C) \
  template <typename SM> \
  struct ::sm::ParentLink<SM,C> { typedef P PARENT; } ; \
*/


//===========================================================================  
//===========================================================================  
//===========================================================================  

// Return position in of E in sequence (starting from 0)

template <typename L, typename E, int P=0>
struct findpos 
{
   typedef typename boost::mpl::front<L>::type front_element; 
   typedef typename boost::mpl::pop_front<L>::type list_remainder; 

   enum { value = (boost::is_same<front_element,E>::value ? 
                      P : findpos<list_remainder, E, P+1>::value) }; 
}; 

template <typename E, int P>
struct findpos<ENDITR,E,P>
{
   enum { value = -1 }; 
};

/*// Return the type of the state with given ID in state machine SM
template <typename SM, int P, typename L = MachineStates<SM>::children>
struct findpos
{
   typedef typename boost::mpl::pop_front<L>::type list_remainder;
   typedef typename findpos<SM, P-1, list_remainder>::type type; 
};

template <typename SM, typename L>
struct findpos<SM, 0, L>
{
   typedef typename boost::mpl::front<L>::type type; 
}; */

//===========================================================================  
//====== State Numbering ====================================================  
//===========================================================================  

// Note: a state does only have an ID in relation to a specific SMDef
// structure

// Same but for an instantiated state SI
template <typename SI>
struct StateID
{
   typedef typename SI::SMDEF SMDEF; 
   enum { value = findpos<typename MachineStates<SMDEF>::states, SI >::value }; 
};  

//===========================================================================  
//====== State Type =========================================================  
//===========================================================================  

/**
 * For a given state machine and ID, return the associated state type
 */

template <typename SM, int P>
struct StateType
{
   typedef typename boost::mpl::at<
              typename MachineStates<SM>::states,
              boost::mpl::int_<P> 
             >::type type; 
}; 


//===========================================================================  
//===========================================================================  
//========================================= State Alias =====================
//===========================================================================  
//===========================================================================  

template <typename SM, typename ALIAS>
struct StateAlias
{
   // default is empty (nodes do not have an alias by default)
}; 


// Make AN point to N in state machine SM
#define SM_MAKE_ALIAS(SM,AN,N) \
  template <> struct StateAlias<SM,AN> \
  { typedef N<SM> type; }

#define SM_ALIAS_STATE(a) \
   struct a {}

//===========================================================================  
//======= Statebase =========================================================  
//===========================================================================  

/**
 * Class every state derives from (indirectly through Link)
 * It ensures every state has a virtual destructor and provides a common
 * pointer type for the whole state machine
 */
template <typename SMDEF>
class StateBase
{
protected:
   friend class StateMachine<SMDEF>;


   /// We cannot force a state to go to another state without the static type
   /// of the state we come from and the state we go to.
   /// gotoInitState is called by the state machine in response to the
   /// destruction of the state machine object itself.
   virtual void gotoInitState () = 0; 

   virtual ~StateBase () {}; 
}; 



//===========================================================================  
//======= StateFunc =========================================================  
//===========================================================================  
/** 
 * StateFunc is a parent of each state.
 * It enables injecting methods and data members into the state
 */


// This is used whenever the state does not declare persistent or normal state
// data. 
struct EmptyDataType
{
}; 

/** 
 * This class is used to inject default definitions of the 
 *   PersistentStateData and StateData structures.
 * We cannot put it into StateFunc because that would require State
 * implementors to specify the qualified name of the structures or to use a
 * 'using' clause / redefinition.
 */
class StateDataInjector
{
public:
   typedef EmptyDataType PersistentStateData;
   typedef EmptyDataType StateData; 
}; 

template <typename SMDEF_, typename S>
class StateFunc : public StateBase<SMDEF_>, protected StateDataInjector
{
protected:
   typedef SMDEF_ SMDEF; 

   template <typename F>
   friend class StateMachine; 

   // We need our own type to be able to pass the static type information 
   // to the StateMachine.
   typedef S   SELF; 

   typedef int  StateData; /* non-peristent default data */ 
   typedef int      PersistentData; /* persistent data */  

protected:
   StateFunc (StateMachine<SMDEF> & sm)
      : sm_(sm)
   {
      cout << "Constructing state " << S::getStateName() << endl; 
   }

   virtual ~StateFunc ()
   {
      cout << "Destructing state " << S::getStateName() << endl; 
   }


   virtual void gotoInitState () 
   {
      this->template setState<typename SMDEF::INITSTATE>(); 
   }

public:
   static std::string getStateName () 
   { return iofwdutil::demangle(typeid(S).name ()); }

protected:

   static int ID () 
   { return StateID<S>::value; }

   
   /// setState for instantiated states
   template <typename NEXT>
   void setState () 
   {
      // call back to the statemachine which will instantiate 
      // code for going from this state to NEXT
      this->sm_.template setState<SELF,NEXT> (); 
   }

   /// setState for instantiated states (1 param)
   template <typename NEXT, typename P1>
   void setState (P1 & p1) 
   { this->sm_.template setState<SELF,NEXT> (p1); }

   /// setState for instantiated states (2 param)
   template <typename NEXT, typename P1, typename P2>
   void setState (P1 & p1, P2 & p2) 
   { this->sm_.template setState<SELF,NEXT> (p1,p2); }

   /// setState for instantiated states (3 param)
   template <typename NEXT, typename P1, typename P2, typename P3>
   void setState (P1 & p1, P2 & p2,P3 & p3) 
   { this->sm_.template setState<SELF,NEXT> (p1,p2,p3); }


   /// makes it simpler for the user: can specify non-instantiated state
   template <template <typename> class ST>
   void setState ()
   {  this->template setState<ST <SMDEF> >(); }

   template <template <typename> class ST, typename P1>
   void setState (P1 & p1)
   {  this->template setState<ST <SMDEF> >(p1); }

   template <template <typename> class ST, typename P1, typename P2>
   void setState (P1 & p1, P2 & p2)
   {  this->template setState<ST <SMDEF> >(p1,p2); }

   template <template <typename> class ST, typename P1, typename P2, 
            typename P3>
   void setState (P1 & p1, P2 & p2, P3 & p3)
   {  this->template setState<ST <SMDEF> >(p1,p2,p3); }


   // Same as setState, but takes state alias name
   template <typename ST>
   void setStateAlias ()
   {  } 

protected:

   // By default, enter and leave are empty
   void enter () {}; 

   // By default, enter and leave are empty
   void leave () {}; 

protected:

   /// Reference to the state machine this state is a part of
   StateMachine<SMDEF> & sm_; 
}; 


//===========================================================================  
//===========================================================================  
//===========================================================================  

template <typename S>
struct StateDepth;

/**
 * Determine depth of a state
 */
template <typename INITSTATE, typename S, int P>
struct StateDepth_Helper
{
   typedef typename IStateDirectParent<S>::type parent;
   enum { initstate = boost::is_same<
                         typename S::SMDEF::INITSTATE,
                         S
                       >::value }; 

   enum { value = StateDepth<parent>::value + 1 }; 
}; 


// The initstate has depth 0 
template <typename S, int P>
struct StateDepth_Helper<S,S, P>
{
   enum { value = 0 }; 
};

// Depth of a state not in the machine is -1
template <typename S1, typename S2>
struct StateDepth_Helper<S1, S2, -1>
{
   enum { value = -1 }; 
}; 

// This template makes it possible to filter out trying to get the depth 
// of the initstate or to get the depth of a state that is not in the machine
template <typename S>
struct StateDepth
{
   enum { id = StateID<S>::value }; 
   enum { value = StateDepth_Helper<typename S::SMDEF::INITSTATE, S, id>::value };
};

//===========================================================================  
//===========================================================================  
//===========================================================================  

// forward
template <typename S1, typename S2>
struct SharedParent;

//
// given two instantiated states, find their first shared parent state
//
template <typename INITSTATE, typename S1, typename S2>
struct SharedParent_Helper
{
   enum { S1_DEPTH = StateDepth<S1>::value,
          S2_DEPTH = StateDepth<S2>::value }; 
   enum { same_depth = (S1_DEPTH == S2_DEPTH) };
   enum { s1_deeper = (S1_DEPTH > S2_DEPTH) }; 

   typedef typename IStateDirectParent<S1>::type S1PARENT;
   typedef typename IStateDirectParent<S2>::type S2PARENT;


   typedef typename boost::mpl::if_c<
               same_depth, // sharedparent of two different states at same
                          // depth is sharedparent of parents
               typename SharedParent<S1PARENT,S2PARENT>::type,
               typename boost::mpl::if_c<
                   s1_deeper,
                   typename SharedParent<S1PARENT,S2>::type,
                   typename SharedParent<S1, S2PARENT>::type
                  >::type
              >::type type; 
}; 

// SharedParent of two equal states is the state itself
template <typename I, typename S>
struct SharedParent_Helper<I, S,S> 
{
   typedef S type;
};  

// Prevent trying to go outside of the tree
template <typename I, typename S>
struct SharedParent_Helper<I, I, S>
{
   typedef typename SharedParent<I, typename
      IStateDirectParent<S>::type>::type type; 
};

// Prevent trying to go outside of the tree
template <typename I, typename S>
struct SharedParent_Helper<I, S, I>
{
   typedef typename SharedParent<I, typename
      IStateDirectParent<S>::type>::type type; 
};


template <typename S1, typename S2>
struct SharedParent
{
   typedef typename SharedParent_Helper<typename S1::SMDEF::INITSTATE,
           S1, S2>::type type; 
}; 

template <typename S>
struct SharedParent<S,S>
{
   typedef S type; 
}; 

//===========================================================================  
//====== Tree Walking =======================================================  
//===========================================================================  

// Do nothing functor
struct WalkTreeVoid 
{
   template <typename S>
   void operator () () const
   {
   }
};

/**
 * Walk up from state S1 to state S2, calling func before and after
 * going to the next node
 *
 * Call on endnode is not included 
 *
 * FUN{BEFORE|AFTER} should have an operator () taking one template argument
 * (the state type)
 */
template <typename S1, typename S2>
struct WalkTree 
{
   typedef typename IStateDirectParent<S1>::type parent; 

   template <typename FUNBEFORE, typename FUNAFTER>
   static void apply (FUNBEFORE & fb, FUNAFTER & fa)
   {
      fb.template operator ()<S1> ();
      WalkTree<parent, S2>::apply (fb, fa); 
      fa.template operator ()<S1> ();
   }

}; 

template <typename S>
struct WalkTree<S,S>
{
   template <typename FUNBEFORE, typename FUNAFTER>
   static void apply (FUNBEFORE & , FUNAFTER & )
   {
      // nothing todo, reached destination
   }
};

//===========================================================================  
//====== State Machine ======================================================  
//===========================================================================  

// TODO:
//   - Currently there is no stateful data within a state instance; 
//     Could reuse state instances when running multiple instances of the 
//     same state machine.

template <typename SMDEF>
class StateMachine 
{
   protected:
      typedef typename SMDEF::INITSTATE  INITSTATE; 

      enum { state_count = MachineStates<SMDEF>::state_count }; 

      /// Pointer to the StateBase class for this SMDEF
      typedef StateBase<SMDEF> * VStatePtr; 

   public:
      StateMachine ();

      ~StateMachine ();

      static int getStateCount ()
      { return state_count; } 
      
      void run (); 

   protected:

      // Easy access to the state type
      template <int ID>
      struct SType
      { 
         typedef typename StateType<SMDEF,ID>::type type; 
      }; 


      /// ensureStateExists makes sure the state is ready to be 
      /// used, constructing if needed
      template <typename S>
      void ensureStateExists ()
      {
         enum { id = StateID<S>::value }; 
         STATIC_ASSERT(id >= 0); // Check that the state is in our machine
         if (states_[id])
            return;
         states_[id] = new typename SType<id>::type (*this);  
      }

      /// Return a reference to the state; Does not check if the state exists
      template <typename S>
      S & getState ()
      { 
         enum { id = StateID<S>::value }; 
         STATIC_ASSERT(id >= 0); // Check that the state is in our machine
         // Make sure the state already exists
         ASSERT(states_[id] && "getState for a non-existant state");
         return static_cast<S &>(*states_[id]); 
      }


      /// Return pointer to current state (note: no static type info, 
      /// so need virtual function 
      VStatePtr getCurrentState ()
      {
         ASSERT(currentState_ >= 0); 
         ASSERT(states_[currentState_]); 
         return states_[currentState_]; 
      }

      /// Make sure tempStateStorageRaw points to enough memory for this state

      /// Called to actually enter a state;
      /// Make sure state exists, calls enter and initializes state data
      /// Adjusts currentState_
      template <typename S>
      void enterState ()
      {
         ensureStateExists<S>(); 
         //reserveTempStateStorage<S>(); 
         this->template getState<S>().enter (); 
         currentState_ = StateID<S>::value; 
      }

      /// Called when leaving a state
      /// Sets currentState_ to the parent state
      template <typename S>
      void leaveState ()
      {
         this->template getState<S>().leave (); 
         currentState_ = StateID<typename IStateDirectParent<S>::type>::value; 
         typedef typename S::StateData SD; 
         // In place destruct non-persistent state data
         //static_cast<SD *>(tempStateStorage)->~SD (); 
         //tempStateStorage=0; 
      }


      /// Called when the machine object is destroyed 
      void leaveMachine ();

      /// Called to make the state transition 
      /// Needs valid transitionFunc_ pointer and nextState_
      void gotoNextState (); 


      /// Called by the states enter/leave method to trigger
      /// a state machine transition
      template <typename FROM,typename TO>
      inline void setState ();

      /// The next method actually instantiates the code 
      /// that performs the actual state change
      template <typename FROM, typename TO>
      inline void doStateTransition (); 

   protected:

      // Provide StateFunc access to our setState template method
      template <typename SM, typename T> 
      friend class StateFunc; 
      
   protected:

      struct EnterFunctor 
      {
         EnterFunctor (StateMachine & sm, int id)
            : sm_ (sm), dest_(id)
         {
         }

         template <typename S>
         void operator () () 
         { 
            if (S::ID() == dest_)
            {
               // We are about to enter the state we transition to.
               // Clear nextState_ to enable the enter method to select 
               // a new destination
               sm_.nextState_ = -1; 
            }
            sm_.template enterState<S> (); 
         }

         StateMachine & sm_; 
         int dest_; 
      }; 

      struct LeaveFunctor 
      {
         LeaveFunctor (StateMachine & sm)
            : sm_ (sm)
         { }

         template <typename S>
         void operator () () 
         { sm_.template leaveState<S> (); }

         StateMachine & sm_; 
      }; 


   protected:

      /// Pointers to the states (ordered by StateID).
      /// Note: states are created on demand
      /// Note2: we can avoid the allocation by constructing the
      /// states in place 
      boost::array<VStatePtr,state_count> states_;

      /// The state ID of the current state (-1 means no state)
      int currentState_; 

      /// The state ID of the state we need to transition to (-1 indicates no
      /// change)
      int nextState_; 


      /// Function prototype of a state transition method
      typedef void (StateMachine<SMDEF>::*TransitionFuncType) (); 

      TransitionFuncType transitionFunc_; 

      ///
      /// Storage for non-persistent state storage
      /// Currently persistent state data is stored within the state class
      //
      void * tempStateStorage_; 

}; 
//===========================================================================  

template <typename SMDEF>
StateMachine<SMDEF>::StateMachine ()
   : currentState_(-1), nextState_(-1), transitionFunc_(0),
   tempStateStorage_(0)
{
   for (unsigned int i=0; i<state_count; ++i)
      states_[i] = 0;

   // Just to make sure boost::array initializes data
   ASSERT(states_[0] == 0); 
}

template <typename SMDEF>
void StateMachine<SMDEF>::gotoNextState ()
{
   // Nothing todo if there is no next state
   // ALWAYS_ASSERT(nextState_ > 0);  // just here to see if this ever happens
   if (nextState_ < 0)
      return; 

   // If there is a nextState_, the transitionFunc_ should have been set
   ASSERT(transitionFunc_); 

   TransitionFuncType tempFunc_ = transitionFunc_; 

   // NOTE: need to check case where the enter func of a parent state tries to
   // make us go somewhere else
   nextState_ = -1; 
   transitionFunc_ = 0; 
   
   // Call generated state transition func
   (this->*tempFunc_) (); 
}

template <typename SMDEF>
void StateMachine<SMDEF>::leaveMachine ()
{
   typedef typename SMDEF::INITSTATE INIT; 
   // Think about what we want here:
   // in principle, destroying the state machine if it isn't in an exit/done
   // state should be considered an error...
   // For now, go just go to the initstate and leave that one too.

   // Check if the machine was already started.
   // If not, nothing todo
   if (currentState_ < 0)
      return; 

   if (currentState_ != StateID<INIT>::value)
   {
      // force a transition from the current state to the initstate
      // NOTE: we cannot force a state change from outside of the states 
      // without support from StateBase because we need both static types... 
      getCurrentState()->gotoInitState (); 

      // TODO: think about what happens if we call leave (because of
      // exception) from within a enter/leave method (and thus run loop)
      gotoNextState (); 
   }

   ASSERT (currentState_ == StateID<INIT>::value); 
   ASSERT (nextState_ < 0); 

   this->template leaveState<INIT>(); 
   ASSERT (currentState_ < 0); 
}

/**
 * Called by the state to indicate a desired state change.
 * Just stores a pointer to a method that will actually perform the state
 * change.
 */
template <typename SMDEF>
template <typename FROM, typename TO>
inline void StateMachine<SMDEF>::setState ()
{
   // Check nobody changed their mind
   ASSERT(nextState_ < 0); 

   transitionFunc_ = &StateMachine<SMDEF>::template doStateTransition<FROM,TO>; 
   nextState_ = TO::ID(); 
}

template <typename SMDEF>
template <typename FROM, typename TO>
inline void StateMachine<SMDEF>::doStateTransition ()
{
   // The following is a compile time check 
   // which will cause dead branches to be removed
   if (boost::is_same<FROM,TO>::value)
   {
      // Reentering state
      // NOTE: May need to handle case where data needs to be destroyed
      // We handle this case separate because setState<INITSTATE,INITSTATE>()
      // would not work, even if we pretend
      // IStateDirectParent<INITSTATE,INITSTATE>::type == SMDEF
      this->template leaveState<FROM>(); 
      this->template enterState<TO>(); 
   }
   else
   {
      typedef typename SharedParent<FROM,TO>::type SharedParent; 
      // Going to another state:
      // call leave up to the first shared parent, then call enter 
      // down to the new state
      EnterFunctor enter (*this, StateID<TO>::value); 
      LeaveFunctor leave (*this); 
      sm::WalkTreeVoid v; 

      // Same here: it must be illegal to change the state we transition to 
      // from a 'leave' method
      // However, we do allow the enter method of the state we transition to 
      // to call setState.

      WalkTree<FROM,SharedParent>::apply (leave,v); 
      WalkTree<TO,SharedParent>::apply (v,enter); 
   }
}

template <typename SMDEF>
StateMachine<SMDEF>::~StateMachine ()
{
   leaveMachine (); 

   ASSERT(currentState_ < 0); 
   ASSERT(nextState_ < 0); 

   // Free all used states
   for (unsigned int i=0; i<state_count; ++i)
      delete (states_[i]); 
}

template <typename SMDEF>
void StateMachine<SMDEF>::run ()
{
   if (currentState_ < 0)
   {
      ASSERT(nextState_ < 0); 

      // First time the machine runs
      // We need to go to the initstate

      // We cannot use the normal setState from here because we don't have 
      // a current state.
      // Note that enterState could trigger a call to setState,
      // so we have to run the transition logic after this
      this->template enterState<typename SMDEF::INITSTATE>(); 
   }


   ASSERT(currentState_ >= 0); 

   // Run until no next state requested
   while (nextState_ >= 0)
   {
      gotoNextState (); 
   }

}

//===========================================================================  
}

#endif
