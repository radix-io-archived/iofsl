#ifndef CUTILS_TOOLS_H
#define CUTILS_TOOLS_H

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif /* UNUSED */

#define zfsmin(a,b) ((a)<(b) ? (a):(b))
#define zfsmax(a,b) ((a)>(b) ? (a):(b))

void always_assert_error (const char * expr, const char * file, int lineno);

#define ALWAYS_ASSERT(a) if (!(a)) always_assert_error(#a,__FILE__, __LINE__);

#define ARRAY_SIZEOF(a) (sizeof(a)/sizeof(a[0]))


#endif /* CUTILS_TOOLS_H */
