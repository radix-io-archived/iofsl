#include "ROSSWriteRequest.hh"
#include "zoidfs/zoidfs-proto.h"
#include "iofwd_config.h"

namespace iofwd
{
   namespace rossfrontend
   {

ROSSWriteRequest::~ROSSWriteRequest ()
{
}

ROSSWriteRequest::ReqParam & ROSSWriteRequest::decodeParam ()
{
   return param_;
}

void ROSSWriteRequest::initRequestParams(ReqParam & UNUSED(p), void *
        UNUSED(bufferMem))
{
}

void ROSSWriteRequest::allocateBuffer(iofwdevent::CBType UNUSED(cb),
        RetrievedBuffer * UNUSED(rb))
{
}

void ROSSWriteRequest::releaseBuffer(RetrievedBuffer * UNUSED(rb))
{
}

void ROSSWriteRequest::recvBuffers(const CBType & UNUSED(cb), RetrievedBuffer *
        UNUSED(rb))
{
}

void ROSSWriteRequest::recvPipelineBufferCB(iofwdevent::CBType UNUSED(cb),
        RetrievedBuffer * UNUSED(rb), size_t UNUSED(size))
{
}

void ROSSWriteRequest::reply(const CBType & UNUSED(cb))
{
}

   }
}
