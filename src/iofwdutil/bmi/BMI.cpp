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


static const char * string2ptr (const std::string & s)
{
   if (s.length())
      return s.c_str(); 
   return 0; 
}

BMI::BMI()
{
   check(BMI_initialize (string2ptr(methodlist_), 
         string2ptr(listen_), flags_)); 
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

std::string BMI::addressToMethod (const char * addr)
{
   const std::string s (addr); 
   string::size_type p= s.find ("://"); 
   if (p == string::npos)
      throw BMIException ("invalid listen address!"); 

   return std::string("bmi_") + 
      s.substr (0, p); 
}

void BMI::setInitClient ()
{
   setInitParams (0,0,0); 
}

void BMI::setInitServer (const char * listen)
{
   if (!listen)
      throw BMIException ("no listen address specified in"
            " BMI::setInitServer"); 

   setInitParams (addressToMethod(listen).c_str(), 
         listen, BMI_INIT_SERVER); 
}

void BMI::setInitParams (const char * methodlist, 
      const char * listen, int flags)
{
   ASSERT (!initparams_); 
   if (methodlist != 0)
      methodlist_ = methodlist;
   if (listen != 0)
      listen_ = listen; 
   flags_ = flags;
   initparams_ = true; 
}

int BMI::handleBMIError (int retcode)
{
   if ((retcode & BMI_NON_ERROR_BIT)==BMI_NON_ERROR_BIT)
   {
      std::cerr << "Warning: BMI_NON_ERROR_BIT\n"; 
      return retcode; 
   }
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
