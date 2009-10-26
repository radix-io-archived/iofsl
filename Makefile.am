bin_PROGRAMS =
noinst_LIBRARIES =
noinst_PROGRAMS =
lib_LIBRARIES =
#BUILD_SOURCES =
noinst_HEADERS =
TESTS =
check_PROGRAMS = 

AM_CPPFLAGS = -I$(top_srcdir)/src \
	      -I$(top_srcdir)/src/iofwdutil \
	      -I$(top_srcdir)/src/zoidfs \
	      @BOOST_CPPFLAGS@ \
	      @FUSE_CPPFLAGS@ \
	      @ZFS_DISPATCHER_CPPFLAGS@
	    
AM_CFLAGS = @FUSE_CFLAGS@ @ZFS_DISPATCHER_CFLAGS@

AM_CXXFLAGS = $(AM_CFLAGS)

# extra build rules
include Make.rules

include $(top_srcdir)/src/spatial/Makefile.subdir
include $(top_srcdir)/src/sm/Makefile.subdir
include $(top_srcdir)/src/iofwdevent/Makefile.subdir
include $(top_srcdir)/src/iofwd/Makefile.subdir
include $(top_srcdir)/src/iofwdutil/Makefile.subdir
include $(top_srcdir)/src/cache/Makefile.subdir
include $(top_srcdir)/src/taskmon/Makefile.subdir
include $(top_srcdir)/src/zoidfs/Makefile.subdir
include $(top_srcdir)/src/fuse/Makefile.subdir
include $(top_srcdir)/src/client/Makefile.subdir
include $(top_srcdir)/src/c-util/Makefile.subdir
include $(top_srcdir)/test/Makefile.subdir
include $(top_srcdir)/sandbox/Makefile.subdir

