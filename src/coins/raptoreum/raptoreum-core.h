#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_RAPTOREUM_CORE_H
#define SOLOMINER_RAPTOREUM_CORE_H

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>
#include <coins/bitcoin-core.h>

//////////////////////////////////////////////////////////////////////////////
#define RTM_CHAIN_NAME      "raptoreum"
#define RTM_COIN_NAME       "RTM"
#define RTM_CORE_NAME       "rtm-core"
#define RTM_POWALGORITHM    PowAlgorithm::GhostRider

#define RTM_DEV_ADDRESS     "RRsoc2xrJgDFiWYGimKcY7AUqy9ghizMea"

//////////////////////////////////////////////////////////////////////////////
namespace solominer::coin::rtm {

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
} //namespace solominer::coin::rtm

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_RAPTOREUM_CORE_H