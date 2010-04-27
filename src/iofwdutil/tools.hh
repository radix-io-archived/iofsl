#ifndef IOFWDUTIL_TOOLS_HH
#define IOFWDUTIL_TOOLS_HH


#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

/* UNUSED for now 
// Class declarations
template<typename T1, typename T2, bool C>
struct TP_IF { };

template <typename T1, typename T2>
struct TP_IF<T1,T2,true> {
   typedef T1 value; 
};

template <typename T1, typename T2>
struct TP_IF<T1,T2, false> {
   typedef T2 value; 
};

template <typename T1, typename T2>
struct TYPE_EQUAL 
{
   enum { value = false }; 
};

template <typename T1>
struct TYPE_EQUAL<T1,T1>
{
   enum { value = true }; 
};
*/
#endif
