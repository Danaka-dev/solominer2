#pragma once

// Copyright (c) 2021-2024 The NExTWave developers          <http://www.nextwave-tech.com>
// Copyright (c) 2023-2024 The solominer developers         <https://github.com/Danaka-dev/SoloMiner>
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef NEXTWAVE_XEGGEX_API_H
#define NEXTWAVE_XEGGEX_API_H

//////////////////////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include <map>

//////////////////////////////////////////////////////////////////////////////
#if !defined(String)
 typedef std::string String;
#endif

#if !defined(ListOf)
 #define ListOf  std::vector
#endif

#if !defined(MapOf)
 #define MapOf  std::map
#endif

//////////////////////////////////////////////////////////////////////////////
namespace nextwave {

//////////////////////////////////////////////////////////////////////////////
namespace xeggex {

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! XeggeX API v2 interface

#define XEGGEX_HOSTNAME         "https://api.xeggex.com"
#define XEGGEX_API2_PATH        "/api/v2"
#define XEGGEX_API2_ENDPOINT    "https://api.xeggex.com/api/v2"

//////////////////////////////////////////////////////////////////////////////
//! Public

struct Asset {
    String id;
    String ticker;
    String name;
    String logo;
    bool isActive;
    bool isToken;
    // Schema String
    // tpkenDetails
    bool useParentAddress;
    String usdValue;
    bool depositActive;
    String depositNotes;
    bool depositPayId;
    bool withdrawalActive;
    String withdrawalNotes;
    bool withdrawalPayid;
    bool withdrawalPayidRequired;
    int confirmsRequired;
    int withdrawDecimals;
    String withdrawFee;

    String explorer;
    String explorerTxid;
    String explorerAddress;
    String website;
    String coinMarketCap;
    String coinGecko;
    String addressRegEx;
    String payidRegEx;
    // "socialCommunity": {},
    int createdAt;
    int updatedAt;
};

//////////////////////////////////////////////////////////////////////////////
//! Market

struct Market {
    String id;              //! Internal ID
    String symbol;          //! market symbol code

    Asset primaryAsset;
    Asset secondaryAsset;

    String lastPrice;       //! last trade price
    String highPrice;       //! 24 hour high price
    String lowPrice;        //! 24 hour low price
    String volume;          //! 24 hour volume in primary currency
    String lineChart;       //! Hourly price data over the last 24 hours

    time_t lastTradeAt;     //! timestamp of last trade
    long priceDecimals;     //! decimal places for price
    bool isActive;          //! is market active
};

struct Market2 {
    String id;              //! Internal ID
    String symbol;          //! market symbol code

    String primaryAsset;    //! primaery asset id
    String secondaryAsset;    //! secondary asset id

    String lastPrice;       //! last trade price
    String highPrice;       //! 24 hour high price
    String lowPrice;        //! 24 hour low price
    String volume;          //! 24 hour volume in primary currency
    String lineChart;       //! Hourly price data over the last 24 hours

    time_t lastTradeAt;     //! timestamp of last trade
    long priceDecimals;     //! decimal places for price
    long quantityDecimals;  //! decimal places for quantity (volume)
    bool isActive;          //! is market active
};

struct MarketByList {
    String _id;
    String symbol;
    String primaryName;
    String primaryTicker;
    String lastPrice;
    String yesterdayPrice;
    String highPrice;
    String lowPrice;
    String volume;
    int64_t lastTradeAt;
    int priceDecimals;
    int quantityDecimals;
    bool isActive;
    String primaryAsset;
    String secondaryAsset;
    String bestAsk;
    String bestBid;
    int64_t createdAt;
    int64_t updatedAt;
    ListOf<double> lineChart;
    double bestAskNumber;
    double bestBidNumber;
    String changePercent;
    double changePercentNumber;
    double highPriceNumber;
    double lastPriceNumber;
    double lowPriceNumber;
    double volumeNumber;
    double yesterdayPriceNumber;
    double volumeUsdNumber;
    double marketcapNumber;
    String primaryCirculation;
    String primaryUsdValue;
    String secondaryCirculation;
    String secondaryUsdValue;
    String spreadPercent;
    String lastPriceUpDown;
    int engineId;
    bool isPaused;
    String imageUUID;
    String volumeSecondary;
    double volumeSecondaryNumber;
    double spreadPercentNumber;
    String id;
};

///-- version
struct Market2_20240222 : Market2 {
    String _id;              //! Internal ID

    String primaryName;
    String primaryTicker;
    String primaryCirculation;
    String primaryUsdValue;

    String secondaryCirculation;
    String secondaryUsdValue;
    String volumeSecondary;
    double volumeSecondaryNumber;

    String yeasterdayPrice;
    String bestAsk;
    String bestBid;
    time_t createdAt;
    time_t updatedAt;
    String changePercent;

    double lastPriceNumber;
    double bestBidNumber;
    double bestAskNumber;
    double yesterdayPriceNumber;
    double changePercentNumber;
    double highPriceNumber;
    double lowPriceNumber;
    double volumeNumber;
    double volumeUsdNumber;
    double spreadPercentNumber;
    long marketcapNumber;

    long minimumQuantity;
    bool isPaused;
    bool pauseBuys;
    bool pauseSells;

    String maxAllowedPrice;
    String minAllowedPrice;
    String spreadPercent;
    String lastPriceUpDown;

    long engineId;
    String imageUUID;
};

struct OrderBook {
    String marketid;
    String symbol;
    time_t timestamp;

    struct Offer {
        String price;
        double numberprice;
        String quantity;
    };

    ListOf<Offer> bids;
    ListOf<Offer> asks;
};

//////////////////////////////////////////////////////////////////////////////
//! Account

struct Balance {
    String asset;
    String name;
    String available;
    String pending;
    String held;
    String assetId;
};

struct DepositAddress {
    String address;
    String paymentId;
    String ticker;
    String network;
};

struct Deposit {
    String id;
    String address;
    String paymentId;
    String ticker;
    String childTicker; //! nullable
    String quantity;
    String status;
    String transactionId;
    bool isPosted;
    bool isReversed;
    int confirmations;
    int64_t firstSeenAt;
    int64_t postedAt;
};

struct Withdrawal {
    String id;
    String address;
    String paymentId;
    String ticker;
    String childTicker; //! nullable
    String quantity;
    String fee;
    String feeCurrency;
    String status;
    String transactionId;
    bool isSent;
    int64_t sentAt;
    bool isConfirmed;
    int64_t requestedAt;
};

struct Order {
    String id;
    String userProvidedId;

    struct Market {
        String id;
        String symbol;
    } market;

    String side;
    String type;
    String price;
    String quantity;

    String executedQuantity;
    String remainQuantity;
    String remainTotal;
    String remainTotalWithFee;

    int64_t lastTradeAt;
    String status;
    bool isActive;
    int64_t createdAt;
    int64_t updatedAt;
};

struct Trade {
    String id;

    struct Market {
        String id;
        String symbol;
    } market;

    String orderId;
    String side;
    String triggeredBy;
    String price;
    String quantity;
    String fee;
    String totalWithFee;
    String alternateFeeAsset; //! nullable
    String alternateFee; //! nullable

    int64_t createdAt;
    int64_t updatedAt;
};

struct PoolTrade {
    String id;

    struct Pool {
        String id;
        String symbol;
    } pool;

    String side;
    String price;
    String quantity;
    String fee;
    String totalWithFee;

    int64_t createdAt;
    int64_t updatedAt;
};

///-- POST
struct OrderToCreate {
    String userProvidedId;
    String symbol;
    String side;
    String type;
    String quantity;
    String price;
    bool strictValidate;
};

struct OrderCreated {
    String id;
    String userProvidedId;

    String market;
    String user;
    String primaryAsset;
    String secondaryAsset;

    String side;
    String type;
    double numberPrice;
    String price;
    String quantity;

    String executedQuantity;
    String remainQuantity;
    String remainTotal;
    String remainTotalWithFee;

    int64_t lastTradeAt;
    String status;
    String source;
    bool isActive;
    bool isNew;
    double feeRate;
    int64_t createdAt;
    int64_t updatedAt;
};

//--
struct OrderToCancel {
    String id;
};

struct OrderCancelled {
    bool success;
    String id;
};

//--
struct OrdersToCancelAll {
    String symbol;
    String side;
};

struct OrdersCancelledAll {
    bool success;
    ListOf<String> ids;
};

//--
struct WithdrawalToCreate {
    String ticker;
    String quantity;
    String address;
    String paymentId;
};

struct WithdrawalCreated {
    String id;
    String address;
    String paymentId;
    String ticker;
    String childTicker; //! nullable
    String quantity;
    String fee;
    String feeCurrency;
    String status;
    String transactionId;
    bool isSent;
    int64_t sentAt;
    bool isConfirmed;
    int64_t requestedAt;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! ApiConfig

struct ApiCredential {
    String apiKey;
    String apiSecret;
};

struct ApiConfig {
    ApiCredential credential;
};

bool ApiSign( const ApiCredential &credential ,const char *url ,const char *body ,const char *nonce ,String &sign );

//////////////////////////////////////////////////////////////////////////////
//! HTTP interface

class IHttp {
public:
    typedef std::map<String,String> Headers;

    virtual bool HttpSend( const char *url ,const Headers &headers ,const char *userpass ,const char *body ,String &response ,long &status ) = 0;
};

bool authorizeApiBasic( const ApiCredential &credential ,IHttp::Headers &headers ,const char *url ,const char *body );
bool authorizeApiHmac( const ApiCredential &credential ,IHttp::Headers &headers ,const char *url ,const char *body );

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! API interface

class CApi2 {
public:
    enum ApiAuthorization {
        authBasic=0 ,authHmac
    } m_apiAuthorization = authBasic;

protected:
    IHttp &m_http;

    ApiConfig m_config;

public:
    CApi2( IHttp &http ) : m_http(http)
    {}

    const ApiConfig &config() const { return m_config; }

    ApiConfig &config() { return m_config; }

//////////////////////////////////////////////////////////////////////////////
//! APIv2

public: ///-- public
    bool assetGetList( ListOf<Asset> &assets );
    bool assetGetById( const char *id ,Asset &asset );
    bool assetGetByTicker( const char *ticker ,Asset &asset );

    bool marketGetList( ListOf<MarketByList> &markets );
    bool marketGetById( const char *id ,Market &market );
    bool marketGetBySymbol( const char *symbol ,Market2 &market );
    bool marketGetOrderBookBySymbol( const char *symbol ,OrderBook &book );
    bool marketGetOrderBookByMarketId( const char *marketId ,OrderBook &book );

    //+ pool

public: ///-- account
    bool balances( ListOf<Balance> &balances );
    bool getDepositAddress( const char *ticker ,DepositAddress &address );

    bool CreateOrder( const OrderToCreate &createOrder ,OrderCreated &order );
    bool CancelOrder( const char *orderId ,OrderCancelled &order );
    bool CancelAllOrders( const OrdersToCancelAll &cancelOrders ,OrdersCancelledAll &orders );
    bool CreateWithdrawal( const WithdrawalToCreate &createWithdrawal ,WithdrawalCreated &withdrawal );

    bool getDeposits( const char *ticker ,ListOf<Deposit> &deposits ,int limit=0 ,int skip=500 );
    bool getWithdrawals( const char *ticker ,ListOf<Withdrawal> &withdrawals ,int limit=0 ,int skip=500 );
    bool getOrder( const char *orderId ,Order &order );
    bool getOrders( const char *ticker ,const char *orderStatus ,ListOf<Order> &orders ,int limit=0 ,int skip=500 );

    bool getTrades( const char *symbol ,ListOf<Trade> &trades ,int limit=0 ,int skip=500 );
    bool getTradesSince( const char *ticker ,ListOf<Trade> &trades ,const char *sinceTimestampMs ,int limit=0 ,int skip=500 );

    bool getPoolTrades( const char *symbol ,ListOf<PoolTrade> &trades ,int limit=0 ,int skip=500 );
    bool getPoolTradesSince( const char *symbol ,ListOf<PoolTrade> &trades ,const char *sinceTimestampMs ,int limit=0 ,int skip=500 );

//////////////////////////////////////////////////////////////////////////////
//! Member functions

protected:
    bool HttpSend( const char *path ,const char *param ,const char *body ,String &response ,long &status ,bool needAuth=false ,bool noCache=false );
};

//////////////////////////////////////////////////////////////////////////////
} //namespace xeggex

//////////////////////////////////////////////////////////////////////////////
} //namespace nextwave

//////////////////////////////////////////////////////////////////////////////
#endif //NEXTWAVE_XEGGEX_API_H