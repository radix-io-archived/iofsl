#ifndef IOFWDCLIENT_SM_CREATECLIENTSM
#define IOFWDCLIENT_SM_CREATECLIENTSM

#include "sm/SMManager.hh"
#include "sm/SimpleSM.hh"
#include "sm/SMClient.hh"
#include "sm/SimpleSlots.hh"

#include "iofwdutil/tools.hh"

#include "iofwdclient/IOFWDClientCB.hh"
#include "iofwdclient/clientsm/RPCServerSM.hh"

#include "zoidfs/zoidfs.h"

#include "iofwdclient/streamwrappers/ZoidFSStreamWrappers.hh"

#include "iofwdclient/clientsm/ClientSMMacros.hh"

#include <cstdio>

CLIENT_GENERATEHEADERSM(CreateClientSM, CreateOutStream, CreateInStream, ((zoidfs_handle_t)(handle))
                                                                        ((EncoderString)(full_path))
                                                                        ((EncoderString)(component_name))              
                                                                        ((zoidfs_sattr_t)(attr))
                                                                        ((int)(created)),
                                                                        (&handle, created),
                                                                        (&handle, &full_path, &component_name, &attr, op_hint))
#endif
