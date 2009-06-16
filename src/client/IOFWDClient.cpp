#include "IOFWDClient.hh"
#include "zoidfs/util/zoidfs-xdr.hh"
#include "zoidfs/util/FileSpecHelper.hh"
#include "iofwdutil/xdr/XDRWrappers.hh"
#include "iofwdutil/typestorage.hh"

using namespace iofwdutil::bmi; 
using namespace iofwdutil::xdr; 
using namespace zoidfs; 


namespace client
{
//===========================================================================


IOFWDClient::IOFWDClient (const char * iofwdhost)
   : comm_(BMI::get().openContext(), BMIAddr (iofwdhost))
{

}
//===========================================================================

int IOFWDClient::init ()
{
   return 0; 
}

int IOFWDClient::finalize ()
{
   return 0; 
}

int IOFWDClient::null ()
{
   return comm_.genericOp (ZOIDFS_PROTO_NULL, 
         TSSTART,
         TSSTART); 
}

int IOFWDClient::lookup (const zoidfs::zoidfs_handle_t * handle,
      const char * component, const char * full, zoidfs::zoidfs_handle_t * dhandle)
{
   return comm_.genericOp (ZOIDFS_PROTO_LOOKUP, 
         TSSTART << FileSpecHelper(handle, component, full), 
         TSSTART << *dhandle); 
}


/** 
 * NOTE: 
 *   - getattr acts as lstat, i.e. it does not follow symbolic links 
 *   - attr.mask indicates which values need to be retrieved.
 *     Other fields are ignored on input.
 */
int IOFWDClient::getattr(const zoidfs::zoidfs_handle_t * handle /* in:ptr */,
                   zoidfs::zoidfs_attr_t * attr /* inout:ptr */)
{
   return comm_.genericOp (ZOIDFS_PROTO_GET_ATTR, 
         TSSTART << *handle << *attr, 
         TSSTART << *attr); 
}

/*
 * NOTE: 
 *   - On input, only the mask field of attr is relevant.
 *   - if sattr.mode and attr.mode contain shared bits,
 *     attr will retrieve the modified values.
 */
int IOFWDClient::setattr(const zoidfs::zoidfs_handle_t * handle /* in:ptr */,
                   const zoidfs_sattr_t * sattr /* in:ptr */,
                   zoidfs_attr_t * attr /* inout:ptr:nullok */)
{
   return comm_.genericOp (ZOIDFS_PROTO_GET_ATTR, 
         TSSTART << *handle << *sattr << *attr,
         TSSTART << *attr); 
}

int IOFWDClient::readlink(const zoidfs_handle_t * handle /* in:ptr */,
                    char * buffer /* out:arr:size=+1 */,
                    size_t buffer_length /* in:obj */)
{
   return comm_.genericOp (ZOIDFS_PROTO_READLINK,
         TSSTART << *handle << buffer_length,
         TSSTART << XDRString (buffer, buffer_length)); 
}

int IOFWDClient::read(const zoidfs_handle_t * UNUSED(handle) /* in:ptr */,
                size_t UNUSED(mem_count) /* in:obj */,
                void * UNUSED(mem_starts[]) /* out:arr2d:size=+1:zerocopy */,
                const size_t UNUSED(mem_sizes[]) /* in:arr:size=-2 */,
                size_t UNUSED(file_count) /* in:obj */,
                const uint64_t UNUSED(file_starts[]) /* in:arr:size=-1 */,
                uint64_t UNUSED(file_sizes[]) /* inout:arr:size=-2 */)
{
   /* TODO */ 
   return 0; 
}

int IOFWDClient::write(const zoidfs_handle_t * handle /* in:ptr */,
                 size_t mem_count /* in:obj */,
                 const void * mem_starts[] /* in:arr2d:size=+1:zerocopy */,
                 const size_t mem_sizes[] /* in:arr:size=-2 */,
                 size_t file_count /* in:obj */,
                 const uint64_t file_starts[] /* in:arr:size=-1 */,
                 uint64_t file_sizes[] /* inout:arr:size=-2 */)
{
  const int64_t * mem_sizes_ = NULL;//(const int64_t*)mem_sizes;
  int64_t mem_count_ = 0;//(int64_t)mem_count;
   return comm_.writeOp (ZOIDFS_PROTO_WRITE,
                         TSSTART << *handle << XDRVarArray(mem_sizes_, mem_count_),
                         TSSTART,
                         mem_starts, mem_sizes, mem_count);
}

int IOFWDClient::commit(const zoidfs_handle_t * handle /* in:ptr */)
{
   return comm_.genericOp (ZOIDFS_PROTO_COMMIT, 
         TSSTART << *handle, 
         TSSTART); 
}

/**
 * NOTE: if the file already exists zoidfs_create will lookup the handle 
 * and return success but set created to 0
 */
int IOFWDClient::create(const zoidfs_handle_t * parent_handle /* in:ptr:nullok */,
                  const char * component_name /* in:str:nullok */,
                  const char * full_path /* in:str:nullok */,
                  const zoidfs_sattr_t * attr /* in:ptr */,
                  zoidfs_handle_t * handle /* out:ptr */,
                  int * created /* out:ptr:nullok */)
{
   return comm_.genericOp (ZOIDFS_PROTO_CREATE, 
         TSSTART << FileSpecHelper (parent_handle, component_name, full_path)
                 << *attr, 
         TSSTART << *handle << *created); 
}

int IOFWDClient::remove(const zoidfs_handle_t * parent_handle /* in:ptr:nullok */,
                  const char * component_name /* in:str:nullok */,
                  const char * full_path /* in:str:nullok */,
                  zoidfs_cache_hint_t * parent_hint /* out:ptr:nullok */)
{
   return comm_.genericOp (ZOIDFS_PROTO_REMOVE, 
         TSSTART << FileSpecHelper (parent_handle, component_name, full_path),
         TSSTART << *parent_hint); 
}

int IOFWDClient::rename(const zoidfs_handle_t * from_parent_handle /* in:ptr:nullok */,
                  const char * from_component_name /* in:str:nullok */,
                  const char * from_full_path /* in:str:nullok */,
                  const zoidfs_handle_t * to_parent_handle /* in:ptr:nullok */,
                  const char * to_component_name /* in:str:nullok */,
                  const char * to_full_path /* in:str:nullok */,
                  zoidfs_cache_hint_t * from_parent_hint /* out:ptr:nullok */,
                  zoidfs_cache_hint_t * to_parent_hint /* out:ptr:nullok */)
{
   return comm_.genericOp (ZOIDFS_PROTO_RENAME, 
         TSSTART << FileSpecHelper (from_parent_handle, from_component_name,
                       from_full_path)
                 << FileSpecHelper (to_parent_handle, to_component_name,
                       to_full_path),
          TSSTART << *from_parent_hint << *to_parent_hint); 
}

int IOFWDClient::link(const zoidfs_handle_t * from_parent_handle /* in:ptr:nullok */,
                const char * from_component_name /* in:str:nullok */,
                const char * from_full_path /* in:str:nullok */,
                const zoidfs_handle_t * to_parent_handle /* in:ptr:nullok */,
                const char * to_component_name /* in:str:nullok */,
                const char * to_full_path /* in:str:nullok */,
                zoidfs_cache_hint_t * from_parent_hint /* out:ptr:nullok */,
                zoidfs_cache_hint_t * to_parent_hint /* out:ptr:nullok */)
{
   return comm_.genericOp (ZOIDFS_PROTO_LINK, 
         TSSTART << FileSpecHelper (from_parent_handle, from_component_name, 
                      from_full_path)
                 << FileSpecHelper (to_parent_handle, to_component_name, 
                      to_full_path), 
         TSSTART << *from_parent_hint << *to_parent_hint); 
}


int IOFWDClient::symlink(const zoidfs_handle_t * from_parent_handle /* in:ptr:nullok */,
                   const char * from_component_name /* in:str:nullok */,
                   const char * from_full_path /* in:str:nullok */,
                   const zoidfs_handle_t * to_parent_handle /* in:ptr:nullok */,
                   const char * to_component_name /* in:str:nullok */,
                   const char * to_full_path /* in:str:nullok */,
                   const zoidfs_sattr_t * attr /* in:ptr */,
                   zoidfs_cache_hint_t * from_parent_hint /* out:ptr:nullok */,
                   zoidfs_cache_hint_t * to_parent_hint /* out:ptr:nullok */)
{
  return comm_.genericOp (ZOIDFS_PROTO_SYMLINK, 
         TSSTART << FileSpecHelper (from_parent_handle, from_component_name, 
                      from_full_path)
                 << FileSpecHelper (to_parent_handle, to_component_name, 
                      to_full_path) 
                 << *attr, 
         TSSTART << *from_parent_hint << *to_parent_hint); }

int IOFWDClient::mkdir(const zoidfs_handle_t * parent_handle /* in:ptr:nullok */,
                 const char * component_name /* in:str:nullok */,
                 const char * full_path /* in:str:nullok */,
                 const zoidfs_sattr_t * attr /* in:ptr */,
                 zoidfs_cache_hint_t * parent_hint /* out:ptr:nullok */)
{
   return comm_.genericOp (ZOIDFS_PROTO_MKDIR, 
         TSSTART << FileSpecHelper (parent_handle, component_name, full_path)
                 << *attr,
         TSSTART << *parent_hint); 
}

int IOFWDClient::readdir(const zoidfs_handle_t * parent_handle /* in:ptr */,
                   zoidfs_dirent_cookie_t cookie /* in:obj */,
                   size_t * entry_count /* inout:ptr */,
                   zoidfs_dirent_t * entries /* out:arr:size=-1 */,
                   uint32_t flags /* in:obj */,
                   zoidfs_cache_hint_t * parent_hint /* out:ptr:nullok */)
{
   /* For the reply, we expect an integer listing the number of array entries
    * that will follow: in other words, we can receive in a XDRVarArray */ 
   return comm_.genericOp (ZOIDFS_PROTO_READDIR,
         TSSTART << *parent_handle << cookie << *entry_count << flags,
         TSSTART << *entry_count 
                 << XDRVarArray(entries,*entry_count) <<  *parent_hint); 
}

int IOFWDClient::resize(const zoidfs_handle_t * handle /* in:ptr */,
                  uint64_t size /* in:obj */)
{
   return comm_.genericOp (ZOIDFS_PROTO_RESIZE, 
         TSSTART << *handle << size,
         TSSTART); 
}



IOFWDClient::~IOFWDClient ()
{
}

//===========================================================================
}



