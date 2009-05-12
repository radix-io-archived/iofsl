#include <iostream>
#include <vector>
#include "iofwdutil/tools.hh"
#include "iofwdutil/assert.hh"
#include "iofwdutil/workqueue/WorkQueue.hh"
#include "iofwdutil/workqueue/WorkItem.hh"
#include "iofwdutil/workqueue/SynchronousWorkQueue.hh"
#include "taskmon/TaskMonitor.hh"

using namespace std; 

class Work : public iofwdutil::workqueue::WorkItem 
{
public:

   Work (unsigned int * dst) : seq_ (dst)
   {
   }

   virtual void doWork ()
   {
      cerr << "Completed " << *seq_ << endl; 
      *seq_ = 0; 
   }

protected:
   unsigned int * seq_; 
};

void testWork (iofwdutil::workqueue::WorkQueue * q)
{
   std::vector<unsigned int> v (1024); 

   for (unsigned int i=0; i<v.size(); ++i)
   {
      v[i] = i; 
      q->queueWork (new Work (&v[i])); 
   }

   q->waitAll (); 

   for (unsigned int i=0; i<v.size(); ++i)
   {
      if (v[i] != 0)
      {
         ASSERT (false && "Not all work completed!"); 
      }
   }
}

int main (int UNUSED(argc), char ** UNUSED(args))
{
   cout << "Testing synchronous work queue..." << endl; 
   iofwdutil::workqueue::SynchronousWorkQueue sq; 
   testWork (&sq); 
}
