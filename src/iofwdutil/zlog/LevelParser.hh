#ifndef IOFWDUTIL_ZLOG_LEVELPARSER_HH
#define IOFWDUTIL_ZLOG_LEVELPARSER_HH


#include <boost/spirit/core.hpp>
#include <boost/spirit/symbols.hpp>
#include <boost/spirit/attribute.hpp>

#include <boost/spirit/phoenix/binders.hpp>

#include <boost/lambda/lambda.hpp>

#include <boost/format.hpp>

#include "ZLog.hh"

namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

struct levelparser_level_closure : 
   boost::spirit::closure<levelparser_level_closure,unsigned int>
{
      member1 value;
};


struct StringLevelParser : boost::spirit::symbols<unsigned int>
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
class LevelParser : public boost::spirit::grammar<LevelParser, 
                            levelparser_level_closure::context_t>
{
   public:

      template <typename ScannerT>
      struct definition 
      {
         definition (LevelParser const  & self)
         {
            level = boost::spirit::uint_p[self.value = phoenix::arg1] | 
               boost::spirit::as_lower_d[self.stringlevel_p[self.value = phoenix::arg1]]; 
         }

         boost::spirit::rule<ScannerT,levelparser_level_closure::context_t> const & start () const
         { return level; } 

         boost::spirit::rule<ScannerT,levelparser_level_closure::context_t > level; 
      };

      StringLevelParser const stringlevel_p; 
};


//===========================================================================
   }
}
#endif
