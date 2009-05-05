#include <iostream>

#include <boost/spirit/core.hpp>
#include <boost/spirit/symbols.hpp>
#include <boost/spirit/attribute.hpp>

#include "boost/spirit/phoenix/binders.hpp"

#include "boost/lambda/lambda.hpp"

#include "iofwdutil/zlog/ZLog.hh"
#include "iofwdutil/zlog/ZLog.hh"
#include "iofwdutil/assert.hh"

using namespace iofwdutil::zlog;
using namespace std; 

using namespace boost::spirit;
using namespace phoenix; 

class handlecomp 
{
public:
   handlecomp (std::vector<bool> & v)
      : v_(v)
   {
   }

   template <typename ITER>
   void operator () (ITER i1, ITER i2) const
   {
   }

protected:
   std::vector<bool> & v_; 

   int value_; 

}; 

   
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
      add ("ERROR", iofwdutil::zlog::ERROR)
          ("CRITICAL", iofwdutil::zlog::CRITICAL)
          ("WARN", iofwdutil::zlog::WARN)
          ("INFO", iofwdutil::zlog::INFO)
          ("DEBUG_MORE", iofwdutil::zlog::DEBUG_MORE)
          ("DEBUG_EXTREME", iofwdutil::zlog::DEBUG_EXTREME)
          ("DEBUG", iofwdutil::zlog::DEBUG);
   }
};


class LevelGrammar : public grammar<LevelGrammar>
{
public:
   LevelGrammar (std::vector<bool> & levels)
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
      cout << "Enabling level " << level << endl; 
      ALWAYS_ASSERT(level < levels_.size()); 
      levels_[level] = true; 
   }

   static bool eqfunc (unsigned int l1, unsigned int l2) { return l1 == l2; } 
   static bool ltfunc (unsigned int l1, unsigned int l2) { return l1 < l2; } 
   static bool gtfunc (unsigned int l1, unsigned int l2) { return l1 > l2; }
   static bool lteqfunc (unsigned int l1, unsigned int l2) { return l1 <= l2; } 
   static bool gteqfunc (unsigned int l1, unsigned int l2) { return l1 >= l2; } 

   void handleCompEntry (int comptype, unsigned int limit) const
   {
      cout << "Compentry: " << comptype << " " << limit << endl; 
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
         level = uint_p[level.value = arg1] | level_p[level.value = arg1];
         compentry = (compop_p[compentry.comptype=arg1]
               >> level[compentry.limit=arg1])
               [bind(&LevelGrammar::handleCompEntry)(self,compentry.comptype,compentry.limit)];
         levellist = level[bind(&LevelGrammar::enableLevel)(self,arg1)] 
            >> *( ',' >> level[bind(&LevelGrammar::enableLevel)(self,arg1)]); 
         entry = compentry | levellist; 
         entrylist = as_lower_d[entry >> *( '|' >> entry )]; 

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
      std::vector<bool> & levels_; 

}; 

bool parseStr (const char * str, std::vector<bool> & vec)
{
   vec.resize (MAX_LEVEL, false); 

   LevelGrammar e (vec); 

   parse_info<> info = parse(str, e, space_p); 

   if (!info.full)
   {
      cerr << "Error parsing: parsed up to character " << info.length << endl; 
   }

   return info.full; 


}

int main (int argc, char ** args)
{
   std::vector<bool> v; 

   if (argc != 2)
   {
      cerr << "Need regexp!\n";
      return 1; 
   }

   if (!parseStr (args[1], v))
   {
      cerr << "Could not parse '" << args[1] << "'!\n"; 
      return 1; 
   }
   for (unsigned int i=0; i<v.size(); ++i)
   {
      cout << i << ": " << (v[i] ? "active" : "disabled") << endl;
   }
   return 0;
}
