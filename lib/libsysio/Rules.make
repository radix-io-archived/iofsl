if WITH_STDFD_DEV
STDFD_DEV_CPPFLAGS =-DSTDFD_DEV=1 -I$(top_srcdir)/dev/stdfd
else
STFD_DEV_CPPFLAGS =
endif
if WITH_STDDEV_DEV
STDDEV_DEV_CPPFLAGS =-DSTDDEV_DEV=1 -I$(top_srcdir)/dev
else
STDEV_DEV_CPPFLAGS =
endif

IOFWD_DEV_CPPFLAGS =-DIOFWD_DEV=1 -I$(top_srcdir)/dev/iofwd -I$(top_srcdir)/misc -I$(top_srcdir)/../../src/zoidfs -I$(top_srcdir)/../../
IOFWD_DEV_LDFLAGS = -L$(top_srcdir)/../../src/zoidfs -lzoidfsclient

DEV_CPPFLAGS = $(STDFD_DEV_CPPFLAGS) $(STDDEV_DEV_CPPFLAGS) $(IOFWD_DEV_CPPFLAGS)
DEV_LDFLAGS = $(IOFWD_DEV_LDFLAGS)

if WITH_THREAD_MODEL_POSIX
THREAD_MODEL_POSIX_COMPILER_FLAGS=-pthread
else
THREAD_MODEL_POSIX_COMPILER_FLAGS=
endif

AM_CPPFLAGS = \
	$(THREAD_MODEL_POSIX_COMPILER_FLAGS) \
	$(TRACING) \
	$(AUTOMOUNT) $(ZERO_SUM_MEMORY) $(DEV_CPPFLAGS) \
	$(DEFER_INIT_CWD) $(SYSIO_LABEL_NAMES) $(_HAVE_STATVFS) \
	-I$(top_srcdir)/include $(IOFWD_DEV_CPPFLAGS) 

AM_LDFLAGS = $(IOFWD_DEV_LDFLAGS)
