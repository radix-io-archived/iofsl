#include "iofwd/extraservice/sfp/SFPService.hh"

#include "iofwdevent/CBType.hh"
#include "iofwd/service/Service.hh"
#include "iofwd/Log.hh"
#include "iofwd/RPCServer.hh"
#include "iofwd/RPCClient.hh"
#include "iofwd/Net.hh"

#include "net/Communicator.hh"

#include "iofwdevent/SingleCompletion.hh"

#include "iofwdutil/IOFWDLog.hh"
#include "iofwdutil/hash/HashFactory.hh"

#include "encoder/xdr/XDRReader.hh"

#include "rpc/RPCInfo.hh"
#include "rpc/RPCEncoder.hh"

#include "zoidfs/util/zoidfs-ops.hh"  // for operator ==,... on zoidfs_handle_t
#include "zoidfs/util/zoidfs-util.hh"

#include <boost/scoped_ptr.hpp>
#include <boost/format.hpp>

SERVICE_REGISTER(iofwd::extraservice::SFPService, sfp);

using namespace iofwdevent;
using namespace boost;
using namespace rpc;

namespace iofwd
{
   namespace extraservice
   {
      //=====================================================================

      namespace
      {

         /**
          * Conveniently create a thread to handle the RPC call
          */
         static void threadRPC (const rpc::RPCHandler & h,
               ZeroCopyInputStream * in, ZeroCopyOutputStream * out,
               const rpc::RPCInfo & info)
         {
            boost::thread (boost::bind (h, in, out, info));
         }

         rpc::RPCHandler rpcExec (const rpc::RPCHandler & orig)
         {
            return boost::bind (&threadRPC, orig, _1, _2, _3);
         }


         template <typename IN, typename OUT>
            void rpcServerHelper (boost::function<void (const IN &, OUT &)> & func,
                  ZeroCopyInputStream * i,
                  ZeroCopyOutputStream * o,
                  const rpc::RPCInfo &)
            {
               scoped_ptr<ZeroCopyInputStream> in (i);
               scoped_ptr<ZeroCopyOutputStream> out (o);

               IN arg_in;
               OUT arg_out;
               SingleCompletion block;

               {
                  const void * read_ptr;
                  size_t read_size;
                  const size_t insize = getRPCEncodedSize (IN()).getMaxSize ();

                  block.reset ();
                  in->read (&read_ptr, &read_size, block, insize);
                  block.wait ();

                  RPCDecoder dec (read_ptr, read_size);
                  process (dec, arg_in);
                  if (dec.getPos () < read_size)
                  {
                     ZLOG_ERROR (ZLOG_DEFAULT, "RPCServer: extra data after"
                           " RPC request?");
                  }
               }

               // call user func
               func (arg_in, arg_out);

               // Send response
               {
                  void * write_ptr;
                  size_t write_size;
                  const size_t outsize = getRPCEncodedSize (OUT()).getMaxSize ();

                  block.reset ();
                  out->write (&write_ptr, &write_size, block, outsize);
                  block.wait ();

                  RPCEncoder enc (write_ptr, write_size);
                  process (enc, arg_out);

                  block.reset ();
                  out->rewindOutput (write_size - enc.getPos (), block);
                  block.wait ();

                  block.reset ();
                  out->flush (block);
                  block.wait ();
               }
            }

         template <typename IN, typename OUT>
            void rpcClientHelper (const IN & in, OUT & out, RPCClientHandle h)
            {
               SingleCompletion block;

               {
                  // wait until outgoing RPC ready
                  block.reset ();
                  h->waitOutReady (block);
                  block.wait ();

                  // Now we can use the outgoing stream
                  scoped_ptr<ZeroCopyOutputStream> out (h->getOut());

                  // encode input arguments for remote RPC
                  void * write_ptr;
                  size_t write_size;


                  const size_t sendsize = getRPCEncodedSize (IN ()).getMaxSize ();

                  block.reset ();
                  out->write (&write_ptr, &write_size, block, sendsize);
                  block.wait ();

                  RPCEncoder enc (write_ptr, write_size);
                  process (enc, in);

                  block.reset ();
                  out->rewindOutput (write_size - enc.getPos (), block);
                  block.wait ();

                  block.reset ();
                  out->flush (block);
                  block.wait ();
               }

               // Read response
               {
                  block.reset ();
                  h->waitInReady (block);
                  block.wait ();

                  scoped_ptr<ZeroCopyInputStream> in (h->getIn ());

                  const void * read_ptr;
                  size_t read_size;

                  const size_t receivesize = getRPCEncodedSize (OUT ()).getMaxSize ();

                  block.reset ();
                  in->read (&read_ptr, &read_size, block, receivesize);
                  block.wait ();

                  RPCDecoder dec (read_ptr, read_size);
                  process (dec, out);

                  if (dec.getPos () != read_size)
                     ZLOG_ERROR (ZLOG_DEFAULT, "Extra bytes at end of RPC response?");
               }
            }
      }


      // --- encoders ---
      template <typename E, typename P>
      void process (E & enc, P & in,
           typename encoder::process_filter<P, SFPService::SFPIn>::type * = 0)
      {
         process (enc, in.handle);
         process (enc, in.sfp_id);
         process (enc, in.op);
         process (enc, in.value);
      }

      template <typename ENC, typename P>
      void process (ENC & enc, P & out,
           typename encoder::process_filter<P, SFPService::SFPOut>::type * = 0)
      {
         process (enc, out.result);
         process (enc, out.value);
      }


      SFPService::SFPService (service::ServiceManager & m)
         : ExtraService (m),
           net_service_ (lookupService<Net> ("net")),
           log_service_ (lookupService<Log> ("log")),
           rpcserver_ (lookupService<RPCServer> ("rpcserver")),
           rpcclient_ (lookupService<RPCClient> ("rpcclient")),
           log_ (log_service_->getSource ("sfp"))
      {
         boost::function<void (const SFPIn &, SFPOut &)> f =
                     boost::bind (&SFPService::rpcSFPModify, this, _1, _2);
         rpc::RPCHandler h = boost::bind (
                     &rpcServerHelper<SFPIn, SFPOut>, f,
                     _1, _2, _3);

         rpcserver_->registerRPC ("sfp.modify", rpcExec (h));

         hash_.reset (iofwdutil::hash::HashFactory::instance().getHash
               ("crc32"));

         comm_ = net_service_->getServerComm ();
         
         commsize_ = comm_->size ();
         commrank_ = comm_->rank ();
         ZLOG_DEBUG_MORE (log_, boost::format("My rank: %u, Total ranks: %u") %
               commrank_ % commsize_);
      }

      SFPService::~SFPService ()
      {
         rpcserver_->unregisterRPC ("sfp.modify");
      }

      void SFPService::configureNested (const iofwdutil::ConfigFile & )
      {
      }


      template <typename T>
      static void cleanupHelper (const iofwdevent::CBType & cb, T * cleanup,
            iofwdevent::CBException & e)
      {
         boost::scoped_ptr<T> clean (cleanup);
         cb (e);
      }

      void SFPService::removeSFP (bool * ret, const zoidfs::zoidfs_handle_t *
            handle, uint64_t sfp_id, const iofwdevent::CBType & cb)
      {
         // This workaround is to ensure that dummy remains valid until the
         // callback has been called (meaning it cannot be on our local stack
         // here)
         zoidfs::zoidfs_file_ofs_t * dummy = new zoidfs::zoidfs_file_ofs_t (0);
         modifySFP (ret, handle, sfp_id, SFP_REMOVE, dummy,
               boost::bind (&cleanupHelper<zoidfs::zoidfs_file_ofs_t>, cb,
                  dummy, _1));
      }

      void SFPService::createSFP (bool * ret, const zoidfs::zoidfs_handle_t *
            handle, uint64_t sfp_id, zoidfs::zoidfs_file_ofs_t init,
                  const iofwdevent::CBType & cb)
      {
         zoidfs::zoidfs_file_ofs_t * val = new zoidfs::zoidfs_file_ofs_t (init);
         modifySFP (ret, handle, sfp_id, SFP_CREATE, val,
               boost::bind(&cleanupHelper<zoidfs::zoidfs_file_ofs_t>, cb,
                  val, _1));
      }

      void SFPService::updateSFP (bool * ret, const zoidfs::zoidfs_handle_t *
            handle, uint64_t sfp_id, int op, zoidfs::zoidfs_file_ofs_t * value,
                  const iofwdevent::CBType & cb)
      {
         modifySFP (ret, handle, sfp_id, op, value, cb);
      }

      void SFPService::rpcSFPModify (const SFPIn & in, SFPOut & out)
      {
         ZLOG_DEBUG_MORE (log_, "incoming rpcSFPModify");
         zoidfs::zoidfs_file_ofs_t value = in.value;
         out.result = localSFPModify (&in.handle, in.sfp_id, in.op, &value);
         out.value = value;
      }

      size_t SFPService::locateOwner (const zoidfs::zoidfs_handle_t * handle,
            uint64_t sfp_id) const
      {
         // @TODO: add Cloneable interface and make HashFunc support it;
         //        then clone the main hash func so we can use this
         //        concurrently
         boost::mutex::scoped_lock l (hash_lock_);
         hash_->reset ();
         // @TODO: the encoder framework could be bridged with the hash
         // calculation to avoid having to manually call these functions
         hash_->process (&handle->data, sizeof (handle->data));
         hash_->process (&sfp_id, sizeof(sfp_id));

         uint32_t value;
         ALWAYS_ASSERT(hash_->getHash (&value, sizeof (value)));
         ZLOG_DEBUG_MORE (log_, format("%u -> %u") % sfp_id % (value % commsize_));
         return value % commsize_;
      }

      bool SFPService::localSFPModify ( const zoidfs::zoidfs_handle_t * handle,
                  uint64_t sfp_id, int op, zoidfs::zoidfs_file_ofs_t * value)
      {
         boost::mutex::scoped_lock l (sfp_lock_);
         SFPKey k (*handle, sfp_id);
         MapType::iterator I = sfp_map_.find (k);
         switch (op)
         {
            case SFP_CREATE:
               if (I!=sfp_map_.end())
                  return false;
               sfp_map_.insert (std::make_pair(k, *value));
               return true;

            case SFP_REMOVE:
               if (I == sfp_map_.end ())
                  return false;

               sfp_map_.erase (I);
               return true;

            case SFP_SET:
               if (I == sfp_map_.end ())
                  return false;
               I->second = *value;
               return true;

            case SFP_GET:
               if (I == sfp_map_.end ())
                  return false;
               ZLOG_DEBUG_MORE (log_, format("sfp_get: ret=%u") % I->second);
               *value = I->second;
               return true;

            case SFP_FETCH_AND_ADD:
               {
                  if (I == sfp_map_.end ())
                     return false;

                  zoidfs::zoidfs_file_ofs_t ofs = I->second;
                  I->second += *value;
                  *value = ofs;
                  return true;
               }

            default:
               ALWAYS_ASSERT(false);
               return false;
         };
      }

      void SFPService::modifySFP (bool * ret, const zoidfs::zoidfs_handle_t *
            handle, uint64_t sfp_id, int op, zoidfs::zoidfs_file_ofs_t * value,
                  const iofwdevent::CBType & cb)
      {
         const size_t dest = locateOwner (handle, sfp_id);

         if (dest == commrank_)
         {
            *ret = localSFPModify (handle, sfp_id, op, value);
               ZLOG_DEBUG_MORE(log_, format("localSFPModify(%s,%u,%u,%u) = %u !")
                     % zoidfs::handle2string (handle) % sfp_id % op % *value %
                     *ret);
            cb (iofwdevent::CBException ());
            return;
         }
         else
         {
            // @TODO: Use proper thread service
            boost::thread (boost::bind (&SFPService::remoteModifySFP, this,
                                        dest, ret, handle, sfp_id, op, value,
                                        cb));
         }
      }

      void SFPService::remoteModifySFP (size_t dest,
            bool * ret, const zoidfs::zoidfs_handle_t * handle, uint64_t
            sfp_id, int op, zoidfs::zoidfs_file_ofs_t * value,
                  const iofwdevent::CBType & cb)
      {
         SFPIn in;
         in.handle = *handle;
         in.sfp_id = sfp_id;
         in.op = op;
         in.value = *value;

         SFPOut out;

         rpcClientHelper (in, out, rpcclient_->rpcConnect ("sfp.modify",
                  (*comm_)[dest]));

         *value = out.value;
         *ret = out.result;
         cb (iofwdevent::CBException ());
      }


      //=====================================================================
   }
}
