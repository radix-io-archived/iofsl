#include "CommChannel.hh"
#include "zoidfs/util/zoidfs-xdr.hh"

using namespace iofwdutil::bmi;
using namespace iofwdutil::xdr;

namespace client
{
//===========================================================================

CommChannel::CommChannel (BMIContextPtr context, BMIAddr iofwdhost)
   : bmi_(context),
     iofwdhost_(iofwdhost),buffer_send_(iofwdhost_, BMI::ALLOC_SEND),
     buffer_receive_(iofwdhost_, BMI::ALLOC_RECEIVE)
{
}

CommChannel::~CommChannel ()
{
}

void CommChannel::executeWriteOp (const void ** buf_list,
   const size_t * size_list, size_t list_count)
{
    // Make sure the receive buffer is large enough for the reply
    const size_t needed = receiveSizeProcessor_.getSize().actual;
    buffer_receive_.resize (needed);
    // send unexpected
    iofwdutil::bmi::BMIOp sendu = bmi_->postSendUnexpected (iofwdhost_,
          buffer_send_.get(), buffer_send_.size(),
          buffer_send_.bmiType(), ZOIDFS_REQUEST_TAG);
    size_t UNUSED(sendubytes) = sendu.wait ();
    // send data
    size_t total_size = 0;
    for (size_t i = 0; i < list_count; i++) total_size += size_list[i];
    iofwdutil::bmi::BMIOp sendlist = bmi_->postSendList(iofwdhost_, buf_list, size_list, list_count,
         total_size, BMI_PRE_ALLOC, ZOIDFS_REQUEST_TAG);
    size_t UNUSED(sendbytes) = sendlist.wait ();
    // post receive
    iofwdutil::bmi::BMIOp receive = bmi_->postReceive (iofwdhost_,
         buffer_receive_.get(), buffer_receive_.size (),
         buffer_receive_.bmiType(), ZOIDFS_REQUEST_TAG);
    size_t UNUSED(received) = receive.wait ();
    // Reset XDR deserialization
    request_reader_.reset (buffer_receive_.get (needed), needed);
}

void CommChannel::executePipelineWriteOp (const void ** buf_list,
   const size_t * size_list, size_t list_count, uint64_t pipeline_size)
{
   // Make sure the receive buffer is large enough for the reply
   const size_t needed = receiveSizeProcessor_.getSize().actual;
   buffer_receive_.resize (needed);
   // send unexpected
   iofwdutil::bmi::BMIOp sendu = bmi_->postSendUnexpected (iofwdhost_,
      buffer_send_.get(), buffer_send_.size(),
      buffer_send_.bmiType(), ZOIDFS_REQUEST_TAG);
   size_t UNUSED(sendubytes) = sendu.wait ();
   // send data as pipeline
   {
     int np = 0;
     uint64_t st = 0;
     uint64_t st_mem = 0;
     uint64_t st_memofs = 0;
     uint64_t total_size = 0;
     for (size_t i = 0; i < list_count; i++)
        total_size += size_list[i];
     while (st < total_size) {
        uint64_t en = 0;
        uint64_t en_mem = 0;
        uint64_t en_memofs = 0;
        for (size_t i = 0; i < list_count; i++) {
           if (st + pipeline_size <= en + size_list[i]) {
              en_mem = i;
              en_memofs = st + pipeline_size - en;
              en += en_memofs;
              break;
           }
           en += size_list[i];
           if (i == list_count -1) {
              en_mem = i;
              en_memofs = size_list[i];
           }
        }
        // create next request
        size_t p_list_count = en_mem + 1 - st_mem;
        const char ** p_buf_list = new const char*[p_list_count];
        size_t * p_size_list = new size_t[p_list_count];
        if (st_mem == en_mem) {
           p_buf_list[0] = ((const char*)buf_list[0]) + st_memofs;
           assert(en_memofs > st_memofs);
           p_size_list[0] = en_memofs - st_memofs;
        } else {
           for (size_t i = st_mem; i <= en_mem; i++) {
             if (i == st_mem) {
                p_buf_list[i] = ((const char*)buf_list[i]) + st_memofs;
                p_size_list[i] = size_list[i] - st_memofs;
             } else if (i == en_mem) {
                p_buf_list[i] = (const char*)buf_list[i];
                p_size_list[i] = en_memofs;
             } else {
                p_buf_list[i] = (const char*)buf_list[i];
                p_size_list[i] = size_list[i];
             }
             assert(p_size_list[i] > 0);
          }
        }
        // send it
        {
           size_t p_total_size = 0;
           for (size_t i = 0; i < p_list_count; i++) p_total_size += p_size_list[i];
           assert(p_total_size <= pipeline_size);
           iofwdutil::bmi::BMIOp sendlist = bmi_->postSendList(iofwdhost_,
              (const void**)p_buf_list, p_size_list, p_list_count,
              p_total_size, BMI_PRE_ALLOC, ZOIDFS_REQUEST_TAG);
           size_t UNUSED(sendbytes) = sendlist.wait ();
        }
        // next
        st = en;
        st_mem = en_mem;
        st_memofs = en_memofs;
        np++;
     }
     // post receive
     iofwdutil::bmi::BMIOp receive = bmi_->postReceive (iofwdhost_,
        buffer_receive_.get(), buffer_receive_.size (),
        buffer_receive_.bmiType(), ZOIDFS_REQUEST_TAG);
     size_t UNUSED(received) = receive.wait ();
   }
   // Reset XDR deserialization
   request_reader_.reset (buffer_receive_.get (needed), needed);
}

//===========================================================================

//===========================================================================
}
