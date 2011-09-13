#ifndef EXTRASERVICE_SFP_SFPSERVICE_HH
#define EXTRASERVICE_SFP_SFPSERVICE_HH

#include "rpc/RPC-fwd.hh"
#include "rpc/RPCInfo.hh"

#include "iofwd/iofwd-fwd.hh"
#include "iofwd/ExtraService.hh"

#include "iofwdutil/hash/hash-fwd.hh"
#include "iofwdutil/IOFWDLog-fwd.hh"

#include "iofwdevent/iofwdevent-fwd.hh"
#include "iofwdevent/CBType.hh"

#include "zoidfs/zoidfs.h"
#include "zoidfs/util/zoidfs-xdr.hh"

#include "net/net-fwd.hh"

#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

namespace iofwd
{
   namespace extraservice
   {
      //=====================================================================


      /**
       *
       * @TODO: Make SFPService use a standalone distributed table
       * (i.e. as in AtomicAppend);
       *
       */
      class SFPService : public ExtraService
      {
         public:
            SFPService (service::ServiceManager & m);

            virtual void configureNested (const iofwdutil::ConfigFile &);

            virtual ~SFPService ();

            void removeSFP (bool * ret, const zoidfs::zoidfs_handle_t *
                  handle, uint64_t sfp_id, const iofwdevent::CBType & cb);

            void createSFP (bool * ret, const zoidfs::zoidfs_handle_t *
                  handle, uint64_t sfp_id, zoidfs::zoidfs_file_ofs_t init,
                  const iofwdevent::CBType & cb);

            enum
            {
               SFP_SET = 0,
               SFP_FETCH_AND_ADD, // Return old value, add value to fp
               SFP_FETCH,
               SFP_LAST
            };

            void updateSFP (bool * ret, const zoidfs::zoidfs_handle_t * handle,
                  uint64_t sfp_id, int op, zoidfs::zoidfs_file_ofs_t * value,
                  const iofwdevent::CBType & cb);


            struct SFPIn
            {
               zoidfs::zoidfs_handle_t handle;
               uint64_t sfp_id;
               uint32_t op;
               zoidfs::zoidfs_file_ofs_t value;
            };

            struct SFPOut
            {
               uint32_t result;
               zoidfs::zoidfs_file_ofs_t value;
            };


         protected:

            enum
            {
               SFP_CREATE = SFP_LAST,
               SFP_REMOVE
            };

            void modifySFP (bool * ret, const zoidfs::zoidfs_handle_t * handle,
                  uint64_t sfp_id, int op, zoidfs::zoidfs_file_ofs_t * value,
                  const iofwdevent::CBType & cb);

            void rpcSFPModify (const SFPIn & in, SFPOut & out);
      
            bool localSFPModify ( const zoidfs::zoidfs_handle_t * handle,
                  uint64_t sfp_id, int op, zoidfs::zoidfs_file_ofs_t * value);

            size_t locateOwner (const zoidfs::zoidfs_handle_t * handle,
                  uint64_t sfp_id) const;

         protected:
            boost::shared_ptr<Net> net_service_;
            boost::shared_ptr<Log> log_service_;
            boost::shared_ptr<RPCServer> rpcserver_;
            boost::shared_ptr<RPCClient> rpcclient_;
            iofwdutil::IOFWDLogSource & log_;

            mutable boost::scoped_ptr<iofwdutil::hash::HashFunc> hash_;
            mutable boost::mutex hash_lock_;

            net::ConstCommunicatorHandle comm_;
            size_t commsize_;
            size_t commrank_;

            typedef std::pair<zoidfs::zoidfs_handle_t, uint64_t> SFPKey;
            typedef boost::unordered_map<SFPKey, zoidfs::zoidfs_file_ofs_t> MapType;
            MapType sfp_map_;
            boost::mutex sfp_lock_;
      };

      //=====================================================================
   }
}

#endif
