// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CONSENSUS_CONSENSUS_H
#define BITCOIN_CONSENSUS_CONSENSUS_H

/** The maximum allowed size for a serialized block, in bytes (network rule) */
static const unsigned int MAX_LEGACY_BLOCK_SIZE = 2000000;
static const unsigned int MAX_DIP0001_BLOCK_SIZE = 2000000;
inline unsigned int MaxBlockSize(bool fDIP0001Active /*= false */)
{
    return fDIP0001Active ? MAX_DIP0001_BLOCK_SIZE : MAX_LEGACY_BLOCK_SIZE;
}
/** The maximum allowed number of signature check operations in a block (network rule) */
inline unsigned int MaxBlockSigOps(bool fDIP0001Active /*= false */)
{
    return MaxBlockSize(fDIP0001Active) / 50;
}
/** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
static const int COINBASE_MATURITY = 23;

/** Flags for nSequence and nLockTIMECCoin locks */
enum {
    /* Interpret sequence numbers as relative lock-time constraints. */
    LOCKTIMEC_VERIFY_SEQUENCE = (1 << 0),

    /* Use GetMedianTIMECCoinPast() instead of nTIMECCoin for end point timestamp. */
    LOCKTIMEC_MEDIAN_TIMEC_PAST = (1 << 1),
};

#endif // BITCOIN_CONSENSUS_CONSENSUS_H
