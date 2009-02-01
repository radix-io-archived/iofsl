#ifndef IOFWDUTIL_TIMEVAL_HH
#define IOFWDUTIL_TIMEVAL_HH

#include <cmath>
#include <iostream>

namespace iofwdutil
{
//===========================================================================
 

/// Describes point in time (unrooted)
class TimeVal
{
public:

   enum { NS_PER_SECOND = 1000000 }; 

   TimeVal ()
      : seconds_(0), nanoseconds_(0)
   {
   }

   TimeVal (long seconds, long ns)
      : seconds_(seconds), nanoseconds_(ns)
   {
   }

   long long getTotalNanoSeconds () const
   {
      return static_cast<long long>(seconds_)*NS_PER_SECOND + nanoseconds_; 
   }
   operator bool () const
   { return ! isZero (); }

   bool isZero () const
   {
      return (seconds_==0 && nanoseconds_ == 0); 
   }

   void clear ()
   {
      seconds_ = 0; 
      nanoseconds_ = 0; 
   }

   TimeVal (double d)
   {
      setFraction (d); 
   }

   long getSeconds () const
   { return seconds_; }

   long getNanoSeconds () const
   { return nanoseconds_; }

   void setSeconds (long s)
   { seconds_ = s; }

   void setNanoSeconds (long s)
   { nanoseconds_ = s; }

   double getFraction () const
   { return seconds_ + (double(nanoseconds_)/NS_PER_SECOND); }


   void setFraction (double frac) 
   { 
      seconds_ = static_cast<long>(trunc(frac)); 
      nanoseconds_ = static_cast<long>((frac - seconds_) * NS_PER_SECOND);
   }


protected:
   long seconds_; 
   long nanoseconds_; 
}; 
   
inline TimeVal  operator + (const TimeVal & t1, const TimeVal & t2)
{
   return TimeVal (t1.getFraction() + t2.getFraction()); 
}

inline TimeVal  operator - (const TimeVal & t1, const TimeVal & t2)
{
   return TimeVal (t1.getFraction() - t2.getFraction()); 
}

std::ostream & operator << (std::ostream & out, const TimeVal & t); 


//===========================================================================
}

#endif
