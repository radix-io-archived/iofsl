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


  /* Commit */

  ENCODERSTRUCT (CommitRequest, ((zoidfs_handle_t)(handle)))
  ENCODERSTRUCT (CommitResponse, ((int)(returnCode)))

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

