#ifndef IOFWDUTIL_ZLOG_ZLOGSINKFILE_HH
#define IOFWDUTIL_ZLOG_ZLOGSINKFILE_HH

#include <fstream>
#include "ZLogSink.hh"

namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

class ZLog; 

class ZLogSinkFile : public ZLogSink
{
public:
   ZLogSinkFile (ZLog & log); 

   virtual void acceptData (int level, const ZLogSource & source, const
         std::string & msg); 

   virtual void setOption (const std::string & name, const std::string & val);

   virtual void initialize (); 

   virtual ~ZLogSinkFile ();

protected:
   ZLog & zlog_; 

   std::ofstream output_; 
   std::string filename_; 
}; 


//===========================================================================
   }
}

#endif
