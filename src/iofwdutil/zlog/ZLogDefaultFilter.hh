#ifndef IOFWDUTIL_ZLOG_ZLOGDEFAULTFILTER_HH
#define IOFWDUTIL_ZLOG_ZLOGDEFAULTFILTER_HH

#include <boost/format.hpp>
#include "ZLogFilter.hh"

namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

class ZLogDefaultFilter : public ZLogFilter
{
public:
   ZLogDefaultFilter (); 

   virtual ~ZLogDefaultFilter (); 

   virtual void initialize (); 

   virtual void filterMsg (const ZLogSource & source, int, std::string & msg); 

   virtual void setOption (const std::string & name, 
         const std::string & val); 

protected:
   std::string getTime () const; 

protected:
   boost::format fmt_; 
}; 

//===========================================================================
   }
}

#endif
