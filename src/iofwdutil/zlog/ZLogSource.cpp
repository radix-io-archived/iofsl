#include <algorithm>
#include "zlog/ZLogSource.hh"


namespace iofwdutil
{
   namespace zlog
   {

      ZLogSource::ZLogSource (const std::string & classname)
         : class_(classname), zlog_(ZLog::get())
      {
         // Initialize to zero 
         std::fill (sink_.begin(), sink_.end(), static_cast<ZLogSink*>(0)); 
         zlog_.doConfig (*this); 
      }


   }
}
