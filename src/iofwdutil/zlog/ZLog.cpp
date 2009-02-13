#include "zlog/ZLog.hh"
#include "zlog/ZLogSinkFile.hh"
#include "zlog/ZLogSource.hh"
#include "iofwdutil/tools.hh"

namespace iofwdutil
{
   namespace zlog
   {

      boost::array<const char *, MAX_LEVEL> ZLog::_levelNames; 


      ZLog::ZLog ()
      {
         _levelNames[DEBUG_EXTREME]="debug_e";
         _levelNames[DEBUG_MORE]="debug_m"; 
         _levelNames[DEBUG]="debug";
         _levelNames[INFO]="info";
         _levelNames[WARN]="warn";
         _levelNames[CRITICAL]="critical";
         _levelNames[ERROR]="error"; 

      }

      ZLogSink * ZLog::getSinkByName (const char * UNUSED(name))
      {
         static ZLogSinkFile sink_stderr; 
         return &sink_stderr; 
      }

      void ZLog::doConfig (ZLogSource & source)
      {
         const std::string & UNUSED(name) (source.getSourceName()); 

         /* no specific config for now: set all levels to stderr */ 
         for (int i=0; i<MAX_LEVEL; ++i)
            source.setSink (i, getSinkByName ("stderr")); 
      }

   }
}
