#include <boost/lambda/bind.hpp>
#include <boost/lambda/construct.hpp>
#include <iostream>
#include <vector>
#include <unistd.h>
#include "iofwdutil/tools.hh"
#include "iofwdutil/assert.hh"
#include "iofwdutil/workqueue/WorkQueue.hh"
#include "iofwdutil/workqueue/WorkItem.hh"
#include "iofwdutil/workqueue/SynchronousWorkQueue.hh"
#include "iofwdutil/workqueue/PoolWorkQueue.hh"

using namespace std; 
using namespace boost::lambda; 
using namespace iofwdutil::workqueue; 

class Work : public iofwdutil::workqueue::WorkItem 
{
public:

   Work (unsigned int * dst) : seq_ (dst)
   {
   }

   virtual void doWork ()
   {
      sleep(1); 
      cerr << "Completed " << *seq_ << endl; 
      *seq_ = 0; 
   }

protected:
   unsigned int * seq_; 
};

void testWork (iofwdutil::workqueue::WorkQueue * q)
{
   const unsigned int COUNT = 32; 
   std::vector<unsigned int> v (COUNT); 
   std::vector<WorkItem *> items; 

   for (unsigned int i=0; i<v.size(); ++i)
   {
      v[i] = i; 
      q->queueWork (new Work (&v[i])); 
   }

   q->waitAll (items); 

   std::for_each (items.begin(), items.end(), boost::lambda::bind(delete_ptr(), boost::lambda::_1)); 

   ASSERT(items.size() == COUNT); 

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
   iofwdutil::workqueue::PoolWorkQueue pq (2, 10); 
   testWork (&pq); 
   testWork (&sq); 
}
