#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include "zlog/ZLog.hh"
#include "zlog/ZLogSinkFile.hh"
#include "zlog/ZLogSource.hh"
#include "iofwdutil/tools.hh"

namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

boost::array<const char *, MAX_LEVEL> ZLog::levelNames_ =
  { { "error", "critical", "warn", "info", "debug", "debug_more",
     "debug_extreme" } }; 

std::string ZLog::filterRegEx (const std::string & reg)
{
   // Replace * by [^.] and quote all other special characters
   static boost::regex replacereg ("([.\\[\\]{}()\\+?|^$])"); 
   std::string res = boost::regex_replace (reg, replacereg, "\\\\\\1"); 
   boost::trim (res); 
   boost::replace_all (res, "*", "[^.]*"); 
   
   return "^" + res + "$"; 
}

bool ZLog::matchSourceName (const std::string & regex, const std::string &
      sourcename)
{
   boost::regex reg (filterRegEx (regex)); 
   return boost::regex_match (sourcename, reg); 
}

ZLog::ZLog ()
{
   levelNames_[DEBUG_EXTREME]="debug_extreme";
   levelNames_[DEBUG_MORE]="debug_more"; 
   levelNames_[DEBUG]="debug";
   levelNames_[INFO]="info";
   levelNames_[WARN]="warn";
   levelNames_[CRITICAL]="critical";
   levelNames_[ERROR]="error"; 
}

int ZLog::name2Level (const std::string & name)
{
   const std::string lower (boost::to_lower_copy (name)); 
   for (unsigned int i=0; i<levelNames_.size(); ++i)
   {
      if (levelNames_[i] == lower)
         return i; 
   }
   return -1; 
}


ZLog::~ZLog ()
{
}

//===========================================================================
   }
}
