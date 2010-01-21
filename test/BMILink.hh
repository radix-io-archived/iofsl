#ifndef TEST_BMILINK_HH
#define TEST_BMILINK_HH

#include <string>

#include "iofwdevent/SingleCompletion.hh"
#include "iofwdevent/BMIResource.hh"
#include "iofwdutil/assert.hh"

#ifdef USE_BOOST_TEST
#include "test/boost/ThreadSafety.hh"
#endif

namespace test
{
   //========================================================================


   inline void checkBMI (int ret)
   {
#ifdef USE_BOOST_TEST
      BOOST_CHECK_TS(ret >= 0);
#else
      ALWAYS_ASSERT(ret >= 0);
#endif
   }


   /**
    * This class forces BMI to make a connection between two addresses
    *
    * This won't be needed any longer in a future BMI branch.
    */
   class SetupLink 
   {
      public:
         SetupLink (iofwdevent::BMIResource & res, const char * remote)
            : bmi_(res),
            remote_(remote)
      {
      }


         void findEndPoints ()
         {
            iofwdevent::SingleCompletion waitReceive;
            iofwdevent::SingleCompletion waitSend;

            char dummy = 66;
            int out;
            BMI_unexpected_info info;

            checkBMI (BMI_addr_lookup (&p1_, remote_.c_str()));
            bmi_.post_sendunexpected (waitSend, p1_,
                  &dummy, sizeof(dummy), BMI_EXT_ALLOC,
                  0, 0);
            bmi_.post_testunexpected (waitReceive, 1,
                  &out, &info);

#ifdef USE_BOOST_TEST
            BOOST_TEST_MESSAGE_TS("Waiting for message arrival");
#endif
            waitSend.wait ();
            waitReceive.wait ();

#ifdef USE_BOOST_TEST
            BOOST_CHECK_EQUAL_TS(info.size, sizeof(dummy));
            BOOST_CHECK_EQUAL_TS(* static_cast<char*>(info.buffer), dummy);
            BOOST_CHECK_EQUAL_TS(info.tag, 0);
            BOOST_CHECK_TS(info.error_code >= 0);
#endif

            p2_ = info.addr;

            BMI_unexpected_free (p2_, info.buffer);

         }

         BMI_addr_t getP1 () const
         { return p1_; }

         BMI_addr_t getP2 () const
         { return p2_; }

      protected:
         iofwdevent::BMIResource & bmi_;
         BMI_addr_t p1_;
         BMI_addr_t p2_;

         std::string remote_;
   };

   //========================================================================
}

#endif
