#ifndef TEST_BENCHMARK_TEST
#define TEST_BENCHMARK_TEST
#include "mpi.h"
#include "iofwd/service/ServiceManager.hh"
#include "iofwd/RPCClient.hh"
#include "iofwd/RPCServer.hh"
#include "iofwd/Net.hh"
#include "iofwd/IofwdLinkHelper.hh"
#include "iofwd/service/Service.hh"
#include "iofwdutil/ZException.hh"
#include "iofwdevent/SingleCompletion.hh"
#include "net/Net.hh"

#include "iofwd/ExtraService.hh"
#include "iofwdutil/IOFWDLog.hh"

#include "rpc/RPCInfo.hh"
#include "rpc/RPCEncoder.hh"
#include "iofwdclient/LinkHelper.hh"
#include <string>
#include <iostream>
#include <unistd.h>
#include "net/loopback/LoopbackConnector.hh"
#include <boost/thread.hpp>
#include <boost/program_options.hpp>
//#include "iofwdclient/iofwdclientlib.hh"
#include "iofwdclient/CommStream.hh"
#include "iofwdclient/IOFWDClient.hh"
#include "zoidfs/util/ZoidFSHints.hh"
#include "iofwd/extraservice/iofslclientrpc/IOFSLClientRPCService.hh"
#include "iofwdutil/IofwdutilLinkHelper.hh"
#include "zoidfs/zoidfs.h"
#define ZOIDFS_ATTR_MODE   (1 << 0)
#define ZOIDFS_ATTR_NLINK  (1 << 1)
#define ZOIDFS_ATTR_UID    (1 << 2)
#define ZOIDFS_ATTR_GID    (1 << 3)
#define ZOIDFS_ATTR_SIZE   (1 << 4)
#define ZOIDFS_ATTR_BSIZE  (1 << 5)
#define ZOIDFS_ATTR_FSID   (1 << 6)
#define ZOIDFS_ATTR_FILEID (1 << 7)
#define ZOIDFS_ATTR_ATIME  (1 << 8)
#define ZOIDFS_ATTR_MTIME  (1 << 9)
#define ZOIDFS_ATTR_CTIME  (1 << 10)
iofwdclient::IOFWDClient * iofslSetup( char * address, std::string opt_config);
//void createOutput (iofwdclient::IOFWDClient * x, zoidfs::zoidfs_handle_t * outHandle, char * filename);
void lookupInput (iofwdclient::IOFWDClient * x, zoidfs::zoidfs_handle_t * outHandle, char * filename);
//void test (char * address, char * config, zoidfs::zoidfs_handle_t inHandle, zoidfs::zoidfs_handle_t outHandle,
//           int readSize, int runs);
void test (char * address, char * config, char * inName, char * outName,
           int readSize, int runs);
#endif
