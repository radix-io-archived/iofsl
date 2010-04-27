#ifndef IOFWD_THREADTASKS_HH
#define IOFWD_THREADTASKS_HH

#include <boost/function.hpp>
#include "iofwdutil/completion/BMIResource.hh"
#include "iofwd/BMIMemoryManager.hh"

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
class RequestScheduler;

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

   // @TODO: this probably needs to be removed.
   iofwdutil::completion::ContextBase ctx_; 
   iofwdutil::completion::BMIResource bmi_; 
}; 

//===========================================================================
}

#endif
