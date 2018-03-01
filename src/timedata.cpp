// Copyright (c) 2014-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "timedata.h"

#include "netaddress.h"
#include "sync.h"
#include "ui_interface.h"
#include "util.h"
#include "utilstrencodings.h"

#include <boost/foreach.hpp>

using namespace std;

static CCriticalSection cs_nTIMECCoinOffset;
static int64_t nTIMECCoinOffset = 0;

/**
 * "Never go to sea with two chronometers; take one or three."
 * Our three time sources are:
 *  - System clock
 *  - Median of other nodes clocks
 *  - The user (asking the user to fix the system clock if the first two disagree)
 */
int64_t GetTIMECCoinOffset()
{
    LOCK(cs_nTIMECCoinOffset);
    return nTIMECCoinOffset;
}

int64_t GetAdjustedTIMECCoin()
{
    return GetTIMECCoin() + GetTIMECCoinOffset();
}

static int64_t abs64(int64_t n)
{
    return (n >= 0 ? n : -n);
}

#define BITCOIN_TIMECDATA_MAX_SAMPLES 200

void AddTIMECCoinData(const CNetAddr& ip, int64_t nOffsetSample)
{
    LOCK(cs_nTIMECCoinOffset);
    // Ignore duplicates
    static set<CNetAddr> setKnown;
    if (setKnown.size() == BITCOIN_TIMECDATA_MAX_SAMPLES)
        return;
    if (!setKnown.insert(ip).second)
        return;

    // Add data
    static CMedianFilter<int64_t> vTIMECCoinOffsets(BITCOIN_TIMECDATA_MAX_SAMPLES, 0);
    vTIMECCoinOffsets.input(nOffsetSample);
    LogPrint("net","added time data, samples %d, offset %+d (%+d minutes)\n", vTIMECCoinOffsets.size(), nOffsetSample, nOffsetSample/60);

    // There is a known issue here (see issue #4521):
    //
    // - The structure vTIMECCoinOffsets contains up to 200 elements, after which
    // any new element added to it will not increase its size, replacing the
    // oldest element.
    //
    // - The condition to update nTIMECCoinOffset includes checking whether the
    // number of elements in vTIMECCoinOffsets is odd, which will never happen after
    // there are 200 elements.
    //
    // But in this case the 'bug' is protective against some attacks, and may
    // actually explain why we've never seen attacks which manipulate the
    // clock offset.
    //
    // So we should hold off on fixing this and clean it up as part of
    // a timing cleanup that strengthens it in a number of other ways.
    //
    if (vTIMECCoinOffsets.size() >= 5 && vTIMECCoinOffsets.size() % 2 == 1)
    {
        int64_t nMedian = vTIMECCoinOffsets.median();
        std::vector<int64_t> vSorted = vTIMECCoinOffsets.sorted();
        // Only let other nodes change our time by so much
        if (abs64(nMedian) < 70 * 60)
        {
            nTIMECCoinOffset = nMedian;
        }
        else
        {
            nTIMECCoinOffset = 0;

            static bool fDone;
            if (!fDone)
            {
                // If nobody has a time different than ours but within 5 minutes of ours, give a warning
                bool fMatch = false;
                BOOST_FOREACH(int64_t nOffset, vSorted)
                    if (nOffset != 0 && abs64(nOffset) < 5 * 60)
                        fMatch = true;

                if (!fMatch)
                {
                    fDone = true;
                    string strMessage = _("Please check that your computer's date and time are correct! If your clock is wrong TIMECCoin Core will not work properly.");
                    strMiscWarning = strMessage;
                    uiInterface.ThreadSafeMessageBox(strMessage, "", CClientUIInterface::MSG_WARNING);
                }
            }
        }
        
        BOOST_FOREACH(int64_t n, vSorted)
            LogPrint("net", "%+d  ", n);
        LogPrint("net", "|  ");
        
        LogPrint("net", "nTIMECCoinOffset = %+d  (%+d minutes)\n", nTIMECCoinOffset, nTIMECCoinOffset/60);
    }
}
