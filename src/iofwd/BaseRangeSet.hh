#ifndef __SRC_IOFWD_BASERANGESET_HH__
#define __SRC_IOFWD_BASERANGESET_HH__

#include <boost/mpl/list.hpp>

#include "iofwd/Range.hh"

/*
 * abstract RangeSet class
 */
namespace iofwd
{

class BaseRangeSet
{
    public:
        /* for the factory */
        typedef boost::mpl::list<> FACTORY_SIGNATURE;

        BaseRangeSet()
        {
        }

        ~BaseRangeSet()
        {
        }

       
        /* add / merge a range */ 
        virtual void add(const iofwd::ChildRange * r) = 0;

        /* get a range from the merger */
        virtual void get(iofwd::ChildRange ** r) = 0;

        /* is the range set empty ? */
        virtual bool empty() const = 0;
};

} /* namespace iofwd */

#endif
