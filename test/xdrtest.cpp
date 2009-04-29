#include <iostream>
#include <rpc/xdr.h>

using namespace std; 



int main (int argc, char ** args)
{
   const char * s = "/test"; 

   XDR xdr; 
   char buf[256];
   xdrmem_create (&xdr, &buf[0], sizeof(buf), XDR_ENCODE);
   xdr_string (&xdr, (char **) &s, 128); 

   return EXIT_SUCCESS; 
}
