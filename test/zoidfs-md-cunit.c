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
static char symlink_component_file_target[NAMESIZE], symlink_component_dir_target[NAMESIZE];
static char symlink_fullpath_file_target[NAMESIZE], symlink_fullpath_dir_target[NAMESIZE];
static char symlink_component_filename[NAMESIZE], symlink_component_dirname[NAMESIZE];
static char symlink_fullpath_filename[NAMESIZE], symlink_fullpath_dirname[NAMESIZE];
static char fullpath_dirname[NAMESIZE], component_dirname[NAMESIZE];
static char fullpath_filename[NAMESIZE], component_filename[NAMESIZE];

/* basedir handle */
static zoidfs_handle_t basedir_handle;

/* setup the file paths */
int init_path_names(char * mpt)
{
    snprintf(symlink_component_filename, NAMESIZE, "symlink_file");
    snprintf(symlink_component_dirname, NAMESIZE, "symlink_dir");
    snprintf(symlink_fullpath_filename, NAMESIZE, "%s/symlink_file", mpt);
    snprintf(symlink_fullpath_dirname, NAMESIZE, "%s/symlink_dir", mpt);
    snprintf(symlink_component_file_target, NAMESIZE, "test-zoidfs-fullpath");
    snprintf(symlink_component_dir_target, NAMESIZE, "test-dir-fullpath");
    snprintf(symlink_fullpath_file_target, NAMESIZE, "%s/test-zoidfs-fullpath", mpt);
    snprintf(symlink_fullpath_dir_target, NAMESIZE, "%s/test-dir-fullpath", mpt);
    snprintf(fullpath_dirname, NAMESIZE, "%s/test-dir-fullpath", mpt);
    snprintf(component_dirname, NAMESIZE, "test-dir-component");
    snprintf(fullpath_filename, NAMESIZE, "%s/test-zoidfs-fullpath", mpt);
    snprintf(component_filename, NAMESIZE, "test-zoidfs-component");

    return 0;
}

int init_basedir_handle(char * mpt)
{
    return zoidfs_lookup(NULL, NULL, mpt, &basedir_handle);
}

int init_suite_lookup(void)
{
    int created = 0;
    struct timeval now;
    zoidfs_sattr_t sattr;
    zoidfs_handle_t fhandle;
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

    /* setup sattr for creates */
    /* create a file */
    zoidfs_create(&basedir_handle, component_filename, NULL, &sattr, &fhandle, &created);

    /* create a dir */
    zoidfs_mkdir(&basedir_handle, component_dirname, NULL, &sattr, &parent_hint);

    /* create a symlink to a file */
    zoidfs_symlink(&basedir_handle, symlink_component_filename, NULL, &basedir_handle, symlink_component_file_target, NULL, &sattr, NULL, NULL);

    /* create a symlink to a dir */
    zoidfs_symlink(&basedir_handle, symlink_component_dirname, NULL, &basedir_handle, symlink_component_dir_target, NULL, &sattr, NULL, NULL);

    return 0;
}

int clean_suite_lookup(void)
{
    /* remove the file */
    zoidfs_remove(&basedir_handle, component_filename, NULL, NULL);

    /* remove the dir */
    zoidfs_remove(&basedir_handle, component_dirname, NULL, NULL);

    /* remove the symlink to the file */
    zoidfs_remove(&basedir_handle, symlink_component_filename, NULL, NULL);

    /* remove the symlink to the dir */
    zoidfs_remove(&basedir_handle, symlink_component_dirname, NULL, NULL); 

    return 0;
}

void testLOOKUP_component(void)
{
   zoidfs_handle_t fhandle;

   CU_ASSERT(ZFS_OK == zoidfs_lookup(&basedir_handle, component_filename, NULL, &fhandle));
   CU_ASSERT(ZFS_OK == zoidfs_lookup(&basedir_handle, component_dirname, NULL, &fhandle));
   CU_ASSERT(ZFS_OK == zoidfs_lookup(&basedir_handle, symlink_component_filename, NULL, &fhandle));
   CU_ASSERT(ZFS_OK == zoidfs_lookup(&basedir_handle, symlink_component_dirname, NULL, &fhandle));
}

void testLOOKUP_fullpath(void)
{
   zoidfs_handle_t fhandle;

   CU_ASSERT(ZFS_OK == zoidfs_lookup(NULL, NULL, fullpath_filename, &fhandle));
   CU_ASSERT(ZFS_OK == zoidfs_lookup(NULL, NULL, fullpath_dirname, &fhandle));
   CU_ASSERT(ZFS_OK == zoidfs_lookup(NULL, NULL, symlink_fullpath_filename, &fhandle));
   CU_ASSERT(ZFS_OK == zoidfs_lookup(NULL, NULL, symlink_fullpath_dirname, &fhandle));
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
   /* setup path names for the tests */
   init_path_names(argv[1]);

   /* start zoidfs */
   zoidfs_init();

   /* init the base handle for the mount point*/
   init_basedir_handle(argv[1]);

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("Lookup Suite", init_suite_lookup, clean_suite_lookup);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   /* NOTE - ORDER IS IMPORTANT - MUST TEST fread() AFTER fprintf() */
   if ((NULL == CU_add_test(pSuite, "test of zoidfs_lookup() with component file names", testLOOKUP_component))
        || (NULL == CU_add_test(pSuite, "test of zoidfs_lookup() with fullpath file names", testLOOKUP_fullpath))
      )
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();

   /* shutdown zoidfs */
   zoidfs_finalize();

   return CU_get_error();
}
