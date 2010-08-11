#include <boost/format.hpp>

#include <boost/test/unit_test.hpp>
#include "ThreadSafety.hh"
#include "iofwdevent/BMIResource.hh"
#include "iofwdevent/SingleCompletion.hh"
#include "iofwdevent/ResourceWrapper.hh"
#include "iofwdevent/NOPCompletion.hh"
#include "iofwdevent/CBException.hh"

extern "C" {
#include <bmi.h>
}


using namespace iofwdevent;

using boost::format;

//===========================================================================
//===========================================================================
//===========================================================================

class BMIInit
{
public:
   enum { SERVER = 0x01, CLIENT = 0x02 };

   BMIInit (int flags);

   BMIInit (const char * methods, const char * listen, int flags);

   ~BMIInit ();

   /**
    * Return BMI address of self for use in send_unexpected.
    */
   BMI_addr_t unexpected_self() const;

protected:
   void inithelper (const char * methods, const char * listen, int flags);

protected:
   int flags_;
   BMI_addr_t unexpected_self_;
};

  BMIInit::BMIInit (int flags)
      {
         int fl = ( flags & SERVER ? BMI_INIT_SERVER : 0);
         inithelper ("bmi_tcp", "tcp://127.0.0.1:1234", fl);
      }

      BMIInit::BMIInit (const char * methods, const char * listen, int flags)
      {
         inithelper (methods, listen, flags);
      }

      BMIInit::~BMIInit ()
      {
         BMI_finalize ();
      }

      void BMIInit::inithelper (const char * methods, const char * listen, int
            flags)
      {
         flags_ = flags;
         if (BMI_initialize (methods, listen, flags)<0)
         {
            throw "Error initializing BMI!";
         }

         if (flags & SERVER)
         {
            BMI_addr_lookup (&unexpected_self_, listen);
         }
      }

      BMI_addr_t BMIInit::unexpected_self() const
      {
         ALWAYS_ASSERT(flags_ & SERVER);
         return unexpected_self_;
      }



// ==========================================================================
// ==========================================================================
// ==========================================================================

BOOST_AUTO_TEST_SUITE( bmiresource )

struct BMIFixture
{
   BMIFixture () : init_(BMIInit::SERVER),
                        wrapper_(&bmi_)
   {
   }

 
   BMIInit init_;
   iofwdevent::BMIResource bmi_;
   iofwdevent::ResourceWrapper wrapper_;
};

BOOST_FIXTURE_TEST_CASE( unexpected, BMIFixture )
{
   BOOST_TEST_MESSAGE_TS("Testing unexpected receives");

   // Waiting for unexpected receive (any receive)
   SingleCompletion waitreceive;

   BMI_unexpected_info info;
   int completed = 0;
   bmi_.post_testunexpected (waitreceive, 1, &completed, &info);

   /* send unexpected message and wait for it to arrive */
   bmi_.post_sendunexpected (NOPCompletion (), init_.unexpected_self(), 
         &completed, sizeof(completed), BMI_EXT_ALLOC, 2,  0);
   waitreceive.wait();
   BMI_unexpected_free (info.addr, info.buffer);

   BOOST_CHECK_EQUAL_TS(completed, 1);
   BOOST_CHECK_EQUAL_TS(info.tag, 2);

   BOOST_TEST_MESSAGE_TS("Unexpected receive with specific tag");

   waitreceive.reset ();

   /* wait for receive with specific tag */
   bmi_.post_testunexpected (waitreceive, 1, &completed, &info,
         std::make_pair(3,3));
   bmi_.post_sendunexpected (NOPCompletion (), init_.unexpected_self(),
         &completed, sizeof(completed), BMI_EXT_ALLOC, 3, 0);
   waitreceive.wait ();
   BMI_unexpected_free (info.addr, info.buffer);

   BOOST_CHECK_EQUAL_TS(completed, 1);
   BOOST_CHECK_EQUAL_TS(info.tag, 3);

   BOOST_TEST_MESSAGE_TS("Unexpected receive with specific tag in presence"
         " of other tags");
   
   SingleCompletion waitreceive2;
   BMI_unexpected_info info2;
   int completed2 = 0;
   completed = 0;
   waitreceive.reset();

   bmi_.post_testunexpected (waitreceive, 1, &completed, &info,
         std::make_pair(3,3));
   bmi_.post_testunexpected (waitreceive2, 1, &completed2, &info2,
         std::make_pair(2,2));
   bmi_.post_sendunexpected (NOPCompletion(), init_.unexpected_self(),
         &completed, sizeof(completed), BMI_EXT_ALLOC, 2, 0);
   waitreceive2.wait();
   BMI_unexpected_free (info2.addr, info2.buffer);

   BOOST_CHECK_EQUAL_TS(completed2, 1);
   BOOST_CHECK_EQUAL_TS(info2.tag, 2);
   BOOST_CHECK_EQUAL_TS(completed, 0);

   bmi_.post_sendunexpected (NOPCompletion(), init_.unexpected_self(),
         &completed, sizeof(completed), BMI_EXT_ALLOC, 3, 0);
   waitreceive.wait();
   BMI_unexpected_free (info.addr, info.buffer);

   BOOST_CHECK_EQUAL_TS(completed, 1);
   BOOST_CHECK_EQUAL_TS(info.tag, 3);
}

// ==========================================================================
// ==========================================================================
// ==========================================================================

static void callback (const iofwdevent::CBException & e, bool & cancelled)
{
   BOOST_CHECK_TS(e.isCancelled());
   cancelled = true;
}


BOOST_FIXTURE_TEST_CASE ( cancelunexpectedreceive, BMIFixture )
{
   int count = 0;
   BMI_unexpected_info info;
   bool cancelled = false;

   iofwdevent::BMIResource::Handle h = bmi_.post_testunexpected (
         boost::bind (&callback, _1, boost::ref(cancelled)),
         1, &count, &info);

   bmi_.cancel (h);

   BOOST_CHECK_EQUAL_TS(cancelled, true);
}

// ==========================================================================
// ==========================================================================
// ==========================================================================

struct UEComm
{
   boost::mutex lock;
   size_t count;
   boost::condition_variable cond;
};

struct UECallback
{
   UECallback (iofwdevent::BMIResource & res, int tag, size_t * i,
         UEComm * c)
      : bmi_(res), tag_(tag), calls_(0), output_(i),
      com_(c)
   {
         handle_ = bmi_.post_testunexpected (*this, info_.size(), 
            &completed_, &info_[0], std::make_pair(tag_,tag_));
   }

   operator iofwdevent::CBType ()
   {
      return boost::bind (&UECallback::callback, boost::ref(*this), _1);
   }

 protected:

   /**
    * Increment counter, and if not finished reregister for callback.
    */
   void callback (const iofwdevent::CBException & e)
   {
      BOOST_TEST_MESSAGE_TS(format("Callback for tag %i called") % tag_);
      BOOST_CHECK_TS(!e.hasException ());
      BOOST_CHECK_TS (completed_ != 0);
      bool stop = false;

      for (int i=0; i<completed_; ++i)
      {
         BOOST_CHECK_EQUAL_TS (info_[i].tag, tag_);
         const size_t val = *static_cast<size_t*> (info_[i].buffer);
         BMI_unexpected_free (info_[i].addr, info_[i].buffer);
         
         if (val == 1)
         {
            stop = true;
            // stop message should be last one for this tag
            BOOST_CHECK_EQUAL_TS (i, completed_ - 1);
            break;
         }
      }

      calls_ += completed_;
      if (!stop)
      {
         handle_ = bmi_.post_testunexpected (*this, info_.size(), 
            &completed_, &info_[0], std::make_pair(tag_,tag_));
      }
      else
      {
         ready ();
      }
   }
   
   void ready ()
   {
      BOOST_TEST_MESSAGE_TS(format("Ready %i") % tag_);
      *output_ = calls_;
      boost::mutex::scoped_lock l(com_->lock);
      ++com_->count;
      com_->cond.notify_one ();
   }

   void cancel ()
   {
      bmi_.cancel (handle_);
   }

 protected:
   iofwdevent::BMIResource & bmi_;
   int tag_;

   size_t calls_;

   iofwdevent::BMIResource::Handle handle_;
   int completed_;
   boost::array<BMI_unexpected_info, 6>  info_;
   size_t * output_;

   UEComm * com_;

   boost::mutex lock_;
};


static void threadfunc (iofwdevent::BMIResource & bmi_,
      size_t maxtag, size_t count, BMI_addr_t dest, 
      unsigned int s)
{
   size_t val = 0;
   unsigned int seed = s;
   SingleCompletion wait;
   for (size_t i=0; i<count; ++i)
   {
      int tag =  rand_r (&seed) % maxtag;
      BOOST_TEST_MESSAGE_TS(format("Sending tag %i (iter %i)") % tag % i);
      bmi_.post_sendunexpected (wait, dest, &val, sizeof(val),
            BMI_EXT_ALLOC, tag, 0);
      wait.wait ();
      wait.reset();
   }
}

BOOST_FIXTURE_TEST_CASE( multithreaded_testunexpected, BMIFixture)
{
   const size_t MAXTAG = 10;
   const size_t COUNT = 100;
   const size_t THREADS = 10;


   UEComm c;
   c.count = 0;

   BOOST_TEST_MESSAGE_TS("Multiple threads handling unexpected messages");

   std::vector<UECallback *> cb;
   std::vector<size_t> call (MAXTAG, 0);

   cb.reserve(MAXTAG);
   for (size_t i=0; i<MAXTAG; ++i)
   {
      cb.push_back(new UECallback (bmi_, i, &call[i], &c));
   }

   boost::thread_group g;
   for (size_t i=0; i<THREADS; ++i)
   {
      g.create_thread (boost::bind (&threadfunc, boost::ref(bmi_),
               MAXTAG, COUNT, init_.unexpected_self(), i));
   }
   g.join_all ();


   // signal end
   size_t val = 1;
   SingleCompletion wait;
   for (size_t i=0; i<MAXTAG; ++i)
   {
      bmi_.post_sendunexpected (wait, init_.unexpected_self(),
            &val, sizeof(val), BMI_EXT_ALLOC, i, 0);
      wait.wait ();
      wait.reset();
   }


   {
     boost::mutex::scoped_lock l(c.lock);
     while (c.count != MAXTAG)
     {
        c.cond.wait (l);
     }
   }

   size_t calls = 0;
   for (size_t i=0; i<MAXTAG; ++i)
   {
      calls += call[i];
      delete (cb[i]);
   }

   BOOST_CHECK_EQUAL_TS(calls, COUNT * THREADS + MAXTAG * 1);

}

// ==========================================================================
// ==========================================================================
// ==========================================================================

BOOST_AUTO_TEST_SUITE_END()
