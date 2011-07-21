#include "src/iofwdutil/SelfPipe.hh"

namespace iofwdutil
{

SelfPipe::~SelfPipe(void)
{
    close(fd_[0]);
    close(fd_[1]);
}

}
