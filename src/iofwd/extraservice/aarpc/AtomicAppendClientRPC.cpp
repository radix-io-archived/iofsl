#include "iofwd/extraservice/aarpc/AtomicAppendClientRPC.hh"

#include <iostream>

namespace iofwd
{
    namespace extraservice
    {

void AtomicAppendClientRPC::getNextOffsetImpl(rpc::RPCClientHandle h,
        zoidfs::zoidfs_handle_t & handle,
        zoidfs::zoidfs_file_size_t incsize,
        zoidfs::zoidfs_file_ofs_t & offset,
        uint64_t & retcode)
{
   iofwdevent::SingleCompletion block;

   {
      // wait until outgoing RPC ready
      block.reset();
      h->waitOutReady(block);
      block.wait();

      // Now we can use the outgoing stream
      boost::scoped_ptr<iofwdevent::ZeroCopyOutputStream> out (h->getOut());

      // encode input arguments for remote RPC
      void * write_ptr;
      size_t write_size;
      const size_t sendsize =
            rpc::getRPCEncodedSize(zoidfs::zoidfs_handle_t()).getMaxSize() +
            rpc::getRPCEncodedSize(zoidfs::zoidfs_file_size_t()).getMaxSize();

      block.reset();
      out->write(&write_ptr, &write_size, block, sendsize);
      block.wait();

      rpc::RPCEncoder enc(write_ptr, write_size);
      process(enc, handle);
      process(enc, incsize);

      block.reset();
      out->rewindOutput(write_size - enc.getPos(), block);
      block.wait();

      block.reset();
      out->flush(block);
      block.wait();
   }

   // Read response
   {
      block.reset();
      h->waitInReady(block);
      block.wait();

      boost::scoped_ptr<iofwdevent::ZeroCopyInputStream> in (h->getIn ());

      const void * read_ptr;
      size_t read_size;

      const size_t recvsize =
            rpc::getRPCEncodedSize(zoidfs::zoidfs_file_size_t()).getMaxSize() +
            rpc::getRPCEncodedSize(uint64_t()).getMaxSize();

      block.reset();
      in->read(&read_ptr, &read_size, block, recvsize);
      block.wait();

      rpc::RPCDecoder dec(read_ptr, read_size);
      process(dec, offset);
      process(dec, retcode);

      if(dec.getPos() != read_size)
         std::cout << "Extra bytes at end of RPC response?" << std::endl;
   }
}

void AtomicAppendClientRPC::createOffsetImpl(rpc::RPCClientHandle h,
        zoidfs::zoidfs_handle_t & handle,
        zoidfs::zoidfs_file_ofs_t offset,
        uint64_t & retcode)
{
   iofwdevent::SingleCompletion block;

   {
      // wait until outgoing RPC ready
      block.reset ();
      h->waitOutReady(block);
      block.wait ();

      // Now we can use the outgoing stream
      boost::scoped_ptr<iofwdevent::ZeroCopyOutputStream> out (h->getOut());

      // encode input arguments for remote RPC
      void * write_ptr;
      size_t write_size;
      const size_t sendsize =
            rpc::getRPCEncodedSize(zoidfs::zoidfs_handle_t()).getMaxSize() +
            rpc::getRPCEncodedSize(zoidfs::zoidfs_file_ofs_t()).getMaxSize();

      block.reset();
      out->write(&write_ptr, &write_size, block, sendsize);
      block.wait();

      rpc::RPCEncoder enc(write_ptr, write_size);
      process(enc, handle);
      process(enc, offset);

      block.reset();
      out->rewindOutput(write_size - enc.getPos(), block);
      block.wait();

      block.reset();
      out->flush(block);
      block.wait();
   }

   // Read response
   {
      block.reset();
      h->waitInReady(block);
      block.wait();

      boost::scoped_ptr<iofwdevent::ZeroCopyInputStream> in (h->getIn ());

      const void * read_ptr;
      size_t read_size;

      const size_t recvsize = rpc::getRPCEncodedSize(uint64_t()).getMaxSize();

      block.reset();
      in->read(&read_ptr, &read_size, block, recvsize);
      block.wait();

      rpc::RPCDecoder dec(read_ptr, read_size);
      process(dec, retcode);

      if(dec.getPos() != read_size)
         std::cout << "Extra bytes at end of RPC response?" << std::endl;
   }
}

void AtomicAppendClientRPC::deleteOffsetImpl(rpc::RPCClientHandle h,
        zoidfs::zoidfs_handle_t & handle,
        uint64_t & retcode)
{
    iofwdevent::SingleCompletion block;

   {
      // wait until outgoing RPC ready
      block.reset ();
      h->waitOutReady(block);
      block.wait ();

      // Now we can use the outgoing stream
      boost::scoped_ptr<iofwdevent::ZeroCopyOutputStream> out (h->getOut());

      // encode input arguments for remote RPC
      void * write_ptr;
      size_t write_size;
      const size_t sendsize =
            rpc::getRPCEncodedSize(zoidfs::zoidfs_handle_t()).getMaxSize();

      block.reset();
      out->write(&write_ptr, &write_size, block, sendsize);
      block.wait();

      rpc::RPCEncoder enc(write_ptr, write_size);
      process(enc, handle);

      block.reset();
      out->rewindOutput(write_size - enc.getPos(), block);
      block.wait();

      block.reset();
      out->flush(block);
      block.wait();
   }

   // Read response
   {
      block.reset();
      h->waitInReady(block);
      block.wait();

      boost::scoped_ptr<iofwdevent::ZeroCopyInputStream> in (h->getIn ());

      const void * read_ptr;
      size_t read_size;

      const size_t recvsize = rpc::getRPCEncodedSize(uint64_t()).getMaxSize();

      block.reset();
      in->read(&read_ptr, &read_size, block, recvsize);
      block.wait();

      rpc::RPCDecoder dec(read_ptr, read_size);
      process(dec, retcode);

      if(dec.getPos() != read_size)
         std::cout << "Extra bytes at end of RPC response?" << std::endl;
   }
}

    }
}
