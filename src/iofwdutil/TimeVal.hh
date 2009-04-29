#ifndef IOFWDUTIL_TIMEVAL_HH
#define IOFWDUTIL_TIMEVAL_HH

#include <cmath>
#include <iostream>
#include <boost/assert.hpp>
#include <stdint.h>

namespace iofwdutil
{
//===========================================================================
 
   class TimeVal; 

/// Describes point in time (unrooted)
class TimeVal
{
public:

   enum { NS_PER_SECOND = 1000000000 }; 
   enum { US_PER_SECOND = 1000000 }; 
   enum { MS_PER_SECOND = 1000 }; 

   TimeVal ()
      : seconds_(0), nanoseconds_(0)
   {
   }

   TimeVal (long seconds, long ns)
      : seconds_(seconds), nanoseconds_(ns)
   {
       BOOST_ASSERT (0 <= nanoseconds_ && nanoseconds_ <= NS_PER_SECOND); 
       BOOST_ASSERT (seconds_ >= 0); 
   }

   int64_t getTotalNanoSeconds () const
   {
      return static_cast<int64_t>(seconds_)*NS_PER_SECOND + nanoseconds_; 
   }

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
   { return double(seconds_) + (double(nanoseconds_)/NS_PER_SECOND); }


   double toDouble () const
   { return getFraction (); }

   void setFraction (double frac) 
   { 
      seconds_ = static_cast<long>(trunc(frac)); 
      nanoseconds_ = static_cast<long>((frac - double(seconds_)) * NS_PER_SECOND);
   }


protected:
   friend TimeVal operator + (const TimeVal & t1, const TimeVal & t2); 
   friend TimeVal operator - (const TimeVal & t1, const TimeVal & t2); 
   friend bool operator < (const TimeVal & t1, const TimeVal & t2); 
   friend bool operator > (const TimeVal & t1, const TimeVal & t2); 
   friend bool operator == (const TimeVal & t1, const TimeVal & t2); 

   long seconds_; 
   long nanoseconds_; 
}; 
  
inline bool operator < (const TimeVal & t1, const TimeVal & t2)
{
   if (t1.seconds_ < t2.seconds_)
      return true; 
   if (t1.seconds_ > t2.seconds_)
      return false; 
   return (t1.nanoseconds_ < t2.nanoseconds_); 
}

inline bool operator > (const TimeVal & t2, const TimeVal & t1)
{
   if (t1.seconds_ < t2.seconds_)
      return true; 
   if (t1.seconds_ > t2.seconds_)
      return false; 
   return (t1.nanoseconds_ < t2.nanoseconds_);   
}

inline bool operator == (const TimeVal & t1, const TimeVal & t2)
{
   return ((t1.seconds_ == t2.seconds_ ) 
         && (t1.nanoseconds_ == t2.nanoseconds_)); 
}

inline TimeVal  operator + (const TimeVal & t1, const TimeVal & t2)
{
   long second = t1.seconds_ + t2.seconds_; 
   long nsecond = t1.nanoseconds_ + t2.nanoseconds_; 
   if (nsecond >= TimeVal::NS_PER_SECOND)
   {
      long c = nsecond / TimeVal::NS_PER_SECOND; 
      second += c; 
      nsecond -= c*TimeVal::NS_PER_SECOND; 
   }
   return TimeVal (second, nsecond); 
}

inline TimeVal  operator - (const TimeVal & t1, const TimeVal & t2)
{
   BOOST_ASSERT (t1.seconds_ >= t2.seconds_); 
   BOOST_ASSERT ((t1.seconds_ != t2.seconds_)
         || (t1.nanoseconds_ >= t2.nanoseconds_));
   BOOST_ASSERT (t1.nanoseconds_ <= TimeVal::NS_PER_SECOND); 
   BOOST_ASSERT (t2.nanoseconds_ <= TimeVal::NS_PER_SECOND); 

   long second = t1.seconds_ - t2.seconds_; 
   long nsecond = t1.nanoseconds_ - t2.nanoseconds_; 
   if (nsecond < 0)
   {
      BOOST_ASSERT (second > 0); 
      --second; 
      nsecond += TimeVal::NS_PER_SECOND; 
      BOOST_ASSERT (nsecond >= 0); 
   }
   return TimeVal (second, nsecond); 
}

std::ostream & operator << (std::ostream & out, const TimeVal & t); 


//===========================================================================
}

#endif
