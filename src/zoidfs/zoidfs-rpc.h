#ifndef ZOIDFS_ZOIDFSRPC_HH
#define ZOIDFS_ZOIDFSRPC_HH

#include <string>

namespace zoidfs
{

#define ZOIDFS_GETATTR_RPC std::string("zoidfs.getattr")
#define ZOIDFS_LOOKUP_RPC std::string("iofslclientrpc.lookup")
#define ZOIDFS_WRITE_RPC std::string("iofslclientrpc.write")
#define ZOIDFS_READ_RPC std::string("iofslclientrpc.read")
#define ZOIDFS_CREATE_RPC std::string("iofslclientrpc.create")
#define ZOIDFS_COMMIT_RPC std::string("iofslclientrpc.commit")
#define ZOIDFS_MKDIR_RPC std::string("iofslclientrpc.mkdir")
}

#endif
