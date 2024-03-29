#ifndef IOFWD_NOTIMPLEMENTEDTASK_HH
#define IOFWD_NOTIMPLEMENTEDTASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "NotImplementedRequest.hh"
#include "TaskHelper.hh"
#include "iofwdutil/InjectPool.hh"

namespace iofwd
{
//===========================================================================


class NotImplementedTask :
   public TaskHelper<NotImplementedRequest>, public iofwdutil::InjectPool<NotImplementedTask>
{
public:
   NotImplementedTask (ThreadTaskParam & p)
      : TaskHelper<NotImplementedRequest>(p)
   {
   }

   /// Not implemented is a fast request. No need to schedule it
   bool isFast () const
   {
      return true;
   }

   void run ()
   {
      // We don't need to set the errorcode, NotImplementedRequest by default
      // returns ZFS_NOTIMPLEMENTED
      request_.reply ((block_));
   }
};

//===========================================================================
}

#endif
