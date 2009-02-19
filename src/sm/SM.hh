#ifndef SM_SMLINKS_HH
#define SM_SMLINKS_HH

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

#include <boost/array.hpp>


// for debugging
#include <iostream>

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

//===========================================================================  
//===========================================================================  
//===========================================================================  
#define SM_STATE(a) \
   template <typename SMDEF> \
   struct a : public ::sm::StateFunc<SMDEF,a>

#define SM_STATEDEF \
   typedef ::sm::StateMachine<SMDEF> STATEMACHINE; \
   typedef typename SMDEF::INITSTATE INITSTATE; \
   typedef SMDEF                     STATEMACHINE_DEF


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
 * Predicate that checks if a state is a child of another state
 */
/*template <typename SMDEF, typename PARENT, typename CHILD>
struct IsStateDirectChild
{
   typedef typename StateChildList<SMDEF,PARENT>::children childlist; 
   enum { value = boost::is_same<
             CHILD, 
             typename boost::mpl::find<childlist,CHILD>::type
           >::value }; 
   typedef typename  boost::is_same<
             CHILD, 
             typename boost::mpl::find<childlist,CHILD>::type
           > type; 
};  */


/**
 * Checks if C is a direct child of the state passed to apply
 * Note: apply is passed the state instantiated for SMDEF
 *
 * NOTE: This does not check if SMDEF::INITSTATE equals C<SMDEF>
 */
template <typename SMDEF, template <typename> class C>
struct IsDirectParentPred
{
   
   template <typename P>
   struct apply 
   {
      typedef typename StateChildList<SMDEF,P>::children searchlist; 
      typedef C<SMDEF> searchitem; 
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

template <typename SMDEF, template <typename> class S>
struct StateDirectParent
{
   typedef typename MachineStates<SMDEF>::states searchlist; 
   typedef typename boost::mpl::find_if<
               searchlist,
               IsDirectParentPred<SMDEF,S>
              >::type parentiter; 
   typedef typename boost::mpl::deref<parentiter>::type realparent;

   typedef typename boost::mpl::if_<
                boost::is_same< S<SMDEF>, typename SMDEF::INITSTATE>,
                SMDEF,
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
template <typename SM, template <typename SMI> class S>
struct StateID
{
   // The state ID is the position of the state in the statemachine list
    enum { value = findpos< typename MachineStates<SM>::states, S<SM> >::value }; 
}; 

// Same but for an instantiated state SI
template <typename SI>
struct IStateID
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
public:
   virtual ~StateBase () {}; 
}; 



//===========================================================================  
//======= StateFunc =========================================================  
//===========================================================================  
/** 
 * StateFunc is a parent of each state.
 * It enables injecting methods and data members into the state
 */


template <typename SMDEF, template <typename> class S>
class StateFunc : public StateBase<SMDEF>
{
protected:
   template <typename F>
   friend class StateMachine; 

   // We need our own type to be able to pass the static type information 
   // to the StateMachine.
   typedef S<SMDEF>   SELF; 

protected:
   StateFunc (StateMachine<SMDEF> & sm)
      : sm_(sm)
   {
      cout << "Constructing state " << typeid(SELF).name() << endl; 
   }

   virtual ~StateFunc ()
   {
      cout << "Destructing state " << typeid(SELF).name() << endl; 
   }

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
//====== State Machine ======================================================  
//===========================================================================  

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

      int getStateCount () const 
      { return state_count; } 

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
         enum { id = IStateID<S>::value }; 
         STATIC_ASSERT(id >= 0); // Check that the state is in our machine
         if (states_[id])
            return;
         states_[id] = new typename SType<id>::type ();  
      }

      /// Return a reference to the state; Does not check if the state exists
      template <typename S>
      S & getState ()
      { 
         enum { id = IStateID<S>::value }; 
         STATIC_ASSERT(id >= 0); // Check that the state is in our machine
         // Make sure the state already exists
         ASSERT(states_[id] && "getState for a non-existant state");
         return static_cast<S &>(states_[id]); 
      }

   protected:
      
      /// Pointers to the states (ordered by StateID).
      /// Note: states are created on demand
      /// Note2: we can avoid the allocation by constructing the
      /// states in place 
      boost::array<VStatePtr,state_count> states_;

      /// The state ID of the state we need to transition to (-1 indicates no
      /// change)
      int nextState_; 

      /// The state ID of the current state (-1 means no state)
      int currentState_; 
}; 
//===========================================================================  

template <typename SMDEF>
StateMachine<SMDEF>::StateMachine ()
{
   for (unsigned int i=0; i<state_count; ++i)
      states_[i] = 0;

   // Just to make sure boost::array initializes data
   ASSERT(states_[0] == 0); 
}

template <typename SMDEF>
StateMachine<SMDEF>::~StateMachine ()
{
   // Free all used states
   for (unsigned int i=0; i<state_count; ++i)
      delete (states_[i]); 
}

//===========================================================================  
}

#endif
