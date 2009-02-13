#ifndef IOFWDUTIL_TYPESTORAGE_HH
#define IOFWDUTIL_TYPESTORAGE_HH

#include <boost/typeof/typeof.hpp>

namespace iofwdutil
{

template <typename VALUETYPE, typename NEXT>
class TypeStorage
{
public:
   typedef NEXT NextType; 
   typedef VALUETYPE    ValueType; 
   typedef TypeStorage<VALUETYPE,NEXT> SelfType; 

   TypeStorage (VALUETYPE & cur, NEXT & next)
      : value_(cur), next_(next)
   { }

   template <typename NEW>
   TypeStorage<NEW, SelfType> operator << (const NEW & next)
   {
      return TypeStorage<NEW, SelfType> (const_cast<NEW&>(next), *this); 
   }

   template <typename NEW>
   TypeStorage<NEW, SelfType> operator , (const NEW & next)
   {
      return TypeStorage<NEW, SelfType> (next, *this); 
   }


   mutable VALUETYPE & value_;
   mutable NEXT & next_; 
}; 

class TypeStorage_EMPTY {}; 

//static EMPTY e; 

inline TypeStorage<TypeStorage_EMPTY, TypeStorage_EMPTY> TSStart ()
{ 
   static TypeStorage_EMPTY e; 
   return TypeStorage<TypeStorage_EMPTY, TypeStorage_EMPTY> (e,e); 
}


template <typename PROC>
inline void applyTypes (PROC & , const TypeStorage<TypeStorage_EMPTY,TypeStorage_EMPTY> & )
{
   /*  nothing todo */ 
}

template <typename PROC, typename TYPE>
inline void applyTypes (PROC & proc, const TYPE & t)
{
   applyTypes (proc, t.next_); 
   proc (t.value_); 
}




#define TSSTART \
   ::iofwdutil::TSStart()
#define TYPESTORAGE(varname,types) \
   BOOST_TYPEOF(TSSTART << types) varname (TSSTART << types);
#define APPLYTYPES(procname,typestorage) \
   applyTypes (procname, typestorage); 


}

#endif
