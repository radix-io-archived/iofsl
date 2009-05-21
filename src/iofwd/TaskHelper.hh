#ifndef IOFWD_TASKHELPER_HH
#define IOFWD_TASKHELPER_HH

namespace iofwd
{

template <typename T>
class TaskHelper 
{
   public:
      /**
       * The task takes ownership of the request
       */
      TaskHelper (Request * req)
         : request_ (dynamic_cast<T &> (*req))
      {
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
}; 



}

#endif
