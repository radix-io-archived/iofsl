#ifndef IOFWDUTIL_ZLOG_ZLOGSINKFILE_HH
#define IOFWDUTIL_ZLOG_ZLOGSINKFILE_HH

#include <fstream>
#include <boost/thread.hpp>
#include <memory>
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
   ZLogSinkFile (); 

   virtual void acceptData (int level, const ZLogSource & source, const
         std::string & msg); 

   virtual void initialize (); 

   virtual void setOption (const std::string & name, const std::string & val);

   virtual ~ZLogSinkFile ();

protected:
   virtual void openFile (); 

protected:

   std::auto_ptr<std::ofstream> output_; 
   std::string filename_; 

   boost::mutex outputlock_; 
}; 


//===========================================================================
   }
}

#endif
