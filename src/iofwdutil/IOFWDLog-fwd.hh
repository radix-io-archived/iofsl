#ifndef IOFWDUTIL_IOFWDLOG_FWD_HH
#define IOFWDUTIL_IOFWDLOG_FWD_HH

namespace iofwdutil
{

   /**
    * This header defines a forward for iofwdutil::IOFWDLogSource
    * to speed up compilation.
    */
   
   namespace zlog
   {
      class ZLogSource;
   }

   typedef zlog::ZLogSource IOFWDLogSource;

}

#endif
