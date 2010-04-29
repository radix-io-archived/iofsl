#ifndef IOFWD_TASKHELPER_HH
#define IOFWD_TASKHELPER_HH

#include "Task.hh"
#include "iofwdevent/SingleCompletion.hh"
#include "zoidfs/util/ZoidFSAsync.hh"


namespace iofwd
{

/**
 * This class was originally introduced to reduce the number of parameters
 * that needed to be passed to each constructed Task. However, over time that
 * number has gone down so this helper class is less helpful.
 *
 * It still saves on typing since any parameter that needs to be passed to the
 * TaskHelper can be stored here, avoiding the need to change all the
 * constructors of the thread tasks.
 */
class ThreadTaskParam
{
public:
   Request   *                          req;
   zoidfs::util::ZoidFSAsync *                  api;

   ThreadTaskParam (Request * r, 
      zoidfs::util::ZoidFSAsync * a1)
      : req(r), api(a1)
   {
   }

} ; 


/**
 * Helper class for Threaded tasks.
 *
 * This is an easy way to inject a member/parameter in each thread task.
 */
template <typename T>
class TaskHelper : public Task
{
   public:
      /**
       * The task takes ownership of the request
       */
      TaskHelper (ThreadTaskParam & param)
         : request_ (static_cast<T &> (*param.req)), 
           api_ (param.api)
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

      zoidfs::util::ZoidFSAsync * api_;

      // All (almost?) threaded tasks call a blocking function at some point.
      // Declaring it here saves on typing.
      iofwdevent::SingleCompletion block_;
};


}

#endif
