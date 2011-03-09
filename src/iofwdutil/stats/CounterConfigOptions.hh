#ifndef IOFWDUTIL_STATS_COUNTERCONFIGOPTIONS_HH
#define IOFWDUTIL_STATS_COUNTERCONFIGOPTIONS_HH

#include "iofwdutil/stats/CounterConfig.hh"
#include "iofwdutil/stats/BaseCounter.hh"

namespace iofwdutil
{
    namespace stats
    {
        /* fwd decl */
        class BaseCounter;

class CounterConfigOptions
{
    public:
        virtual void operator()(void) = 0;

        virtual void reset(iofwdutil::stats::BaseCounter * counter)
        {
            counter_ = counter;
            (*this)();
        }

        CounterConfigOptions(iofwdutil::stats::BaseCounter * counter=NULL) :
            counter_(counter)
        {
            /* run the counter config */
        }

        virtual ~CounterConfigOptions()
        {
        }

    protected:
        iofwdutil::stats::BaseCounter * counter_;
};

class CounterConfigQuery : public CounterConfigOptions
{
    public:
        CounterConfigQuery(iofwdutil::stats::BaseCounter * counter=NULL)
            : CounterConfigOptions(counter)
        {
            (*this)();
        }

        ~CounterConfigQuery()
        {
        }

        virtual void operator()(void)
        {
            std::string & key = getCounterKey(counter_);
            bool enabled =
                counterEnabled(iofwdutil::stats::CounterConfig::instance(), key);

            configureCounter(counter_, enabled);
        }
};

class CounterConfigOverride : public CounterConfigOptions
{
    public:
        CounterConfigOverride(iofwdutil::stats::BaseCounter * counter=NULL,
                bool enabled=true) :
            CounterConfigOptions(counter),
            enabled_(enabled)
        {
            (*this)();
        }

        ~CounterConfigOverride()
        {
        }

        virtual void operator()(void)
        {
            configureCounter(counter_, enabled_);
        }

    protected:
        bool enabled_;
};

/* by default, counters are disabled */
class CounterConfigEnable : public CounterConfigOptions
{
    public:
        CounterConfigEnable(iofwdutil::stats::BaseCounter * counter=NULL) :
            CounterConfigOptions(counter)
        {
            (*this)();
        }

        ~CounterConfigEnable()
        {
        }

        virtual void operator()(void)
        {
            configureCounter(counter_, true);
        }
};

/* by default, counters are disabled */
class CounterConfigDisable : public CounterConfigOptions
{
    public:
        CounterConfigDisable(iofwdutil::stats::BaseCounter * counter=NULL) :
            CounterConfigOptions(counter)
        {
            (*this)();
        }

        ~CounterConfigDisable()
        {
        }

        virtual void operator()(void)
        {
            configureCounter(counter_, false);
        }
};

/* by default, counters are disabled */
class CounterConfigDefault : public CounterConfigOptions
{
    public:
        CounterConfigDefault(iofwdutil::stats::BaseCounter * counter=NULL) :
            CounterConfigOptions(counter)
        {
            (*this)();
        }

        ~CounterConfigDefault()
        {
        }

        virtual void operator()(void)
        {
            configureCounter(counter_, false);
        }
};
    }
}
#endif
