// Copyright (c) 2014-2017 The TIMECCoin Core developers

#include "governance.h"

#include "test/test_time.h"

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(ratecheck_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(ratecheck_test)
{
    CRateCheckBuffer buffer;

    BOOST_CHECK(buffer.GetCount() == 0);
    BOOST_CHECK(buffer.GetMinTIMECCoinstamp() == numeric_limits<int64_t>::max());
    BOOST_CHECK(buffer.GetMaxTIMECCoinstamp() == 0);
    BOOST_CHECK(buffer.GetRate() == 0.0);

    buffer.AddTIMECCoinstamp(1);

    std::cout << "buffer.GetMinTIMECCoinstamp() = " << buffer.GetMinTIMECCoinstamp() << std::endl;

    BOOST_CHECK(buffer.GetCount() == 1);
    BOOST_CHECK(buffer.GetMinTIMECCoinstamp() == 1);
    BOOST_CHECK(buffer.GetMaxTIMECCoinstamp() == 1);
    BOOST_CHECK(buffer.GetRate() == 0.0);

    buffer.AddTIMECCoinstamp(2);
    BOOST_CHECK(buffer.GetCount() == 2);
    BOOST_CHECK(buffer.GetMinTIMECCoinstamp() == 1);
    BOOST_CHECK(buffer.GetMaxTIMECCoinstamp() == 2);
    //BOOST_CHECK(fabs(buffer.GetRate() - 2.0) < 1.0e-9);
    BOOST_CHECK(buffer.GetRate() == 0.0);

    buffer.AddTIMECCoinstamp(3);
    BOOST_CHECK(buffer.GetCount() == 3);
    BOOST_CHECK(buffer.GetMinTIMECCoinstamp() == 1);
    BOOST_CHECK(buffer.GetMaxTIMECCoinstamp() == 3);

    int64_t nMin = buffer.GetMinTIMECCoinstamp();
    int64_t nMax = buffer.GetMaxTIMECCoinstamp();
    double dRate = buffer.GetRate();

    std::cout << "buffer.GetCount() = " << buffer.GetCount() << std::endl;
    std::cout << "nMin = " << nMin << std::endl;
    std::cout << "nMax = " << nMax << std::endl;
    std::cout << "buffer.GetRate() = " << dRate << std::endl;

    //BOOST_CHECK(fabs(buffer.GetRate() - (3.0/2.0)) < 1.0e-9);
    BOOST_CHECK(buffer.GetRate() == 0.0);

    buffer.AddTIMECCoinstamp(4);
    BOOST_CHECK(buffer.GetCount() == 4);
    BOOST_CHECK(buffer.GetMinTIMECCoinstamp() == 1);
    BOOST_CHECK(buffer.GetMaxTIMECCoinstamp() == 4);
    //BOOST_CHECK(fabs(buffer.GetRate() - (4.0/3.0)) < 1.0e-9);
    BOOST_CHECK(buffer.GetRate() == 0.0);

    buffer.AddTIMECCoinstamp(5);
    BOOST_CHECK(buffer.GetCount() == 5);
    BOOST_CHECK(buffer.GetMinTIMECCoinstamp() == 1);
    BOOST_CHECK(buffer.GetMaxTIMECCoinstamp() == 5);
    BOOST_CHECK(fabs(buffer.GetRate() - (5.0/4.0)) < 1.0e-9);

    buffer.AddTIMECCoinstamp(6);
    BOOST_CHECK(buffer.GetCount() == 5);
    BOOST_CHECK(buffer.GetMinTIMECCoinstamp() == 2);
    BOOST_CHECK(buffer.GetMaxTIMECCoinstamp() == 6);
    BOOST_CHECK(fabs(buffer.GetRate() - (5.0/4.0)) < 1.0e-9);

    CRateCheckBuffer buffer2;

    std::cout << "Before loop tests" << std::endl;
    for(int64_t i = 1; i < 11; ++i)  {
        std::cout << "In loop: i = " << i << std::endl;
        buffer2.AddTIMECCoinstamp(i);
        BOOST_CHECK(buffer2.GetCount() == (i <= 5 ? i : 5));
        BOOST_CHECK(buffer2.GetMinTIMECCoinstamp() == max(int64_t(1), i - 4));
        BOOST_CHECK(buffer2.GetMaxTIMECCoinstamp() == i);
    }
}

BOOST_AUTO_TEST_SUITE_END()
