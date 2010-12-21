#ifndef SRC_IOFWD_BASERANGESET_HH
#define SRC_IOFWD_BASERANGESET_HH

#include "iofwd/Range.hh"

#include "iofwdutil/FactoryClient.hh"

/*
 * abstract RangeSet class
 */
namespace iofwd
{

class BaseRangeSet
{
    public:
        /* for the factory */
        FACTORY_CONSTRUCTOR_PARAMS();

        BaseRangeSet()
        {
        }

        virtual ~BaseRangeSet()
        {
        }

       
        /* add / merge a range */ 
        virtual void add(iofwd::ChildRange * r) = 0;

        /* get a range from the merger */
        virtual void get(iofwd::ChildRange ** r) = 0;

        /* is the range set empty ? */
        virtual bool empty() const = 0;
};

} /* namespace iofwd */

#endif
