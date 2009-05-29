#ifndef IOFWD_TASKHELPER_HH
#define IOFWD_TASKHELPER_HH

namespace zoidfs
{
   class ZoidFSAPI;
}

namespace iofwd
{

template <typename T>
class TaskHelper 
{
   public:
      /**
       * The task takes ownership of the request
       */
      TaskHelper (Request * req, zoidfs::ZoidFSAPI * api)
         : request_ (static_cast<T &> (*req)), 
           api_ (api)
      {
#ifndef NDEBUG
         dynamic_cast<T &> (*req); 
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

      zoidfs::ZoidFSAPI * api_; 
}; 



}

#endif
