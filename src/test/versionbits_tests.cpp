// Copyright (c) 2014-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chain.h"
#include "random.h"
#include "versionbits.h"
#include "test/test_time.h"
#include "chainparams.h"
#include "validation.h"
#include "consensus/params.h"

#include <boost/test/unit_test.hpp>

/* Define a virtual block time, one block per 10 minutes after Nov 14 2014, 0:55:36am */
int32_t TestTIMECoin(int nHeight) { return 1415926536 + 600 * nHeight; }

static const Consensus::Params paramsDummy = Consensus::Params();

class TestConditionChecker : public AbstractThresholdConditionChecker
{
private:
    mutable ThresholdConditionCache cache;

public:
    int64_t BeginTIMECoin(const Consensus::Params& params) const { return TestTIMECoin(10000); }
    int64_t EndTIMECoin(const Consensus::Params& params) const { return TestTIMECoin(20000); }
    int Period(const Consensus::Params& params) const { return 1000; }
    int Threshold(const Consensus::Params& params) const { return 900; }
    bool Condition(const CBlockIndex* pindex, const Consensus::Params& params) const { return (pindex->nVersion & 0x100); }

    ThresholdState GetStateFor(const CBlockIndex* pindexPrev) const { return AbstractThresholdConditionChecker::GetStateFor(pindexPrev, paramsDummy, cache); }
};

#define CHECKERS 6

class VersionBitsTester
{
    // A fake blockchain
    std::vector<CBlockIndex*> vpblock;

    // 6 independent checkers for the same bit.
    // The first one performs all checks, the second only 50%, the third only 25%, etc...
    // This is to test whether lack of cached information leads to the same results.
    TestConditionChecker checker[CHECKERS];

    // Test counter (to identify failures)
    int num;

public:
    VersionBitsTester() : num(0) {}

    VersionBitsTester& Reset() {
        for (unsigned int i = 0; i < vpblock.size(); i++) {
            delete vpblock[i];
        }
        for (unsigned int  i = 0; i < CHECKERS; i++) {
            checker[i] = TestConditionChecker();
        }
        vpblock.clear();
        return *this;
    }

    ~VersionBitsTester() {
         Reset();
    }

    VersionBitsTester& Mine(unsigned int height, int32_t nTIMECoin, int32_t nVersion) {
        while (vpblock.size() < height) {
            CBlockIndex* pindex = new CBlockIndex();
            pindex->nHeight = vpblock.size();
            pindex->pprev = vpblock.size() > 0 ? vpblock.back() : NULL;
            pindex->nTIMECoin = nTIMECoin;
            pindex->nVersion = nVersion;
            pindex->BuildSkip();
            vpblock.push_back(pindex);
        }
        return *this;
    }

    VersionBitsTester& TestDefined() {
        for (int i = 0; i < CHECKERS; i++) {
            if ((insecure_rand() & ((1 << i) - 1)) == 0) {
                BOOST_CHECK_MESSAGE(checker[i].GetStateFor(vpblock.empty() ? NULL : vpblock.back()) == THRESHOLD_DEFINED, strprintf("Test %i for DEFINED", num));
            }
        }
        num++;
        return *this;
    }

    VersionBitsTester& TestStarted() {
        for (int i = 0; i < CHECKERS; i++) {
            if ((insecure_rand() & ((1 << i) - 1)) == 0) {
                BOOST_CHECK_MESSAGE(checker[i].GetStateFor(vpblock.empty() ? NULL : vpblock.back()) == THRESHOLD_STARTED, strprintf("Test %i for STARTED", num));
            }
        }
        num++;
        return *this;
    }

    VersionBitsTester& TestLockedIn() {
        for (int i = 0; i < CHECKERS; i++) {
            if ((insecure_rand() & ((1 << i) - 1)) == 0) {
                BOOST_CHECK_MESSAGE(checker[i].GetStateFor(vpblock.empty() ? NULL : vpblock.back()) == THRESHOLD_LOCKED_IN, strprintf("Test %i for LOCKED_IN", num));
            }
        }
        num++;
        return *this;
    }

    VersionBitsTester& TestActive() {
        for (int i = 0; i < CHECKERS; i++) {
            if ((insecure_rand() & ((1 << i) - 1)) == 0) {
                BOOST_CHECK_MESSAGE(checker[i].GetStateFor(vpblock.empty() ? NULL : vpblock.back()) == THRESHOLD_ACTIVE, strprintf("Test %i for ACTIVE", num));
            }
        }
        num++;
        return *this;
    }

    VersionBitsTester& TestFailed() {
        for (int i = 0; i < CHECKERS; i++) {
            if ((insecure_rand() & ((1 << i) - 1)) == 0) {
                BOOST_CHECK_MESSAGE(checker[i].GetStateFor(vpblock.empty() ? NULL : vpblock.back()) == THRESHOLD_FAILED, strprintf("Test %i for FAILED", num));
            }
        }
        num++;
        return *this;
    }

    CBlockIndex * Tip() { return vpblock.size() ? vpblock.back() : NULL; }
};

BOOST_FIXTURE_TEST_SUITE(versionbits_tests, TestingSetup)

BOOST_AUTO_TEST_CASE(versionbits_test)
{
    for (int i = 0; i < 64; i++) {
        BOOST_TEST_MESSAGE("versionbits_test " << i);

        // DEFINED -> FAILED
        VersionBitsTester().TestDefined()
                           .Mine(1, TestTIMECoin(1), 0x100).TestDefined()
                           .Mine(11, TestTIMECoin(11), 0x100).TestDefined()
                           .Mine(989, TestTIMECoin(989), 0x100).TestDefined()
                           .Mine(999, TestTIMECoin(20000), 0x100).TestDefined()
                           .Mine(1000, TestTIMECoin(20000), 0x100).TestFailed()
                           .Mine(1999, TestTIMECoin(30001), 0x100).TestFailed()
                           .Mine(2000, TestTIMECoin(30002), 0x100).TestFailed()
                           .Mine(2001, TestTIMECoin(30003), 0x100).TestFailed()
                           .Mine(2999, TestTIMECoin(30004), 0x100).TestFailed()
                           .Mine(3000, TestTIMECoin(30005), 0x100).TestFailed()

        // DEFINED -> STARTED -> FAILED
                           .Reset().TestDefined()
                           .Mine(1, TestTIMECoin(1), 0).TestDefined()
                           .Mine(1000, TestTIMECoin(10000) - 1, 0x100).TestDefined() // One second more and it would be defined
                           .Mine(2000, TestTIMECoin(10000), 0x100).TestStarted() // So that's what happens the next period
                           .Mine(2051, TestTIMECoin(10010), 0).TestStarted() // 51 old blocks
                           .Mine(2950, TestTIMECoin(10020), 0x100).TestStarted() // 899 new blocks
                           .Mine(3000, TestTIMECoin(20000), 0).TestFailed() // 50 old blocks (so 899 out of the past 1000)
                           .Mine(4000, TestTIMECoin(20010), 0x100).TestFailed()

        // DEFINED -> STARTED -> FAILED while threshold reached
                           .Reset().TestDefined()
                           .Mine(1, TestTIMECoin(1), 0).TestDefined()
                           .Mine(1000, TestTIMECoin(10000) - 1, 0x101).TestDefined() // One second more and it would be defined
                           .Mine(2000, TestTIMECoin(10000), 0x101).TestStarted() // So that's what happens the next period
                           .Mine(2999, TestTIMECoin(30000), 0x100).TestStarted() // 999 new blocks
                           .Mine(3000, TestTIMECoin(30000), 0x100).TestFailed() // 1 new block (so 1000 out of the past 1000 are new)
                           .Mine(3999, TestTIMECoin(30001), 0).TestFailed()
                           .Mine(4000, TestTIMECoin(30002), 0).TestFailed()
                           .Mine(14333, TestTIMECoin(30003), 0).TestFailed()
                           .Mine(24000, TestTIMECoin(40000), 0).TestFailed()

        // DEFINED -> STARTED -> LOCKEDIN at the last minute -> ACTIVE
                           .Reset().TestDefined()
                           .Mine(1, TestTIMECoin(1), 0).TestDefined()
                           .Mine(1000, TestTIMECoin(10000) - 1, 0x101).TestDefined() // One second more and it would be defined
                           .Mine(2000, TestTIMECoin(10000), 0x101).TestStarted() // So that's what happens the next period
                           .Mine(2050, TestTIMECoin(10010), 0x200).TestStarted() // 50 old blocks
                           .Mine(2950, TestTIMECoin(10020), 0x100).TestStarted() // 900 new blocks
                           .Mine(2999, TestTIMECoin(19999), 0x200).TestStarted() // 49 old blocks
                           .Mine(3000, TestTIMECoin(29999), 0x200).TestLockedIn() // 1 old block (so 900 out of the past 1000)
                           .Mine(3999, TestTIMECoin(30001), 0).TestLockedIn()
                           .Mine(4000, TestTIMECoin(30002), 0).TestActive()
                           .Mine(14333, TestTIMECoin(30003), 0).TestActive()
                           .Mine(24000, TestTIMECoin(40000), 0).TestActive();
    }

    // Sanity checks of version bit deployments
    const Consensus::Params &mainnetParams = Params(CBaseChainParams::MAIN).GetConsensus();
    for (int i=0; i<(int) Consensus::MAX_VERSION_BITS_DEPLOYMENTS; i++) {
        uint32_t bitmask = VersionBitsMask(mainnetParams, (Consensus::DeploymentPos)i);
        // Make sure that no deployment tries to set an invalid bit.
        BOOST_CHECK_EQUAL(bitmask & ~(uint32_t)VERSIONBITS_TOP_MASK, bitmask);

        // Verify that the deployment windows of different deployment using the
        // same bit are disjoint.
        // This test may need modification at such time as a new deployment
        // is proposed that reuses the bit of an activated soft fork, before the
        // end time of that soft fork.  (Alternatively, the end time of that
        // activated soft fork could be later changed to be earlier to avoid
        // overlap.)
        for (int j=i+1; j<(int) Consensus::MAX_VERSION_BITS_DEPLOYMENTS; j++) {
            if (VersionBitsMask(mainnetParams, (Consensus::DeploymentPos)j) == bitmask) {
                BOOST_CHECK(mainnetParams.vDeployments[j].nStartTIMECoin > mainnetParams.vDeployments[i].nTIMECoinout ||
                        mainnetParams.vDeployments[i].nStartTIMECoin > mainnetParams.vDeployments[j].nTIMECoinout);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(versionbits_computeblockversion)
{
    // Check that ComputeBlockVersion will set the appropriate bit correctly
    // on mainnet.
    const Consensus::Params &mainnetParams = Params(CBaseChainParams::MAIN).GetConsensus();

    // Use the TESTDUMMY deployment for testing purposes.
    int64_t bit = mainnetParams.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit;
    int64_t nStartTIMECoin = mainnetParams.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTIMECoin;
    int64_t nTIMECoinout = mainnetParams.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTIMECoinout;

    assert(nStartTIMECoin < nTIMECoinout);

    // In the first chain, test that the bit is set by CBV until it has failed.
    // In the second chain, test the bit is set by CBV while STARTED and
    // LOCKED-IN, and then no longer set while ACTIVE.
    VersionBitsTester firstChain, secondChain;

    // Start generating blocks before nStartTIMECoin
    int64_t nTIMECoin = nStartTIMECoin - 1;

    // Before MedianTIMECoinPast of the chain has crossed nStartTIMECoin, the bit
    // should not be set.
    CBlockIndex *lastBlock = NULL;
    lastBlock = firstChain.Mine(2016, nTIMECoin, VERSIONBITS_LAST_OLD_BLOCK_VERSION).Tip();
    BOOST_CHECK_EQUAL(ComputeBlockVersion(lastBlock, mainnetParams) & (1<<bit), 0);

    // Mine 2011 more blocks at the old time, and check that CBV isn't setting the bit yet.
    for (int i=1; i<2012; i++) {
        lastBlock = firstChain.Mine(2016+i, nTIMECoin, VERSIONBITS_LAST_OLD_BLOCK_VERSION).Tip();
        // This works because VERSIONBITS_LAST_OLD_BLOCK_VERSION happens
        // to be 4, and the bit we're testing happens to be bit 28.
        BOOST_CHECK_EQUAL(ComputeBlockVersion(lastBlock, mainnetParams) & (1<<bit), 0);
    }
    // Now mine 5 more blocks at the start time -- MTP should not have passed yet, so
    // CBV should still not yet set the bit.
    nTIMECoin = nStartTIMECoin;
    for (int i=2012; i<=2016; i++) {
        lastBlock = firstChain.Mine(2016+i, nTIMECoin, VERSIONBITS_LAST_OLD_BLOCK_VERSION).Tip();
        BOOST_CHECK_EQUAL(ComputeBlockVersion(lastBlock, mainnetParams) & (1<<bit), 0);
    }

    // Advance to the next period and transition to STARTED,
    lastBlock = firstChain.Mine(6048, nTIMECoin, VERSIONBITS_LAST_OLD_BLOCK_VERSION).Tip();
    // so ComputeBlockVersion should now set the bit,
    BOOST_CHECK((ComputeBlockVersion(lastBlock, mainnetParams) & (1<<bit)) != 0);
    // and should also be using the VERSIONBITS_TOP_BITS.
    BOOST_CHECK_EQUAL(ComputeBlockVersion(lastBlock, mainnetParams) & VERSIONBITS_TOP_MASK, VERSIONBITS_TOP_BITS);

    // Check that ComputeBlockVersion will set the bit until nTIMECoinout
    nTIMECoin += 600;
    int blocksToMine = 4032; // test blocks for up to 2 time periods
    int nHeight = 6048;
    // These blocks are all before nTIMECoinout is reached.
    while (nTIMECoin < nTIMECoinout && blocksToMine > 0) {
        lastBlock = firstChain.Mine(nHeight+1, nTIMECoin, VERSIONBITS_LAST_OLD_BLOCK_VERSION).Tip();
        BOOST_CHECK((ComputeBlockVersion(lastBlock, mainnetParams) & (1<<bit)) != 0);
        BOOST_CHECK_EQUAL(ComputeBlockVersion(lastBlock, mainnetParams) & VERSIONBITS_TOP_MASK, VERSIONBITS_TOP_BITS);
        blocksToMine--;
        nTIMECoin += 600;
        nHeight += 1;
    };

    nTIMECoin = nTIMECoinout;
    // FAILED is only triggered at the end of a period, so CBV should be setting
    // the bit until the period transition.
    for (int i=0; i<2015; i++) {
        lastBlock = firstChain.Mine(nHeight+1, nTIMECoin, VERSIONBITS_LAST_OLD_BLOCK_VERSION).Tip();
        BOOST_CHECK((ComputeBlockVersion(lastBlock, mainnetParams) & (1<<bit)) != 0);
        nHeight += 1;
    }
    // The next block should trigger no longer setting the bit.
    lastBlock = firstChain.Mine(nHeight+1, nTIMECoin, VERSIONBITS_LAST_OLD_BLOCK_VERSION).Tip();
    BOOST_CHECK_EQUAL(ComputeBlockVersion(lastBlock, mainnetParams) & (1<<bit), 0);

    // On a new chain:
    // verify that the bit will be set after lock-in, and then stop being set
    // after activation.
    nTIMECoin = nStartTIMECoin;

    // Mine one period worth of blocks, and check that the bit will be on for the
    // next period.
    lastBlock = secondChain.Mine(2016, nStartTIMECoin, VERSIONBITS_LAST_OLD_BLOCK_VERSION).Tip();
    BOOST_CHECK((ComputeBlockVersion(lastBlock, mainnetParams) & (1<<bit)) != 0);

    // Mine another period worth of blocks, signaling the new bit.
    lastBlock = secondChain.Mine(4032, nStartTIMECoin, VERSIONBITS_TOP_BITS | (1<<bit)).Tip();
    // After one period of setting the bit on each block, it should have locked in.
    // We keep setting the bit for one more period though, until activation.
    BOOST_CHECK((ComputeBlockVersion(lastBlock, mainnetParams) & (1<<bit)) != 0);

    // Now check that we keep mining the block until the end of this period, and
    // then stop at the beginning of the next period.
    lastBlock = secondChain.Mine(6047, nStartTIMECoin, VERSIONBITS_LAST_OLD_BLOCK_VERSION).Tip();
    BOOST_CHECK((ComputeBlockVersion(lastBlock, mainnetParams) & (1<<bit)) != 0);
    lastBlock = secondChain.Mine(6048, nStartTIMECoin, VERSIONBITS_LAST_OLD_BLOCK_VERSION).Tip();
    BOOST_CHECK_EQUAL(ComputeBlockVersion(lastBlock, mainnetParams) & (1<<bit), 0);

    // Finally, verify that after a soft fork has activated, CBV no longer uses
    // VERSIONBITS_LAST_OLD_BLOCK_VERSION.
    //BOOST_CHECK_EQUAL(ComputeBlockVersion(lastBlock, mainnetParams) & VERSIONBITS_TOP_MASK, VERSIONBITS_TOP_BITS);
}


BOOST_AUTO_TEST_SUITE_END()
