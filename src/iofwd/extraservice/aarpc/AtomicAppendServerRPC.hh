#ifndef EXTRASERVICE_AARPC_ATOMICAPPENDSERVERRPC_HH
#define EXTRASERVICE_AARPC_ATOMICAPPENDSERVERRPC_HH

#include "iofwd/ExtraService.hh"
#include "rpc/RPCHandler.hh"

#include "iofwdutil/IOFWDLog-fwd.hh"

#include "zoidfs/zoidfs.h"

namespace iofwd
{
   class Log;
   class RPCServer;

   namespace extraservice
   {
      class AtomicAppendServerRPC : public ExtraService
      {
         public:
            AtomicAppendServerRPC(service::ServiceManager & m);

            virtual void configureNested(const iofwdutil::ConfigFile & f)
            {
                aarpc_master_addr_ =
                    f.getKeyAsDefault<std::string>("master",""); 
            }

            virtual ~AtomicAppendServerRPC();

            static std::string aarpc_master_addr_;

         protected:

            /* rpc handlers */

            void getNextOffset(iofwdevent::ZeroCopyInputStream *,
                  iofwdevent::ZeroCopyOutputStream *,
                  const rpc::RPCInfo &);

            void createOffset(iofwdevent::ZeroCopyInputStream *,
                  iofwdevent::ZeroCopyOutputStream *,
                  const rpc::RPCInfo &);

            void deleteOffset(iofwdevent::ZeroCopyInputStream *,
                  iofwdevent::ZeroCopyOutputStream *,
                  const rpc::RPCInfo &);

            /* key value storage helpers */

            uint64_t getNextOffsetFromStorage(zoidfs::zoidfs_file_size_t inc,
                    zoidfs::zoidfs_handle_t & handle,
                    zoidfs::zoidfs_file_size_t & atomic_append_base_offset);
            
            uint64_t createOffsetInStorage(zoidfs::zoidfs_handle_t & handle,
                    zoidfs::zoidfs_file_size_t & atomic_append_base_offset);

            uint64_t deleteOffsetInStorage(zoidfs::zoidfs_handle_t & handle);

         protected:
            boost::shared_ptr<Log> log_service_;
            boost::shared_ptr<RPCServer> rpcserver_;
            iofwdutil::IOFWDLogSource & log_;
      };
   }
}

#endif
