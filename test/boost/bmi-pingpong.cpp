#define BOOST_TEST_MODULE "BMI Ping pong using threads"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>

#include <iostream>
#include <cstdlib>

#include "iofwdevent/BMIResource.hh"
#include "iofwdevent/ThreadedResource.hh"
#include "iofwdevent/ResourceWrapper.hh"
#include "iofwdevent/SingleCompletion.hh"


#include "ThreadSafety.hh"

const char *        ADDRESS = "tcp://127.0.0.1:1236";
const unsigned int  LOOPS   = 10000;

extern "C"
{
#include <bmi.h>
}

using namespace std;
using namespace iofwdevent;
using namespace boost;

class BMIInit 
{
public:
   BMIInit (const char * methods, const char * listen, int flags)
   {
      if (BMI_initialize (methods, listen, flags)<0)
      {
         throw "Error initializing BMI!";
      }
   }

   ~BMIInit ()
   {
      BMI_finalize ();
   }
};


//____________________________________________________________________________//
   
inline void checkBMI (int ret)
   {
      BOOST_CHECK_TS(ret >= 0);
   }

//____________________________________________________________________________//

class Pingpong 
{
public:
   Pingpong (bool sender, BMIResource & bmires)
      : sender_(sender),
      bmi_ (bmires)
   {
      checkBMI (BMI_addr_lookup (&addr_, ADDRESS));
   }


   void operator () ()
   {
      SingleCompletion waitsend;
      SingleCompletion waitreceive;

      const int receivetag = (sender_ ? 0 : 1);
      const int sendtag = (sender_ ? 1 : 0);

      for (unsigned int i=0; i<LOOPS; ++i)
      {
         unsigned int received;
         bmi_size_t actual;

         if (!sender_)
         {
            bmi_.post_recv (&waitreceive, addr_, &received, sizeof(received),
                  &actual, BMI_EXT_ALLOC, receivetag, 0);
            waitreceive.wait ();

            BOOST_CHECK_EQUAL_TS (i, received);

            bmi_.post_send (&waitsend, addr_, &received, sizeof (received),
                BMI_EXT_ALLOC, sendtag, 0);
            waitsend.wait ();
         }
         else
         {
            bmi_.post_recv (&waitreceive, addr_, &received, sizeof(received),
                  &actual, BMI_EXT_ALLOC, receivetag, 0);
            bmi_.post_send (&waitsend, addr_, &i, sizeof (i), 
                BMI_EXT_ALLOC, sendtag, 0);

            waitreceive.wait ();

            BOOST_CHECK_EQUAL_TS (i, received);

            waitsend.wait ();
         }
      }
   }

protected:
   bool sender_;
   BMI_addr_t addr_;
   BMIResource & bmi_;
};

class SetupLink 
{
public:
   SetupLink (BMIResource & res)
      : bmi_(res)
   {
   }


   void findEndPoints ()
   {
      SingleCompletion waitReceive;
      SingleCompletion waitSend;

      char dummy = 66;
      int out;
      BMI_unexpected_info info;

      checkBMI (BMI_addr_lookup (&p1_, ADDRESS));
      bmi_.post_sendunexpected (&waitSend, p1_,
            &dummy, sizeof(dummy), BMI_EXT_ALLOC,
            0, 0);
      bmi_.testunexpected (&waitReceive, 1,
            &out, &info);

      BOOST_TEST_MESSAGE_TS("Waiting for message arrival");
      waitSend.wait ();
      waitReceive.wait ();

      BOOST_CHECK_EQUAL_TS(info.size, sizeof(dummy));
      BOOST_CHECK_EQUAL_TS(* static_cast<char*>(info.buffer), dummy);
      BOOST_CHECK_EQUAL_TS(info.tag, 0);
      BOOST_CHECK_TS(info.error_code >= 0);

      p2_ = info.addr;

      BMI_unexpected_free (p2_, info.buffer);

   }

   BMI_addr_t getP1 () const
   { return p1_; }

   BMI_addr_t getP2 () const
   { return p2_; }

protected:
   BMIResource & bmi_;
   BMI_addr_t p1_;
   BMI_addr_t p2_;
};
//____________________________________________________________________________//

BOOST_AUTO_TEST_SUITE( bmipingpong );

struct Fixture {
    Fixture()
       : bmiinit_ ("bmi_tcp", ADDRESS, BMI_INIT_SERVER),
         bmi_(&bmires_)
    {
       BOOST_TEST_MESSAGE_TS( "setup fixture" ); 
    }

    ~Fixture()
    {
       BOOST_TEST_MESSAGE_TS( "teardown fixture" );
    }

protected:
    BMIInit bmiinit_;
    BMIResource bmires_;
    ResourceWrapper bmi_;
};

//____________________________________________________________________________//

BOOST_FIXTURE_TEST_CASE( unexpected, Fixture )
{
   SetupLink link (bmires_);

   BOOST_TEST_MESSAGE_TS("Testing unexpected receive");
   link.findEndPoints ();
   BOOST_TEST_MESSAGE_TS("Endpoint P1: " << BMI_addr_rev_lookup(link.getP1()));
   BOOST_TEST_MESSAGE_TS("Unexpected P2: " <<
         BMI_addr_rev_lookup_unexpected(link.getP2()));
}

BOOST_FIXTURE_TEST_CASE( talkself, Fixture )
{
   BMI_addr_t addr_;
   checkBMI (BMI_addr_lookup (&addr_, ADDRESS));
   SingleCompletion waitsend;
   SingleCompletion waitreceive;
   unsigned int received;
   bmi_size_t actual;
   SetupLink link (bmires_);

   BOOST_TEST_MESSAGE_TS("Making connection");
   link.findEndPoints ();

   BMI_addr_t sendaddr = link.getP1();
   BMI_addr_t receiveaddr = link.getP2();

   BOOST_TEST_MESSAGE_TS("Starting test\n");
   for (unsigned int i=0; i<LOOPS; ++i)
   {
      BOOST_TEST_MESSAGE_TS(format("Posting for iteration %i") % i);
      if (i)
      {
         waitsend.reset();
         waitreceive.reset();
      }
      bmires_.post_recv (&waitreceive, sendaddr, &received, sizeof(received),
            &actual, BMI_EXT_ALLOC, 0, 0);
      bmires_.post_send (&waitsend, receiveaddr, &i, sizeof (i),
            BMI_EXT_ALLOC, 0, 0);
      BOOST_TEST_MESSAGE_TS("Waiting for send to complete");
      waitsend.wait ();
      BOOST_TEST_MESSAGE_TS("Waiting for receive to complete");
      waitreceive.wait ();
      BOOST_CHECK_EQUAL_TS(actual, sizeof(i));
      BOOST_CHECK_EQUAL_TS(received, i);
      BOOST_TEST_MESSAGE_TS("Receive completed");
   }
   BOOST_TEST_MESSAGE_TS("Test done");
}

/*
// this test case will use struct F as fixture
BOOST_FIXTURE_TEST_CASE( pingpong, Fixture )
{
   Pingpong sender (true, bmires_);
   Pingpong receiver (false, bmires_);
   boost::thread t1 (sender);
   boost::thread t2 (receiver);

   t1.join ();
   t2.join ();
}

*/

BOOST_AUTO_TEST_SUITE_END()


// EOF



