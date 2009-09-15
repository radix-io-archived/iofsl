#include "Frontend.hh"
#include "iofwdutil/assert.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

void Frontend::setConfig (const iofwdutil::ConfigFile & c)
{
   config_ = c;
}

void Frontend::setHandler (RequestHandler * handler)
{
   ALWAYS_ASSERT (handler); 
   handler_ = handler; 
}

Frontend::~Frontend ()
{
}

        
//===========================================================================
   }
}
