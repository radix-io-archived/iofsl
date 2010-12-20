#include <iostream>
#include <time.h>

#include "Timer.hh"
#include "always_assert.hh"

namespace 
{
   class FindClock 
   {
   public:

      // Determine the best posix walltime clock to use
      FindClock ()
      {
         clockid_t clocks[] = { CLOCK_REALTIME, CLOCK_MONOTONIC }; 
         resolution_.clear (); 
         iofwdutil::TimeVal t; 
         struct timespec ts; 

         for (unsigned int i=0; i<sizeof(clocks)/sizeof(clocks[0]); ++i)
         {
            if (clock_getres (clocks[i], &ts)<0)
               continue; 
            t.setSeconds (ts.tv_sec); 
            t.setNanoSeconds (ts.tv_nsec); 
            if (resolution_.isZero() || t < resolution_)
            {
               resolution_ = t; 
               clockid_ = clocks[i]; 
            }
         }

         // If this fails, we didn't find a single usable clock!
         ALWAYS_ASSERT (!resolution_.isZero() ); 
      }

      clockid_t getClockID () const
      {
         return clockid_; 
      }

      const iofwdutil::TimeVal & getResolution () const
      {
         return resolution_; 
      }

   protected:
      clockid_t clockid_; 
      iofwdutil::TimeVal   resolution_; 
   }; 
      
   
   static FindClock clkid; 
}

namespace iofwdutil
{

   Timer::Timer ()
   {
      clockid_ = clkid.getClockID (); 
      reset (); 
   }

   void Timer::dumpInfo (std::ostream & out)
   {
      out << "Using POSIX clock "; 
      switch (clkid.getClockID())
      {
         case CLOCK_REALTIME:
            out << "CLOCK_REALTIME"; 
            break;
         case CLOCK_MONOTONIC:
            out << "CLOCK_MONOTONIC";
            break;
         default:
            out << "(UNKNOWN)"; 
      }
      out << std::endl;
      out << "Resolution: " << clkid.getResolution () << std::endl; 
   }
}
