#include "iofwd/extraservice/aarpc/AtomicAppendServerRPC.hh"
#include "iofwd/service/Service.hh"
#include "iofwd/Log.hh"
#include "iofwd/RPCServer.hh"

#include "iofwdevent/SingleCompletion.hh"

#include "iofwdutil/IOFWDLog.hh"

#include "encoder/xdr/XDRReader.hh"
#include "encoder/xdr/XDRWriter.hh"
#include "zoidfs/util/zoidfs-xdr.hh"

#include "rpc/RPCInfo.hh"
#include "rpc/RPCEncoder.hh"

#include <boost/scoped_ptr.hpp>
#include <boost/format.hpp>

#include <iostream>

#include "iofwdutil/IOFSLKey.hh"
#include "iofwdutil/IOFSLKeyValueStorage.hh"
#include "iofwdutil/ThreadPool.hh"

SERVICE_REGISTER(iofwd::extraservice::AtomicAppendServerRPC, aarpc);

using namespace iofwdevent;
using namespace boost;

namespace iofwd
{
   namespace extraservice
   {
      namespace
      {

         /**
          * Conveniently create a thread to handle the RPC call
          */
         static void threadRPC(const rpc::RPCHandler & h,
               ZeroCopyInputStream * in, ZeroCopyOutputStream * out,
               const rpc::RPCInfo & info)
         {
             /* create a new thread */
             boost::thread(boost::bind(h, in, out, info));
         }
         static void threadpoolRPC(const rpc::RPCHandler & h,
               ZeroCopyInputStream * in, ZeroCopyOutputStream * out,
               const rpc::RPCInfo & info)
         {
             boost::function<void(void)> f = boost::bind(h, in, out, info);

             /* submit to the threadpool */
             iofwdutil::ThreadPool::instance().submitWorkUnit(f,
                     iofwdutil::ThreadPool::HIGH);
         }

         rpc::RPCHandler rpcExec(const rpc::RPCHandler & orig)
         {
//#ifdef USE_CRAY_TP 
#if 0 
            return boost::bind(&threadpoolRPC, orig, _1, _2, _3);
#else
            return boost::bind(&threadRPC, orig, _1, _2, _3);
#endif
         }
      }

      std::string AtomicAppendServerRPC::aarpc_master_addr_ = std::string("");
      std::string AtomicAppendServerRPC::aarpc_mode_ = std::string("");

      AtomicAppendServerRPC::AtomicAppendServerRPC(service::ServiceManager & m)
         : ExtraService(m),
           log_service_ (lookupService<Log>("log")),
           rpcserver_ (lookupService<RPCServer>("rpcserver")),
           log_ (log_service_->getSource("aarpc")),
           offset_init_func_(&AtomicAppendServerRPC::aarpcOffsetInitializer)
      {
         rpcserver_->registerRPC("aarpc.getnextoffset",
                 rpcExec(boost::bind(
                         &AtomicAppendServerRPC::aarpcServerHelper<AARPCGetNextOffsetIn,
                         AARPCGetNextOffsetOut >, this,
                         &AtomicAppendServerRPC::getNextOffset,
                         _1, _2, _3)));

         rpcserver_->registerRPC("aarpc.createoffset",
                 rpcExec(boost::bind(
                         &AtomicAppendServerRPC::aarpcServerHelper<AARPCCreateOffsetIn,
                        AARPCCreateOffsetOut >, this,
                         &AtomicAppendServerRPC::createOffset,
                         _1, _2, _3)));

         rpcserver_->registerRPC("aarpc.deleteoffset",
                 rpcExec(boost::bind(
                         &AtomicAppendServerRPC::aarpcServerHelper<AARPCDeleteOffsetIn,
                         AARPCDeleteOffsetOut >, this,
                         &AtomicAppendServerRPC::deleteOffset,
                         _1, _2, _3)));
      }

      AtomicAppendServerRPC::~AtomicAppendServerRPC()
      {
         rpcserver_->unregisterRPC("aarpc.getnextoffset");
         rpcserver_->unregisterRPC("aarpc.createoffset");
         rpcserver_->unregisterRPC("aarpc.deleteoffset");
      }

      void AtomicAppendServerRPC::createOffset(const AARPCCreateOffsetIn & in,
              AARPCCreateOffsetOut & out)
      {
          out.retcode = createOffsetInStorage(in.handle, out.offset);
          
          /* reschedule the thread with more work from the tp */
//#ifndef USE_CRAY_TP
#if 0
          boost::this_thread::at_thread_exit(iofwdutil::ThreadPoolKick(bwu->tp_));
#endif
      }

      void AtomicAppendServerRPC::deleteOffset(const AARPCDeleteOffsetIn & in,
              AARPCDeleteOffsetOut & out)
      {
          out.retcode = deleteOffsetInStorage(in.handle);
          
          /* reschedule the thread with more work from the tp */
//#ifndef USE_CRAY_TP
#if 0
          boost::this_thread::at_thread_exit(iofwdutil::ThreadPoolKick(bwu->tp_));
#endif
      }

      void AtomicAppendServerRPC::getNextOffset(const AARPCGetNextOffsetIn & in,
              AARPCGetNextOffsetOut & out)
      {
          out.retcode = getNextOffsetFromStorage(in.inc, in.handle, out.offset);
          
          /* reschedule the thread with more work from the tp */
//#ifndef USE_CRAY_TP
#if 0
          boost::this_thread::at_thread_exit(iofwdutil::ThreadPoolKick(bwu->tp_));
#endif
      }

      uint64_t AtomicAppendServerRPC::createOffsetInStorage(const zoidfs::zoidfs_handle_t & handle,
              zoidfs::zoidfs_file_size_t & offset)
      {
          iofwdutil::IOFSLKey::IOFSLKey key = iofwdutil::IOFSLKey();
        
          /* build the key */ 
          key.setFileHandle(&handle);
          key.setDataKey(std::string("NEXTAPPENDOFFSET"));

          /* create the key / value pair */
          iofwdutil::IOFSLKeyValueStorage::instance().rpcInitKeyValue<zoidfs::zoidfs_file_size_t>(key, offset);

          return 0;
      }

      uint64_t AtomicAppendServerRPC::deleteOffsetInStorage(const zoidfs::zoidfs_handle_t & handle)
      {
          iofwdutil::IOFSLKey::IOFSLKey key = iofwdutil::IOFSLKey();
       
          /* build the key */ 
          key.setFileHandle(&handle);
          key.setDataKey(std::string("NEXTAPPENDOFFSET"));

          /* delete the key / value pair */
          iofwdutil::IOFSLKeyValueStorage::instance().rpcFetchAndDrop<zoidfs::zoidfs_file_size_t>(key);

          return 0;
      }

      uint64_t AtomicAppendServerRPC::getNextOffsetFromStorage(const zoidfs::zoidfs_file_size_t inc,
              const zoidfs::zoidfs_handle_t & handle,
              zoidfs::zoidfs_file_size_t & offset)
      {
          AARPCOffsetInfo offset_init_args(&handle);

          iofwdutil::IOFSLKey::IOFSLKey key = iofwdutil::IOFSLKey();
        
          /* build the key */ 
          key.setFileHandle(&handle);
          key.setDataKey(std::string("NEXTAPPENDOFFSET"));
         
          /* fetch and inc the offset for the key */ 
          iofwdutil::IOFSLKeyValueStorage::instance().rpcFetchAndInc(key,
                  inc, &offset, offset_init_func_, &offset_init_args);

          return 0;
      }
   }
}
