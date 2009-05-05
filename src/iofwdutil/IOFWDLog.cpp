#include "IOFWDLog.hh"
#include "tools.hh"

namespace iofwdutil
{
//===========================================================================

zlog::ZLogSource & IOFWDLog::getSourceInt (const char * UNUSED(name))
{
}


IOFWDLog::IOFWDLog ()
{
}

IOFWDLog::~IOFWDLog ()
{
   /*for (SourceMap::const_iterator I = sources_.begin();
         I!=sources_.end(); ++I)
   {
      delete I->second; 
   }

   for (SinkMap::const_iterator I = sinks_.begin(); 
         I!=sinks_.end(); ++I)
   {
      delete I->second; 
   } */
}

//===========================================================================
}
