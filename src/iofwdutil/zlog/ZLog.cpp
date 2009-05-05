#include "zlog/ZLog.hh"
#include "zlog/ZLogSinkFile.hh"
#include "zlog/ZLogSource.hh"
#include "iofwdutil/tools.hh"

namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

boost::array<const char *, MAX_LEVEL> ZLog::levelNames_; 


ZLog::ZLog ()
{
   levelNames_[DEBUG_EXTREME]="debug_e";
   levelNames_[DEBUG_MORE]="debug_m"; 
   levelNames_[DEBUG]="debug";
   levelNames_[INFO]="info";
   levelNames_[WARN]="warn";
   levelNames_[CRITICAL]="critical";
   levelNames_[ERROR]="error"; 
}

ZLog::~ZLog ()
{
}

//===========================================================================
   }
}
