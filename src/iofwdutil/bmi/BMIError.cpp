#include "BMIError.hh"


extern "C"
{
#include <bmi.h>
}

namespace iofwdutil
{
   namespace bmi
   {
//===========================================================================

const char * BMIError::errorString (int error)
{
   switch (error)
   {
#define BE(a) case a: return #a;
        BE(BMI_EPERM);
        BE(BMI_ENOENT);
        BE(BMI_EINTR);
        BE(BMI_EIO);
        BE(BMI_ENXIO);
        BE(BMI_EBADF);
        BE(BMI_EAGAIN);
        BE(BMI_ENOMEM);
        BE(BMI_EFAULT);
        BE(BMI_EBUSY);
        BE(BMI_EEXIST);
        BE(BMI_ENODEV);
        BE(BMI_ENOTDIR);
        BE(BMI_EISDIR);
        BE(BMI_EINVAL);
        BE(BMI_EMFILE);
        BE(BMI_EFBIG);
        BE(BMI_ENOSPC);
        BE(BMI_EROFS);
        BE(BMI_EMLINK);
        BE(BMI_EPIPE);
        BE(BMI_EDEADLK);
        BE(BMI_ENAMETOOLONG);
        BE(BMI_ENOLCK);
        BE(BMI_ENOSYS);
        BE(BMI_ENOTEMPTY);
        BE(BMI_ELOOP);
        BE(BMI_EWOULDBLOCK);
        BE(BMI_ENOMSG);
        BE(BMI_EUNATCH);
        BE(BMI_EBADR);
        BE(BMI_EDEADLOCK);
        BE(BMI_ENODATA);
        BE(BMI_ETIME);
        BE(BMI_ENONET);
        BE(BMI_EREMOTE);
        BE(BMI_ECOMM);
        BE(BMI_EPROTO);
        BE(BMI_EBADMSG);
        BE(BMI_EOVERFLOW);
        BE(BMI_ERESTART);
        BE(BMI_EMSGSIZE);
        BE(BMI_EPROTOTYPE);
        BE(BMI_ENOPROTOOPT);
        BE(BMI_EPROTONOSUPPORT);
        BE(BMI_EOPNOTSUPP);
        BE(BMI_EADDRINUSE);
        BE(BMI_EADDRNOTAVAIL);
        BE(BMI_ENETDOWN);
        BE(BMI_ENETUNREACH);
        BE(BMI_ENETRESET);
        BE(BMI_ENOBUFS);
        BE(BMI_ETIMEDOUT);
        BE(BMI_ECONNREFUSED);
        BE(BMI_EHOSTDOWN);
        BE(BMI_EHOSTUNREACH);
        BE(BMI_EALREADY);
        BE(BMI_EACCES);
        BE(BMI_ECONNRESET);

        /* non-fatal errors */ 
        BE(BMI_ECANCEL);
        BE(BMI_EDEVINIT);
        BE(BMI_EDETAIL);
        BE(BMI_EHOSTNTFD);
        BE(BMI_EADDRNTFD);
        BE(BMI_ENORECVR);
        BE(BMI_ETRYAGAIN);
#undef BE

      default:
        return "(UNKNOWN)";
   }
}

//===========================================================================
     }
}

