#include <iostream>
#include <boost/lambda/lambda.hpp>
#include "iofwdutil/completion/CompletionID.hh"
#include "iofwdutil/completion/TimerResource.hh"

using namespace std;
using namespace iofwdutil::completion; 
using namespace boost; 

int main (int argc, char ** args)
{
   ContextBase ctx; 
   TimerResource timer (ctx); 

   std::vector<CompletionID> lst; 

   while (!ctx.done ())
   {
      lst.clear (); 
      ctx.waitAny (back_inserter (lst)); 
      foreach (lst, cout << "Operation " << _1 << " completed!\n"); 
   }
   return 0; 
}
