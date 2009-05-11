#ifndef IOFWDUTIL_ZLOG_ZLOGSINKSTD_HH
#define IOFWDUTIL_ZLOG_ZLOGSINKSTD_HH

#include "ZLogSinkFile.hh"

namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================


/**
 * Sink that outputs to stdout / stderr
 */
class ZLogSinkStd : public ZLogSinkFile 
{
public:

   ZLogSinkStd (); 

   virtual ~ZLogSinkStd (); 

   virtual void setOption (const std::string & name, const std::string & val);

protected:
   virtual void openFile (); 

protected:

   /// If output goes to stderr or stdout
   bool stderr_; 
   
};
//===========================================================================
   }
}

#endif

