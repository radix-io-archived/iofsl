#ifndef IOFWDCLIENT_STREAMWRAPPERS_CREATESTREAM_HH
#define IOFWDCLIENT_STREAMWRAPPERS_CREATESTREAM_HH
#include "zoidfs/zoidfs.h"
#include "zoidfs/util/zoidfs-xdr.hh"
#include "zoidfs/util/OpHintHelper.hh"
#include "iofwdutil/tools.hh"
#include "encoder/Util.hh"
#include "encoder/EncoderString.hh"
#include "iofwdclient/streamwrappers/StreamMacro.hh"

typedef zoidfs::zoidfs_handle_t zoidfs_handle_t;
typedef zoidfs::zoidfs_sattr_t zoidfs_sattr_t;
typedef zoidfs::zoidfs_op_hint_t zoidfs_op_hint_t;
typedef encoder::EncoderString<0, ZOIDFS_PATH_MAX> EncoderString;
CLIENT_GENSTREAMWRAP (CreateStream, CreateInStream, ((zoidfs_handle_t *)(handle))
                                                    ((int)(created)),
                                                    ((*)(handle_))
                                                    (()(created_)),
                                                    CreateOutStream,
                                                    ((zoidfs_handle_t *)(handle)) 
                                                    ((EncoderString *)(full_path))
                                                    ((EncoderString *)(component_name))              
                                                    ((zoidfs_sattr_t *)(attr)),
                                                    ((*)(handle_)) 
                                                    ((*)(full_path_))
                                                    ((*)(component_name_))              
                                                    ((*)(attr_)))

#endif
