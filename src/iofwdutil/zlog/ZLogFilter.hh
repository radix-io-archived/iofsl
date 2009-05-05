#ifndef IOFWDUTIL_ZLOG_ZLOGFILTER_HH
#define IOFWDUTIL_ZLOG_ZLOGFILTER_HH

namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

/**
 * This class enables modification of the log messages before they
 * hit the sink. Filters can be stacked and have access to the source 
 * the log message came from.
 */
class ZLogFilter
{
public:

   virtual ~ZLogFilter (); 
}; 

//===========================================================================
   }
}


#endif
