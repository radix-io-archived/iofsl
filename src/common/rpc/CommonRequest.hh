#ifndef COMMON_COMMONREQUEST
#define COMMON_COMMONREQUEST
#include "encoder/EncoderString.hh"
#include "encoder/EncoderStruct.hh"
#include "zoidfs/util/ZoidFSFileSpec.hh"
#include "zoidfs/util/ZoidfsFileOfsStruct.hh"

namespace common
{
  //typedef encoder::EncoderString<0, ZOIDFS_PATH_MAX> EncoderString;
  typedef zoidfs::ZoidFSFileSpec ZoidFSFileSpec;
  typedef zoidfs::zoidfs_handle_t zoidfs_handle_t;
  typedef zoidfs::ZoidfsFileOfsStruct ZoidfsFileOfsStruct;
  typedef zoidfs::zoidfs_sattr_t zoidfs_sattr_t;
  typedef zoidfs::zoidfs_attr_t zoidfs_attr_t;
  typedef zoidfs::zoidfs_cache_hint_t zoidfs_cache_hint_t;

  /* Commit */

  ENCODERSTRUCT (CommitRequest, ((zoidfs_handle_t)(handle)))
  ENCODERSTRUCT (CommitResponse, ((int)(returnCode)))

  /* Create */
  
  ENCODERSTRUCT (CreateRequest, ((ZoidFSFileSpec)(info))            
                                ((zoidfs_sattr_t)(attr)))

  ENCODERSTRUCT (CreateResponse, ((int)(returnCode))
                                 ((zoidfs_handle_t)(handle))
                                 ((int)(created)))

  /* GetAttr */

  ENCODERSTRUCT (GetAttributeRequest, ((zoidfs_handle_t)(handle))
                                 ((zoidfs_attr_t)(attr)))
  ENCODERSTRUCT (GetAttributeResponse, ((int)(returnCode))
                                  ((zoidfs_attr_t)(attr_enc)))


  /* Link Request */

  ENCODERSTRUCT (RPCLinkRequest, ((ZoidFSFileSpec)(from))
                                 ((ZoidFSFileSpec)(to)))

  ENCODERSTRUCT (RPCLinkResponse, ((int)(returnCode))
                                  ((zoidfs_cache_hint_t)(from_parent_hint))
                                  ((zoidfs_cache_hint_t)(to_parent_hint)))

  /* Lookup */

  ENCODERSTRUCT(LookupRequest,  ((ZoidFSFileSpec)(info)))
  ENCODERSTRUCT(LookupResponse, ((int)(returnCode))
                                ((zoidfs_handle_t)(handle)))

  /* Write */  

  ENCODERSTRUCT(WriteRequest, ((zoidfs_handle_t)(handle))
                              ((ZoidfsFileOfsStruct)(file)))
  ENCODERSTRUCT(WriteResponse, ((int)(returnCode)))

  /* Read */

  ENCODERSTRUCT(ReadRequest, ((zoidfs_handle_t)(handle))
                             ((ZoidfsFileOfsStruct)(file)))
  ENCODERSTRUCT(ReadResponse, ((int)(returnCode)))



}



#endif 

