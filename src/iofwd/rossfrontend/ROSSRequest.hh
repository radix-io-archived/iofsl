#ifndef ROSS_ROSSFRONTEND_ROSSREQUEST_HH
#define ROSS_ROSSFRONTEND_ROSSREQUEST_HH

#include "zoidfs/util/FileSpecHelper.hh"
#include "zoidfs/util/OpHintHelper.hh"

#include "zoidfs/util/zoidfs-wrapped.hh"

#include "iofwd/Request.hh"
#include "iofwdutil/typestorage.hh"

namespace iofwd
{
   namespace rossfrontend
   {

class ROSSRequest
{
public:
   ROSSRequest();

   virtual ~ROSSRequest();

protected:

   typedef struct
   {
      zoidfs::zoidfs_handle_t parent_handle;
      char full_path[ZOIDFS_PATH_MAX];
      char component_name[ZOIDFS_NAME_MAX];
   } FileInfo;

};

   }
}

#endif
