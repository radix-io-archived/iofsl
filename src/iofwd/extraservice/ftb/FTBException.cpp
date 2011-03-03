#include "FTBException.hh"

#include <boost/format.hpp>
#include <ftb.h>

namespace iofwd
{
   //========================================================================

   std::string getFTBErrorString (int ret)
   {
      switch (ret)
      {
#define ERRORDEF(a) case a: return std::string(#a)
ERRORDEF(FTB_SUCCESS);
ERRORDEF(FTB_ERR_GENERAL);
ERRORDEF(FTB_ERR_EVENTSPACE_FORMAT);
ERRORDEF(FTB_ERR_SUBSCRIPTION_STYLE);
ERRORDEF(FTB_ERR_INVALID_VALUE);
ERRORDEF(FTB_ERR_DUP_CALL);
ERRORDEF(FTB_ERR_NULL_POINTER);
ERRORDEF(FTB_ERR_NOT_SUPPORTED);
ERRORDEF(FTB_ERR_INVALID_FIELD);
ERRORDEF(FTB_ERR_INVALID_HANDLE);
ERRORDEF(FTB_ERR_DUP_EVENT);
ERRORDEF(FTB_ERR_INVALID_SCHEMA_FILE);
ERRORDEF(FTB_ERR_INVALID_EVENT_NAME);
ERRORDEF(FTB_ERR_INVALID_EVENT_TYPE);
ERRORDEF(FTB_ERR_SUBSCRIPTION_STR);
ERRORDEF(FTB_ERR_FILTER_ATTR);
ERRORDEF(FTB_ERR_FILTER_VALUE);
ERRORDEF(FTB_GOT_NO_EVENT);
ERRORDEF(FTB_FAILURE);
ERRORDEF(FTB_ERR_INVALID_PARAMETER);
ERRORDEF(FTB_ERR_NETWORK_GENERAL);
ERRORDEF(FTB_ERR_NETWORK_NO_ROUTE);
#undef ERRORDEF
         default:
            return str(boost::format("Unknown error (%i)")
                         % ret);
      }
   }

   /**
    * Convert ftb_errorcode tag to human readable text.
    */
   std::string to_string (const ftb_errorcode & e)
   {
      return str(boost::format("FTB error: %s")
            % getFTBErrorString(e.value ()));
   }

   /**
    * Convert FTB exception to user error
    */
   std::string getFTBErrorString (const FTBError & e)
   {
      const int * err = iofwdutil::zexception_info <ftb_errorcode> (e);
      if (!err)
         return "(no error information)";
      else
         return std::string (getFTBErrorString (*err));
   }



   //========================================================================
}
