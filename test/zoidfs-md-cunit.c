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
#define PATHSIZE 4096

#include "CUnit/Basic.h"

/* path variables */
static char link_component_filename[NAMESIZE], link_component_dirname[NAMESIZE];
static char link_fullpath_filename[NAMESIZE], link_fullpath_dirname[NAMESIZE];
static char symlink_component_filename[NAMESIZE], symlink_component_dirname[NAMESIZE];
static char symlink_fullpath_filename[NAMESIZE], symlink_fullpath_dirname[NAMESIZE];
static char symlink_fullpath_dirname_slash[NAMESIZE], symlink_component_dirname_slash[NAMESIZE];
static char fullpath_dirname[NAMESIZE], component_dirname[NAMESIZE];
static char fullpath_filename[NAMESIZE], component_filename[NAMESIZE];
static char rename_fullpath_dirname[NAMESIZE], rename_component_dirname[NAMESIZE];
static char rename_fullpath_filename[NAMESIZE], rename_component_filename[NAMESIZE];
static char base_component_dirname[NAMESIZE], base_component_filename[NAMESIZE];
static char long_fullpath_dirpath[PATHSIZE], long_component_dirpath[PATHSIZE];

/* basedir handle */
static zoidfs_handle_t basedir_handle;

static int total_file_count = 0;

/* setup the file paths */
int init_path_names(char * testDir, char * mpt)
{
    snprintf(symlink_component_filename, NAMESIZE, "%s/symlink_comp_file", testDir);
    snprintf(symlink_component_dirname, NAMESIZE, "%s/symlink_comp_dir", testDir);
    snprintf(symlink_component_dirname_slash, NAMESIZE, "%s/symlink_comp_dir/", testDir);
    snprintf(symlink_fullpath_filename, NAMESIZE, "%s/%s/symlink_full_file", mpt, testDir);
    snprintf(symlink_fullpath_dirname, NAMESIZE, "%s/%s/symlink_full_dir", mpt, testDir);
    snprintf(symlink_fullpath_dirname_slash, NAMESIZE, "%s/%s/symlink_full_dir/", mpt, testDir);

    snprintf(link_component_filename, NAMESIZE, "%s/link_comp_file", testDir);
    snprintf(link_component_dirname, NAMESIZE, "%s/link_comp_dir", testDir);
    snprintf(link_fullpath_filename, NAMESIZE, "%s/%s/link_full_file", mpt, testDir);
    snprintf(link_fullpath_dirname, NAMESIZE, "%s/%s/link_full_dir", mpt, testDir);

    snprintf(component_filename, NAMESIZE, "%s/test-zoidfs-file-comp", testDir);
    snprintf(base_component_filename, NAMESIZE, "test-zoidfs-file-comp");
    snprintf(component_dirname, NAMESIZE, "%s/test-zoidfs-dir-comp", testDir);
    snprintf(base_component_dirname, NAMESIZE, "test-zoidfs-dir-comp");
    snprintf(fullpath_filename, NAMESIZE, "%s/%s/test-zoidfs-file-full", mpt, testDir);
    snprintf(fullpath_dirname, NAMESIZE, "%s/%s/test-zoidfs-dir-full", mpt, testDir);

    snprintf(rename_component_filename, NAMESIZE, "%s/test-zoidfs-file-comp-rename", testDir);
    snprintf(rename_component_dirname, NAMESIZE, "%s/test-zoidfs-dir-comp-rename", testDir);
    snprintf(rename_fullpath_filename, NAMESIZE, "%s/%s/test-zoidfs-file-full-rename", mpt, testDir);
    snprintf(rename_fullpath_dirname, NAMESIZE, "%s/%s/test-zoidfs-dir-full-rename", mpt, testDir);

    snprintf(long_fullpath_dirpath, PATHSIZE, "%s/%s/a/b/c/d/e/f", mpt, testDir);
    snprintf(long_component_dirpath, PATHSIZE, "%s/a/b/c/d/e/f", testDir);

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
    CU_ASSERT(created == 1);
    total_file_count++;

    /* file already exists, create should equal 0 */
    CU_ASSERT(ZFS_OK == zoidfs_create(&basedir_handle, component_filename, NULL, &sattr, &fhandle, &created));
    CU_ASSERT(created == 0);

    /* create a file using the base handle and component name*/
    fprintf(stderr, "%s\n", fullpath_filename);
    CU_ASSERT(ZFS_OK == zoidfs_create(NULL, NULL, fullpath_filename, &sattr, &fhandle, &created));
    CU_ASSERT(created == 1);
    total_file_count++;

    /* file already exists, create should equal 0 */
    CU_ASSERT(ZFS_OK == zoidfs_create(NULL, NULL, fullpath_filename, &sattr, &fhandle, &created));
    CU_ASSERT(created == 0);

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
    total_file_count++;

    /* try to create directories if the parents do not exist... this should fail */ 
    CU_ASSERT_NOT_EQUAL(ZFS_OK, zoidfs_mkdir(&basedir_handle, long_component_dirpath, NULL, &sattr, &parent_hint));

    /* create several subdirs using the component dir name */
    char p1[4096];
    sprintf(p1, "%s/a", component_dirname); 
    CU_ASSERT(ZFS_OK == zoidfs_mkdir(&basedir_handle, p1, NULL, &sattr, &parent_hint));
    total_file_count += 3;
    sprintf(p1, "%s/b", p1); 
    CU_ASSERT(ZFS_OK == zoidfs_mkdir(&basedir_handle, p1, NULL, &sattr, &parent_hint));
    total_file_count += 3;
    sprintf(p1, "%s/c", p1); 
    CU_ASSERT(ZFS_OK == zoidfs_mkdir(&basedir_handle, p1, NULL, &sattr, &parent_hint));
    total_file_count += 3;
    sprintf(p1, "%s/d", p1); 
    CU_ASSERT(ZFS_OK == zoidfs_mkdir(&basedir_handle, p1, NULL, &sattr, &parent_hint));
    total_file_count += 3;
    sprintf(p1, "%s/e", p1); 
    CU_ASSERT(ZFS_OK == zoidfs_mkdir(&basedir_handle, p1, NULL, &sattr, &parent_hint));
    total_file_count += 3;

    /* create a file using the base handle and component name*/
    CU_ASSERT(ZFS_OK == zoidfs_mkdir(NULL, NULL, fullpath_dirname, &sattr, &parent_hint));
    total_file_count++;

    /* try to create directories if the parents do not exist... this should fail */ 
    CU_ASSERT_NOT_EQUAL(ZFS_OK, zoidfs_mkdir(&basedir_handle, long_component_dirpath, NULL, &sattr, &parent_hint));

    /* create several subdirs using the fullpath dir name */
    char p2[4096];
    sprintf(p2, "%s/a", fullpath_dirname); 
    CU_ASSERT(ZFS_OK == zoidfs_mkdir(NULL, NULL, p2, &sattr, &parent_hint));
    total_file_count += 3;
    sprintf(p2, "%s/b", p2); 
    CU_ASSERT(ZFS_OK == zoidfs_mkdir(NULL, NULL, p2, &sattr, &parent_hint));
    total_file_count += 3;
    sprintf(p2, "%s/c", p2); 
    CU_ASSERT(ZFS_OK == zoidfs_mkdir(NULL, NULL, p2, &sattr, &parent_hint));
    total_file_count += 3;
    sprintf(p2, "%s/d", p2); 
    CU_ASSERT(ZFS_OK == zoidfs_mkdir(NULL, NULL, p2, &sattr, &parent_hint));
    total_file_count += 3;
    sprintf(p2, "%s/e", p2); 
    CU_ASSERT(ZFS_OK == zoidfs_mkdir(NULL, NULL, p2, &sattr, &parent_hint));
    total_file_count += 3;

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
    CU_ASSERT(ZFS_OK == zoidfs_symlink(&basedir_handle, base_component_filename,
             NULL, &basedir_handle, symlink_component_filename, NULL, &sattr,
             NULL, NULL));
    CU_ASSERT(ZFSERR_EXIST == zoidfs_symlink(&basedir_handle, component_filename,
             NULL, &basedir_handle, symlink_component_filename, NULL, &sattr,
             NULL, NULL));
    total_file_count++;

    /* create a file using the base handle and component name*/
    CU_ASSERT(ZFS_OK == zoidfs_symlink(&basedir_handle, base_component_dirname,
             NULL, &basedir_handle, symlink_component_dirname, NULL, &sattr,
             NULL, NULL));
    CU_ASSERT(ZFSERR_EXIST == zoidfs_symlink(&basedir_handle, component_dirname,
             NULL, &basedir_handle, symlink_component_dirname, NULL, &sattr,
             NULL, NULL));
    total_file_count++;

    /* create a file using the base handle and component name*/
    CU_ASSERT(ZFS_OK == zoidfs_symlink(NULL, NULL, fullpath_filename, NULL,
             NULL, symlink_fullpath_filename, &sattr, NULL, NULL));
    CU_ASSERT(ZFSERR_EXIST == zoidfs_symlink(NULL, NULL, fullpath_filename, NULL,
             NULL, symlink_fullpath_filename, &sattr, NULL, NULL));
    total_file_count++;

    /* create a file using the base handle and component name*/
    CU_ASSERT(ZFS_OK == zoidfs_symlink(NULL, NULL, fullpath_dirname, NULL,
             NULL, symlink_fullpath_dirname, &sattr, NULL, NULL));
    CU_ASSERT(ZFSERR_EXIST == zoidfs_symlink(NULL, NULL, fullpath_dirname, NULL,
             NULL, symlink_fullpath_dirname, &sattr, NULL, NULL));
    total_file_count++;

    return 0;
}

int testLINK(void)
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

    /* Remove files in case they already exist; Ignore possible errors */
    zoidfs_remove(&basedir_handle, link_component_filename, NULL, NULL); 
    zoidfs_remove(NULL, NULL, link_fullpath_filename, NULL);

    /* create a file using the base handle and component name*/
    CU_ASSERT_EQUAL(ZFS_OK, zoidfs_link(&basedir_handle, component_filename,
             NULL, &basedir_handle, link_component_filename, NULL, NULL,
             NULL)); 
    
    CU_ASSERT_EQUAL(ZFSERR_EXIST,
             zoidfs_link(&basedir_handle, component_filename, NULL,
                &basedir_handle, link_component_filename, NULL, NULL, NULL));
    total_file_count++;

    /* create a file using the base handle and component name*/
    CU_ASSERT_NOT_EQUAL(ZFS_OK, zoidfs_link(&basedir_handle,
             component_dirname, NULL, &basedir_handle, link_component_dirname,
             NULL, NULL, NULL));

    /* create a file using the base handle and component name*/
    CU_ASSERT_EQUAL(ZFS_OK, zoidfs_link(NULL, NULL, fullpath_filename, NULL,
             NULL, link_fullpath_filename, NULL, NULL));
    CU_ASSERT_EQUAL(ZFSERR_EXIST, zoidfs_link(NULL, NULL, fullpath_filename,
             NULL, NULL, link_fullpath_filename, NULL, NULL));
    total_file_count++;

    /* create a file using the base handle and component name*/
    CU_ASSERT_NOT_EQUAL(ZFS_OK, zoidfs_link(NULL, NULL, fullpath_dirname,
             NULL, NULL, link_fullpath_dirname, NULL, NULL));

    /* remove the links... this should be in the REMOVE test, but
     * ESTALE is triggered for libsysio dispatcher... leave here until 
     * corrected
     */
    CU_ASSERT(ZFS_OK == zoidfs_remove(&basedir_handle, link_component_filename,
             NULL, NULL)); 
    total_file_count--;
    
    CU_ASSERT(ZFS_OK == zoidfs_remove(NULL, NULL, link_fullpath_filename, NULL));
    total_file_count--;

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
    /* rm several subdirs using the component dir name */
    char p1[4096];
    sprintf(p1, "%s/a/b/c/d/e", component_dirname);
    CU_ASSERT(ZFS_OK == zoidfs_remove(&basedir_handle, p1, NULL, NULL));
    p1[strlen(p1) - 2] = '\0'; 
    CU_ASSERT(ZFS_OK == zoidfs_remove(&basedir_handle, p1, NULL, NULL));
    p1[strlen(p1) - 2] = '\0'; 
    CU_ASSERT(ZFS_OK == zoidfs_remove(&basedir_handle, p1, NULL, NULL));
    p1[strlen(p1) - 2] = '\0'; 
    CU_ASSERT(ZFS_OK == zoidfs_remove(&basedir_handle, p1, NULL, NULL));
    p1[strlen(p1) - 2] = '\0'; 
    CU_ASSERT(ZFS_OK == zoidfs_remove(&basedir_handle, p1, NULL, NULL));
 
    /* rm several subdirs using the fullpath dir name */
    char p2[4096];
    sprintf(p2, "%s/a/b/c/d/e", fullpath_dirname);
    CU_ASSERT(ZFS_OK == zoidfs_remove(NULL, NULL, p2, NULL));
    p2[strlen(p2) - 2] = '\0'; 
    CU_ASSERT(ZFS_OK == zoidfs_remove(NULL, NULL, p2, NULL));
    p2[strlen(p2) - 2] = '\0'; 
    CU_ASSERT(ZFS_OK == zoidfs_remove(NULL, NULL, p2, NULL));
    p2[strlen(p2) - 2] = '\0'; 
    CU_ASSERT(ZFS_OK == zoidfs_remove(NULL, NULL, p2, NULL));
    p2[strlen(p2) - 2] = '\0'; 
    CU_ASSERT(ZFS_OK == zoidfs_remove(NULL, NULL, p2, NULL));
 
    CU_ASSERT(ZFS_OK == zoidfs_remove(&basedir_handle, component_filename, NULL, NULL));
    total_file_count--;
    CU_ASSERT(ZFS_OK == zoidfs_remove(&basedir_handle, component_dirname, NULL, NULL));
    total_file_count--;
    CU_ASSERT(ZFS_OK == zoidfs_remove(NULL, NULL, fullpath_filename, NULL));
    total_file_count--;
    CU_ASSERT(ZFS_OK == zoidfs_remove(NULL, NULL, fullpath_dirname, NULL));
    total_file_count--;
    CU_ASSERT(ZFS_OK == zoidfs_remove(&basedir_handle, symlink_component_filename, NULL, NULL));
    total_file_count--;
    CU_ASSERT(ZFS_OK == zoidfs_remove(&basedir_handle, symlink_component_dirname, NULL, NULL));
    total_file_count--;
    CU_ASSERT(ZFS_OK == zoidfs_remove(NULL, NULL, symlink_fullpath_filename, NULL));
    total_file_count--;
    CU_ASSERT(ZFS_OK == zoidfs_remove(NULL, NULL, symlink_fullpath_dirname, NULL));
    total_file_count--;

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

    /* get all attributes for a regular file */
    gattr.mask = ZOIDFS_ATTR_ALL;
    zoidfs_lookup(&basedir_handle, component_filename, NULL, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_getattr(&fhandle, &gattr));
    CU_ASSERT(gattr.mask == ZOIDFS_ATTR_ALL);
    CU_ASSERT(gattr.type == ZOIDFS_REG);
    CU_ASSERT((gattr.mode & 0777) == 0755);

    /* get size attributes for a regular file */
    gattr.mask = ZOIDFS_ATTR_SIZE;
    zoidfs_lookup(&basedir_handle, component_filename, NULL, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_getattr(&fhandle, &gattr));
    CU_ASSERT(gattr.mask == ZOIDFS_ATTR_SIZE);

    /* get blocksize attributes for a regular file */
    gattr.mask = ZOIDFS_ATTR_BSIZE;
    zoidfs_lookup(&basedir_handle, component_filename, NULL, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_getattr(&fhandle, &gattr));
    CU_ASSERT(gattr.mask == ZOIDFS_ATTR_BSIZE);

    /* get nlink attributes for a regular file */
    gattr.mask = ZOIDFS_ATTR_NLINK;
    zoidfs_lookup(&basedir_handle, component_filename, NULL, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_getattr(&fhandle, &gattr));
    CU_ASSERT(gattr.mask == ZOIDFS_ATTR_NLINK);

    /* get atime attributes for a regular file */
    gattr.mask = ZOIDFS_ATTR_ATIME;
    zoidfs_lookup(&basedir_handle, component_filename, NULL, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_getattr(&fhandle, &gattr));
    CU_ASSERT(gattr.mask == ZOIDFS_ATTR_ATIME);

    /* get mtime attributes for a regular file */
    gattr.mask = ZOIDFS_ATTR_MTIME;
    zoidfs_lookup(&basedir_handle, component_filename, NULL, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_getattr(&fhandle, &gattr));
    CU_ASSERT(gattr.mask == ZOIDFS_ATTR_MTIME);

    /* get ctime attributes for a regular file */
    gattr.mask = ZOIDFS_ATTR_CTIME;
    zoidfs_lookup(&basedir_handle, component_filename, NULL, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_getattr(&fhandle, &gattr));
    CU_ASSERT(gattr.mask == ZOIDFS_ATTR_CTIME);

    /* get fsid attributes for a regular file */
    gattr.mask = ZOIDFS_ATTR_FSID;
    zoidfs_lookup(&basedir_handle, component_filename, NULL, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_getattr(&fhandle, &gattr));
    CU_ASSERT(gattr.mask == ZOIDFS_ATTR_FSID);

    /* get fileid attributes for a regular file */
    gattr.mask = ZOIDFS_ATTR_FILEID;
    zoidfs_lookup(&basedir_handle, component_filename, NULL, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_getattr(&fhandle, &gattr));
    CU_ASSERT(gattr.mask == ZOIDFS_ATTR_FILEID);

    /* get all attributes for a directory */
    gattr.mask = ZOIDFS_ATTR_ALL;
    zoidfs_lookup(&basedir_handle, component_dirname, NULL, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_getattr(&fhandle, &gattr));
    CU_ASSERT(gattr.mask == ZOIDFS_ATTR_ALL);
    CU_ASSERT(gattr.type == ZOIDFS_DIR);
    CU_ASSERT((gattr.mode & 0777) == 0755);

    /* get all attributes for a regular file */
    gattr.mask = ZOIDFS_ATTR_ALL;
    zoidfs_lookup(NULL, NULL, fullpath_filename, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_getattr(&fhandle, &gattr));
    CU_ASSERT(gattr.mask == ZOIDFS_ATTR_ALL);
    CU_ASSERT(gattr.type == ZOIDFS_REG);
    CU_ASSERT((gattr.mode & 0777) == 0755);

    /* get all attributes for a directory */
    gattr.mask = ZOIDFS_ATTR_ALL;
    zoidfs_lookup(NULL, NULL, fullpath_dirname, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_getattr(&fhandle, &gattr));
    CU_ASSERT(gattr.mask == ZOIDFS_ATTR_ALL);
    CU_ASSERT(gattr.type == ZOIDFS_DIR);
    CU_ASSERT((gattr.mode & 0777) == 0755);

    /* get all attributes for a regular file */
    gattr.mask = ZOIDFS_ATTR_ALL;
    zoidfs_lookup(&basedir_handle, symlink_component_filename, NULL, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_getattr(&fhandle, &gattr));
    CU_ASSERT(gattr.mask == ZOIDFS_ATTR_ALL);
    CU_ASSERT(gattr.type == ZOIDFS_LNK);

    /* get all attributes for a regular file */
    gattr.mask = ZOIDFS_ATTR_ALL;
    zoidfs_lookup(&basedir_handle, symlink_component_dirname, NULL, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_getattr(&fhandle, &gattr));
    CU_ASSERT(gattr.mask == ZOIDFS_ATTR_ALL);
    CU_ASSERT(gattr.type == ZOIDFS_LNK);

    /* get all attributes for a regular file */
    gattr.mask = ZOIDFS_ATTR_ALL;
    zoidfs_lookup(NULL, NULL, symlink_fullpath_filename, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_getattr(&fhandle, &gattr));
    CU_ASSERT(gattr.mask == ZOIDFS_ATTR_ALL);
    CU_ASSERT(gattr.type == ZOIDFS_LNK);

    /* get all attributes for a regular file */
    gattr.mask = ZOIDFS_ATTR_ALL;
    zoidfs_lookup(NULL, NULL, symlink_fullpath_dirname, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_getattr(&fhandle, &gattr));
    CU_ASSERT(gattr.mask == ZOIDFS_ATTR_ALL);
    CU_ASSERT(gattr.type == ZOIDFS_LNK);

    return 0;
}

int testSETATTR(void)
{
    zoidfs_handle_t fhandle;
    zoidfs_attr_t gattr;
    zoidfs_sattr_t sattr;

#define SATTR_MASK_MODE_0644 \
do{ \
    sattr.mask = ZOIDFS_ATTR_MODE; \
    sattr.mode = 0644; \
}while(0)

#define SATTR_MASK_MODE_0755 \
do{ \
    sattr.mask = ZOIDFS_ATTR_MODE; \
    sattr.mode = 0755; \
}while(0)

#define ATTR_MASK_MODE \
do{ \
    gattr.mask = ZOIDFS_ATTR_MODE; \
}while(0)

    /* set the attrs */
    SATTR_MASK_MODE_0644;
    ATTR_MASK_MODE;
    zoidfs_lookup(&basedir_handle, component_filename, NULL, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_setattr(&fhandle, &sattr, &gattr));
    CU_ASSERT(gattr.mode == 0644);
    SATTR_MASK_MODE_0755;
    ATTR_MASK_MODE;
    CU_ASSERT(ZFS_OK == zoidfs_setattr(&fhandle, &sattr, &gattr));
    CU_ASSERT(gattr.mode == 0755);
    
    SATTR_MASK_MODE_0644;
    ATTR_MASK_MODE;
    zoidfs_lookup(&basedir_handle, component_dirname, NULL, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_setattr(&fhandle, &sattr, &gattr));
    CU_ASSERT(gattr.mode == 0644);
    SATTR_MASK_MODE_0755;
    ATTR_MASK_MODE;
    CU_ASSERT(ZFS_OK == zoidfs_setattr(&fhandle, &sattr, &gattr));
    CU_ASSERT(gattr.mode == 0755);
    
    SATTR_MASK_MODE_0644;
    ATTR_MASK_MODE;
    zoidfs_lookup(NULL, NULL, fullpath_filename, &fhandle);
    CU_ASSERT_EQUAL(ZFS_OK, zoidfs_setattr(&fhandle, &sattr, &gattr));
    CU_ASSERT_EQUAL(gattr.mode, 0644);
    SATTR_MASK_MODE_0755;
    ATTR_MASK_MODE;
    CU_ASSERT(ZFS_OK == zoidfs_setattr(&fhandle, &sattr, &gattr));
    CU_ASSERT(gattr.mode == 0755);
    
    SATTR_MASK_MODE_0644;
    ATTR_MASK_MODE;
    zoidfs_lookup(NULL, NULL, fullpath_dirname, &fhandle);
    CU_ASSERT_EQUAL(ZFS_OK, zoidfs_setattr(&fhandle, &sattr, &gattr));
    CU_ASSERT_EQUAL(gattr.mode, 0644);
    SATTR_MASK_MODE_0755;
    ATTR_MASK_MODE;
    CU_ASSERT(ZFS_OK == zoidfs_setattr(&fhandle, &sattr, &gattr));
    CU_ASSERT(gattr.mode == 0755);
    
    zoidfs_lookup(&basedir_handle, symlink_component_filename, NULL, &fhandle);
    CU_ASSERT_EQUAL(ZFS_OK, zoidfs_setattr(&fhandle, &sattr, &gattr));

    zoidfs_lookup(&basedir_handle, symlink_component_dirname, NULL, &fhandle);
    CU_ASSERT_EQUAL(ZFS_OK, zoidfs_setattr(&fhandle, &sattr, &gattr));

    zoidfs_lookup(NULL, NULL, symlink_fullpath_filename, &fhandle);
    CU_ASSERT_EQUAL(ZFS_OK, zoidfs_setattr(&fhandle, &sattr, &gattr));

    zoidfs_lookup(NULL, NULL, symlink_fullpath_dirname, &fhandle);
    CU_ASSERT_EQUAL(ZFS_OK, zoidfs_setattr(&fhandle, &sattr, &gattr));

    return 0;
}

int testREADLINK(void)
{
    zoidfs_handle_t fhandle;
    char buffer[4096];
    size_t buffer_length = 4096;

    zoidfs_lookup(&basedir_handle, symlink_component_filename, NULL, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_readlink(&fhandle, buffer, buffer_length));

    zoidfs_lookup(&basedir_handle, symlink_component_dirname, NULL, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_readlink(&fhandle, buffer, buffer_length));

    zoidfs_lookup(NULL, NULL, symlink_fullpath_filename, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_readlink(&fhandle, buffer, buffer_length));

    zoidfs_lookup(NULL, NULL, symlink_fullpath_dirname, &fhandle);
    CU_ASSERT(ZFS_OK == zoidfs_readlink(&fhandle, buffer, buffer_length));

    zoidfs_lookup(&basedir_handle, component_filename, NULL, &fhandle);
    CU_ASSERT(ZFS_OK != zoidfs_readlink(&fhandle, buffer, buffer_length));

    zoidfs_lookup(&basedir_handle, component_dirname, NULL, &fhandle);
    CU_ASSERT(ZFS_OK != zoidfs_readlink(&fhandle, buffer, buffer_length));

    zoidfs_lookup(NULL, NULL, fullpath_filename, &fhandle);
    CU_ASSERT(ZFS_OK != zoidfs_readlink(&fhandle, buffer, buffer_length));

    zoidfs_lookup(NULL, NULL, fullpath_dirname, &fhandle);
    CU_ASSERT(ZFS_OK != zoidfs_readlink(&fhandle, buffer, buffer_length));

    return 0;
}

int testREADDIR(void)
{
    zoidfs_handle_t fhandle;
    zoidfs_dirent_cookie_t cookie = 0;
    size_t entry_count = 32;
    zoidfs_dirent_t * entries;
    int flags = 0;

    cookie = 0;
    entry_count = 32;
    entries = malloc(entry_count * sizeof(zoidfs_dirent_t));
    memset(entries, 0, entry_count * sizeof(zoidfs_dirent_t));
    CU_ASSERT_EQUAL(ZFS_OK, zoidfs_readdir(&basedir_handle, cookie, &entry_count, entries, flags, NULL)); 
    CU_ASSERT(total_file_count + 2 == entry_count);
    free(entries);

    cookie = 0;
    entry_count = 32;
    entries = malloc(entry_count * sizeof(zoidfs_dirent_t));
    memset(entries, 0, entry_count * sizeof(zoidfs_dirent_t));
    zoidfs_lookup(&basedir_handle, component_dirname, NULL, &fhandle);
    CU_ASSERT_EQUAL(ZFS_OK, zoidfs_readdir(&fhandle, cookie, &entry_count, entries, flags, NULL)); 
    CU_ASSERT(2 == entry_count); 
    free(entries);
    
    cookie = 0;
    entry_count = 32;
    entries = malloc(entry_count * sizeof(zoidfs_dirent_t));
    memset(entries, 0, entry_count * sizeof(zoidfs_dirent_t));
    zoidfs_lookup(NULL, NULL, fullpath_dirname, &fhandle);
    CU_ASSERT_EQUAL(ZFS_OK, zoidfs_readdir(&fhandle, cookie, &entry_count, entries, flags, NULL)); 
    CU_ASSERT(2 == entry_count); 
    free(entries);
    
    cookie = 0;
    entry_count = 32;
    entries = malloc(entry_count * sizeof(zoidfs_dirent_t));
    memset(entries, 0, entry_count * sizeof(zoidfs_dirent_t));
    zoidfs_lookup(&basedir_handle, symlink_component_dirname, NULL, &fhandle);
    CU_ASSERT_NOT_EQUAL(ZFS_OK, zoidfs_readdir(&fhandle, cookie, &entry_count, entries, flags, NULL)); 
    free(entries);
    
    cookie = 0;
    entry_count = 32;
    entries = malloc(entry_count * sizeof(zoidfs_dirent_t));
    memset(entries, 0, entry_count * sizeof(zoidfs_dirent_t));
    zoidfs_lookup(&basedir_handle, symlink_component_dirname_slash, NULL, &fhandle);
    CU_ASSERT_EQUAL(ZFS_OK, zoidfs_readdir(&fhandle, cookie, &entry_count, entries, flags, NULL)); 
    free(entries);

    cookie = 0;
    entry_count = 32;
    entries = malloc(entry_count * sizeof(zoidfs_dirent_t));
    memset(entries, 0, entry_count * sizeof(zoidfs_dirent_t));
    zoidfs_lookup(NULL, NULL, symlink_fullpath_dirname_slash, &fhandle);
    CU_ASSERT_EQUAL(ZFS_OK, zoidfs_readdir(&fhandle, cookie, &entry_count, entries, flags, NULL)); 
    free(entries);

    cookie = 0;
    entry_count = 32;
    entries = malloc(entry_count * sizeof(zoidfs_dirent_t));
    memset(entries, 0, entry_count * sizeof(zoidfs_dirent_t));
    zoidfs_lookup(&basedir_handle, component_filename, NULL, &fhandle);
    CU_ASSERT_NOT_EQUAL(ZFS_OK, zoidfs_readdir(&fhandle, cookie, &entry_count, entries, flags, NULL)); 
    free(entries);

    cookie = 0;
    entry_count = 32;
    entries = malloc(entry_count * sizeof(zoidfs_dirent_t));
    memset(entries, 0, entry_count * sizeof(zoidfs_dirent_t));
    zoidfs_lookup(NULL, NULL, fullpath_filename, &fhandle);
    CU_ASSERT_NOT_EQUAL(ZFS_OK, zoidfs_readdir(&fhandle, cookie, &entry_count, entries, flags, NULL)); 
    free(entries);
    
    cookie = 0;
    entry_count = 32;
    entries = malloc(entry_count * sizeof(zoidfs_dirent_t));
    memset(entries, 0, entry_count * sizeof(zoidfs_dirent_t));
    zoidfs_lookup(&basedir_handle, symlink_component_filename, NULL, &fhandle);
    CU_ASSERT_NOT_EQUAL(ZFS_OK, zoidfs_readdir(&fhandle, cookie, &entry_count, entries, flags, NULL)); 
    free(entries);
    
    cookie = 0;
    entry_count = 32;
    entries = malloc(entry_count * sizeof(zoidfs_dirent_t));
    memset(entries, 0, entry_count * sizeof(zoidfs_dirent_t));
    zoidfs_lookup(NULL, NULL, symlink_fullpath_filename, &fhandle);
    CU_ASSERT_NOT_EQUAL(ZFS_OK, zoidfs_readdir(&fhandle, cookie, &entry_count, entries, flags, NULL)); 
    free(entries);
    
    return 0;
}

int testRESIZE(void)
{
    zoidfs_handle_t fhandle;
    zoidfs_attr_t gattr;
    unsigned int size; 

    size = random () % 2048;
    gattr.mask = ZOIDFS_ATTR_ALL;
    zoidfs_lookup(&basedir_handle, component_filename, NULL, &fhandle);
    CU_ASSERT_EQUAL(ZFS_OK , zoidfs_resize(&fhandle, size));
    zoidfs_getattr(&fhandle, &gattr);
    CU_ASSERT_EQUAL(gattr.size , size);

    /* directory, should fail */ 
    zoidfs_lookup(&basedir_handle, component_dirname, NULL, &fhandle);
    CU_ASSERT_NOT_EQUAL(ZFS_OK, zoidfs_resize(&fhandle, size));

    size = random () % 2048;
    gattr.mask = ZOIDFS_ATTR_ALL;
    zoidfs_lookup(NULL, NULL, fullpath_filename, &fhandle);
    CU_ASSERT_EQUAL(ZFS_OK, zoidfs_resize(&fhandle, size))
    zoidfs_getattr(&fhandle, &gattr);
    CU_ASSERT_EQUAL(gattr.size , size);

    /* directory, should fail */
    zoidfs_lookup(NULL, NULL, fullpath_dirname, &fhandle);
    CU_ASSERT_NOT_EQUAL(ZFS_OK, zoidfs_resize(&fhandle, size));

    zoidfs_lookup(&basedir_handle, symlink_component_filename, NULL, &fhandle);
    CU_ASSERT_EQUAL(ZFS_OK, zoidfs_resize(&fhandle, size));

    /* directory, should fail */
    zoidfs_lookup(&basedir_handle, symlink_component_dirname, NULL, &fhandle);
    CU_ASSERT_NOT_EQUAL(ZFS_OK, zoidfs_resize(&fhandle, size));

    zoidfs_lookup(NULL, NULL, symlink_fullpath_filename, &fhandle);
    CU_ASSERT_EQUAL(ZFS_OK, zoidfs_resize(&fhandle, size));

    /* directory, should fail */
    zoidfs_lookup(NULL, NULL, symlink_fullpath_dirname, &fhandle);
    CU_ASSERT_NOT_EQUAL(ZFS_OK, zoidfs_resize(&fhandle, size));

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

    /* setup test directory */
    char testDir[256];
    sprintf(testDir, "%s/%s", argv[1], "zoidfs-test-dir");
    mkdir(testDir, 0755);

    zoidfs_init();

   /* setup path names for the tests */
   init_path_names("zoidfs-test-dir", argv[1]);

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
        (NULL == CU_add_test(pSuite, "test of zoidfs_link()", (CU_TestFunc) testLINK)) ||
        (NULL == CU_add_test(pSuite, "test of zoidfs_rename()", (CU_TestFunc) testRENAME)) ||
        (NULL == CU_add_test(pSuite, "test of zoidfs_lookup()", (CU_TestFunc) testLOOKUP)) ||
        (NULL == CU_add_test(pSuite, "test of zoidfs_getattr()", (CU_TestFunc) testGETATTR)) ||
        (NULL == CU_add_test(pSuite, "test of zoidfs_setattr()", (CU_TestFunc) testSETATTR)) ||
        (NULL == CU_add_test(pSuite, "test of zoidfs_resize()", (CU_TestFunc) testRESIZE)) ||
        (NULL == CU_add_test(pSuite, "test of zoidfs_readdir()", (CU_TestFunc) testREADDIR)) ||
        (NULL == CU_add_test(pSuite, "test of zoidfs_readlink()", (CU_TestFunc) testREADLINK)) ||
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
