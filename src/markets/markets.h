#pragma once

// Copyright (c) 2018-2023 The NExTWave developers      <http://www.nextwave-techs.com/>
// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_MARKETS_H
#define SOLOMINER_MARKETS_H

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>
#include <common/service.h>

#include <interface/IMarket.h>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
void fromMarketSymbol( const String &s ,String &primary ,String &secondary ,const char sep='/' );

String &toMarketSymbol( String &s ,const String &primary ,const String &secondary ,const char sep='/' );

//////////////////////////////////////////////////////////////////////////////
DEFINE_MEMBER_API(MarketDeposit);
DEFINE_WITHSCHEMA_API(MarketDeposit);

DEFINE_MEMBER_API(MarketWithdraw);
DEFINE_WITHSCHEMA_API(MarketWithdraw);

DEFINE_MEMBER_API(MarketOrder);
DEFINE_WITHSCHEMA_API(MarketOrder);

//////////////////////////////////////////////////////////////////////////////
#define CMARKETSERVICEBASE_PUID 0x05351473824d4dd47
#define CMARKETSETUPBASE_PUID   0x08bb2d1fe3ea10a8d
#define CMARKETSTOREBASE_PUID   0x04012d23f1dbf3d3a

///--
class CMarketService;
class CMarketSetup;
class CMarketStore;

//////////////////////////////////////////////////////////////////////////////
class CMarketService : public IMarket ,public CService
{
public:
    CMarketService( IServiceSetupRef &setup ) : CService(setup)
    {
        this->info().category = MARKET_SERVICE_CATEGORY;
    }

    DECLARE_OBJECT(CMarketService,CMARKETSERVICEBASE_PUID);

    static const char *category() { return "market"; }

public: ///-- IService
    //...

public: ///-- IMarket
    IAPI_IMPL getAssetList( ListOf<String> &assets ) IOVERRIDE { return ENOEXEC; }

    IAPI_IMPL getAssetInfo( const char *ticker ,MarketAsset &info ) IOVERRIDE { return ENOEXEC; }

    IAPI_IMPL listMarkets( ListOf<MarketPair> &markets ) { return ENOEXEC; }

    IAPI_IMPL getMarketPair( const char *primary ,const char *secondary ,MarketPair &pair ) IOVERRIDE { return ENOEXEC; }

    IAPI_IMPL getOrderBook( const char *primary ,const char *secondary ,MarketOrderBook &book ) IOVERRIDE { return ENOEXEC; }

///-- account
    IAPI_IMPL getBalance( const char *value ,MarketBalance &balance ) IOVERRIDE { return ENOEXEC; }

    IAPI_IMPL getDepositAddress( const char *coin ,String &address ) IOVERRIDE { return ENOEXEC; }

    IAPI_IMPL getDeposit( const char *value ,const char *transactionId ,MarketDeposit &deposit ) IOVERRIDE { return ENOEXEC; }

    IAPI_IMPL CreateWithdraw( MarketWithdraw &withdraw ) IOVERRIDE { return ENOEXEC; }

    IAPI_IMPL getWithdraw( const char *value ,const char *transactionId ,MarketWithdraw &withdraw ) IOVERRIDE { return ENOEXEC; }

    IAPI_IMPL CancelWithdraw( const char *id ) IOVERRIDE { return ENOEXEC; }

    IAPI_IMPL CreateOrder( MarketOrder &order ) IOVERRIDE { return ENOEXEC; }

    IAPI_IMPL getOrder( const char *id ,MarketOrder &order ) IOVERRIDE { return ENOEXEC; }

    IAPI_IMPL CancelOrder( const char *id ,MarketOrder &order ) IOVERRIDE { return ENOEXEC; }

//--
    IAPI_IMPL listBalances( ListOf<MarketBalance> &balances ,int from=0 ,int count=0 ,bool includeZeroBalance=false ) IOVERRIDE { return ENOEXEC; }

    IAPI_IMPL listDeposits( const char *value ,ListOf<MarketDeposit> &deposits ,int from=0 ,int count=0 ) IOVERRIDE { return ENOEXEC; }

    IAPI_IMPL listWithdrawals( const char *value ,ListOf<MarketWithdraw> &deposits ,int from=0 ,int count=0 ) IOVERRIDE { return ENOEXEC; }

    IAPI_IMPL listOrders( const char *pair ,ListOf<MarketOrder> &orders ,int from=0 ,int count=0 ) IOVERRIDE { return ENOEXEC; }
};

typedef RefOf<CMarketService> CMarketServiceRef;

//////////////////////////////////////////////////////////////////////////////
class CMarketSetup : public IMarketSetup ,public CServiceSetup
{
public: ///-- IBase
    DECLARE_OBJECT(CMarketSetup,CMARKETSETUPBASE_PUID);

public: ///-- IMarketSetup
    //...
};

typedef RefOf<CMarketSetup> CMarketSetupRef;

//////////////////////////////////////////////////////////////////////////////
class CMarketStore : public IMarketStore
    ,public CServiceStore
    ,public Singleton_<CMarketStore>
{
public: ///-- IBase
    DECLARE_OBJECT(CMarketStore,CMARKETSTOREBASE_PUID);

public: ///-- IMarketStore
    //...
};

//////////////////////////////////////////////////////////////////////////////
inline CMarketStore &getMarketStore() {
    return getStore_<CMarketStore>();
}

//--
inline bool StartMarket( const char *name ,const Params *params=NullPtr ) {
    return StartService( CMarketService::category() ,name ,params );
}

inline bool getMarket( const char *name ,CMarketServiceRef &service ) {
    return getService_( name ,service );
}

inline bool getMarket( const char *name ,IMarketRef &service ) {
    return getService_( "market" ,name ,service );
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_MARKETS_H