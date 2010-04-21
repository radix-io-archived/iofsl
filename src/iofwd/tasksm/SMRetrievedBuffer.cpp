#include "iofwd/tasksm/SMRetrievedBuffer.hh"

namespace iofwd
{
    namespace tasksm
    {

    SMRetrievedBuffer::SMRetrievedBuffer(iofwdutil::bmi::BMIAddr addr, iofwdutil::bmi::BMI::AllocType allocType, size_t p_size)
        : buffer(NULL), siz(0), p_siz(p_size), off(0), mem_starts(NULL), mem_sizes(NULL),
          file_starts(NULL), file_sizes(NULL), ret(NULL), addr_(addr), allocType_(allocType)
    {
    }

    SMRetrievedBuffer::~SMRetrievedBuffer()
    {
       cleanup();
    }

    }
}
