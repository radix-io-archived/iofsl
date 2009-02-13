#include <string>
#include "BMI.hh"
#include "BMIContext.hh"
#include "BMIException.hh"

using namespace boost; 
using namespace std; 

extern "C" 
{
#include <bmi.h>
}

namespace iofwdutil
{
   namespace bmi
   {
//===========================================================================

bool            BMI::initparams_  = false; 
std::string     BMI::methodlist_; 
std::string     BMI::listen_; 
int             BMI::flags_; 


BMI::BMI()
{
   BMI_initialize (methodlist_.c_str(), 
         listen_.c_str(), flags_); 
}

BMI::~BMI()
{
}

BMIContextPtr BMI::openContext ()
{
   bmi_context_id ctx; 
   check(BMI_open_context (&ctx)); 
   return BMIContextPtr(new BMIContext (ctx)); 
}

void BMI::setInitServer (const char * listen)
{
   if (!listen)
      throw BMIException ("no listen address specified in"
            " BMI::setInitServer"); 

   const std::string s (listen); 
   string::size_type p= s.find ("://"); 
   if (p == string::npos)
   {
      throw BMIException ("invalid server listen address in"
            " BMI::setInitServer"); 
   }

   const std::string method = std::string("bmi_") + 
      s.substr (0, p); 

   setInitParams (method.c_str(), listen, BMI_INIT_SERVER); 
}

void BMI::setInitParams (const char * methodlist, 
      const char * listen, int flags)
{
   BOOST_ASSERT (!initparams_); 
   if (methodlist != 0)
      methodlist_ = methodlist;
   if (listen != 0)
      listen_ = listen; 
   flags_ = flags;
   initparams_ = true; 
}

void BMI::handleBMIError (int retcode)
{
   // Should handle BMI_NON_ERROR_BIT differently here
   throw BMIException (retcode); 
}

int BMI::testUnexpected (int in, struct BMI_unexpected_info * info, 
      int maxidle)
{
   int returned = 0; 
   BMI::check(BMI_testunexpected (in, &returned, info, maxidle)); 
   return returned; 
}
//===========================================================================
   }
}
