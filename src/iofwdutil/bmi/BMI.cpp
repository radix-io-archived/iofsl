#include "BMI.hh"
#include "BMIContext.hh"
#include "BMIException.hh"

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
}

BMI::~BMI()
{
}

BMIContext BMI::openContext ()
{
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


//===========================================================================
   }
}
