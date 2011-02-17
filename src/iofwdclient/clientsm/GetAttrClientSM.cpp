#include "iofwdclient/clientsm/GetAttrClientSM.hh"
#include "iofwdclient/FakeBlocker.hh"

#include "zoidfs/zoidfs-async.h"

#include <cstdio>

namespace iofwdclient
{
    namespace clientsm
    {

GetAttrClientSM::~GetAttrClientSM()
{
}

void GetAttrClientSM::init(iofwdevent::CBException e)
{
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    e.check();
    setNextMethod(&GetAttrClientSM::postOpenConnection);
}

/*
   step 1 open stream connection with the server '
 */
void GetAttrClientSM::postOpenConnection(iofwdevent::CBException e)
{
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    e.check();
    iofwdclient_fake_blocker_nb(slots_[BASE_SLOT], e);
    slots_.wait(BASE_SLOT, &GetAttrClientSM::waitOpenConnection);
}

void GetAttrClientSM::waitOpenConnection(iofwdevent::CBException e)
{
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    e.check();
    setNextMethod(&GetAttrClientSM::postEncodeRequest);
}

/*
   step 2: encode client request data into the stream
   completion of this state is considered client or immeadiate completion
*/
void GetAttrClientSM::postEncodeRequest(iofwdevent::CBException e)
{
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    e.check();
    iofwdclient_fake_blocker_nb(slots_[BASE_SLOT], e);
    slots_.wait(BASE_SLOT, &GetAttrClientSM::waitEncodeRequest);
}

void GetAttrClientSM::waitEncodeRequest(iofwdevent::CBException e)
{
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    e.check();
    setNextMethod(&GetAttrClientSM::postFlushRequest);
}

/*
   step 3: flush the client request data to the server
   completion of this state is considered server or remote completion
   client input buffers are free
 */
void GetAttrClientSM::postFlushRequest(iofwdevent::CBException e)
{
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    e.check();
    iofwdclient_fake_blocker_nb(slots_[BASE_SLOT], e);
    slots_.wait(BASE_SLOT, &GetAttrClientSM::waitFlushRequest);
}

void GetAttrClientSM::waitFlushRequest(iofwdevent::CBException e)
{
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    e.check();
    //cb_(zoidfs::ZFS_COMP_LOCAL, e);
    setNextMethod(&GetAttrClientSM::postWaitResponse);
}

/*
   step 4: wait for the server response
 */
void GetAttrClientSM::postWaitResponse(iofwdevent::CBException e)
{
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    e.check();
    iofwdclient_fake_blocker_nb(slots_[BASE_SLOT], e);
    slots_.wait(BASE_SLOT, &GetAttrClientSM::waitWaitResponse);
}

void GetAttrClientSM::waitWaitResponse(iofwdevent::CBException e)
{
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    e.check();
    setNextMethod(&GetAttrClientSM::postDecodeResponse);
}

/*
   step 5: decode the server response
   this is the terminal state for the state machine
 */
void GetAttrClientSM::postDecodeResponse(iofwdevent::CBException e)
{
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    e.check();
    iofwdclient_fake_blocker_nb(slots_[BASE_SLOT], e);
    slots_.wait(BASE_SLOT, &GetAttrClientSM::waitDecodeResponse);
}

void GetAttrClientSM::waitDecodeResponse(iofwdevent::CBException e)
{
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    e.check();
    cb_(zoidfs::ZFS_COMP_DONE, e);
}

void GetAttrClientSM::postSMErrorState(iofwdevent::CBException e)
{
    fprintf(stderr, "%s:%i\n", __func__, __LINE__);
    e.check();
    //cb_(zoidfs::ZFS_COMP_ERROR, e);
}

    }
}
