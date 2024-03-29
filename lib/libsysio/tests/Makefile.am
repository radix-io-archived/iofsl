if WITH_STATVFS
STATVFS_PROG=test_statvfs
else
STATVFS_PROG=
endif

if WITH_FILE_HANDLE_INTERFACE
FHI_PROG=test_fhichmod test_fhicreate test_fhilink test_fhilist test_fhimkdir \
	test_fhipath test_fhirename test_fhirmdir \
	test_fhisymlink test_fhitrunc test_fhiunlink
else
FHI_PROG=
endif

noinst_PROGRAMS = test_copy test_stats test_path test_list \
	test_getcwd test_link test_unlink test_symlink \
	test_rename test_regions test_stddir test_fcntl_lock test_mknod \
	test_mkdir test_chown $(STATVFS_PROG) $(FHI_PROG)

CLEANFILES=drv_data.c

if WITH_NATIVE_DRIVER
NATIVE_DRIVER_NAME=native
NATIVE_DRIVER_CFLAGS= -I$(top_srcdir)/drivers/native
else
NATIVE_DRIVER_NAME=
NATIVE_DRIVER_CFLAGS=
endif

if WITH_INCORE_DRIVER
INCORE_DRIVER_NAME=incore
INCORE_DRIVER_CFLAGS= -I$(top_srcdir)/drivers/incore
else
INCORE_DRIVER_NAME=
INCORE_DRIVER_CFLAGS=
endif

DRIVERS=$(NATIVE_DRIVER_NAME) $(INCORE_DRIVER_NAME) $(STFD_DEV_NAME)

CMNSRC=startup.c drv_init_all.c drv_data.c

BUILT_SOURCES=drv_data.c

CFL=$(AM_CFLAGS) $(AM_CPPFLAGS) \
	$(NATIVE_DRIVER_CFLAGS) $(INCORE_DRIVER_CFLAGS) \
	$(STDFD_DEV_CFLAGS)

LIBS=$(LIBBUILD_DIR)/libsysio.a

test_copy_SOURCES=test_copy.c $(CMNSRC)
test_copy_CFLAGS=$(CFL)
test_copy_DEPENDENCIES=$(LIBS)

test_stats_SOURCES=test_stats.c $(CMNSRC)
test_stats_CFLAGS=$(CFL)
test_stats_DEPENDENCIES=$(LIBS)

test_path_SOURCES=test_path.c $(CMNSRC)
test_path_CFLAGS=$(CFL)
test_path_DEPENDENCIES=$(LIBS)

test_list_SOURCES=test_list.c $(CMNSRC)
test_list_CFLAGS=$(CFL)
test_list_DEPENDENCIES=$(LIBS)

test_getcwd_SOURCES=test_getcwd.c $(CMNSRC)
test_getcwd_CFLAGS=$(CFL)
test_getcwd_DEPENDENCIES=$(LIBS)

test_link_SOURCES=test_link.c $(CMNSRC)
test_link_CFLAGS=$(CFL)
test_link_DEPENDENCIES=$(LIBS)

test_unlink_SOURCES=test_unlink.c $(CMNSRC)
test_unlink_CFLAGS=$(CFL)
test_unlink_DEPENDENCIES=$(LIBS)

test_symlink_SOURCES=test_symlink.c $(CMNSRC)
test_symlink_CFLAGS=$(CFL)
test_symlink_DEPENDENCIES=$(LIBS)

test_rename_SOURCES=test_rename.c $(CMNSRC)
test_rename_CFLAGS=$(CFL)
test_rename_DEPENDENCIES=$(LIBS)

test_regions_SOURCES=test_regions.c $(CMNSRC)
test_regions_CFLAGS=$(CFL)
test_regions_DEPENDENCIES=$(LIBS)

test_stddir_SOURCES=test_stddir.c $(CMNSRC)
test_stddir_CFLAGS=$(CFL)
test_stddir_DEPENDENCIES=$(LIBS)

test_fcntl_lock_SOURCES=test_fcntl_lock.c $(CMNSRC)
test_fcntl_lock_CFLAGS=$(CFL)
test_fcntl_lock_DEPENDENCIES=$(LIBS)

test_mknod_SOURCES=test_mknod.c $(CMNSRC)
test_mknod_CFLAGS=$(CFL)
test_mknod_DEPENDENCIES=$(LIBS)

test_mkdir_SOURCES=test_mkdir.c $(CMNSRC)
test_mkdir_CFLAGS=$(CFL)
test_mkdir_DEPENDENCIES=$(LIBS)

test_chown_SOURCES=test_chown.c $(CMNSRC)
test_chown_CFLAGS=$(CFL)
test_chown_DEPENDENCIES=$(LIBS)

test_statvfs_SOURCES=test_statvfs.c $(CMNSRC)
test_statvfs_CFLAGS=$(CFL)
test_statvfs_DEPENDENCIES=$(LIBS)

CMNFHISRC=fhi_support.c

test_fhichmod_SOURCES=test_fhichmod.c $(CMNFHISRC) $(CMNSRC)
test_fhichmod_CFLAGS=$(CFL)
test_fhichmod_DEPENDENCIES=$(LIBS)

test_fhicreate_SOURCES=test_fhicreate.c $(CMNFHISRC) $(CMNSRC)
test_fhicreate_CFLAGS=$(CFL)
test_fhicreate_DEPENDENCIES=$(LIBS)

test_fhilink_SOURCES=test_fhilink.c $(CMNFHISRC) $(CMNSRC)
test_fhilink_CFLAGS=$(CFL)
test_fhilink_DEPENDENCIES=$(LIBS)

test_fhilist_SOURCES=test_fhilist.c $(CMNFHISRC) $(CMNSRC)
test_fhilist_CFLAGS=$(CFL)
test_fhilist_DEPENDENCIES=$(LIBS)

test_fhimkdir_SOURCES=test_fhimkdir.c $(CMNFHISRC) $(CMNSRC)
test_fhimkdir_CFLAGS=$(CFL)
test_fhimkdir_DEPENDENCIES=$(LIBS)

test_fhipath_SOURCES=test_fhipath.c $(CMNFHISRC) $(CMNSRC)
test_fhipath_CFLAGS=$(CFL)
test_fhipath_DEPENDENCIES=$(LIBS)

test_fhirename_SOURCES=test_fhirename.c $(CMNFHISRC) $(CMNSRC)
test_fhirename_CFLAGS=$(CFL)
test_fhirename_DEPENDENCIES=$(LIBS)

test_fhirmdir_SOURCES=test_fhirmdir.c $(CMNFHISRC) $(CMNSRC)
test_fhirmdir_CFLAGS=$(CFL)
test_fhirmdir_DEPENDENCIES=$(LIBS)

test_fhisymlink_SOURCES=test_fhisymlink.c $(CMNFHISRC) $(CMNSRC)
test_fhisymlink_CFLAGS=$(CFL)
test_fhisymlink_DEPENDENCIES=$(LIBS)

test_fhitrunc_SOURCES=test_fhitrunc.c $(CMNFHISRC) $(CMNSRC)
test_fhitrunc_CFLAGS=$(CFL)
test_fhitrunc_DEPENDENCIES=$(LIBS)

test_fhiunlink_SOURCES=test_fhiunlink.c $(CMNFHISRC) $(CMNSRC)
test_fhiunlink_CFLAGS=$(CFL)
test_fhiunlink_DEPENDENCIES=$(LIBS)

drv_data.c: $(CONFIG_DEPENDENCIES) $(top_srcdir)/tests/gendrvdata.sh
	test -z "drv_data.c" && rm -f drv_data.c; \
	$(SHELL) $(top_srcdir)/tests/gendrvdata.sh $(DRIVERS) > drv_data.c

lib_LIBRARIES=libruntime.a

libruntime_a_SOURCES=sysio-run-start.c startup.c drv_init_all.c drv_data.c

AM_CFLAGS = -L$(LIBBUILD_DIR)
include $(top_srcdir)/Rules.make
