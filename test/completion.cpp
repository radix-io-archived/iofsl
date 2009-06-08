#include <iostream>
#include <vector>
#include <boost/lambda/lambda.hpp>
#include "iofwdutil/completion/CompletionID.hh"
#include "iofwdutil/completion/TimerResource.hh"
#include "iofwdutil/tools.hh"

using namespace std;
using namespace iofwdutil::completion; 
using namespace boost; 

int main (int UNUSED(argc), char ** UNUSED(args))
{
   ContextBase ctx; 
//   TimerResource timer (ctx); 

   std::vector<CompletionID> lst; 

   /*&while (!ctx.done ())
   {
      lst.clear (); 
      ctx.waitAny (back_inserter (lst)); 
      foreach (lst, cout << "Operation " << _1 << " completed!\n"); 
   } */ 
   return 0; 
}
