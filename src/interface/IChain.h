#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_ICHAIN_H
#define SOLOMINER_ICHAIN_H

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>

#include <interface/IService.h>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
#define CHAIN_SERVICE_CATEGORY   "chain"

//////////////////////////////////////////////////////////////////////////////
#define ICHAIN_UUID           0x0a42196a7b3fad02f
#define ICHAINSETUP_UUID      0x0fc3e45e33638486c
#define ICHAINSTORE_UUID      0x03e9ecca2951e9f73

///--
class IChain;
class IChainSetup;
class IChainStore;

//////////////////////////////////////////////////////////////////////////////
struct ChainTransaction {
    String coin;
    double amount;

    String fromAddress;
    String ToAddress;
    String communication;

    uint64_t blockHeight;

    // signature
    // txid
    // + per coin specific info ?
};

typedef PtrOf<ChainTransaction> ChainTransactionPtr;

//--
struct ChainAddress {
    String coin;
    String value; //! encoding ?
};

typedef PtrOf<ChainAddress> ChainAddressPtr;

//////////////////////////////////////////////////////////////////////////////
struct ChainInfo {
    String name; //! coin/chain name

    long blockHeight;

    double networkDiff;
    double networkHPS; //! hash per second
    double blockReward; //! current reward per block
    double blockPerHour; //! network current block per hour

    double price; //TODO NOT HERE

    //? + extra info
    // double secondsPerBlock; //! network target second per block
};

enum ChainInfoFlags {
    noFlags=0 ,blockHeight=1 ,currentDiff=2 ,networkHPS=4 ,secondsPerBlock=8 ,blockReward=16
};

//////////////////////////////////////////////////////////////////////////////
struct ChainConfig {
    String host;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class IChain : IOBJECT_PARENT
{
public: ///-- IBase
    static UUID getClassId() { return ICHAIN_UUID; };

public: ///-- IChain
    IAPI_DECL getInfo( const char *coin ,ChainInfo &info ,ChainInfoFlags flags ) = 0;

    // IAPI_DECL getInfo( MarketInfo &info ) = 0;

    // IAPI_DECL getInfo( PoolInfo &info ) = 0;

public: ///-- wallet as a service
    // IAPI_DECL walletAsAService( IWalletRef &wallet ) ;

    // IAPI_DECL getMarketService( IMarketRef &market );

public:
    // IAPI_DECL makeRawTransaction( JSON &doc ) = 0;

    // IAPI_DECL sendRawTransaction() = 0;
};

typedef RefOf<IChain> IChainRef;

//////////////////////////////////////////////////////////////////////////////
class IChainSetup : IOBJECT_PARENT
{
public: ///-- IBase
    static UUID getClassId() { return ICHAINSETUP_UUID; };

public: ///--- ICoreSetup
    //...
};

typedef RefOf<IChainSetup> IChainSetupRef;

//////////////////////////////////////////////////////////////////////////////
class IChainStore : IOBJECT_PARENT
{
public: ///-- IBase
    static UUID getClassId() { return ICHAINSTORE_UUID; };

public: ///--- ICoreSetup
    //...
};

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_ICHAIN_H