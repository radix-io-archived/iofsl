#ifndef SRC_ZOIDFS_CLIENT_PZOIDFSDEF
#define SRC_ZOIDFS_CLIENT_PZOIDFSDEF

/*
 * profiling symbols for the zoidfs api
 */

#pragma weak zoidfs_getattr = Pzoidfs_getattr
#undef zoidfs_getattr
#define zoidfs_getattr Pzoidfs_getattr

#pragma weak zoidfs_setattr = Pzoidfs_setattr
#undef zoidfs_setattr
#define zoidfs_setattr Pzoidfs_setattr

#pragma weak zoidfs_lookup = Pzoidfs_lookup
#undef zoidfs_lookup
#define zoidfs_lookup Pzoidfs_lookup

#pragma weak zoidfs_readlink = Pzoidfs_readlink
#undef zoidfs_readlink
#define zoidfs_readlink Pzoidfs_readlink

#pragma weak zoidfs_read = Pzoidfs_read
#undef zoidfs_read
#define zoidfs_read Pzoidfs_read

#pragma weak zoidfs_write = Pzoidfs_write
#undef zoidfs_write
#define zoidfs_write Pzoidfs_write

#pragma weak zoidfs_commit = Pzoidfs_commit
#undef zoidfs_commit
#define zoidfs_commit Pzoidfs_commit

#pragma weak zoidfs_create = Pzoidfs_create
#undef zoidfs_create
#define zoidfs_create Pzoidfs_create

#pragma weak zoidfs_remove = Pzoidfs_remove
#undef zoidfs_remove
#define zoidfs_remove Pzoidfs_remove

#pragma weak zoidfs_rename = Pzoidfs_rename
#undef zoidfs_rename
#define zoidfs_rename Pzoidfs_rename

#pragma weak zoidfs_link = Pzoidfs_link
#undef zoidfs_link
#define zoidfs_link Pzoidfs_link

#pragma weak zoidfs_symlink = Pzoidfs_symlink
#undef zoidfs_symlink
#define zoidfs_symlink Pzoidfs_symlink

#pragma weak zoidfs_mkdir = Pzoidfs_mkdir
#undef zoidfs_mkdir
#define zoidfs_mkdir Pzoidfs_mkdir

#pragma weak zoidfs_readdir = Pzoidfs_readdir
#undef zoidfs_readdir
#define zoidfs_readdir Pzoidfs_readdir

#pragma weak zoidfs_resize = Pzoidfs_resize
#undef zoidfs_resize
#define zoidfs_resize Pzoidfs_resize

#pragma weak zoidfs_init = Pzoidfs_init
#undef zoidfs_init
#define zoidfs_init Pzoidfs_init

#pragma weak zoidfs_finalize = Pzoidfs_finalize
#undef zoidfs_finalize
#define zoidfs_finalize Pzoidfs_finalize

#pragma weak zoidfs_null = Pzoidfs_null
#undef zoidfs_null
#define zoidfs_null Pzoidfs_null

#endif
