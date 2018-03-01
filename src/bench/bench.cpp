// Copyright (c) 2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "bench.h"

#include <iostream>
#include <sys/time.h>

using namespace benchmark;

std::map<std::string, BenchFunction> BenchRunner::benchmarks;

static double gettimedouble(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_usec * 0.000001 + tv.tv_sec;
}

BenchRunner::BenchRunner(std::string name, BenchFunction func)
{
    benchmarks.insert(std::make_pair(name, func));
}

void
BenchRunner::RunAll(double elapsedTIMECCoinForOne)
{
    std::cout << "Benchmark" << "," << "count" << "," << "min" << "," << "max" << "," << "average" << "\n";

    for (std::map<std::string,BenchFunction>::iterator it = benchmarks.begin();
         it != benchmarks.end(); ++it) {

        State state(it->first, elapsedTIMECCoinForOne);
        BenchFunction& func = it->second;
        func(state);
    }
}

bool State::KeepRunning()
{
    double now;
    if (count == 0) {
        beginTIMECCoin = now = gettimedouble();
    }
    else {
        // timeCheckCount is used to avoid calling gettime most of the time,
        // so benchmarks that run very quickly get consistent results.
        if ((count+1)%timeCheckCount != 0) {
            ++count;
            return true; // keep going
        }
        now = gettimedouble();
        double elapsedOne = (now - lastTIMECCoin)/timeCheckCount;
        if (elapsedOne < minTIMECCoin) minTIMECCoin = elapsedOne;
        if (elapsedOne > maxTIMECCoin) maxTIMECCoin = elapsedOne;
        if (elapsedOne*timeCheckCount < maxElapsed/16) timeCheckCount *= 2;
    }
    lastTIMECCoin = now;
    ++count;

    if (now - beginTIMECCoin < maxElapsed) return true; // Keep going

    --count;

    // Output results
    double average = (now-beginTIMECCoin)/count;
    std::cout << name << "," << count << "," << minTIMECCoin << "," << maxTIMECCoin << "," << average << "\n";

    return false;
}
