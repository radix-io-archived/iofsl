#ifndef SPATIAL_RANGESET_HH
#define SPATIAL_RANGESET_HH

namespace spatial
{
//===========================================================================

/**
 * One dimensional rangeset that allows marking ranges
 * (and checking if two rangesets overlap, ...)
 */
class RangeSet
{
public:
   RangeSet (); 


protected:

}; 

//===========================================================================

// Note: might be useful to return ovelrapping ranges to avoid recalculating
// the info afterwards
bool checkOverlap (const RangeSet & r1, const RangeSet & r2); 

//===========================================================================
}

#endif
