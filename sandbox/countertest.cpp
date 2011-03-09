#include <iostream>

#include "iofwdutil/stats/ByteCounter.hh"
#include "iofwdutil/stats/ByteCounterWindow.hh"
#include "iofwdutil/stats/DoublePrecisionCounter.hh"
#include "iofwdutil/stats/DoublePrecisionCounterWindow.hh"
#include "iofwdutil/stats/IncCounter.hh"
#include "iofwdutil/stats/TimeCounter.hh"
#include "iofwdutil/stats/TimeCounterWindow.hh"
#include "iofwdutil/stats/ScopedTimer.hh"
#include "iofwdutil/stats/ScopedCounter.hh"
#include "iofwdutil/stats/CounterConfig.hh"

int main()
{
    size_t i = 0;

    iofwdutil::stats::CounterConfig::instance().configAllCounters(true);

    for(int j = 0 ; j < 10 ; j++)
    {
        for(i = 0 ; i < 1024 ; i++)
        {
            iofwdutil::stats::ByteCounter * b1 =
                iofwdutil::stats::ByteCounter::get("byte_test1");
            iofwdutil::stats::ByteCounter * b2 =
                iofwdutil::stats::ByteCounter::get("byte_test2");
            iofwdutil::stats::ByteCounter * b3 =
                iofwdutil::stats::ByteCounter::get("byte_test3");
            iofwdutil::stats::ByteCounter * b4 =
                iofwdutil::stats::ByteCounter::get("byte_test4");

            b1->update(i);
            b2->update(2 * i);
            b3->update(1024);
            b4->update(2048);
        }

        for(i = 0 ; i < 1024 ; i++)
        {
            iofwdutil::stats::ByteCounter * b1 =
                iofwdutil::stats::ByteCounter::get("byte_test5");
            iofwdutil::stats::ByteCounter * b2 =
                iofwdutil::stats::ByteCounter::get("byte_test6");
            iofwdutil::stats::ByteCounter * b3 =
                iofwdutil::stats::ByteCounter::get("byte_test7");
            iofwdutil::stats::ByteCounter * b4 =
                iofwdutil::stats::ByteCounter::get("byte_test8");

            b1->update(i);
            b2->update(2 * i);
            b3->update(1024);
            b4->update(2048);

            if(i == 1000)
            {
                std::cout << "Min: " << b2->counter_min() << std::endl;
                std::cout << "Max: " << b2->counter_max() << std::endl;
                std::cout << "Mean: " << b2->counter_mean() << std::endl;
                std::cout << "Var: " << b2->counter_variance() << std::endl;
                std::cout << "Count: " << b2->counter_count() << std::endl;
            }
        }

        for(i = 0 ; i < 1024 ; i++)
        {
            iofwdutil::stats::DoublePrecisionCounter * d1 =
                iofwdutil::stats::DoublePrecisionCounter::get("double_test5");
            iofwdutil::stats::DoublePrecisionCounter * d2 =
                iofwdutil::stats::DoublePrecisionCounter::get("double_test6");
            iofwdutil::stats::DoublePrecisionCounter * d3 =
                iofwdutil::stats::DoublePrecisionCounter::get("double_test7");
            iofwdutil::stats::DoublePrecisionCounter * d4 =
                iofwdutil::stats::DoublePrecisionCounter::get("double_test8");

            d1->update(i * 1.0);
            d2->update(2.0 * i);
            d3->update(i * 3.145678);
            d4->update(2048 * 3.145678);

            if(i == 1000)
            {
                std::cout << "Min: " << d3->counter_min() << std::endl;
                std::cout << "Max: " << d3->counter_max() << std::endl;
                std::cout << "Mean: " << d3->counter_mean() << std::endl;
                std::cout << "Var: " << d3->counter_variance() << std::endl;
                std::cout << "Count: " << d3->counter_count() << std::endl;
            }
        }

        for(i = 0 ; i < 1024 ; i++)
        {
            iofwdutil::stats::IncCounter * i1 =
                iofwdutil::stats::IncCounter::get("inc_test1");
            iofwdutil::stats::IncCounter * i2 =
                iofwdutil::stats::IncCounter::get("inc_test2");
            iofwdutil::stats::IncCounter * i3 =
                iofwdutil::stats::IncCounter::get("inc_test3");
            iofwdutil::stats::IncCounter * i4 =
                iofwdutil::stats::IncCounter::get("inc_test4");

            i1->update();
            i2->update();
            i3->update();
            i4->update();

            if(i == 1000)
            {
                std::cout << "Min: " << i1->counter_min() << std::endl;
                std::cout << "Max: " << i1->counter_max() << std::endl;
                std::cout << "Mean: " << i1->counter_mean() << std::endl;
                std::cout << "Var: " << i1->counter_variance() << std::endl;
                std::cout << "Count: " << i1->counter_count() << std::endl;
            }
        }

        for(i = 0 ; i < 1024 ; i++)
        {
            iofwdutil::stats::TimeCounter * t1 =
                iofwdutil::stats::TimeCounter::get("time_test1");
            iofwdutil::stats::TimeCounter * t2 =
                iofwdutil::stats::TimeCounter::get("time_test2");
            iofwdutil::stats::TimeCounter * t3 =
                iofwdutil::stats::TimeCounter::get("time_test3");
            iofwdutil::stats::TimeCounter * t4 =
                iofwdutil::stats::TimeCounter::get("time_test4");

            double ts1 = t1->start();
            double ts2 = t2->start();
            double ts3 = t3->start();
            double ts4 = t4->start();
            double te1 = t1->stop();
            double te2 = t2->stop();
            double te3 = t3->stop();
            double te4 = t4->stop();

            t1->update(te1 - ts1);
            t2->update(te2 - ts2);
            t3->update(te3 - ts3);
            t4->update(te4 - ts4);
        }

        for(i = 0 ; i < 1024 ; i++)
        {
            iofwdutil::stats::ScopedTimer s1("stest1");
            iofwdutil::stats::ScopedTimer s2("stest2");
            iofwdutil::stats::ScopedTimer s3("stest3");
        }

        for(i = 0 ; i < 1024 ; i++)
        {
            iofwdutil::stats::ScopedCounter<
                iofwdutil::stats::DoublePrecisionCounter,
                iofwdutil::stats::CounterConfigDefault > s1("sctest1", 3.124);
            iofwdutil::stats::ScopedCounter<
                iofwdutil::stats::IncCounter,
                iofwdutil::stats::CounterConfigDefault > s2("sctest2", 0);
            iofwdutil::stats::ScopedCounter<
                iofwdutil::stats::ByteCounter,
                iofwdutil::stats::CounterConfigDefault > s3("sctest3", 1024);
        }

        for(i = 0 ; i < 1010 ; i++)
        {
            iofwdutil::stats::ByteCounterWindow<17> * b1 =
                iofwdutil::stats::ByteCounterWindow<17>::get("bcw_test1");

            b1->update(i);
        }

        for(i = 0 ; i < 1010 ; i++)
        {
            iofwdutil::stats::TimeCounterWindow<9> * t1 =
                iofwdutil::stats::TimeCounterWindow<9>::get("tcw_test1");

            double ts = t1->start();
            double te = t1->stop();
            t1->update(te - ts);
        }

        for(i = 0 ; i < 1010 ; i++)
        {
            iofwdutil::stats::DoublePrecisionCounterWindow<17> * b1 =
                iofwdutil::stats::DoublePrecisionCounterWindow<17>::get("dpcw_test1");

            b1->update(i * 3.14);

            if(i == 1000)
            {
                std::cout << "Window Mean: " << b1->window_mean() << std::endl;
                std::cout << "Counter Mean: " << b1->counter_mean() << std::endl;
            }
        }
    }

    delete &iofwdutil::stats::CounterTable::instance();

    return 0;
}
