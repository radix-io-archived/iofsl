#include "IOFWDMain.hh"
#include "frontend/IOFWDFrontend.hh"
#include "iofwdutil/bmi/BMI.hh"
#include "iofwdutil/IOFWDLog.hh"

using namespace iofwdutil::bmi; 
using namespace iofwdutil; 

namespace iofwd
{
//===========================================================================

IOFWDMain::IOFWDMain ()
   : mainlog_ (IOFWDLog::getSource ())
{
}

void IOFWDMain::boot ()
{
   ZLOG_DEBUG (mainlog_, "Initializing BMI"); 
   // init BMI

   BMI::setInitServer ("tcp://127.0.0.1:1234"); 
   
   // Make sure we have a context open
   BMIContextPtr ctx = BMI::get().openContext (); 
   
   ZLOG_DEBUG (mainlog_, "Starting IOFWD Frontend"); 
   frontend_.reset (new frontend::IOFWDFrontend ()); 
   frontend_->init (); 
}


void IOFWDMain::shutdown ()
{
   ZLOG_DEBUG (mainlog_, "Stopping IOFWD Frontend"); 
   frontend_->destroy (); 
}


void IOFWDMain::run ()
{
}


//===========================================================================
}
