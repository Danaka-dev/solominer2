#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_REACTION_CORE_H
#define SOLOMINER_REACTION_CORE_H

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>
#include <coins/bitcoin-core.h>

//////////////////////////////////////////////////////////////////////////////
#define RTC_CHAIN_NAME      "reaction"
#define RTC_COIN_NAME       "RTC"
#define RTC_CORE_NAME       "rtc-core"
#define RTC_POWALGORITHM    PowAlgorithm::GhostRider

#define RTC_DEV_ADDRESS     "Rvb4NY1detbptykhhDeqq3S57bWJFRAqbr"

//////////////////////////////////////////////////////////////////////////////
namespace solominer::coin::rtc {

//////////////////////////////////////////////////////////////////////////////
class CWallet;
class CChain;
class CCore;

//--
typedef CCoreBitcoinBase_<CWallet,CChain> CCoreParent;
typedef CCoreBitcoinSetup_<CCore> CCoreSetup;

//////////////////////////////////////////////////////////////////////////////
class CWallet : public CWalletBitcoinBase {
public:
    CWallet( CCoreBitcoinBase &core ,IServiceSetupRef &setup ) :
        CWalletBitcoinBase(core,setup)
    {}
};

//////////////////////////////////////////////////////////////////////////////
class CChain : public CChainBitcoinBase {
public:
    CChain( CCoreBitcoinBase &core ,IServiceSetupRef &setup ) :
        CChainBitcoinBase(core,setup)
    {}
};

//////////////////////////////////////////////////////////////////////////////
class CCore : public CCoreParent {
public:
    CCore( IServiceSetupRef &setup ) :
        CCoreParent( setup )
    {}
};

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer::coin::rtc

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_REACTION_CORE_H