#ifndef IOFWDUTIL_COMPLETION_TIMERRESOURCE_HH
#define IOFWDUTIL_COMPLETION_TIMERRESOURCE_HH

#include <boost/thread.hpp>
#include <queue>
#include "CompletionID.hh"
#include "ContextBase.hh"
#include "Resource.hh"
#include "boost/date_time/posix_time/posix_time_types.hpp"
#include "TimerCompletionID.hh"
#include "iofwdutil/assert.hh"


namespace iofwdutil
{
   namespace completion
   {
//===========================================================================

class TimerResource  
{
public:
   TimerResource (); 

   void createTimer (TimerCompletionID * id, unsigned int mstimeout); 


   bool isActive () const; 

   void waitAny (std::vector<TimerCompletionID *> & completed); 

   void testAny (std::vector<TimerCompletionID *> & completed, unsigned int maxms); 

   bool test (TimerCompletionID * id, unsigned int maxms); 

   void wait (TimerCompletionID * id); 
   

protected:

   /// Comparison operator for the heap
   class TimerComp 
   {
      public:
         TimerComp (TimerResource & res)
            : timer_ (res)
         {
         }

         bool operator () (const TimerCompletionID * v1, const TimerCompletionID * v2)
         {
            return v1->alarm_ > v2->alarm_; 
         }

      protected:
         TimerResource & timer_; 
   }; 
  
protected:

   /// Return alarm of next scheduled timer
   const boost::posix_time::ptime & peekNext () const
   { 
      ASSERT(heap_.size()); 
      return heap_.top()->alarm_; 
   }

   TimerCompletionID * pop_heap (); 

   /// Remove already completed entries. Must be called with complock held
   /// Returns true if any entries were found
   bool removeCompleted (std::vector<TimerCompletionID *> & com); 

   bool haveFreeTimers () const
   { return freetimers_ != 0; }

protected:

   TimerComp comp_; 

   std::priority_queue<TimerCompletionID *,
       std::vector<TimerCompletionID *>, TimerComp> heap_; 

   std::vector<TimerCompletionID *> completed_; 


   // Protects heap
   boost::mutex lock_; 

   // Protects completed list
   boost::mutex complock_; 

   // Number of unreserved timers in list
   unsigned int freetimers_; 
}; 


//===========================================================================
   }
}


#endif
