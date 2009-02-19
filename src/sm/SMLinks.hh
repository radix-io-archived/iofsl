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

namespace sm
{
//===========================================================================  

template <typename SM, typename STATE>
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

// By default, a node has no children
template <typename SM, typename N>
struct StateChildList { typedef boost::mpl::list<>::type children; }; 


//
// NOTE: this macro needs to be used within the 'sm' namespace
// 
#define SM_STATE_CHILDREN_BEGIN(S) \
    template <typename SM> \
   struct StateChildList<SM,S> { typedef boost::mpl::list<

#define SM_STATE_CHILDREN_END \
      > children; }; 

// ISO C does not support variadic macro's
//#define SM_STATE_CHILDREN(P,C...) 
//   SM_STATE_CHILDREN_BEGIN(P) C SM_STATE_CHILDREN_END

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
template <typename SM, typename L>
struct AllChildren
{
   typedef typename boost::mpl::front<L>::type list_head;
   typedef typename boost::mpl::pop_front<L>::type list_remainder; 

   // The list of our childern
   typedef typename StateChildList<SM, list_head>::children our_children; 

   // our_children + list_remainder
   typedef typename merge_lists<our_children,list_remainder>::type combined_list; 

   // The list of children of the remainder of the argument list
   typedef typename AllChildren<SM,combined_list>::children children_list_remainder;

   typedef typename boost::mpl::push_front<children_list_remainder,
           list_head>::type children;

}; 

template <typename SM>
struct AllChildren<SM,ENDITR>
{
   typedef boost::mpl::list<>::type children; 
}; 

//===========================================================================
//===========================================================================
//===========================================================================


// Contains a set of all the states in the machine
template <typename SM>
struct MachineStates 
{
   // THe set of all states of a state machine
   typedef typename AllChildren<
      SM,typename boost::mpl::list<typename SM::INITSTATE>::type >::children states; 

   enum { state_count = boost::mpl::size<states>::type::value }; 
}; 

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

//===========================================================================  
//====== State Numbering ====================================================  
//===========================================================================  
template <typename SM, typename S>
struct StateID
{
   // The state ID is the position of the state in the statemachine list
    enum { value = findpos< typename MachineStates<SM>::states, S>::value }; 
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
  template <> struct ::sm::StateAlias<SM,AN> \
  { typedef N alias; }

  
//===========================================================================  
}

#endif
