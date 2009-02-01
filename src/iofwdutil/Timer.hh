#ifndef IOFWDUTIL_TIMER_HH
#define IOFWDUTIL_TIMER_HH

#include <iostream>
#include <time.h>
#include <boost/assert.hpp>
#include "TimeVal.hh"

namespace iofwdutil
{
//===========================================================================

/**
 * Wallclock timer, based on POSIX timers
 */
class Timer
{
public:
   typedef iofwdutil::TimeVal TimeVal; 

   enum { NS_PER_SECOND = TimeVal::NS_PER_SECOND };  
public:
   Timer (); 

   /// Stop clock and reset internal values
   void reset ()
   {
      start_.clear(); 
      stop_.clear(); 
   } 

   /// Return elapsed time since start but do not stop clock
   TimeVal current () const
   {
      return getCurrent () - start_; 
   }

   /// Return time between start and stop call
   TimeVal elapsed () const
   {
      return stop_ - start_; 
   }

   /// Start clock
   void start ()
   {
      start_ = getCurrent(); 
   }

   /// Stop clock
   void stop ()
   {
      stop_ = getCurrent(); 
   }

   /// Write clock information to ostream
   static void dumpInfo (std::ostream & o);

protected:
   TimeVal getCurrent () const
   {
      struct timespec tp;
      int ret = clock_gettime (clockid_, &tp); 
      BOOST_ASSERT(ret == 0); 
      return TimeVal (tp.tv_sec, tp.tv_nsec); 
   }


protected:
   TimeVal start_; 
   TimeVal stop_; 
   clockid_t clockid_; 

}; 
   

//===========================================================================
}

#endif
