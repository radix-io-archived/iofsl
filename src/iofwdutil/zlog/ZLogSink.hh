#ifndef IOFWDUTIL_ZLOG_ZLOGSINK_HH
#define IOFWDUTIL_ZLOG_ZLOGSINK_HH

#include <string>

namespace iofwdutil
{
   namespace zlog
   {

class ZLogSink
{
public:
   virtual void acceptData (int level, const std::string & str) = 0; 

   virtual ~ZLogSink (); 
}; 


   }
}

#endif
