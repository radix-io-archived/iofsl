#ifndef SPATIAL_RANGE_HH
#define SPATIAL_RANGE_HH

#include "Point.hh"

namespace spatial
{
//===========================================================================

template <int DIM> 
class Range
{
public:
   typedef Point<DIM> PointType; 

   Range (const PointType & p1, PointType & p2);

protected:
   PointType points_[2]; 
}; 


template <int N>
void intersect (const Range<N> & r1, const Range<N> & r2, Range<N> dest)
{
}


template <int N>
bool checkOverlap (const Range<N> & r1, const Range<N> & r2)
{
   return false;
}


//===========================================================================
}

#endif
