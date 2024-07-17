#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_BITOREUM_CORE_H
#define SOLOMINER_BITOREUM_CORE_H

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>
#include <coins/bitcoin-core.h>

//////////////////////////////////////////////////////////////////////////////
#define BTRM_CHAIN_NAME      "bitoreum"
#define BTRM_COIN_NAME       "BTRM"
#define BTRM_CORE_NAME       "btrm-core"
#define BTRM_POWALGORITHM    PowAlgorithm::GhostRider

#define BTRM_DEV_ADDRESS     "BoEw9Nqe6P6UpijRTdRN4Sw25A8zXV2wfU"

//////////////////////////////////////////////////////////////////////////////
namespace solominer::coin::btrm {

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
} //namespace solominer::coin::btrm

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_BITOREUM_CORE_H