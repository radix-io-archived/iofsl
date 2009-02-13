#include <iostream>
#include "Timer.hh"
#include <unistd.h>
#include "iofwdutil/always_assert.hh"
#include "iofwdutil/tools.hh"

using namespace std; 
using namespace iofwdutil; 

void testTimeVal ()
{
   TimeVal t; 
   t.setFraction (0); 
   ALWAYS_ASSERT(t.getSeconds()==0); 
   ALWAYS_ASSERT(t.getNanoSeconds()==0); 
}

int main (int UNUSED(argc), char ** UNUSED(args))
{
   Timer t;
   int failed = 0; 

   Timer::dumpInfo (cout); 

   t.start (); 

   cout << "Testing timeval" << endl; 
   testTimeVal (); 
   
   cout << "Testing if clock is monotonic... "; 
   cout.flush (); 
   TimeVal s = t.current (); 
   for (unsigned int i=0; i<10000; ++i)
   {
      if (s > t.current ())
      {
         failed = 1; 
         break; 
      }
   }
   s = t.current(); 
   usleep (20); 
   if (s > t.current ())
      failed=1; 
   cout << (failed ? "FAILED!" : "OK") << endl; 

   for (unsigned int i=0; i<10; ++i)
   {
      const unsigned int mdelay = 2000; 
      cout << "Waiting " << mdelay << " mseconds...";
      cout.flush (); 
      TimeVal t1 = t.current (); 
      usleep (mdelay); 
      TimeVal t2 = t.current (); 
      double diff = t2.toDouble() - t1.toDouble(); 
      cout << "elapsed: " << t2-t1 << " (" << diff << ")" << endl; 
      if (diff > double(mdelay*2)/TimeVal::US_PER_SECOND)
         cerr << "Warning: inaccurate clock??\n"; 
   }


   enum { LOOPCOUNT = 10 }; 
   for (unsigned int i=0; i<LOOPCOUNT; ++i)
   {
      TimeVal s = t.current (); 
      cout << s.getSeconds() << "s" << s.getNanoSeconds () << "ns" << endl; 
      usleep (TimeVal::US_PER_SECOND/LOOPCOUNT); 
   }
}
