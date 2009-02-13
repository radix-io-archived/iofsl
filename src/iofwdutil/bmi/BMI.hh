#ifndef IOFWDUTIL_BMI_HH
#define IOFWDUTIL_BMI_HH

#include <string>
#include <boost/assert.hpp>
#include "BMIContext.hh"

extern "C"
{
#include <bmi.h>
}

namespace iofwdutil
{

   namespace bmi
   {
//===========================================================================

   // Forward
   class BMIContext; 
   class BMIAddr; 

   /**
    * OO Interface to BMI library
    */
   class BMI 
   {
   public:
      enum AllocType { ALLOC_SEND = ::BMI_SEND, ALLOC_RECEIVE = ::BMI_RECV }; 
   public:
      /** 
       * Needs to be called before get() to make sure that we can provide
       * correct paramaters to BMI initialize
       */
      static void  setInitParams (const char * list,
            const char * listen, int flags); 


      /**
       * Start a listening socket on the specified address 
       */
      static void setInitServer (const char * listen);

      static BMI & get ()
      {
         // NOTE: not thread safe
         static BMI singleton; 
         BOOST_ASSERT (initparams_); 
         return singleton; 
      }

      BMI (); 

      ~BMI (); 

      BMIContextPtr openContext (); 

      void * alloc (BMIAddr addr, size_t memsize, AllocType type );
      
      void free (BMIAddr addr, void * buffer, size_t memsize, AllocType type); 

      int testUnexpected (int in, struct BMI_unexpected_info * info,
            int max_idle);

   protected:
      friend class BMIContext;
      friend class BMIAddr; 
      friend class BMIOp; 

      inline 
      static bool check (int retcode)
      {
         if (!retcode)
            return true; 
         handleBMIError (retcode); 
         /* might not get here (exception above?) */ 
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
