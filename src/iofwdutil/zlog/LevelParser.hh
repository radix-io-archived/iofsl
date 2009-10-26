#ifndef IOFWDUTIL_ZLOG_LEVELPARSER_HH
#define IOFWDUTIL_ZLOG_LEVELPARSER_HH


#include "iofwdutil/boost-spirit.hh"

#include <boost/lambda/lambda.hpp>
#include <boost/format.hpp>

#include "ZLog.hh"

namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

struct levelparser_level_closure : 
   ourspirit::closure<levelparser_level_closure,unsigned int>
{
      member1 value;
};


struct StringLevelParser : ourspirit::symbols<unsigned int>
{
   StringLevelParser()
   {
      for (unsigned int i=0; i<MAX_LEVEL; ++i)
      {
         add (ZLog::getLevelName (i), i); 
      }
   }
};


/**
 * Parser that accepts either a string or a numeric specification of the
 * loglevel
 */
class LevelParser : public ourspirit::grammar<LevelParser, 
                            levelparser_level_closure::context_t>
{
   public:

      template <typename ScannerT>
      struct definition 
      {
         definition (LevelParser const  & self)
         {
            level = ourspirit::uint_p[self.value = ourphoenix::arg1] |
               ourspirit::as_lower_d[self.stringlevel_p[self.value = ourphoenix::arg1]];
         }

         ourspirit::rule<ScannerT,levelparser_level_closure::context_t> const & start () const
         { return level; } 

         ourspirit::rule<ScannerT,levelparser_level_closure::context_t > level; 
      };

      StringLevelParser const stringlevel_p; 
};


//===========================================================================
   }
}
#endif
