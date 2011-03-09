#include "iofwdutil/stats/CounterConfig.hh"

namespace iofwdutil
{
    namespace stats
    {

CounterConfig::CounterConfig() :
    log_(IOFWDLog::getSource("counterconfig")),
    all_mode_(false),
    all_mode_set_(false)
{
}

CounterConfig::CounterConfig(const CounterConfig & rhs) :
    counter_config_(rhs.counter_config_),
    log_(rhs.log_),
    all_mode_(rhs.all_mode_),
    all_mode_set_(rhs.all_mode_set_)
{
}

CounterConfig & CounterConfig::operator=(const CounterConfig & rhs)
{
    /* copy the rhs data */
    counter_config_ = rhs.counter_config_;
    all_mode_ = rhs.all_mode_;
    all_mode_set_ = rhs.all_mode_set_;

    /* return ref to this object */
    return (*this);
}

CounterConfig::~CounterConfig()
{
    counter_config_.clear();
}

bool const & counterEnabled(CounterConfig & cfg, std::string & key)
{
    return cfg.counter_config_[key];
}

    }
}
