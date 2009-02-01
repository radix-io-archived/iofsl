#include "BMI.hh"
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
   methodlist_ = methodlist;
   listen_ = listen; 
   flags_ = flags;
   initparams_ = true; 
}

void BMI::handleBMIError (int retcode)
{
   
}

//===========================================================================
   }
}
