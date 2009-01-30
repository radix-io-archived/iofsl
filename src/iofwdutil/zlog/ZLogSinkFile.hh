#ifndef IOFWDUTIL_ZLOG_ZLOGSINKFILE_HH
#define IOFWDUTIL_ZLOG_ZLOGSINKFILE_HH

#include "ZLogSink.hh"

namespace iofwdutil
{
   namespace zlog
   {

class ZLog; 

class ZLogSinkFile : public ZLogSink
{
public:
   ZLogSinkFile (); 

   virtual void acceptData (int level, const ZLogSource & source, const
         std::string & msg); 

   virtual ~ZLogSinkFile ();

protected:
   ZLog & zlog_; 
}; 


   }
}

#endif
