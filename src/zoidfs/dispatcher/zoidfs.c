#include <errno.h>
#include <assert.h>
#include <string.h>
#include "zoidfs/zoidfs.h"
#include "zint-handler.h"

int zoidfs_null(void)
{
   return zint_ping_handlers (); 
}

int zoidfs_create(const zoidfs_handle_t * parent_handle,
                  const char * component_name,
                  const char * full_path,
                  const zoidfs_sattr_t * sattr,
                  zoidfs_handle_t * handle,
                  int * created)
{
    int ret;
    char newpath[ZOIDFS_PATH_MAX];
    zint_handler_t * handler;
    int id; 
    zoidfs_handle_t newhandle;
    int usenew = 0; 

    if (full_path)
       id = zint_locate_handler_path (full_path, &handler, newpath, ZOIDFS_PATH_MAX,
             &newhandle, &usenew);
    else
       id = zint_locate_handler_handle(parent_handle, &handler);
    if(id < 0)
    {
        return ZFSERR_INVAL;
    }

    ret = handler->create(usenew ? &newhandle : parent_handle,
                           component_name,
                           full_path ? newpath : NULL,
                           sattr,
                           handle,
                           created);
    ZINT_MARK_HANDLE(handle,id);  
    return ret; 
}

int zoidfs_getattr(const zoidfs_handle_t * handle,
                   zoidfs_attr_t * attr)
{
    int ret;
    zint_handler_t * handler;

    ret = zint_locate_handler_handle(handle, &handler);
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }

    return handler->getattr(handle, attr);
}
    
int zoidfs_setattr(const zoidfs_handle_t * handle,
                   const zoidfs_sattr_t * sattr,
                   zoidfs_attr_t * attr)
{
    zint_handler_t * handler;
    int ret;

    ret = zint_locate_handler_handle(handle, &handler);
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }

    return handler->setattr(handle, sattr, attr);
}
 
int zoidfs_lookup(const zoidfs_handle_t * parent_handle,
                  const char * component_name,
                  const char * full_path,
                  zoidfs_handle_t * handle)
{
    zint_handler_t * handler;
    char new_path[ZOIDFS_PATH_MAX];
    int hid;
    int ret; 
    zoidfs_handle_t newhandle; 
    int usenew = 0; 

    if (full_path)
       hid = zint_locate_handler_path (full_path, &handler, new_path,
             ZOIDFS_PATH_MAX, &newhandle, &usenew);
    else
       hid = zint_locate_handler_handle(parent_handle, &handler);
    if(hid < 0)
    {
        return ZFSERR_INVAL;
    }

    ret = handler->lookup(usenew ? &newhandle : parent_handle, 
                           component_name, 
                           full_path ? new_path : NULL, 
                           handle);
    ZINT_MARK_HANDLE(handle, hid); 
    return ret; 
}

int zoidfs_readlink(const zoidfs_handle_t * handle,
                    char * buffer,
                    size_t buffer_length)
{
    zint_handler_t * handler;
    int ret;

    ret = zint_locate_handler_handle(handle, &handler); 
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }
    return handler->readlink(handle, buffer, buffer_length);
}

int zoidfs_read(const zoidfs_handle_t * handle,
                size_t mem_count,
                void * mem_starts[],
                const size_t mem_sizes[],
                size_t file_count,
                const uint64_t file_starts[],
                uint64_t file_sizes[])
{
    zint_handler_t * handler;
    int ret;

    ret = zint_locate_handler_handle(handle, &handler);
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }

    return handler->read(handle, 
                         mem_count, 
                         mem_starts, 
                         mem_sizes, 
                         file_count, 
                         file_starts, 
                         file_sizes);
}
     
int zoidfs_write(const zoidfs_handle_t * handle,
                 size_t mem_count,
                 const void * mem_starts[],
                 const size_t mem_sizes[],
                 size_t file_count,
                 const uint64_t file_starts[],
                 uint64_t file_sizes[])
{
    zint_handler_t * handler;
    int ret;

    ret = zint_locate_handler_handle(handle, &handler);
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }

    return handler->write(handle,
                          mem_count,
                          mem_starts,
                          mem_sizes,
                          file_count,
                          file_starts,
                          file_sizes);
}

int zoidfs_remove(const zoidfs_handle_t * parent_handle,
                  const char * component_name,
                  const char * full_path,
                  zoidfs_cache_hint_t * parent_hint)
{
    int ret;
    zint_handler_t * handler;
    char newpath[ZOIDFS_PATH_MAX];
    zoidfs_handle_t newhandle;
    int usenew = 0; 
   
    if (full_path)
       ret = zint_locate_handler_path (full_path, &handler, newpath, ZOIDFS_PATH_MAX,
             &newhandle, &usenew);
    else
       ret = zint_locate_handler_handle(parent_handle, &handler);
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }

    return handler->remove(usenew ? &newhandle : parent_handle,
                           component_name,
                           full_path ? newpath : NULL,
                           parent_hint);
}

int zoidfs_commit(const zoidfs_handle_t * handle)
{
    int ret;
    zint_handler_t * handler;
    
    ret = zint_locate_handler_handle(handle, &handler);
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }

    return handler->commit(handle);
}

int zoidfs_rename(const zoidfs_handle_t * from_parent_handle,
                  const char * from_component_name,
                  const char * from_full_path,
                  const zoidfs_handle_t * to_parent_handle,
                  const char * to_component_name,
                  const char * to_full_path,
                  zoidfs_cache_hint_t * from_parent_hint,
                  zoidfs_cache_hint_t * to_parent_hint)
{
    int ret;
    zint_handler_t * handler;
    char  new_from_path[ZOIDFS_PATH_MAX], new_to_path[ZOIDFS_PATH_MAX];
    int from_usenew = 0; 
    int to_usenew = 0; 
    zoidfs_handle_t from_newhandle; 
    zoidfs_handle_t to_newhandle; 

    if (from_full_path)
       ret = zint_locate_handler_path (from_full_path, &handler, new_from_path, ZOIDFS_PATH_MAX,
             &from_newhandle, &from_usenew);
    else
       ret = zint_locate_handler_handle(from_parent_handle, &handler);
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }

    if (to_full_path)
       ret = zint_locate_handler_path (to_full_path, &handler, new_to_path, ZOIDFS_PATH_MAX,
             &to_newhandle, &to_usenew);
    else
       ret = zint_locate_handler_handle(to_parent_handle, &handler);
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }

    return handler->rename(from_usenew ? &from_newhandle :from_parent_handle,
                           from_component_name,
                           from_full_path ? new_from_path : from_full_path,
                           to_usenew ? &to_newhandle : to_parent_handle,
                           to_component_name,
                           to_full_path ? new_to_path : to_full_path,
                           from_parent_hint,
                           to_parent_hint);
}

int zoidfs_link(const zoidfs_handle_t * from_parent_handle,
                const char * from_component_name,
                const char * from_full_path,
                const zoidfs_handle_t * to_parent_handle,
                const char * to_component_name,
                const char * to_full_path,
                zoidfs_cache_hint_t * from_parent_hint,
                zoidfs_cache_hint_t * to_parent_hint)
{
   /* doesn't this need a check to make sure both handlers are the same?? */
    int ret;
    zint_handler_t * handler;
    char new_from_path[ZOIDFS_PATH_MAX];
    char new_to_path[ZOIDFS_PATH_MAX];
    int to_usenew = 0; 
    int from_usenew = 0; 
    zoidfs_handle_t to_newhandle; 
    zoidfs_handle_t from_newhandle; 

    if (from_full_path)
       ret = zint_locate_handler_path (from_full_path, &handler, new_from_path, ZOIDFS_PATH_MAX,
             &from_newhandle, &from_usenew);
    else
       ret = zint_locate_handler_handle(from_parent_handle, &handler);
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }

    if (to_full_path)
       ret = zint_locate_handler_path (to_full_path, &handler, new_to_path, ZOIDFS_PATH_MAX,
             &to_newhandle, &to_usenew);
    else
       ret = zint_locate_handler_handle(to_parent_handle, &handler);
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }

    return handler->link(from_usenew ? &from_newhandle : from_parent_handle, 
                         from_component_name,
                         (from_full_path ? new_from_path : from_full_path),
                         to_usenew ? &to_newhandle : to_parent_handle,
                         to_component_name,
                         (to_full_path ? new_to_path : to_full_path),
                         from_parent_hint,
                         to_parent_hint);
}

int zoidfs_symlink(const zoidfs_handle_t * from_parent_handle,
                   const char * from_component_name,
                   const char * from_full_path,
                   const zoidfs_handle_t * to_parent_handle,
                   const char * to_component_name,
                   const char * to_full_path,
                   const zoidfs_sattr_t * attr,
                   zoidfs_cache_hint_t * from_parent_hint,
                   zoidfs_cache_hint_t * to_parent_hint)
{
   /* doesn't this need a check to make sure both handlers are the same?? */
    int ret;
    zint_handler_t * handler;
    char new_from_path[ZOIDFS_PATH_MAX];
    char new_to_path[ZOIDFS_PATH_MAX];
    int to_usenew = 0;
    int from_usenew = 0; 
    zoidfs_handle_t to_newhandle; 
    zoidfs_handle_t from_newhandle; 

    if (from_full_path)
       ret = zint_locate_handler_path (from_full_path, &handler, new_from_path, ZOIDFS_PATH_MAX,
             &from_newhandle, &from_usenew);
    else
       ret = zint_locate_handler_handle(from_parent_handle, &handler);
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }

    if (to_full_path)
       ret = zint_locate_handler_path (to_full_path, &handler, new_to_path, ZOIDFS_PATH_MAX,
             &to_newhandle, &to_usenew);
    else
       ret = zint_locate_handler_handle(to_parent_handle, &handler);
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }

    return handler->symlink(from_usenew ? &from_newhandle : from_parent_handle, 
                            from_component_name,
                            (from_full_path ? new_from_path : from_full_path),
                            to_usenew ? &to_newhandle : to_parent_handle,
                            to_component_name,
                            to_full_path ? new_to_path : to_full_path,
                            attr,
                            from_parent_hint,
                            to_parent_hint);
}

int zoidfs_mkdir(const zoidfs_handle_t * parent_handle,
                 const char * component_name,
                 const char * full_path,
                 const zoidfs_sattr_t * attr,
                 zoidfs_cache_hint_t * parent_hint)
{
    int ret;
    zint_handler_t * handler;
    char new_path[ZOIDFS_PATH_MAX];
    int usenew =0; 
    zoidfs_handle_t newhandle; 

    if (full_path)
       ret = zint_locate_handler_path (full_path, &handler, new_path, ZOIDFS_PATH_MAX,
             &newhandle, &usenew);
    else
       ret = zint_locate_handler_handle(parent_handle, &handler);
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }

    return handler->mkdir(usenew ? &newhandle :parent_handle,
                          component_name,
                          full_path ? new_path : NULL,
                          attr,
                          parent_hint);
}

int zoidfs_readdir(const zoidfs_handle_t * parent_handle,
                   zoidfs_dirent_cookie_t cookie,
                   size_t * entry_count,
                   zoidfs_dirent_t * entries,
                   uint32_t flags,
                   zoidfs_cache_hint_t * parent_hint)
{
    int ret;
    zint_handler_t * handler;
    
    ret = zint_locate_handler_handle(parent_handle, &handler);
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }

    return handler->readdir(parent_handle,
                            cookie,
                            entry_count,
                            entries,
                            flags,
                            parent_hint);
}

int zoidfs_resize(const zoidfs_handle_t * handle,
                  uint64_t size)
{
    int ret;
    zint_handler_t * handler;

    ret = zint_locate_handler_handle(handle, &handler);
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }

    return handler->resize(handle,
                           size);
}    

/**
 * OPTIONAL
 */
int zoidfs_init(void)
{
   return zint_initialize_handlers ();
}

/**
 * OPTIONAL
 */
int zoidfs_finalize(void)
{
   return zint_finalize_handlers (); 
}

/* Only needed by the BGP ZOID version.  */
void __zoidfs_init(int pset_mpi_proc_count, int argc, int envc,
                   const char* argenvint)
{
    zoidfs_init();
}

/* Only needed by the BGP ZOID version.  */
void __zoidfs_finalize(void)
{
    zoidfs_finalize();
}


/*
 * Local variables:
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 *
 * vim: ts=8 sts=4 sw=4 expandtab
 */
