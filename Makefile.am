bin_PROGRAMS =
noinst_LIBRARIES =
noinst_PROGRAMS =
lib_LIBRARIES =
BUILD_SOURCES =

CPPFLAGS+=-I$(top_srcdir)/src

# extra build rules
include Make.rules

include $(top_srcdir)/src/spatial/Makefile.subdir
include $(top_srcdir)/src/sm/Makefile.subdir
include $(top_srcdir)/src/iofwdevent/Makefile.subdir
include $(top_srcdir)/src/iofwd/Makefile.subdir
include $(top_srcdir)/src/iofwdutil/Makefile.subdir
include $(top_srcdir)/src/metacache/Makefile.subdir
include $(top_srcdir)/src/cache/Makefile.subdir
include $(top_srcdir)/src/taskmon/Makefile.subdir
include $(top_srcdir)/src/zoidfs/Makefile.subdir
include $(top_srcdir)/src/fuse/Makefile.subdir
include $(top_srcdir)/src/client/Makefile.subdir
include $(top_srcdir)/src/c-util/Makefile.subdir
include $(top_srcdir)/test/Makefile.subdir
include $(top_srcdir)/sandbox/Makefile.subdir

