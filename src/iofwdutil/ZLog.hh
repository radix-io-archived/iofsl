#ifndef IOFWDUTIL_ZLOG_HH
#define IOFWDUTIL_ZLOG_HH

namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

enum 
{ 
       ZLOG_DEBUG_EXTREME = 0,
       ZLOG_DEBUG_MODERATE, 
       ZLOG_DEBUG,
       ZLOG_NOTICE,
       ZLOG_INFO,
       ZLOG_WARN,
       ZLOG_CRITICAL,
       ZLOG_ERROR = 99
}; 


//===========================================================================
   }
}

#endif
