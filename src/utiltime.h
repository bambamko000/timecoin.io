// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_UTILTIMEC_H
#define BITCOIN_UTILTIMEC_H

#include <stdint.h>
#include <string>

/**
 * GetTIMECCoinMicros() and GetTIMECCoinMillis() both return the system time, but in
 * different units. GetTIMECCoin() returns the sytem time in seconds, but also
 * supports mocktime, where the time can be specified by the user, eg for
 * testing (eg with the setmocktime rpc, or -mocktime argument).
 *
 * TODO: Rework these functions to be type-safe (so that we don't inadvertently
 * compare numbers with different units, or compare a mocktime to system time).
 */

int64_t GetTIMECCoin();
int64_t GetTIMECCoinMillis();
int64_t GetTIMECCoinMicros();
int64_t GetSystemTIMECCoinInSeconds(); // Like GetTIMECCoin(), but not mockable
int64_t GetLogTIMECCoinMicros();
void SetMockTIMECCoin(int64_t nMockTIMECCoinIn);
void MilliSleep(int64_t n);

std::string DateTIMECCoinStrFormat(const char* pszFormat, int64_t nTIMECCoin);
std::string DurationToDHMS(int64_t nDurationTIMECCoin);

#endif // BITCOIN_UTILTIMEC_H
