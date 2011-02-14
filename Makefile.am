bin_PROGRAMS =
noinst_LIBRARIES =
noinst_PROGRAMS =
lib_LIBRARIES =
BUILT_SOURCES =
noinst_HEADERS =
TESTS =
check_PROGRAMS =
EXTRA_PROGRAMS =
EXTRA_DIST =

#
# Client library (C++)
#   List all the .cpp / .hh files that need to go in the library in this
#   variable
#
CPPCLIENTLIB =

#
# Client librar (C)
#   List all the .c / .h files
CLIENTLIB =

AM_CPPFLAGS = -I$(top_srcdir)/src \
	      -I$(top_srcdir)/src/iofwdutil \
	      -I$(top_srcdir)/src/zoidfs \
	      @BOOST_CPPFLAGS@ \
	      @FUSE_CPPFLAGS@ \
	      @ZFS_DISPATCHER_CPPFLAGS@ \
	      @BMI_CPPFLAGS@ \
          @CUNIT_CPPFLAGS@ \
	  @ATOMICS_CPPFLAGS@
	
AM_CFLAGS = @FUSE_CFLAGS@ @ZFS_DISPATCHER_CFLAGS@ @ATOMICS_CFLAGS@

AM_LIBS = @BOOST_DATE_TIME_LIB@ @BOOST_PROGRAM_OPTIONS_LIB@ @BOOST_REGEX_LIB@ \
	  @BOOST_THREAD_LIB@ @BOOST_UNIT_TEST_FRAMEWORK_LIB@ \
	  @CUNIT_LIBS@ @FUSE_LIBS@ @GLIB_LIBS@ @OPENPA_LIBS@ @OPENSSL_LIBS@ \
	  @PVFS2_LIBS@ @SYSIO_LIBS@ @ZLIB_LIBS@ @ATOMICS_LIBS@ @BMI_LIBS@ \
	  @ATOMICS_LIBS@

AM_LDFLAGS = @BOOST_LDFLAGS@ @BMI_LDFLAGS@ @CUNIT_LDFLAGS@ @FUSE_LDFLAGS@ \
	     @GLIB_LDFLAGS@ @GRIDFTP_LDFLAGS@ @OPENPA_LDFLAGS@ \
	     @OPENSSL_LDFLAGS@ \
	     @PVFS2_LDFLAGS@ @SYSIO_LDFLAGS@ @ZFS_DISPATCHER_LDFLAGS@ \
	     @ZLIB_LDFLAGS@ @BMI_LDFLAGS@

AM_CXXFLAGS = $(AM_CFLAGS)

#
# Default server config
#
EXTRA_DIST += defaultconfig.cf

#
# pkgconfig files
#
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = iofsl.pc iofslrouter.pc
$(pkgconfig_DATA): config.status

#
# extra build rules
#
include Make.rules

include $(top_srcdir)/src/iofwdclient/Makefile.subdir
include $(top_srcdir)/src/net/Makefile.subdir
include $(top_srcdir)/src/sm/Makefile.subdir
include $(top_srcdir)/src/iofwdevent/Makefile.subdir
include $(top_srcdir)/src/iofwd/Makefile.subdir
include $(top_srcdir)/src/iofwdutil/Makefile.subdir
include $(top_srcdir)/src/encoder/Makefile.subdir
include $(top_srcdir)/src/zoidfs/Makefile.subdir
include $(top_srcdir)/src/fuse/Makefile.subdir
include $(top_srcdir)/src/c-util/Makefile.subdir
include $(top_srcdir)/test/Makefile.subdir
include $(top_srcdir)/sandbox/Makefile.subdir

