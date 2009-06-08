#ifndef IOFWD_TASKHELPER_HH
#define IOFWD_TASKHELPER_HH

#include "Task.hh"
#include "iofwdutil/completion/BMIResource.hh"

namespace zoidfs
{
   class ZoidFSAPI;
}

namespace iofwd
{

class ThreadTaskParam
{
public:
   Request   *                          req; 
   iofwdutil::completion::BMIResource & bmi; 
   zoidfs::ZoidFSAPI *                  api; 
   boost::function<void (Task*)>        resched; 

   ThreadTaskParam (Request * r, 
         boost::function<void (Task*)> r2,zoidfs::ZoidFSAPI * a, iofwdutil::completion::BMIResource & b) 
      : req(r), bmi(b), api(a), resched(r2)
   {
   }

} ; 

template <typename T>

/**
 * Helper class for Threaded tasks.
 */
class TaskHelper : public Task
{
   public:
      /**
       * The task takes ownership of the request
       */
      TaskHelper (ThreadTaskParam & param)
         : Task (param.resched), request_ (static_cast<T &> (*param.req)), 
           api_ (param.api), bmi_ (param.bmi)
      {
#ifndef NDEBUG
         // This will throw if the request is not of the expected type
         dynamic_cast<T &> (*param.req); 
#endif
      }

      T * getRequest ()
      { return &request_; }

      ~TaskHelper ()
      {
         // The task owns the request and needs to destroy it
         delete (&request_); 
      }

   protected:
      T & request_; 

      zoidfs::ZoidFSAPI *                  api_; 
      iofwdutil::completion::BMIResource & bmi_; 
}; 



}

#endif
