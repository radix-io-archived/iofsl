#ifndef SPATIAL_POINT_HH
#define SPATIAL_POINT_HH

#include <inttypes.h>

namespace spatial
{
//===========================================================================

/// Basic offset type (signed integral in our case)
typedef int64_t ctype;  

template <int DIM>
class Point
{
public:
   inline Point ();

protected:
   ctype coordinate_ [DIM]; 
};
//===========================================================================

template <int DIM> 
inline Point<DIM>::Point ()
{
   // If debug mode, initialize coordinate to 0
#ifndef NDEBUG
   memset (&coordinate_[0], 0, sizeof (coordinate_)); 
#endif
}


//===========================================================================
}



#endif
