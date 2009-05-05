#ifndef IOFWDUTIL_ZLOG_ZLOGCLASS_HH
#define IOFWDUTIL_ZLOG_ZLOGCLASS_HH

#include <string>
#include <vector>
#include <boost/array.hpp>
#include <boost/format/format_fwd.hpp>
#include "zlog/ZLog.hh"
#include <zlog/ZLogSink.hh>

namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

class ZLogFilter; 

class ZLogSource
{
public:
   ZLogSource (const std::string & classname); 

   const std::string & getSourceName () const
   { 
      return class_; 
   }

   void doLog (int level, const boost::format & fmt); 

   void doLog (int level, const std::string & str)
   {
      // Does not need locking: adding filters / changing sinks is not allowed
      // once logging starts.
      if (sink_[level] != 0)
         sink_[level]->acceptData (level,*this, str); 
   }

   // Source does not take ownership of the sink
   void setSink (int level, ZLogSink * sink);

   // Source does not take ownership of the filter
   void addFilter (ZLogFilter * filter); 

private:
   const std::string class_; 

   boost::array<ZLogSink *, MAX_LEVEL> sink_; 
   std::vector<ZLogFilter *> filters_; 
}; 


//===========================================================================
   }
}
#endif
