#ifndef IOFWDUTIL_BMI_HH
#define IOFWDUTIL_BMI_HH

#include <iostream>
#include <string>
#include <boost/assert.hpp>
#include "BMIContext.hh"
#include "iofwdutil/assert.hh"
#include "iofwdutil/Singleton.hh"

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
   class BMI : public Singleton<BMI>
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

      /**
       * Determine method from address and initialize BMI as a client
       * using that method.
       */
      static void setInitClient ();

      static bool isCreated ()
      { return created_; };

      static BMI & get ()
      {
        return instance();
      }

      BMI ();

      ~BMI ();

      BMIContextPtr openContext ();

      void * alloc (BMIAddr addr, size_t memsize, AllocType type )
      {
         void * ret = BMI_memalloc (addr, memsize, static_cast<bmi_op_type>(type));
         ASSERT(ret);
         return ret;
      }

      void free (BMIAddr addr, void * buffer, size_t memsize, AllocType type)
      {
         check(BMI_memfree(addr, buffer, memsize, static_cast<bmi_op_type>(type)));
      }

      int testUnexpected (int in, struct BMI_unexpected_info * info,
            int max_idle);


      // Shut down BMI
      void finalize ();

      // Throw exception if a BMI error occurs
      inline
      static int check (int retcode)
      {
         if (retcode>=0)
            return retcode;

         return handleBMIError (retcode);
      }


   protected:
      friend class BMIContext;
      friend class BMIAddr;
      friend class BMIOp;

      static int handleBMIError (int retcode);

      static std::string addressToMethod (const char * addr);


   protected:
      static std::string methodlist_;
      static std::string listen_;
      static int         flags_;
      static bool        initparams_;

      bool active_;

      static bool created_;
   };



//===========================================================================
   }
}

#endif
