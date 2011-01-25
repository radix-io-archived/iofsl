#ifndef IOFWD_EXTRASERVICE_FTB_FTBEXCEPTION_HH
#define IOFWD_EXTRASERVICE_FTB_FTBEXCEPTION_HH

#include "iofwdutil/ZException.hh"

namespace iofwd
{
   //========================================================================

   struct FTBError : public iofwdutil::ZException {};

   typedef boost::error_info<struct tag_ftb_errorcode,int> ftb_errorcode;

   // How to display this error tag
   std::string to_string (const ftb_errorcode & e);

   // Convert FTB error to string (if possible)
   std::string getFTBErrorString (int ret);

   //========================================================================
}

#endif
