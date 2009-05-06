
#include <boost/spirit/core.hpp>
#include <boost/spirit/symbols.hpp>
#include <boost/spirit/attribute.hpp>

#include <boost/spirit/phoenix/binders.hpp>

#include <boost/lambda/lambda.hpp>

#include <boost/format.hpp>

#include <algorithm>

#include "iofwdutil/zlog/ZLog.hh"
#include "iofwdutil/zlog/ZLogException.hh"
#include "iofwdutil/assert.hh"

#include "LevelMatch.hh"

using namespace iofwdutil::zlog;

using namespace boost::spirit;
using namespace phoenix; 


namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

typedef LevelMatch::MaskType MaskType; 

namespace
{

   
typedef enum { LT = 0, GT, LTEQ, GTEQ, EQ } CompTypes;

struct OpParser : boost ::spirit::symbols<int>
{
   OpParser ()
   {
      add (">", GT)
          ("<", LT)
          (">=", GTEQ)
          ("<=", LTEQ)
          ("=", EQ);
   }
};

struct LevelParser : boost::spirit::symbols<unsigned int>
{
   LevelParser()
   {
      add ("error", iofwdutil::zlog::ERROR)
          ("critical", iofwdutil::zlog::CRITICAL)
          ("warn", iofwdutil::zlog::WARN)
          ("info", iofwdutil::zlog::INFO)
          ("debug_more", iofwdutil::zlog::DEBUG_MORE)
          ("debug_extreme", iofwdutil::zlog::DEBUG_EXTREME)
          ("debug", iofwdutil::zlog::DEBUG);
   }
};


class LevelGrammar : public grammar<LevelGrammar>
{
public:
   LevelGrammar (MaskType & levels)
      : levels_(levels)
   {
   }

   struct level_closure : boost::spirit::closure<level_closure,unsigned int>
   {
      member1 value;
   };


   struct compentry_closure :
      boost::spirit::closure<compentry_closure,int,unsigned int>
   {
      member1 comptype;
      member2 limit; 
   }; 
      
   void enableLevel (unsigned int level) const
   {
      if (level >= levels_.size ())
         throw ZLogException (str(boost::format ("Invalid level (%u)!") % level)); 
      
      levels_[level] = true; 
   }

   static bool eqfunc (unsigned int l1, unsigned int l2) { return l1 == l2; } 
   static bool ltfunc (unsigned int l1, unsigned int l2) { return l1 < l2; } 
   static bool gtfunc (unsigned int l1, unsigned int l2) { return l1 > l2; }
   static bool lteqfunc (unsigned int l1, unsigned int l2) { return l1 <= l2; } 
   static bool gteqfunc (unsigned int l1, unsigned int l2) { return l1 >= l2; } 

   void handleCompEntry (int comptype, unsigned int limit) const
   {
      if (limit >= levels_.size ())
         throw ZLogException (str(boost::format ("Invalid level (%u)!") % limit)); 

      bool (*func) (unsigned int, unsigned int);
      switch (comptype)
      {
         case LT: func = &ltfunc; break;
         case GT: func = &gtfunc; break;
         case LTEQ: func = &lteqfunc; break;
         case GTEQ: func = &gteqfunc; break;
         case EQ: func = &eqfunc; break; 
         default:
                  ALWAYS_ASSERT(false && "Added operator type??"); 
      }
      for (unsigned int i=0; i<levels_.size(); ++i)
      {
         if (func (i, limit)) 
            enableLevel (i);
      }
   }

   template <typename ScannerT>
   struct definition
   {

      definition(const LevelGrammar & self)  
         : self_(self)
      { 
         /* define grammar rules here */ 
         level = uint_p[level.value = arg1] | as_lower_d[level_p[level.value = arg1]];
         compentry = (compop_p[compentry.comptype=arg1]
               >> level[compentry.limit=arg1])
               [bind(&LevelGrammar::handleCompEntry)(self,compentry.comptype,compentry.limit)];
         /*levellist = level[bind(&LevelGrammar::enableLevel)(self,arg1)] 
            >> *( ',' >> level[bind(&LevelGrammar::enableLevel)(self,arg1)]);
            */
         entry = compentry | level[bind(&LevelGrammar::enableLevel)(self,arg1)];
         entrylist = entry >> *( ',' >> entry ); 

      }


      const rule<ScannerT> & start() const 
      { 
         return entrylist; 
      }

      rule<ScannerT,level_closure::context_t> level; 
      rule<ScannerT,compentry_closure::context_t> compentry; 
      rule<ScannerT> levellist,entry,entrylist; 
      const LevelGrammar & self_; 
      const LevelParser level_p;
      const OpParser compop_p; 

   };

protected:
   MaskType & levels_; 

}; 


}


LevelMatch::LevelMatch (const char * stri)
{
   std::fill(levelMask_.begin(), levelMask_.end(),false); 

   LevelGrammar e (levelMask_); 

   parse_info<> info;

   try
   {
      info = parse(stri, e, space_p); 
   }
   catch (ZLogException & e)
   {
      e.pushMsg ("in '" + std::string(stri) + "'!\n"); 
      throw e; 
   }

   if (!info.full)
   {
      throw ZLogException (str(boost::format 
               ("Could not parse '%s': error at: '%s'!\n") 
               % stri % info.stop)); 
   }
}

//===========================================================================

   }
}
