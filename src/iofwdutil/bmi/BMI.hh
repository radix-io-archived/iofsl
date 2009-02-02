#ifndef IOFWDUTIL_BMI_HH
#define IOFWDUTIL_BMI_HH

#include <string>
#include <boost/assert.hpp>


namespace iofwdutil
{

   namespace bmi
   {
//===========================================================================

   // Forward
   class BMIContext; 

   /**
    * OO Interface to BMI library
    */
   class BMI
   {
   public:
      /** 
       * Needs to be called before get() to make sure that we can provide
       * correct paramaters to BMI initialize
       */
      static void  setInitParams (const char * list,
            const char * listen, int flags); 


      static BMI & get ()
      {
         static BMI singleton; 
         BOOST_ASSERT (initparams_); 
         return singleton; 
      }

      BMI (); 

      ~BMI (); 

      BMIContext openContext (); 

   protected:
      friend class BMIContext;
      friend class BMIAddr; 

      inline 
      static bool check (int retcode)
      {
         if (!retcode)
            return true; 
         handleBMIError (retcode); 
         return false; 
      }

      static void handleBMIError (int retcode);

   protected:
      static std::string methodlist_; 
      static std::string listen_; 
      static int         flags_; 
      static bool        initparams_; 

   }; 



//===========================================================================
   }
}

#endif
