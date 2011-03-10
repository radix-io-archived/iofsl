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
            boost::thread(boost::bind(h, in, out, info));
         }

         rpc::RPCHandler rpcExec(const rpc::RPCHandler & orig)
         {
            return boost::bind(&threadRPC, orig, _1, _2, _3);
         }


      }

      std::string AtomicAppendServerRPC::aarpc_master_addr_ = std::string("");

      AtomicAppendServerRPC::AtomicAppendServerRPC(service::ServiceManager & m)
         : ExtraService (m),
           log_service_ (lookupService<Log>("log")),
           rpcserver_ (lookupService<RPCServer>("rpcserver")),
           log_ (log_service_->getSource("aarpc"))
      {
         rpcserver_->registerRPC("aarpc.getnextoffset",
               rpcExec(boost::bind(&AtomicAppendServerRPC::getNextOffset, this, _1, _2, _3)));
         rpcserver_->registerRPC("aarpc.createoffset",
               rpcExec(boost::bind(&AtomicAppendServerRPC::createOffset, this, _1, _2, _3)));
         rpcserver_->registerRPC("aarpc.deleteoffset",
               rpcExec(boost::bind(&AtomicAppendServerRPC::deleteOffset, this, _1, _2, _3)));
      }

      AtomicAppendServerRPC::~AtomicAppendServerRPC()
      {
         rpcserver_->unregisterRPC("aarpc.getnextoffset");
         rpcserver_->unregisterRPC("aarpc.createoffset");
         rpcserver_->unregisterRPC("aarpc.deleteoffset");
      }

      void AtomicAppendServerRPC::createOffset(ZeroCopyInputStream * in,
            ZeroCopyOutputStream * out,
            const rpc::RPCInfo & )
      {
          /* streams */
          boost::scoped_ptr<ZeroCopyInputStream> instream(in);
          boost::scoped_ptr<ZeroCopyOutputStream> outstream(out);

          /* read data tracking */
          const char * read_ptr;
          size_t read_size = 0;
          
          /* write data tracking */
          char * write_ptr;
          size_t write_size = 0;

          /* rpc in data */
          zoidfs::zoidfs_handle_t handle;
          zoidfs::zoidfs_file_ofs_t offset = 0;

          /* rpc out data */
          uint64_t retcode = 0;

          /* max request size */
          const size_t insize =
              rpc::getRPCEncodedSize(zoidfs::zoidfs_handle_t()).getMaxSize () +
              rpc::getRPCEncodedSize(zoidfs::zoidfs_file_ofs_t()).getMaxSize();

          /* max response size */
          const size_t outsize =
              rpc::getRPCEncodedSize(uint64_t()).getMaxSize();

          /* comp block */
          SingleCompletion block;

          /* prepare to read from the stream */
          in->read(reinterpret_cast<const void **>(&read_ptr),
                  &read_size, block, insize);
          block.wait();
          
          /* decode the XDR in the RPC request */
          rpc::RPCDecoder dec(read_ptr, read_size);
          process(dec, handle);
          process(dec, offset);

          /* get the atomic offset */
          retcode = createOffsetInStorage(handle, offset);

          /* prepare to send the response back to the caller */
          block.reset();
          out->write(reinterpret_cast<void**>(&write_ptr), &write_size,
                  block, outsize);
          block.wait();

          /* encode the offset into XDR for the RPC response */
          rpc::RPCEncoder enc(write_ptr, write_size);
          process(enc, retcode);

          /* rewind */
          block.reset();
          out->rewindOutput(write_size - enc.getPos(), block);
          block.wait();

          /* flush the reponse */
          block.reset();
          out->flush(block);
          block.wait(); 
      }

      void AtomicAppendServerRPC::deleteOffset(ZeroCopyInputStream * in,
            ZeroCopyOutputStream * out,
            const rpc::RPCInfo & )
      {
          /* streams */
          boost::scoped_ptr<ZeroCopyInputStream> instream(in);
          boost::scoped_ptr<ZeroCopyOutputStream> outstream(out);

          /* read data tracking */
          const char * read_ptr;
          size_t read_size = 0;
          
          /* write data tracking */
          char * write_ptr;
          size_t write_size = 0;

          /* rpc in data */
          zoidfs::zoidfs_handle_t handle;

          /* rpc out data */
          uint64_t retcode = 0;

          /* max request size */
          const size_t insize =
              rpc::getRPCEncodedSize(zoidfs::zoidfs_handle_t()).getMaxSize ();

          /* max response size */
          const size_t outsize =
              rpc::getRPCEncodedSize(uint64_t()).getMaxSize();

          /* comp block */
          SingleCompletion block;

          /* prepare to read from the stream */
          in->read(reinterpret_cast<const void **>(&read_ptr),
                  &read_size, block, insize);
          block.wait();
          
          /* decode the XDR in the RPC request */
          rpc::RPCDecoder dec(read_ptr, read_size);
          process(dec, handle);

          /* get the atomic offset */
          retcode = deleteOffsetInStorage(handle);

          /* prepare to send the response back to the caller */
          block.reset();
          out->write(reinterpret_cast<void**>(&write_ptr), &write_size,
                  block, outsize);
          block.wait();

          /* encode the offset into XDR for the RPC response */
          rpc::RPCEncoder enc(write_ptr, write_size);
          process(enc, retcode);

          /* rewind */
          block.reset();
          out->rewindOutput(write_size - enc.getPos(), block);
          block.wait();

          /* flush the reponse */
          block.reset();
          out->flush(block);
          block.wait();
      }

      void AtomicAppendServerRPC::getNextOffset(ZeroCopyInputStream * in,
            ZeroCopyOutputStream * out,
            const rpc::RPCInfo & )
      {
          /* streams */
          boost::scoped_ptr<ZeroCopyInputStream> instream(in);
          boost::scoped_ptr<ZeroCopyOutputStream> outstream(out);

          /* read data tracking */
          const char * read_ptr;
          size_t read_size = 0;
          
          /* write data tracking */
          char * write_ptr;
          size_t write_size = 0;

          /* rpc in data */
          zoidfs::zoidfs_handle_t handle;
          zoidfs::zoidfs_file_size_t inc = 0;

          /* rpc out data */
          uint64_t retcode = 0;
          zoidfs::zoidfs_file_ofs_t offset = 0;

          /* max request size */
          const size_t insize =
              rpc::getRPCEncodedSize(zoidfs::zoidfs_handle_t()).getMaxSize () +
              rpc::getRPCEncodedSize(zoidfs::zoidfs_file_size_t()).getMaxSize();

          /* max response size */
          const size_t outsize =
              rpc::getRPCEncodedSize(zoidfs::zoidfs_file_ofs_t()).getMaxSize() +
              rpc::getRPCEncodedSize(uint64_t()).getMaxSize();

          /* comp block */
          SingleCompletion block;

          /* prepare to read from the stream */
          in->read(reinterpret_cast<const void **>(&read_ptr),
                  &read_size, block, insize);
          block.wait();
          
          /* decode the XDR in the RPC request */
          rpc::RPCDecoder dec(read_ptr, read_size);
          process(dec, handle);
          process(dec, inc);

          /* get the atomic offset */
          retcode = getNextOffsetFromStorage(inc, handle, offset);

          /* prepare to send the response back to the caller */
          block.reset();
          out->write(reinterpret_cast<void**>(&write_ptr), &write_size,
                  block, outsize);
          block.wait();

          /* encode the offset into XDR for the RPC response */
          rpc::RPCEncoder enc(write_ptr, write_size);
          process(enc, offset);
          process(enc, retcode);

          /* rewind */
          block.reset();
          out->rewindOutput(write_size - enc.getPos(), block);
          block.wait();

          /* flush the reponse */
          block.reset();
          out->flush(block);
          block.wait(); 
      }

      uint64_t AtomicAppendServerRPC::createOffsetInStorage(zoidfs::zoidfs_handle_t & handle,
              zoidfs::zoidfs_file_size_t & offset)
      {
          iofwdutil::IOFSLKey::IOFSLKey key = iofwdutil::IOFSLKey();
        
          /* build the key */ 
          key.setFileHandle(&handle);
          key.setDataKey(std::string("NEXTAPPENDOFFSET"));

          /* create the key / value pair */
          iofwdutil::IOFSLKeyValueStorage::instance().rpcInitKeyValue<zoidfs::zoidfs_file_size_t>(key, offset);

          std::cout << "AtomicAppendServerRPC::createOffsetInStorage" << std::endl;

          return 0;
      }

      uint64_t AtomicAppendServerRPC::deleteOffsetInStorage(zoidfs::zoidfs_handle_t & handle)
      {
          iofwdutil::IOFSLKey::IOFSLKey key = iofwdutil::IOFSLKey();
       
          /* build the key */ 
          key.setFileHandle(&handle);
          key.setDataKey(std::string("NEXTAPPENDOFFSET"));

          /* delete the key / value pair */
          iofwdutil::IOFSLKeyValueStorage::instance().rpcFetchAndDrop<zoidfs::zoidfs_file_size_t>(key);

          std::cout << "AtomicAppendServerRPC::deleteOffsetInStorage" << std::endl;

          return 0;
      }

      uint64_t AtomicAppendServerRPC::getNextOffsetFromStorage(zoidfs::zoidfs_file_size_t inc,
              zoidfs::zoidfs_handle_t & handle,
              zoidfs::zoidfs_file_size_t & offset)
      {
          iofwdutil::IOFSLKey::IOFSLKey key = iofwdutil::IOFSLKey();
        
          /* build the key */ 
          key.setFileHandle(&handle);
          key.setDataKey(std::string("NEXTAPPENDOFFSET"));
         
          /* fetch and inc the offset for the key */ 
          iofwdutil::IOFSLKeyValueStorage::instance().rpcFetchAndInc(key,
                  inc, &offset);

          std::cout << "AtomicAppendServerRPC::getNextOffsetFromStorage" << std::endl;

          return 0;
      }
   }
}
