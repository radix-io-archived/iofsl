#include <typeinfo>
#include <iostream>
#include "iofwdutil/tools.hh"
#include "iofwdutil/typestorage.hh"

using namespace std; 

//===========================================================================

//===========================================================================

template <typename T>
const char * getName ()
{
   return typeid(T).name(); 
}

class Proc1
{
public:
   template <typename T>
   void operator () (const T & val)
   {
      cout << "Proc1 << " << getName<T>() << "(" << val << ") " << endl; 
   }
}; 

class Proc2
{
public:
   template <typename T>
   void operator () (const T & val)
   {
      cout << "Proc2 << " << getName<T>() << "(" << val << ") " << endl; 
   }
}; 


int main (int UNUSED(argc), char ** UNUSED(args))
{
   int a;
   double b; 

   Proc1 p1; 
   Proc2 p2; 

   TYPESTORAGE(types,a << b << a << b); 
   APPLYTYPES(p1, types); 
   APPLYTYPES(p2, types); 
}
