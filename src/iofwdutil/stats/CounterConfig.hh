#ifndef IOFWDUTIL_STATS_COUNTERCONFIG_HH
#define IOFWDUTIL_STATS_COUNTERCONFIG_HH

#include <string>
#include <map>
#include <iostream>
#include "iofwdutil/Singleton.hh"
#include <boost/thread/mutex.hpp>
#include <boost/algorithm/string.hpp>
#include "iofwdutil/ConfigFile.hh"
#include "iofwdutil/IOFWDLog.hh"

namespace iofwdutil
{
    namespace stats
    {

class CounterConfig : public iofwdutil::Singleton< CounterConfig >
{
    public:
        void configAllCounters(bool all_mode)
        {
            all_mode_ = all_mode;
            all_mode_set_ = true;
        }

        void parseConfig(iofwdutil::ConfigFile config_section)
        {
            /* get the config for the counters */
            parseLine(std::string("sm"), config_section);
            parseLine(std::string("request"), config_section);
            parseLine(std::string("bmi"), config_section);
            parseLine(std::string("zoidfsapi"), config_section);

            all_mode_set_ = false;
        }

    protected:
        /* only counters can access the counter Config */
        friend class BaseCounter;

        friend bool const & counterEnabled(std::string & key);

        void enable(std::string & key)
        {
            counter_config_[key] = true;
        }

        bool const & operator[] (std::string & name)
        {
            if(all_mode_set_)
                return all_mode_;

            return counter_config_[name];
        }

        void parseLine(std::string config_key, iofwdutil::ConfigFile &
                config_section)
        {
            std::string current_line("");
            std::vector<std::string> config_tokens;
            std::vector<std::string>::iterator i;

            ZLOG_INFO(log_, std::string("Counter config field: ") + config_key);

            /* grab the current line and split on the , */
            current_line = config_section.getKeyDefault(config_key.c_str(),
                    std::string(""));
            boost::split(config_tokens, current_line, boost::is_any_of(","));

            /* if no counters specified, disable them all */
            if(i == config_tokens.end())
            {
                ZLOG_INFO(log_, std::string("\tCounters disabled"));
            }
            /* else, enable the listed counters */
            else
            {
                /* for each counter name, enable it */
                for(i = config_tokens.begin() ; i != config_tokens.end() ; i++)
                {
                    enable((*i));
                    ZLOG_INFO(log_, std::string("\tEnable: ") + (*i));
                }
            } 
        }

    public:
        /* constructors, destructors, and assign ops */
        CounterConfig(): log_ (iofwdutil::IOFWDLog::getSource ("counters")) {};
        CounterConfig(const CounterConfig & rhs);
        CounterConfig & operator= (const CounterConfig & rhs);
        ~CounterConfig();

        std::map<std::string, bool> counter_config_;

        iofwdutil::IOFWDLogSource & log_;

        bool all_mode_;
        bool all_mode_set_;
};

bool const & counterEnabled(CounterConfig & cfg,
        std::string & key);

    }
}

#endif
