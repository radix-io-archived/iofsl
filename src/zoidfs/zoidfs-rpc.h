#ifndef ZOIDFS_ZOIDFSRPC_HH
#define ZOIDFS_ZOIDFSRPC_HH

#include <string>

namespace zoidfs
{

#define ZOIDFS_GETATTR_RPC std::string("iofslclientrpc.getattr")
#define ZOIDFS_LOOKUP_RPC std::string("iofslclientrpc.lookup")
#define ZOIDFS_WRITE_RPC std::string("iofslclientrpc.write")
#define ZOIDFS_READ_RPC std::string("iofslclientrpc.read")
#define ZOIDFS_CREATE_RPC std::string("iofslclientrpc.create")
#define ZOIDFS_COMMIT_RPC std::string("iofslclientrpc.commit")
#define ZOIDFS_MKDIR_RPC std::string("iofslclientrpc.mkdir")
#define ZOIDFS_LINK_RPC  std::string("iofslclientrpc.link")
#define ZOIDFS_SETATTR_RPC std::string("iofslclientrpc.setattr")
#define ZOIDFS_READLINK_RPC std::string("iofslclientrpc.readlink")
#define ZOIDFS_REMOVE_RPC std::string("iofslclientrpc.remove")
#define ZOIDFS_RENAME_RPC std::string("iofslclientrpc.rename")
#define ZOIDFS_SYMLINK_RPC std::string("iofslclientrpc.symlink")
#define ZOIDFS_READDIR_RPC std::string("iofslclientrpc.readdir")

}

#endif
