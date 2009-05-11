#ifndef IOFWDUTIL_ZLOG_ZLOGFILTER_HH
#define IOFWDUTIL_ZLOG_ZLOGFILTER_HH

#include <string>


namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

class ZLogSource; 

/**
 * This class enables modification of the log messages before they
 * hit the sink. Filters can be stacked and have access to the source 
 * the log message came from.
 */
class ZLogFilter
{
public:

   virtual void setOption (const std::string & name, 
         const std::string & value); 

   virtual void initialize () = 0; 

   virtual void filterMsg (const ZLogSource &, int level, std::string & msg) = 0; 

   virtual ~ZLogFilter (); 
}; 

//===========================================================================
   }
}


#endif
