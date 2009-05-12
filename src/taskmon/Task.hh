#ifndef TASKMON_TASK_HH
#define TASKMON_TASK_HH

namespace taskmon
{

class Task
{
public:

   /// Called on completion: takes 
   // void bind (boost::function<bool>  

   /// Wait for completion of this task
   virtual void wait () = 0; 

   /// Test 
   virtual bool test () = 0; 

   /// Cancel task (not always supported)
   virtual bool cancel () = 0; 
   
   /// Disconnect the task; (No longer possible to wait/test)
   virtual void release () = 0; 

   virtual ~Task (); 

protected:

}; 

}

#endif
