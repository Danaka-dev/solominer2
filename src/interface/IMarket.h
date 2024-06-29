#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_IMARKET_H
#define SOLOMINER_IMARKET_H

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>

#include <interface/ICommon.h>
#include <interface/IService.h>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
#define MARKET_SERVICE_CATEGORY   "market"

//////////////////////////////////////////////////////////////////////////////
#define IMARKET_PUID           0x0aca9c6676d47bdf2
#define IMARKETSETUP_PUID      0x0f9e4b02224f700cb
#define IMARKETSTORE_PUID      0x09fdeb05ad29bf531

///--
class IMarket;
class IMarketSetup;
class IMarketStore;

//////////////////////////////////////////////////////////////////////////////
struct MarketAsset {
    String ticker;
    String name;

    //TODO use AmountValue
    double value;
    String currency; //! value in reference currency

    MapOf<String,String> urls; //! urls of interest, eg 'website' 'explorer' ...
};

struct MarketPair {
    String primaryCoin; //TODO rename value / asset //! asset id
    String secondaryCoin; //! asset id

    String lastPrice;
    String highPrice24h;
    String lowPrice24h;
    String volume24h;

    String chart24h;
    int lastTradeTime;
    int priceDecimal;

    bool hasMarket;
};

struct MarketOrderBook {
    String marketId;
    String primaryAsset;
    String secondaryAsset;
    TimeSec timestamp;

    struct Offer {
        String price;
        String quantity;
    };

    ListOf<Offer> bids; //! buying
    ListOf<Offer> asks; //! selling
};

//////////////////////////////////////////////////////////////////////////////
struct MarketBalance {
    String id;

    AmountValue amount;

    double pending;
};

struct MarketDeposit {
    String id; //! market id (market unique)
    String txid; //! on-chain id (globally unique)

    AmountValue amount;

    String fromAddress;
    String toAddress;

    TimeSec seenAt;
    int confirmations; //! on-chain confirmations
    int requiredConfirmations; //! market confirmations required
};

template <> inline MarketDeposit &Zero( MarketDeposit &p ) {
    p = { "" ,"" ,{ 0. ,"" } ,"" ,"" ,0 ,0 ,0 }; return p;
}

inline bool isConfirmed( const MarketDeposit &deposit ) {
    return deposit.confirmations >= deposit.requiredConfirmations;
}

///--
struct MarketWithdraw {
    String id; //! market id
    String txid;

    AmountValue amount;
    AmountValue fee;

    String fromAddress;
    String toAddress;

    String status; //! posted ,sent ,confirmed
    TimeSec postedAt;
    TimeSec sentAt;
};

template <> inline MarketWithdraw &Zero( MarketWithdraw &p ) {
    p = { "" ,"" ,{ 0. ,"" } ,{ 0. ,"" } ,"" ,"" ,"" ,0 ,0 }; return p;
}

///--
struct MarketOrder {
    String id; //! market id
    String userId; //! user defined id
        //NB LATER have default support from CMarketBase ledger that duplicates market book-keeping

    AmountValue amount;

    String toValue;
    double price;
        //! <0 market rate ,else limit price
        //! NB stop order handled using IBroker

    TimeSec validity; //! time until order is valid

    double quantityFilled; //! order quantity already filled

    String status; //! active ,filled ,cancelled
    TimeSec seenAt;
    TimeSec createdAt;
    TimeSec lastTradeAt;
    TimeSec completedAt;
};

inline bool isSpotOrder( const MarketOrder &order ) {
    return order.price < 0.;
}

template <> inline MarketOrder &Zero( MarketOrder &p ) {
    p = { "" ,"" ,{ 0. ,"" } ,"" ,0. }; return p;
}

//////////////////////////////////////////////////////////////////////////////
struct MarketCredential {
    String apiKey;
};

struct MarketConfig {
    MarketCredential credential;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! IMarket Service

class IMarket : IOBJECT_PARENT
{
public: ///-- IBase
    static PUID getClassId() { return IMARKET_PUID; };

public: ///-- IMarket
    // loadConfig
    //=> api key
    // saveConfig
    // updateConfig ...

public: ///-- IMarket
    IAPI_DECL getAssetList( ListOf<String> &assets ) = 0;

    IAPI_DECL getAssetInfo( const char *ticker ,MarketAsset &asset ) = 0;

    IAPI_DECL listMarkets( ListOf<MarketPair> &markets ) = 0;

    IAPI_DECL getMarketPair( const char *primary ,const char *secondary ,MarketPair &pair ) = 0;

    IAPI_DECL getOrderBook( const char *primary ,const char *secondary ,MarketOrderBook &book ) = 0;

///-- account
    IAPI_DECL getBalance( const char *value ,MarketBalance &balance ) = 0;

    IAPI_DECL getDepositAddress( const char *value ,String &address ) = 0;

    IAPI_DECL getDeposit( const char *value ,const char *transactionId ,MarketDeposit &deposit ) = 0;

    IAPI_DECL CreateWithdraw( MarketWithdraw &withdraw ) = 0;

    IAPI_DECL getWithdraw( const char *value ,const char *transactionId ,MarketWithdraw &withdraw ) = 0;

    IAPI_DECL CancelWithdraw( const char *id ) = 0;

    IAPI_DECL CreateOrder( MarketOrder &order ) = 0;

    IAPI_DECL getOrder( const char *id ,MarketOrder &order ) = 0;

    IAPI_DECL CancelOrder( const char *id ,MarketOrder &order ) = 0;

//--
    IAPI_DECL listBalances( ListOf<MarketBalance> &balances ,int from=0 ,int count=0 ,bool includeZeroBalance=false ) = 0;

    IAPI_DECL listDeposits( const char *value ,ListOf<MarketDeposit> &deposits ,int from=0 ,int count=0 ) = 0;

    IAPI_DECL listWithdrawals( const char *value ,ListOf<MarketWithdraw> &deposits ,int from=0 ,int count=0 ) = 0;

    IAPI_DECL listOrders( const char *pair ,ListOf<MarketOrder> &orders ,int from=0 ,int count=0 ) = 0;
};

typedef RefOf<IMarket> IMarketRef;

//////////////////////////////////////////////////////////////////////////////
class IMarketSetup : IOBJECT_PARENT
{
public: ///-- IBase
    static PUID getClassId() { return IMARKETSETUP_PUID; };

public: ///-- IMarketSetup
    //...
};

typedef RefOf<IMarketSetup> IMarketSetupRef;

//////////////////////////////////////////////////////////////////////////////
class IMarketStore : IOBJECT_PARENT
{
public: ///-- IBase
    static PUID getClassId() { return IMARKETSTORE_PUID; };

public: ///-- IMarketStore
    //...
};

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_IMARKET_H