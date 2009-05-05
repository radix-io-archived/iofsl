#ifndef IOFWDUTIL_ZLOG_LEVELMATCH_HH
#define IOFWDUITL_ZLOG_LEVELMATCH_HH

#include <boost/array.hpp>
#include "ZLog.hh"

namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

/**
 * Takes care of judging if a level regex matches a level
 */
class LevelMatch 
{
public:
   LevelMatch (const char * str); 

   bool operator () (int level) const
   { return levelMask_[level]; }

protected:
   boost::array<bool,ZLog::MAX_LEVEL> levelMask_; 
}; 

//===========================================================================
   }
}
#endif
