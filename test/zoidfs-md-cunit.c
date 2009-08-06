#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>

#include "zoidfs.h"
#include "zoidfs-proto.h"
#include "iofwd_config.h"

#define NAMESIZE 255

#include "CUnit/Basic.h"

/* path variables */
static char symlink_component_filename[NAMESIZE], symlink_component_dirname[NAMESIZE];
static char symlink_fullpath_filename[NAMESIZE], symlink_fullpath_dirname[NAMESIZE];
static char fullpath_dirname[NAMESIZE], component_dirname[NAMESIZE];
static char fullpath_filename[NAMESIZE], component_filename[NAMESIZE];
static char rename_fullpath_dirname[NAMESIZE], rename_component_dirname[NAMESIZE];
static char rename_fullpath_filename[NAMESIZE], rename_component_filename[NAMESIZE];

/* basedir handle */
static zoidfs_handle_t basedir_handle;

/* setup the file paths */
int init_path_names(char * mpt)
{
    snprintf(symlink_component_filename, NAMESIZE, "symlink_comp_file");
    snprintf(symlink_component_dirname, NAMESIZE, "symlink_comp_dir");
    snprintf(symlink_fullpath_filename, NAMESIZE, "%s/symlink_full_file", mpt);
    snprintf(symlink_fullpath_dirname, NAMESIZE, "%s/symlink_full_dir", mpt);

    snprintf(component_filename, NAMESIZE, "test-zoidfs-file-comp");
    snprintf(component_dirname, NAMESIZE, "test-zoidfs-dir-comp");
    snprintf(fullpath_filename, NAMESIZE, "%s/test-zoidfs-file-full", mpt);
    snprintf(fullpath_dirname, NAMESIZE, "%s/test-zoidfs-dir-full", mpt);

    snprintf(rename_component_filename, NAMESIZE, "test-zoidfs-file-comp-rename");
    snprintf(rename_component_dirname, NAMESIZE, "test-zoidfs-dir-comp-rename");
    snprintf(rename_fullpath_filename, NAMESIZE, "%s/test-zoidfs-file-full-rename", mpt);
    snprintf(rename_fullpath_dirname, NAMESIZE, "%s/test-zoidfs-dir-full-rename", mpt);

    return 0;
}

int init_basedir_handle(char * mpt)
{
    return zoidfs_lookup(NULL, NULL, mpt, &basedir_handle);
}

int testCREATE(void)
{
    int created = 0;
    struct timeval now;
    zoidfs_sattr_t sattr;
    zoidfs_handle_t fhandle;

    /* set the attrs */
    sattr.mask = ZOIDFS_ATTR_SETABLE;
    sattr.mode = 0755;
    sattr.uid = getuid();
    sattr.gid = getgid();

    gettimeofday(&now, NULL);
    sattr.atime.seconds = now.tv_sec;
    sattr.atime.nseconds = now.tv_usec;
    sattr.mtime.seconds = now.tv_sec;
    sattr.mtime.nseconds = now.tv_usec;

    /* create a file using the base handle and component name*/
    CU_ASSERT(ZFS_OK == zoidfs_create(&basedir_handle, component_filename, NULL, &sattr, &fhandle, &created));

    /* create a file using the base handle and component name*/
    CU_ASSERT(ZFS_OK == zoidfs_create(NULL, NULL, fullpath_filename, &sattr, &fhandle, &created));

    return 0;
}

int testMKDIR(void)
{
    struct timeval now;
    zoidfs_sattr_t sattr;
    zoidfs_cache_hint_t parent_hint;

    /* set the attrs */
    sattr.mask = ZOIDFS_ATTR_SETABLE;
    sattr.mode = 0755;
    sattr.uid = getuid();
    sattr.gid = getgid();

    gettimeofday(&now, NULL);
    sattr.atime.seconds = now.tv_sec;
    sattr.atime.nseconds = now.tv_usec;
    sattr.mtime.seconds = now.tv_sec;
    sattr.mtime.nseconds = now.tv_usec;

    /* create a file using the base handle and component name*/
    CU_ASSERT(ZFS_OK == zoidfs_mkdir(&basedir_handle, component_dirname, NULL, &sattr, &parent_hint));

    /* create a file using the base handle and component name*/
    CU_ASSERT(ZFS_OK == zoidfs_mkdir(NULL, NULL, fullpath_dirname, &sattr, &parent_hint));

    return 0;
}

int testSYMLINK(void)
{
    struct timeval now;
    zoidfs_sattr_t sattr;

    /* set the attrs */
    sattr.mask = ZOIDFS_ATTR_SETABLE;
    sattr.mode = 0755;
    sattr.uid = getuid();
    sattr.gid = getgid();

    gettimeofday(&now, NULL);
    sattr.atime.seconds = now.tv_sec;
    sattr.atime.nseconds = now.tv_usec;
    sattr.mtime.seconds = now.tv_sec;
    sattr.mtime.nseconds = now.tv_usec;

    /* create a file using the base handle and component name*/
    CU_ASSERT(ZFS_OK == zoidfs_symlink(&basedir_handle, symlink_component_filename, NULL, &basedir_handle, component_filename, NULL, &sattr, NULL, NULL));

    /* create a file using the base handle and component name*/
    CU_ASSERT(ZFS_OK == zoidfs_symlink(&basedir_handle, symlink_component_dirname, NULL, &basedir_handle, component_dirname, NULL, &sattr, NULL, NULL));

    /* create a file using the base handle and component name*/
    CU_ASSERT(ZFS_OK == zoidfs_symlink(NULL, NULL, symlink_fullpath_filename, NULL, NULL, fullpath_filename, &sattr, NULL, NULL));

    /* create a file using the base handle and component name*/
    CU_ASSERT(ZFS_OK == zoidfs_symlink(NULL, NULL, symlink_fullpath_dirname, NULL, NULL, fullpath_dirname, &sattr, NULL, NULL));

    return 0;
}

int testRENAME(void)
{
    /* create a file using the base handle and component name*/
    CU_ASSERT(ZFS_OK == zoidfs_rename(&basedir_handle, component_filename, NULL, &basedir_handle, rename_component_filename, NULL, NULL, NULL));

    /* create a file using the base handle and component name*/
    CU_ASSERT(ZFS_OK == zoidfs_rename(&basedir_handle, component_dirname, NULL, &basedir_handle, rename_component_dirname, NULL, NULL, NULL));

    /* create a file using the base handle and component name*/
    CU_ASSERT(ZFS_OK == zoidfs_rename(NULL, NULL, fullpath_filename, NULL, NULL, rename_fullpath_filename, NULL, NULL));

    /* create a file using the base handle and component name*/
    CU_ASSERT(ZFS_OK == zoidfs_rename(NULL, NULL, fullpath_dirname, NULL, NULL, rename_fullpath_dirname, NULL, NULL));

    /*
     * reverse the renames
     */
    /* create a file using the base handle and component name*/
    CU_ASSERT(ZFS_OK == zoidfs_rename(&basedir_handle, rename_component_filename, NULL, &basedir_handle, component_filename, NULL, NULL, NULL));

    /* create a file using the base handle and component name*/
    CU_ASSERT(ZFS_OK == zoidfs_rename(&basedir_handle, rename_component_dirname, NULL, &basedir_handle, component_dirname, NULL, NULL, NULL));

    /* create a file using the base handle and component name*/
    CU_ASSERT(ZFS_OK == zoidfs_rename(NULL, NULL, rename_fullpath_filename, NULL, NULL, fullpath_filename, NULL, NULL));

    /* create a file using the base handle and component name*/
    CU_ASSERT(ZFS_OK == zoidfs_rename(NULL, NULL, rename_fullpath_dirname, NULL, NULL, fullpath_dirname, NULL, NULL));

    return 0;
}

int testREMOVE(void)
{
    CU_ASSERT(ZFS_OK == zoidfs_remove(&basedir_handle, component_filename, NULL, NULL));
    CU_ASSERT(ZFS_OK == zoidfs_remove(&basedir_handle, component_dirname, NULL, NULL));
    CU_ASSERT(ZFS_OK == zoidfs_remove(NULL, NULL, fullpath_filename, NULL));
    CU_ASSERT(ZFS_OK == zoidfs_remove(NULL, NULL, fullpath_dirname, NULL));
    CU_ASSERT(ZFS_OK == zoidfs_remove(&basedir_handle, symlink_component_filename, NULL, NULL));
    CU_ASSERT(ZFS_OK == zoidfs_remove(&basedir_handle, symlink_component_dirname, NULL, NULL));
    CU_ASSERT(ZFS_OK == zoidfs_remove(NULL, NULL, symlink_fullpath_filename, NULL));
    CU_ASSERT(ZFS_OK == zoidfs_remove(NULL, NULL, symlink_fullpath_dirname, NULL));
    return 0;
}

int testLOOKUP(void)
{
    zoidfs_handle_t fhandle;

    CU_ASSERT(ZFS_OK == zoidfs_lookup(&basedir_handle, component_filename, NULL, &fhandle));
    CU_ASSERT(ZFS_OK == zoidfs_lookup(&basedir_handle, component_dirname, NULL, &fhandle));
    CU_ASSERT(ZFS_OK == zoidfs_lookup(NULL, NULL, fullpath_filename, &fhandle));
    CU_ASSERT(ZFS_OK == zoidfs_lookup(NULL, NULL, fullpath_dirname, &fhandle));
    CU_ASSERT(ZFS_OK == zoidfs_lookup(&basedir_handle, symlink_component_filename, NULL, &fhandle));
    CU_ASSERT(ZFS_OK == zoidfs_lookup(&basedir_handle, symlink_component_dirname, NULL, &fhandle));
    CU_ASSERT(ZFS_OK == zoidfs_lookup(NULL, NULL, symlink_fullpath_filename, &fhandle));
    CU_ASSERT(ZFS_OK == zoidfs_lookup(NULL, NULL, symlink_fullpath_dirname, &fhandle));

    return 0;
}

int testGETATTR(void)
{
    zoidfs_handle_t fhandle;
    zoidfs_attr_t gattr;

    zoidfs_lookup(&basedir_handle, component_filename, NULL, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_getattr(&fhandle, &gattr));

    zoidfs_lookup(&basedir_handle, component_dirname, NULL, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_getattr(&fhandle, &gattr));

    zoidfs_lookup(NULL, NULL, fullpath_filename, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_getattr(&fhandle, &gattr));

    zoidfs_lookup(NULL, NULL, fullpath_dirname, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_getattr(&fhandle, &gattr));

    zoidfs_lookup(&basedir_handle, symlink_component_filename, NULL, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_getattr(&fhandle, &gattr));

    zoidfs_lookup(&basedir_handle, symlink_component_dirname, NULL, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_getattr(&fhandle, &gattr));

    zoidfs_lookup(NULL, NULL, symlink_fullpath_filename, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_getattr(&fhandle, &gattr));

    zoidfs_lookup(NULL, NULL, symlink_fullpath_dirname, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_getattr(&fhandle, &gattr));

    return 0;
}

int testSETATTR(void)
{
    zoidfs_handle_t fhandle;
    zoidfs_attr_t gattr;
    zoidfs_sattr_t sattr;

    /* set the attrs */
    sattr.mask = ZOIDFS_ATTR_MODE;
    sattr.mode = 0644;

    zoidfs_lookup(&basedir_handle, component_filename, NULL, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_setattr(&fhandle, &sattr, &gattr));
    CU_ASSERT(gattr.mode == 0644);

    zoidfs_lookup(&basedir_handle, component_dirname, NULL, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_setattr(&fhandle, &sattr, &gattr));
    CU_ASSERT(gattr.mode == 0644);

    zoidfs_lookup(NULL, NULL, fullpath_filename, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_setattr(&fhandle, &sattr, &gattr));
    CU_ASSERT(gattr.mode == 0644);

    zoidfs_lookup(NULL, NULL, fullpath_dirname, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_setattr(&fhandle, &sattr, &gattr));
    CU_ASSERT(gattr.mode == 0644);

    zoidfs_lookup(&basedir_handle, symlink_component_filename, NULL, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_setattr(&fhandle, &sattr, &gattr));

    zoidfs_lookup(&basedir_handle, symlink_component_dirname, NULL, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_setattr(&fhandle, &sattr, &gattr));

    zoidfs_lookup(NULL, NULL, symlink_fullpath_filename, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_setattr(&fhandle, &sattr, &gattr));

    zoidfs_lookup(NULL, NULL, symlink_fullpath_dirname, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_setattr(&fhandle, &sattr, &gattr));

    return 0;
}

int init_suite_dispatch_basic(void)
{
    return 0;
}

int clean_suite_dispatch_basic(void)
{
    return 0;
}

/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main(int argc, char ** argv)
{
   CU_pSuite pSuite = NULL;

   if(argc < 2)
   {
    fprintf(stderr, "incorrect cmd line args!\n");
    return -1;
   }

    zoidfs_init();

   /* setup path names for the tests */
   init_path_names(argv[1]);

   /* init the base handle for the mount point*/
   init_basedir_handle(argv[1]);

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("Basic Dispatcher Test Suite", init_suite_dispatch_basic, clean_suite_dispatch_basic);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   /* NOTE - ORDER IS IMPORTANT - MUST TEST fread() AFTER fprintf() */
   if (
        (NULL == CU_add_test(pSuite, "test of zoidfs_create()", (CU_TestFunc) testCREATE)) ||
        (NULL == CU_add_test(pSuite, "test of zoidfs_mkdir()", (CU_TestFunc) testMKDIR)) ||
        (NULL == CU_add_test(pSuite, "test of zoidfs_symlink()", (CU_TestFunc) testSYMLINK)) ||
        (NULL == CU_add_test(pSuite, "test of zoidfs_rename()", (CU_TestFunc) testRENAME)) ||
        (NULL == CU_add_test(pSuite, "test of zoidfs_lookup()", (CU_TestFunc) testLOOKUP)) ||
        (NULL == CU_add_test(pSuite, "test of zoidfs_getattr()", (CU_TestFunc) testGETATTR)) ||
        (NULL == CU_add_test(pSuite, "test of zoidfs_setattr()", (CU_TestFunc) testSETATTR)) ||
        (NULL == CU_add_test(pSuite, "test of zoidfs_remove()", (CU_TestFunc) testREMOVE))
      )
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();

   zoidfs_finalize();

   return CU_get_error();
}
