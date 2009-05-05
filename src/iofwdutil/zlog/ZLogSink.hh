#ifndef IOFWDUTIL_ZLOG_ZLOGSINK_HH
#define IOFWDUTIL_ZLOG_ZLOGSINK_HH

#include <string>

namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

// forward
class ZLogSource; 

class ZLogSink
{
public:

   virtual void acceptData (int level, const ZLogSource & source, 
         const std::string & str) = 0; 

   virtual void setOption (const std::string & name, 
         const std::string & val) = 0; 

   /// Called before the actual logging starts
   virtual void initialize () = 0; 

   virtual ~ZLogSink (); 
}; 


//===========================================================================
   }
}

#endif
