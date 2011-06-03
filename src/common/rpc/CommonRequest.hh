#ifndef COMMON_COMMONREQUEST
#define COMMON_COMMONREQUEST
#include "encoder/EncoderString.hh"
#include "encoder/EncoderStruct.hh"
#include "zoidfs/util/ZoidFSFileSpec.hh"
#include "zoidfs/util/ZoidfsFileOfsStruct.hh"
#include "zoidfs/util/EncodeDirentT.hh"
#include "zoidfs/util/Hint.hh"
namespace common
{
  typedef encoder::EncoderString<0, ZOIDFS_PATH_MAX> EncoderString;
  typedef zoidfs::ZoidFSFileSpec ZoidFSFileSpec;
  typedef zoidfs::zoidfs_handle_t zoidfs_handle_t;
  typedef zoidfs::ZoidfsFileOfsStruct ZoidfsFileOfsStruct;
  typedef zoidfs::zoidfs_sattr_t zoidfs_sattr_t;
  typedef zoidfs::zoidfs_attr_t zoidfs_attr_t;
  typedef zoidfs::zoidfs_cache_hint_t zoidfs_cache_hint_t;
  typedef zoidfs::zoidfs_dirent_cookie_t zoidfs_dirent_cookie_t;
  typedef zoidfs::zoidfs_dirent_t zoidfs_dirent_t;
  typedef zoidfs::EncodeDirentT EncodeDirentT;
  /* Commit */

  ENCODERSTRUCT (RPCCommitRequest, ((zoidfs_handle_t)(handle)))
  ENCODERSTRUCT (RPCCommitResponse, ((int)(returnCode)))

  /* Create */
  
  ENCODERSTRUCT (RPCCreateRequest, ((ZoidFSFileSpec)(info))            
                                ((zoidfs_sattr_t)(attr)))

  ENCODERSTRUCT (RPCCreateResponse, ((int)(returnCode))
                                 ((zoidfs_handle_t)(handle))
                                 ((int)(created)))

  /* GetAttr */

  ENCODERSTRUCT (RPCGetAttrRequest, ((zoidfs_handle_t)(handle))
                                 ((zoidfs_attr_t)(attr)))
  ENCODERSTRUCT (RPCGetAttrResponse, ((int)(returnCode))
                                  ((zoidfs_attr_t)(attr_enc)))

  /* Link Request */

  ENCODERSTRUCT (RPCLinkRequest, ((ZoidFSFileSpec)(from))
                                 ((ZoidFSFileSpec)(to)))

  ENCODERSTRUCT (RPCLinkResponse, ((int)(returnCode))
                                  ((zoidfs_cache_hint_t)(from_parent_hint))
                                  ((zoidfs_cache_hint_t)(to_parent_hint)))

  /* mkdir request */

  ENCODERSTRUCT (RPCMkdirRequest, ((ZoidFSFileSpec)(dir))
                                  ((zoidfs_sattr_t)(sattr)))

  ENCODERSTRUCT (RPCMkdirResponse, ((int)(returnCode))
                                   ((zoidfs_cache_hint_t)(parent_hint)))

  /* Lookup */

  ENCODERSTRUCT(RPCLookupRequest,  ((ZoidFSFileSpec)(info)))
  ENCODERSTRUCT(RPCLookupResponse, ((int)(returnCode))
                                ((zoidfs_handle_t)(handle)))


  /* Read Dir */

  ENCODERSTRUCT (RPCReadDirRequest, ((zoidfs_handle_t)(handle)) 
                                    ((zoidfs_dirent_cookie_t)(cookie))
                                    ((uint32_t)(entry_count))              
                                    ((uint32_t)(flags)))
  ENCODERSTRUCT (RPCReadDirResponse, ((int)(returnCode))
                                     ((EncodeDirentT)(entries))
                                     ((zoidfs_cache_hint_t)(cache)))


  /* Read Link */

  ENCODERSTRUCT (RPCReadLinkRequest,  ((zoidfs_handle_t)(handle))
                                      ((size_t)(buffer_length)))

  ENCODERSTRUCT (RPCReadLinkResponse, ((int)(returnCode))
                                      ((EncoderString)(buffer)))

  /* Remove Request */

  ENCODERSTRUCT (RPCRemoveRequest, ((ZoidFSFileSpec)(info)))

  ENCODERSTRUCT (RPCRemoveResponse, ((int)(returnCode))
                                    ((zoidfs_cache_hint_t)(parent_hint)))

  /* Rename Request */

  ENCODERSTRUCT (RPCRenameRequest, ((ZoidFSFileSpec)(from))
                                   ((ZoidFSFileSpec)(to)))

  ENCODERSTRUCT (RPCRenameResponse, ((int)(returnCode))
                                    ((zoidfs_cache_hint_t)(from_parent_hint))
                                    ((zoidfs_cache_hint_t)(to_parent_hint)))
  
  /* Set Attr */
  ENCODERSTRUCT (RPCSetAttrRequest,  ((zoidfs_handle_t)(handle))
                                     ((zoidfs_attr_t)(attr))
                                     ((zoidfs_sattr_t)(sattr)))
  ENCODERSTRUCT (RPCSetAttrResponse, ((int)(returnCode))
                                     ((zoidfs_attr_t)(attr_enc)))


  /* Sym link request */

  ENCODERSTRUCT (RPCSymlinkRequest, ((ZoidFSFileSpec)(from))
                                    ((zoidfs_sattr_t)(sattr))
                                    ((ZoidFSFileSpec)(to)))

  ENCODERSTRUCT (RPCSymlinkResponse, ((int)(returnCode))
                                     ((zoidfs_cache_hint_t)(from_parent_hint))
                                     ((zoidfs_cache_hint_t)(to_parent_hint)))


  /* Write */  

  ENCODERSTRUCT(RPCWriteRequest, ((zoidfs_handle_t)(handle))
                              ((ZoidfsFileOfsStruct)(file)))
  ENCODERSTRUCT(RPCWriteResponse, ((int)(returnCode)))

  /* Read */

  ENCODERSTRUCT(RPCReadRequest, ((zoidfs_handle_t)(handle))
                             ((ZoidfsFileOfsStruct)(file)))
  ENCODERSTRUCT(RPCReadResponse, ((int)(returnCode)))



}



#endif 

