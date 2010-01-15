
#include "iofwdutil/boost-spirit.hh"


#include <boost/lambda/lambda.hpp>
#include <boost/format.hpp>
#include <algorithm>

#include "iofwdutil/zlog/ZLog.hh"
#include "iofwdutil/zlog/ZLogException.hh"
#include "iofwdutil/assert.hh"

#include "LevelMatch.hh"
#include "LevelParser.hh"

using namespace iofwdutil::zlog;

using namespace ourspirit;
using namespace ourphoenix;


namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

typedef LevelMatch::MaskType MaskType; 

namespace
{

   
typedef enum { LT = 0, GT, LTEQ, GTEQ, EQ } CompTypes;

struct OpParser : ourspirit::symbols<int>
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

/*
struct LevelParser : ourspirit::symbols<unsigned int>
{
   LevelParser()
   {
      for (unsigned int i=0; i<MAX_LEVEL; ++i)
      {
         add (ZLog::getLevelName (i), i); 
      }
   }
}; */


class LevelGrammar : public grammar<LevelGrammar>
{
public:
   LevelGrammar (MaskType & levels)
      : levels_(levels)
   {
   }

   struct level_closure : ourspirit::closure<level_closure,unsigned int>
   {
      member1 value;
   };


   struct compentry_closure :
      ourspirit::closure<compentry_closure,int,unsigned int>
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

      bool (*func) (unsigned int, unsigned int) = 0;
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
         : self_(self), level_p()
      { 
         /* define grammar rules here */ 
         //level = uint_p[level.value = arg1] | as_lower_d[level_p[level.value = arg1]];
         compentry = (compop_p[compentry.comptype=arg1]
               >> level_p[compentry.limit=arg1])
               [bind(&LevelGrammar::handleCompEntry)(self,compentry.comptype,compentry.limit)];
         /*levellist = level[bind(&LevelGrammar::enableLevel)(self,arg1)] 
            >> *( ',' >> level[bind(&LevelGrammar::enableLevel)(self,arg1)]);
            */
         entry = compentry | level_p[bind(&LevelGrammar::enableLevel)(self,arg1)];
         entrylist = entry >> *( ',' >> entry ); 

      }


      const rule<ScannerT> & start() const 
      { 
         return entrylist; 
      }

      //rule<ScannerT,level_closure::context_t> level; 
      rule<ScannerT,compentry_closure::context_t> compentry; 
      rule<ScannerT> levellist,entry,entrylist; 
      const LevelGrammar & self_; 
      //const LevelParser level_p;
      const OpParser compop_p; 
      const LevelParser level_p; 

   };

protected:
   MaskType & levels_; 
}; 


}


LevelMatch::LevelMatch (const char * stri) 
   : ready_(false)
{
   setup (stri); 
}

LevelMatch::LevelMatch ()
   : ready_(false)
{
}

void LevelMatch::allowAll ()
{
   std::fill(levelMask_.begin(), levelMask_.end(), true); 
}

void LevelMatch::setup (const char * stri)
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

   ready_ = true; 
}

//===========================================================================

   }
}
