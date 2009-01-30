#ifndef IOFWDUTIL_ZLOG_ZLOGCLASS_HH
#define IOFWDUTIL_ZLOG_ZLOGCLASS_HH

#include <string>
#include <boost/array.hpp>
#include "zlog/ZLog.hh"
#include <zlog/ZLogSink.hh>

namespace iofwdutil
{
   namespace zlog
   {

class ZLogSource
{
public:
   ZLogSource (const std::string & classname); 

   void doLog (int level, const std::string & str)
   {
      if (sink_[level] != 0)
         sink_[level]->acceptData (level, str); 
   }

private:
   const std::string class_; 
   ZLog & zlog_; 
   boost::array<ZLogSink *, MAX_LEVEL> sink_; 
}; 


   }
}
#endif
