#pragma once

// Copyright (c) 2018-2023 The NExTWave developers      <http://www.nextwave-techs.com/>
// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_XEGGEX_MARKET_H
#define SOLOMINER_XEGGEX_MARKET_H

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>
#include <common/http.h>

#include <markets/markets.h>

#include "xeggex-api.h"

//TODO check httplib ,maybe <https://github.com/yhirose/cpp-httplib>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

using namespace nextwave;

//////////////////////////////////////////////////////////////////////////////
#define XEGGEX_NAME         "xeggex"
// #define XEGGEX_HOSTNAME     "https://api.xeggex.com"
// #define XEGGEX_APIPATH      "/api/v2"

//////////////////////////////////////////////////////////////////////////////
class CMarketXeggex : public CMarketService ,public xeggex::IHttp {
protected:
    CHttpConnection m_http;

    xeggex::CApi2 m_api;

    //TODO url with
    // XEGGEX_HOSTNAME

public:
    CMarketXeggex( IServiceSetupRef &setup ) : CMarketService(setup)
        ,m_http() ,m_api(*this)
    {
        this->info().name = XEGGEX_NAME;

        m_http.cache().Load( "xeggex-cache.dat" );
    }

    ~CMarketXeggex() {
        m_http.cache().Save();
    }

    xeggex::CApi2 &api() { return m_api; }

public: ///-- IMarket
    IAPI_IMPL getAssetList( ListOf<String> &assets ) IOVERRIDE;

    IAPI_IMPL getAssetInfo( const char *coin ,MarketAsset &info ) IOVERRIDE;

    IAPI_IMPL listMarkets( ListOf<MarketPair> &markets ) IOVERRIDE;

    IAPI_IMPL getMarketPair( const char *primary ,const char *secondary ,MarketPair &pair ) IOVERRIDE;

    IAPI_IMPL getOrderBook( const char *primary ,const char *secondary ,MarketOrderBook &book ) IOVERRIDE;

///-- account
    IAPI_IMPL getBalance( const char *value ,MarketBalance &balance ) IOVERRIDE;

    IAPI_IMPL getDepositAddress( const char *coin ,String &address ) IOVERRIDE;

    IAPI_IMPL getDeposit( const char *value ,const char *transactionId ,MarketDeposit &deposit ) IOVERRIDE;

    IAPI_IMPL CreateWithdraw( MarketWithdraw &withdraw ) IOVERRIDE;

    IAPI_IMPL getWithdraw( const char *value ,const char *transactionId ,MarketWithdraw &withdraw ) IOVERRIDE;

    IAPI_IMPL CancelWithdraw( const char *id ) IOVERRIDE;

    IAPI_IMPL CreateOrder( MarketOrder &order ) IOVERRIDE;

    IAPI_IMPL getOrder( const char *id ,MarketOrder &order ) IOVERRIDE;

    IAPI_IMPL CancelOrder( const char *id ,MarketOrder &order ) IOVERRIDE;

//--
    IAPI_IMPL listBalances( ListOf<MarketBalance> &balances ,int from=0 ,int count=0 ,bool includeZeroBalance=false ) IOVERRIDE;

    IAPI_IMPL listDeposits( const char *value ,ListOf<MarketDeposit> &deposits ,int from=0 ,int count=0 ) IOVERRIDE;

    IAPI_IMPL listWithdrawals( const char *value ,ListOf<MarketWithdraw> &deposits ,int from=0 ,int count=0 ) IOVERRIDE;

    IAPI_IMPL listOrders( const char *pair ,ListOf<MarketOrder> &orders ,int from=0 ,int count=0 ) IOVERRIDE;

public: ///-- xeggex::IHttp

    //TODO should be able to get asynch

    API_IMPL(bool) HttpSend( const char *url ,const Headers &headers ,const char *userpass ,const char *body ,String &response ,long &status ) IOVERRIDE;
};

//////////////////////////////////////////////////////////////////////////////
class CMarketSetupXeggex : public CMarketSetup {
public:
    API_IMPL(bool) connectNew( ServiceInfo &info ,const Params &params ,IServiceRef &service ) IOVERRIDE;
};

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_XEGGEX_MARKET_H