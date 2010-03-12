#include "iofwd/tasksm/SMRetrievedBuffer.hh"

namespace iofwd
{
    namespace tasksm
    {

    SMRetrievedBuffer::SMRetrievedBuffer()
        : buffer(NULL), siz(0), off(0), mem_starts(NULL), mem_sizes(NULL),
          file_starts(NULL), file_sizes(NULL), ret(NULL)
    {
    }

    SMRetrievedBuffer::~SMRetrievedBuffer()
    {
       cleanup();
    }

    }
}
