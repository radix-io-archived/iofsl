#include <syscall.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include "iofsl_syscall_wrapper.h"

#ifdef IOFWDLIBSYSIO_WRAP_TRACE_ENABLE
#define _IOFWDLIBSYSIO_WRAP_TRACE fprintf(stderr, "_IOFWDLIBSYSIO_WRAP_TRACE %s(), line# = %i, file = %s\n", __func__,  __LINE__, __FILE__)
#else
#define _IOFWDLIBSYSIO_WRAP_TRACE /* do nothing */
#endif

inline int __iofwdlibsysio_wrap_fcntl(int fd, int cmd, long val)
{
    _IOFWDLIBSYSIO_WRAP_TRACE;
    return syscall(SYS_fcntl, fd, cmd, val);
}

inline int __iofwdlibsysio_wrap_close(int fd)
{
    _IOFWDLIBSYSIO_WRAP_TRACE;
    return syscall(SYS_close, fd);
}

inline int __iofwdlibsysio_wrap_read(int fd, void * buf, size_t nbyte)
{
    _IOFWDLIBSYSIO_WRAP_TRACE;
    return syscall(SYS_read, fd, buf, nbyte);
}

inline int __iofwdlibsysio_wrap_write(int fd, void * buf, size_t nbyte)
{
    _IOFWDLIBSYSIO_WRAP_TRACE;
    return syscall(SYS_write, fd, buf, nbyte);
}

inline int __iofwdlibsysio_wrap_readv(int fd, const struct iovec * iov, int iovcnt)
{
    _IOFWDLIBSYSIO_WRAP_TRACE;
    return syscall(SYS_readv, fd, iov, iovcnt);
}

inline int __iofwdlibsysio_wrap_writev(int fd, const struct iovec * iov, int iovcnt)
{
    _IOFWDLIBSYSIO_WRAP_TRACE;
    return syscall(SYS_writev, fd, iov, iovcnt);
} 
