#include "ROSSReadRequest.hh"
#include "zoidfs/zoidfs-proto.h"
#include "iofwd_config.h"

#include "iofwdevent/BMIResource.hh"

namespace iofwd
{
   namespace rossfrontend
   {

ROSSReadRequest::~ROSSReadRequest ()
{
}

ROSSReadRequest::ReqParam & ROSSReadRequest::decodeParam ()
{
   return param_;
}

void ROSSReadRequest::initRequestParams(ReqParam & UNUSED(p), void *
        UNUSED(bufferMem))
{
}

void ROSSReadRequest::sendBuffers(const iofwdevent::CBType & UNUSED(cb),
        RetrievedBuffer * UNUSED(rb))
{
}

void ROSSReadRequest::sendPipelineBufferCB(const iofwdevent::CBType UNUSED(cb),
        RetrievedBuffer * UNUSED(rb), size_t UNUSED(size))
{
}

void ROSSReadRequest::reply(const CBType & UNUSED(cb))
{
}

void ROSSReadRequest::allocateBuffer(iofwdevent::CBType UNUSED(cb),
        RetrievedBuffer * UNUSED(rb))
{
}

void ROSSReadRequest::releaseBuffer(RetrievedBuffer * UNUSED(rb))
{
}

   }
}

