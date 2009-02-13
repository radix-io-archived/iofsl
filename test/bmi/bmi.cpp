#include <iostream>
#include <boost/thread.hpp>
#include "iofwdutil/tools.hh"
#include "iofwdutil/bmi/BMI.hh"

using namespace std; 
using namespace iofwdutil::bmi; 
using namespace boost; 

class SenderThread 
{
public: 
   bool operator ()  ()
   {
      cerr << "Hello from senderthread!\n"; 
      return false; 
   }

}; 

class ReceiverThread : public SenderThread
{
public:
   bool operator () ()
   {
      cerr << "Hello from receiverthread\n"; 
      return false; 
   }
}; 

int main (int UNUSED(argc), char ** UNUSED(args))
{
   BMI::setInitParams (0, 0, 0); 

   //BMI & bmi = BMI::get(); 
   

   SenderThread sender;
   ReceiverThread receiver; 

   // Create send and receive thread
   boost::thread_group threads;

   threads.create_thread (sender);
   threads.create_thread (receiver); 

   threads.join_all (); 
   return EXIT_SUCCESS; 
}
