#ifndef EXTRASERVICE_SFP_SFPSERVICE_HH
#define EXTRASERVICE_SFP_SFPSERVICE_HH

#include "iofwd/ExtraService.hh"
#include "rpc/RPCHandler.hh"

#include "iofwdutil/IOFWDLog-fwd.hh"

namespace iofwd
{
   //========================================================================

   class Log;
   class RPCServer;

   namespace extraservice
   {
      //=====================================================================


      class SFPService : public ExtraService
      {
         public:
            SFPService (service::ServiceManager & m);

            virtual void configureNested (const iofwdutil::ConfigFile &);

            virtual ~SFPService ();

/*
            virtual int removeEntry (const std::string & key);

            virtual int createEntry (const std::string & key,
                  zoidfs::zoidfs_file_ofs_t init = 0);

            virtual int modifyEntry (const std::string & key,
                  const char * op, zoidfs::zoidfs_file_ofs_t * val);
*/
         protected:

/*
            void null (iofwdevent::ZeroCopyInputStream *,
                  iofwdevent::ZeroCopyOutputStream *,
                  const rpc::RPCInfo &);

            void echo (iofwdevent::ZeroCopyInputStream *,
                  iofwdevent::ZeroCopyOutputStream *,
                  const rpc::RPCInfo &);
*/
         protected:
            boost::shared_ptr<Log> log_service_;
            boost::shared_ptr<RPCServer> rpcserver_;
            iofwdutil::IOFWDLogSource & log_;
      };

      //=====================================================================
   }

   //========================================================================
}

#endif
