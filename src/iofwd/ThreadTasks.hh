#ifndef IOFWD_THREADTASKS_HH
#define IOFWD_THREADTASKS_HH

#include <boost/function.hpp>

namespace zoidfs
{
   namespace util
   {
   class ZoidFSAsync;
   }
}

namespace iofwd
{
//===========================================================================

class Task;
class Request; 

/**
 * Task factory that generates task which block until complete.
 */
class ThreadTasks
{
public:

   ThreadTasks (
         zoidfs::util::ZoidFSAsync * api)
      : api_(api)
   {
   }

   Task * operator () (Request * req);

protected:
   zoidfs::util::ZoidFSAsync * api_;
};

//===========================================================================
}

#endif
