#ifndef IOFWDUTIL_XDR_XDRPROCESSOR_HH
#define IOFWDUTIL_XDR_XDRPROCESSOR_HH

namespace iofwdutil
{
   namespace xdr
   {
//===========================================================================


      /**
       * Base class for XDR processing classes
       */
      class XDRProcessor
      {
         public:
            enum { READER = 1, 
                   WRITER , 
                   SIZE     
                 }; 
      }; 


//===========================================================================
   }
}

#endif
