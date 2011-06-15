#include <errno.h>
#include <assert.h>
#include <string.h>
#include "zoidfs/zoidfs.h"
#include "zint-handler.h"
#include "c-util/tools.h"
#include "zoidfs/client/pzoidfsdef.h"

/*
 * Make sure that the full path is an absolute path
 */
static inline int zoidfs_full_path_validate(const char * path)
{
    if(path[0] == '/')
    {
        return 1;
    }
    return 0;
}

int zoidfs_null(zoidfs_op_hint_t * UNUSED(op_hint))
{
   return zint_ping_handlers (); 
}

int zoidfs_create(const zoidfs_handle_t * parent_handle,
                  const char * component_name,
                  const char * full_path,
                  const zoidfs_sattr_t * sattr,
                  zoidfs_handle_t * handle,
                  int * created,
                  zoidfs_op_hint_t * op_hint)
{
    int ret;
    char newpath[ZOIDFS_PATH_MAX];
    zint_handler_t * handler;
    int id; 
    zoidfs_handle_t newhandle;
    int usenew = 0; 

    if (full_path)
    {
       if(!zoidfs_full_path_validate(full_path))
            return ZFSERR_OTHER;
       id = zint_locate_handler_path (full_path, &handler, newpath, ZOIDFS_PATH_MAX,
             &newhandle, &usenew);
    }
    else
       id = zint_locate_handler_handle(parent_handle, &handler);
    if(id < 0)
    {
        return ZFSERR_INVAL;
    }

#ifdef ZFS_DISPATCHER_JOURNAL
    /* add to journal */
    int op = ZOIDFS_OP_CREATE;
    int fp = full_path ? 1 : 0;

    ret = fwrite(&op, sizeof(int), 1, journalFP);
    ret = fwrite(&fp, sizeof(char), 1, journalFP);
    if(fp)
    {
        ret = fwrite(full_path, strlen(full_path), 1, journalFP);
    }
    else
    {
        ret = fwrite(parent_handle, sizeof(*parent_handle), 1, journalFP);
        ret = fwrite(component_name, strlen(full_path), 1, journalFP);
    }
    ret = fwrite(sattr, sizeof(zoidfs_sattr_t), 1, journalFP);
#endif

    ret = handler->create(usenew ? &newhandle : parent_handle,
                           component_name,
                           full_path ? newpath : NULL,
                           sattr,
                           handle,
                           created,
                           op_hint);
    ZINT_MARK_HANDLE(handle,id);  
    return ret; 
}

int zoidfs_getattr(const zoidfs_handle_t * handle,
                   zoidfs_attr_t * attr,
                   zoidfs_op_hint_t * op_hint)
{
    int ret;
    zint_handler_t * handler;

    ret = zint_locate_handler_handle(handle, &handler);
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }

    return handler->getattr(handle, attr, op_hint);
}
    
int zoidfs_setattr(const zoidfs_handle_t * handle,
                   const zoidfs_sattr_t * sattr,
                   zoidfs_attr_t * attr,
                   zoidfs_op_hint_t * op_hint)
{
    zint_handler_t * handler;
    int ret;

    ret = zint_locate_handler_handle(handle, &handler);
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }

    return handler->setattr(handle, sattr, attr,
                           op_hint);
}
 
int zoidfs_lookup(const zoidfs_handle_t * parent_handle,
                  const char * component_name,
                  const char * full_path,
                  zoidfs_handle_t * handle,
                  zoidfs_op_hint_t * op_hint)
{
    zint_handler_t * handler;
    char new_path[ZOIDFS_PATH_MAX];
    int hid;
    int ret; 
    zoidfs_handle_t newhandle; 
    int usenew = 0;

    if (full_path)
    {
       if(!zoidfs_full_path_validate(full_path))
            return ZFSERR_OTHER;
       hid = zint_locate_handler_path (full_path, &handler, new_path,
             ZOIDFS_PATH_MAX, &newhandle, &usenew);
    }
    else
       hid = zint_locate_handler_handle(parent_handle, &handler);
    if(hid < 0)
    {
        return ZFSERR_INVAL;
    }

    ret = handler->lookup(usenew ? &newhandle : parent_handle, 
                           component_name, 
                           full_path ? new_path : NULL, 
                           handle,
                           op_hint);
    ZINT_MARK_HANDLE(handle, hid); 
    return ret; 
}

int zoidfs_readlink(const zoidfs_handle_t * handle,
                    char * buffer,
                    size_t buffer_length,
                    zoidfs_op_hint_t * op_hint)
{
    zint_handler_t * handler;
    int ret;

    ret = zint_locate_handler_handle(handle, &handler); 
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }

    ret = handler->readlink(handle, buffer, buffer_length, op_hint);

    /* make sure buffer is null terminated */
    buffer[buffer_length - 1] = '\0';

    return ret;
}

int zoidfs_read(const zoidfs_handle_t * handle,
                size_t mem_count,
                void * mem_starts[],
                const size_t mem_sizes[],
                size_t file_count,
                const zoidfs_file_ofs_t file_starts[],
                zoidfs_file_size_t file_sizes[],
                zoidfs_op_hint_t * op_hint)
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
                         file_sizes,
                         op_hint);
}
     
int zoidfs_write(const zoidfs_handle_t * handle,
                 size_t mem_count,
                 const void * mem_starts[],
                 const size_t mem_sizes[],
                 size_t file_count,
                 const zoidfs_file_ofs_t file_starts[],
                 zoidfs_file_size_t file_sizes[],
                 zoidfs_op_hint_t * op_hint)
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
                          file_sizes,
                          op_hint);
}

int zoidfs_remove(const zoidfs_handle_t * parent_handle,
                  const char * component_name,
                  const char * full_path,
                  zoidfs_cache_hint_t * parent_hint,
                  zoidfs_op_hint_t * op_hint)
{
    int ret;
    zint_handler_t * handler;
    char newpath[ZOIDFS_PATH_MAX];
    zoidfs_handle_t newhandle;
    int usenew = 0; 
   
    if (full_path)
    {
       if(!zoidfs_full_path_validate(full_path))
            return ZFSERR_OTHER;
       ret = zint_locate_handler_path (full_path, &handler, newpath, ZOIDFS_PATH_MAX,
             &newhandle, &usenew);
    }
    else
       ret = zint_locate_handler_handle(parent_handle, &handler);
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }

    return handler->remove(usenew ? &newhandle : parent_handle,
                           component_name,
                           full_path ? newpath : NULL,
                           parent_hint,
                           op_hint);
}

int zoidfs_commit(const zoidfs_handle_t * handle,
                  zoidfs_op_hint_t * op_hint)
{
    int ret;
    zint_handler_t * handler;
    
    ret = zint_locate_handler_handle(handle, &handler);
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }

    return handler->commit(handle, op_hint);
}

int zoidfs_rename(const zoidfs_handle_t * from_parent_handle,
                  const char * from_component_name,
                  const char * from_full_path,
                  const zoidfs_handle_t * to_parent_handle,
                  const char * to_component_name,
                  const char * to_full_path,
                  zoidfs_cache_hint_t * from_parent_hint,
                  zoidfs_cache_hint_t * to_parent_hint,
                  zoidfs_op_hint_t * op_hint)
{
    int ret;
    zint_handler_t * handler;
    char  new_from_path[ZOIDFS_PATH_MAX], new_to_path[ZOIDFS_PATH_MAX];
    int from_usenew = 0; 
    int to_usenew = 0; 
    zoidfs_handle_t from_newhandle; 
    zoidfs_handle_t to_newhandle; 

    if (from_full_path)
    {
       if(!zoidfs_full_path_validate(from_full_path))
            return ZFSERR_OTHER;
       ret = zint_locate_handler_path (from_full_path, &handler, new_from_path, ZOIDFS_PATH_MAX,
             &from_newhandle, &from_usenew);
    }
    else
       ret = zint_locate_handler_handle(from_parent_handle, &handler);
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }

    if (to_full_path)
    {
       if(!zoidfs_full_path_validate(to_full_path))
            return ZFSERR_OTHER;
       ret = zint_locate_handler_path (to_full_path, &handler, new_to_path, ZOIDFS_PATH_MAX,
             &to_newhandle, &to_usenew);
    }
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
                           to_parent_hint,
                           op_hint);
}

int zoidfs_link(const zoidfs_handle_t * from_parent_handle,
                const char * from_component_name,
                const char * from_full_path,
                const zoidfs_handle_t * to_parent_handle,
                const char * to_component_name,
                const char * to_full_path,
                zoidfs_cache_hint_t * from_parent_hint,
                zoidfs_cache_hint_t * to_parent_hint,
                zoidfs_op_hint_t * op_hint)
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
    {
       if(!zoidfs_full_path_validate(from_full_path))
            return ZFSERR_OTHER;
       ret = zint_locate_handler_path (from_full_path, &handler, new_from_path, ZOIDFS_PATH_MAX,
             &from_newhandle, &from_usenew);
    }
    else
       ret = zint_locate_handler_handle(from_parent_handle, &handler);
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }

    if (to_full_path)
    {
       if(!zoidfs_full_path_validate(to_full_path))
            return ZFSERR_OTHER;
       ret = zint_locate_handler_path (to_full_path, &handler, new_to_path, ZOIDFS_PATH_MAX,
             &to_newhandle, &to_usenew);
    }
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
                         to_parent_hint,
                         op_hint);
}

int zoidfs_symlink(const zoidfs_handle_t * from_parent_handle,
                   const char * from_component_name,
                   const char * from_full_path,
                   const zoidfs_handle_t * to_parent_handle,
                   const char * to_component_name,
                   const char * to_full_path,
                   const zoidfs_sattr_t * attr,
                   zoidfs_cache_hint_t * from_parent_hint,
                   zoidfs_cache_hint_t * to_parent_hint,
                   zoidfs_op_hint_t * op_hint)
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
    {
       if(!zoidfs_full_path_validate(from_full_path))
            return ZFSERR_OTHER;
       ret = zint_locate_handler_path (from_full_path, &handler, new_from_path, ZOIDFS_PATH_MAX,
             &from_newhandle, &from_usenew);
    }
    else
       ret = zint_locate_handler_handle(from_parent_handle, &handler);
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }

    if (to_full_path)
    {
       if(!zoidfs_full_path_validate(to_full_path))
            return ZFSERR_OTHER;
       ret = zint_locate_handler_path (to_full_path, &handler, new_to_path, ZOIDFS_PATH_MAX,
             &to_newhandle, &to_usenew);
    }
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
                            to_parent_hint,
                            op_hint);
}

int zoidfs_mkdir(const zoidfs_handle_t * parent_handle,
                 const char * component_name,
                 const char * full_path,
                 const zoidfs_sattr_t * attr,
                 zoidfs_cache_hint_t * parent_hint,
                 zoidfs_op_hint_t * op_hint)
{
    int ret;
    zint_handler_t * handler;
    char new_path[ZOIDFS_PATH_MAX];
    int usenew =0; 
    zoidfs_handle_t newhandle; 

    if (full_path)
    {
       if(!zoidfs_full_path_validate(full_path))
            return ZFSERR_OTHER;
       ret = zint_locate_handler_path (full_path, &handler, new_path, ZOIDFS_PATH_MAX,
             &newhandle, &usenew);
    }
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
                          parent_hint,
                          op_hint);
}

int zoidfs_readdir(const zoidfs_handle_t * parent_handle,
                   zoidfs_dirent_cookie_t cookie,
                   size_t * entry_count,
                   zoidfs_dirent_t * entries,
                   uint32_t flags,
                   zoidfs_cache_hint_t * parent_hint,
                   zoidfs_op_hint_t * op_hint)
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
                            parent_hint,
                            op_hint);
}

int zoidfs_resize(const zoidfs_handle_t * handle,
                  zoidfs_file_size_t size,
                  zoidfs_op_hint_t * op_hint)
{
    int ret;
    zint_handler_t * handler;

    ret = zint_locate_handler_handle(handle, &handler);
    if(ret < 0)
    {
        return ZFSERR_INVAL;
    }

    return handler->resize(handle,
                           size,
                           op_hint);
}    

/**
 * OPTIONAL
 */
int zoidfs_init(zoidfs_op_hint_t * UNUSED(op_hint))
{
   return zint_initialize_handlers ();
}

/**
 * OPTIONAL
 */
int zoidfs_finalize(zoidfs_op_hint_t * UNUSED(op_hint))
{
   return zint_finalize_handlers (); 
}

/* Only needed by the BGP ZOID version.  */
void __zoidfs_init(int UNUSED(pset_mpi_proc_count), int UNUSED(argc), 
      int UNUSED(envc), const char* UNUSED(argenvint))
{
    zoidfs_init(ZOIDFS_NO_OP_HINT);
}

/* Only needed by the BGP ZOID version.  */
void __zoidfs_finalize(void)
{
    zoidfs_finalize(ZOIDFS_NO_OP_HINT);
}


/*
 * Local variables:
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 *
 * vim: ts=8 sts=4 sw=4 expandtab
 */
