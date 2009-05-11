#ifndef IOFWDUTIL_ZLOG_LEVELMATCH_HH
#define IOFWDUTIL_ZLOG_LEVELMATCH_HH

#include <boost/array.hpp>
#include "ZLog.hh"
#include "iofwdutil/assert.hh"

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
   LevelMatch (); 

   void setup (const char * str); 

   void allowAll (); 

   bool operator () (int level) const
   { 
      ALWAYS_ASSERT(level < static_cast<int>(levelMask_.size())); 
      return levelMask_[level]; 
   }

public:
   typedef boost::array<bool,MAX_LEVEL> MaskType;
protected:
   MaskType levelMask_; 
   bool ready_; 
}; 

//===========================================================================
   }
}
#endif
