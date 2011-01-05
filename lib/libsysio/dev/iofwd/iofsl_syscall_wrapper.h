/*
 * Purpose: wrappers for libc / posix calls that libsyio will intercept.
 *          These wrappers ensure that the original libc / posix calls
 *          execute and not the libsysio versions. These calls are also
 *          traced.
 *
 *          NOTE: This has only been tested using the TCP BMI driver.
 *                Other networks (MX, IB, ...) have not been tested.
 *
 * Author: Jason Cope
 * Email: copej@mcs.anl.gov
 */
#ifndef __IOFSL_SYSCALL_WRAPPER_H__
#define __IOFSL_SYSCALL_WRAPPER_H__

inline int _iofwdlibsysio_wrap_fcntl(int fd, int cmd, long val);
inline int _iofwdlibsysio_wrap_close(int fd);
inline int _iofwdlibsysio_wrap_read(int fd, void * buf, size_t nbyte);
inline int _iofwdlibsysio_wrap_write(int fd, void * buf, size_t nbyte);
inline int _iofwdlibsysio_wrap_readv(int fd, const struct iovec * iov, int iovcnt);
inline int _iofwdlibsysio_wrap_writev(int fd, const struct iovec * iov, int iovcnt);

#endif
