#include "iofwdutil/stats/CounterTable.hh"

namespace iofwdutil
{
    namespace stats
    {
        CounterTable::CounterTable() : log_(IOFWDLog::getSource ("countertable"))
        {
        }

        CounterTable::~CounterTable()
        {
            dumpCounters();
        }

        void CounterTable::dumpCounters()
        {
            /* cleanup counters */
            std::map<std::string, CounterData *>::iterator it;

            /* for each counter, delete the counter mem */
            for(it = counters_.begin() ; it != counters_.end() ; it++)
            {
                std::string fmt = std::string(it->second->fmt_.c_str());
                std::string fmtline;

                /* generate the format line */
                fmtline = std::string("%s ") + std::string(fmt);

                /* based on the format, cast the counter value to the correct type and print the counter value */
                if(strcmp(fmt.c_str(), "%f") == 0)
                {
                    ZLOG_INFO(log_, format(fmtline.c_str()) % it->first.c_str() % *(static_cast<double *>(it->second->mem_)));
                }
                else if(strcmp(fmt.c_str(), "%d") == 0 || strcmp(fmt.c_str(), "%i") == 0)
                {
                    ZLOG_INFO(log_, format(fmtline.c_str()) % it->first.c_str() % *(static_cast<int *>(it->second->mem_)));
                }
                else if(strcmp(fmt.c_str(), "%ld") == 0 || strcmp(fmt.c_str(), "%li") == 0)
                {
                    ZLOG_INFO(log_, format(fmtline.c_str()) % it->first.c_str() % *(static_cast<long int *>(it->second->mem_)));
                }
                else if(strcmp(fmt.c_str(), "%lld") == 0 || strcmp(fmt.c_str(), "%lli") == 0)
                {
                    ZLOG_INFO(log_, format(fmtline.c_str()) % it->first.c_str() % *(static_cast<long long int *>(it->second->mem_)));
                }
                else if(strcmp(fmt.c_str(), "%u") == 0)
                {
                    ZLOG_INFO(log_, format(fmtline.c_str()) % it->first.c_str() % *(static_cast<unsigned *>(it->second->mem_)));
                }
                else if(strcmp(fmt.c_str(), "%lu") == 0)
                {
                    ZLOG_INFO(log_, format(fmtline.c_str()) % it->first.c_str() % *(static_cast<long unsigned *>(it->second->mem_)));
                }
                else if(strcmp(fmt.c_str(), "%llu") == 0)
                {
                    ZLOG_INFO(log_, format(fmtline.c_str()) % it->first.c_str() % *(static_cast<long long unsigned *>(it->second->mem_)));
                }
            }
        }
    }
}
