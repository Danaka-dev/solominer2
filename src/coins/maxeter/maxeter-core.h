#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_MAXETER_CORE_H
#define SOLOMINER_MAXETER_CORE_H

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>
#include <coins/bitcoin-core.h>

//////////////////////////////////////////////////////////////////////////////
#define MAXE_CHAIN_NAME      "maxeter"
#define MAXE_COIN_NAME       "MAXE"
#define MAXE_CORE_NAME       "maxe-core"
#define MAXE_POWALGORITHM    PowAlgorithm::GhostRider

#define MAXE_DEV_ADDRESS     "MHVgLsiERQJo66h7woo2L6AhBaX6NYz6iC"

//////////////////////////////////////////////////////////////////////////////
namespace solominer::coin::maxe {

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
} //namespace solominer::coin::maxe

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_MAXETER_CORE_H