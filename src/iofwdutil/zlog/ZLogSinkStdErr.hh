#ifndef IOFWDUTIL_ZLOG_ZLOGSINKSTDERR_HH
#define IOFWDUTIL_ZLOG_ZLOGSINKSTDERR_HH

#include "ZLogSinkStd.hh"

namespace iofwdutil
{
   namespace zlog
   {
      //=====================================================================

      class ZLogSinkStdErr : public ZLogSinkStd
      {
      public:
         ZLogSinkStdErr () 
         {
            stderr_ = true; 
         }

         virtual ~ZLogSinkStdErr (); 
         
      }; 
      
      //=====================================================================
   }
}
#endif
