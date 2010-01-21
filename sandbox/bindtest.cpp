#include <iostream>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;
using namespace boost;

const size_t LOOPS = 20000000;

/**
 * Try to figure out how to connect resources and clients.
 */


struct TimeScope 
{
   TimeScope ()
      : t1 (get_system_time())
   {
   }

   ~TimeScope ()
   {
      cout << "Time (ms): " << (get_system_time()-t1).total_milliseconds () <<
                                endl;
   }

   system_time t1;
};

template <typename FUNCSTORE, typename T>
void doTest (const T & t)
{
   FUNCSTORE f (t);
   cout << "Size of delegate = " << sizeof(f) << endl;
   cout << "Size of initializer = " << sizeof(t) << endl;
   {
      TimeScope time;
      for (size_t i=0; i<LOOPS; ++i)
      {
         FUNCSTORE f2 (t);
         f2 ();
      }
   }
   cout << endl;
}

static void testfunc ()
{
}

struct A 
{
   public:
      void func ()
      {
      }

      int val;
};


struct B : public A
{
   public:
      virtual void func ()
      {
      }

      virtual void func3 (const int & i)
      {
      }

      virtual void func2 (int i, int j)
      {
      }
};


int main (int argc, char ** args)
{
   cout << "Normal function pointer\n";
   doTest<void (*) ()> (&testfunc);

   // ---

   cout << "boost::bind of free function\n";
   doTest<boost::function< void () > > (boost::bind (&testfunc));

   // ---
   A a;

   cout << "boost::bind of non-static member without param\n";
   doTest<boost::function<void ()> > (boost::bind (&A::func, boost::ref(a)));

   B b ;
   cout << "boost::bind of non-static virtual member without param \n";
   doTest<boost::function<void ()> > (boost::bind (&B::func,boost::ref(b) ));
   
   cout << "boost::bind of non-static virtual member with one param\n";
   doTest<boost::function<void ()> > (boost::bind (&B::func3,boost::ref(b), 1));

   cout << "boost::bind of non-static virtual member with multiple params\n";
   doTest<boost::function<void ()> > (boost::bind (&B::func2,boost::ref(b), 1,
            2));


}
