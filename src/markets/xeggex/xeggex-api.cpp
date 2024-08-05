// Copyright (c) 2018-2024 The NExTWave developers          <http://www.nextwave-tech.com>
// Copyright (c) 2023-2024 The solominer developers         <https://github.com/Danaka-dev/SoloMiner>
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include <solominer.h>

#include "xeggex-api.h"

#include <sstream>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/reader.h>
#include "rapidjson/writer.h"

//////////////////////////////////////////////////////////////////////////////
//! import crypto

#define CRYPTO_NAMESPACE TW::Hash

namespace CRYPTO_NAMESPACE {
    using Data = std::vector<unsigned char>;

    Data hmac256( const Data& key ,const Data& message );
}

//////////////////////////////////////////////////////////////////////////////
namespace nextwave {

//////////////////////////////////////////////////////////////////////////////
namespace xeggex {

//////////////////////////////////////////////////////////////////////////////
//! Json generate

template <class T ,class TValue ,class TContext>
void generateBodyValue_( const T &s ,TValue &v ,TContext &context ) {
    throw std::exception(); //! NOEXEC
}

typedef rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> json_context_t;

template <class T>
bool generateBodyStruct_( const T &s ,String &body ) {

    try {
        using namespace rapidjson;

        Document doc;

        doc.SetObject();
        auto &allocator = doc.GetAllocator();

        Value &v = doc;
        {
            generateBodyValue_( s ,v ,allocator );
        }

        StringBuffer sb;
        Writer<StringBuffer> writer(sb);

        doc.Accept( writer );
        body = sb.GetString();

    } catch( ... ) {
        return false;
    }

    return true;
}

///--
rapidjson::GenericStringRef<char> toJsonString( const String &s ) {
    return rapidjson::StringRef(s.c_str());
}

//////////////////////////////////////////////////////////////////////////////
//! Json parse

template <class TValue ,class T>
void parseResponseValue_( TValue &v ,T &s ) { // TODO TValue const ?
    throw std::exception(); //! NOEXEC
}

template <class T>
bool parseResponseStruct_( const String &response ,T &s ) {
    if( response.empty() )
        return false;

    try {
        rapidjson::Document jsondoc;

        jsondoc.Parse( response.c_str() );

        rapidjson::Value &v = jsondoc;
        {
            if( v.IsNull() || v.HasMember("error") )
                return false;

            parseResponseValue_( v ,s );
        }

    } catch( ... ) {
        return false;
    }

    return true;
}

template <class T>
void parseResponseList_( rapidjson::Value &v ,ListOf<T> &list ) {
    assert( v.IsArray() );

    if( !v.IsArray() )
        return;

    for( auto &it : v.GetArray() ) {
        T s;

        parseResponseValue_( it ,s );

        list.emplace_back( s );
    }
}

template <class T>
bool parseResponseList_( const String &response ,ListOf<T> &list ) {
    if( response.empty() )
        return false;

    try {
        rapidjson::Document jsondoc;

        if( strncmp( response.c_str() ,"error" ,6 ) == 0 )
            return false;

        jsondoc.Parse( response.c_str() );

        rapidjson::Value &v = jsondoc;

        if( !v.IsArray() ) return false;

        parseResponseList_( v ,list );

    } catch( ... ) {
        return false;
    }

    return true;
}

///--
const char *getJsonString( rapidjson::Value &v ,const char *memberId ,const char *defaultValue="" ) {
    try {
        return v.HasMember(memberId) && !v[memberId].IsNull() ? v[memberId].GetString() : defaultValue;
    } catch(...) {
        return defaultValue;
    }
}

int64_t getJsonInt64( rapidjson::Value &v ,const char *memberId ,int64_t defaultValue=0 ) {
    try {
        return v.HasMember(memberId) && !v[memberId].IsNull() ? v[memberId].GetInt64() : defaultValue;
    } catch(...) {
        return defaultValue;
    }
}

double getJsonDouble( rapidjson::Value &v ,const char *memberId ,double defaultValue=0. ) {
    try {
        return v.HasMember(memberId) && !v[memberId].IsNull() ? v[memberId].GetDouble() : defaultValue;
    } catch(...) {
        return defaultValue;
    }
}

template <>
void parseResponseValue_( rapidjson::Value &v ,String &s ) {
    s = v.IsNull() ? "" : v.GetString();
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! public

template <>
void parseResponseValue_( rapidjson::Value &v ,Asset &asset ) {
    asset.id = v["id"].GetString();
    asset.ticker = v["ticker"].GetString();
    asset.name = v["name"].GetString();
    asset.logo = v["logo"].GetString();
    asset.isActive = v["isActive"].GetBool();

    asset.usdValue = v["usdValue"].GetString();
}

template <>
void parseResponseValue_( rapidjson::Value &v ,Market &market ) {
    market.id = v["id"].GetString();
    market.symbol = v["symbol"].GetString();

    //TODO test
    String primaryAsset = v["primaryAsset"].GetString();
    parseResponseStruct_( primaryAsset ,market.primaryAsset );
    String secondaryAsset = v["secondaryAsset"].GetString();
    parseResponseStruct_( secondaryAsset ,market.secondaryAsset );

    market.lastPrice = v["lastPrice"].GetString();
    market.highPrice = v["highPrice"].GetString();
    market.lowPrice = v["lowPrice"].GetString();
    market.volume = v["volume"].GetString();
    market.lineChart = v["lineChart"].GetString();

    market.lastTradeAt = (time_t) v["lastTradeAt"].GetInt();
    market.priceDecimals = (long) v["priceDecimals"].GetInt();
    market.isActive = v["isActive"].GetBool();
}

template <>
void parseResponseValue_( rapidjson::Value &v ,Market2 &market ) {
    market.id = v["id"].GetString();
    market.symbol = v["symbol"].GetString();

    market.primaryAsset = v["primaryAsset"].GetString();
    market.secondaryAsset = v["secondaryAsset"].GetString();

    market.lastPrice = v["lastPrice"].GetString();
    market.highPrice = v["highPrice"].GetString();
    market.lowPrice = v["lowPrice"].GetString();
    market.volume = v["volume"].GetString();
    market.lineChart = v["lineChart"].GetString();

    market.lastTradeAt = (time_t) v["lastTradeAt"].GetInt64();
    market.priceDecimals = (long) v["priceDecimals"].GetInt();
    market.quantityDecimals = (long) v["quantityDecimals"].GetInt();
    market.isActive = v["isActive"].GetBool();
}

template <>
void parseResponseValue_( rapidjson::Value &v ,MarketByList &market ) {
    market._id = v["_id"].GetString();
    market.symbol = v["symbol"].GetString();
    market.primaryName = v["primaryName"].GetString();
    market.primaryTicker = v["primaryTicker"].GetString();
    market.lastPrice = v["lastPrice"].GetString();
    market.yesterdayPrice = v["yesterdayPrice"].GetString();
    market.highPrice = v["highPrice"].GetString();
    market.lowPrice = v["lowPrice"].GetString();
    market.volume = v["volume"].GetString();
    market.lastTradeAt = v["lastTradeAt"].GetInt64();
    market.priceDecimals = v["priceDecimals"].GetInt();
    market.quantityDecimals = v["quantityDecimals"].GetInt();
    market.isActive = v["isActive"].GetBool();
    market.primaryAsset = v["primaryAsset"].GetString();
    market.secondaryAsset = v["secondaryAsset"].GetString();
    market.bestAsk = v["bestAsk"].GetString();
    market.bestBid = v["bestBid"].GetString();
    market.createdAt = v["createdAt"].GetInt64();
    market.updatedAt = v["updatedAt"].GetInt64();
//TODO    ListOf<double> lineChart;
    market.bestAskNumber = v["bestAskNumber"].GetDouble();
    market.bestBidNumber = v["bestBidNumber"].GetDouble();
    market.changePercent = v["changePercent"].GetString();
    market.changePercentNumber = v["changePercentNumber"].GetDouble();
    market.highPriceNumber = v["highPriceNumber"].GetDouble();
    market.lastPriceNumber = v["lastPriceNumber"].GetDouble();
    market.lowPriceNumber = v["lowPriceNumber"].GetDouble();
    market.volumeNumber = v["volumeNumber"].GetDouble();
    market.yesterdayPriceNumber = v["yesterdayPriceNumber"].GetDouble();
    market.volumeUsdNumber = v["volumeUsdNumber"].GetDouble();
    market.marketcapNumber = v["marketcapNumber"].GetDouble();
    market.primaryCirculation = getJsonString( v ,"primaryCirculation" );
    market.primaryUsdValue = v["primaryUsdValue"].GetString();
    market.secondaryCirculation = v["secondaryCirculation"].GetString();
    market.secondaryUsdValue = v["secondaryUsdValue"].GetString();
    market.spreadPercent = v["spreadPercent"].GetString();
    market.lastPriceUpDown = v["lastPriceUpDown"].GetString();
    market.engineId = v["engineId"].GetInt();
    market.isPaused = v["isPaused"].GetBool();
    market.imageUUID = v["imageUUID"].GetString();
    market.volumeSecondary = v["volumeSecondary"].GetString();
    market.volumeSecondaryNumber = v["volumeSecondaryNumber"].GetDouble();
    market.spreadPercentNumber = getJsonDouble( v ,"spreadPercentNumber" );
    market.id = v["id"].GetString();
}

//--
template <>
void parseResponseValue_( rapidjson::Value &v ,OrderBook::Offer &offer ) {
    offer.price = v["price"].GetString();
    offer.numberprice = v["numberprice"].GetDouble();
    offer.quantity = v["quantity"].GetString();
}

template <>
void parseResponseValue_( rapidjson::Value &v ,OrderBook &order ) {
    order.marketid = v["marketid"].GetString();
    order.symbol = v["symbol"].GetString();
    order.timestamp = v["timestamp"].GetInt64();

    rapidjson::Value &bids = v["bids"].GetArray();
    parseResponseList_( bids ,order.bids );

    rapidjson::Value &asks = v["asks"].GetArray();
    parseResponseList_( asks ,order.asks );
}

//////////////////////////////////////////////////////////////////////////////
//! account

template <>
void parseResponseValue_( rapidjson::Value &v ,Balance &balance ) {
    balance.asset = v["asset"].GetString();
    balance.name = v["name"].GetString();
    balance.available = v["available"].GetString();
    balance.pending = v["pending"].GetString();
    balance.held = v["held"].GetString();
    balance.assetId = v["assetid"].GetString();
}

template <>
void parseResponseValue_( rapidjson::Value &v ,DepositAddress &address ) {
    address.address = v["address"].GetString();
    address.paymentId = v["paymentid"].GetString();
    address.ticker = v["ticker"].GetString();
    address.network = v["network"].GetString();
}

template <>
void parseResponseValue_( rapidjson::Value &v ,Deposit &deposit ) {
    deposit.id = v["id"].GetString();
    deposit.address = v["address"].GetString();
    deposit.paymentId = getJsonString( v ,"paymentid" );
    deposit.ticker = v["ticker"].GetString();
    deposit.childTicker =  getJsonString( v ,"childticker" );
    deposit.quantity = v["quantity"].GetString();
    deposit.status = v["status"].GetString();
    deposit.transactionId = v["transactionid"].GetString();

    deposit.isPosted = v["isposted"].GetBool();
    deposit.isReversed = v["isreversed"].GetBool();

    deposit.confirmations = v["confirmations"].GetInt();
    deposit.firstSeenAt = v["firstseenat"].GetInt64();
    deposit.postedAt = v["postedat"].GetInt64();
}

template <>
void parseResponseValue_( rapidjson::Value &v ,Withdrawal &withdrawal ) {
    withdrawal.id = v["id"].GetString();
    withdrawal.address = v["address"].GetString();
    withdrawal.paymentId = v["paymentid"].GetString();
    withdrawal.ticker = v["ticker"].GetString();
    withdrawal.childTicker = getJsonString( v ,"childticker" );
    withdrawal.quantity = v["quantity"].GetString();
    withdrawal.fee = v["fee"].GetString();
    withdrawal.feeCurrency = v["feecurrency"].GetString();
    withdrawal.status = v["status"].GetString();
    withdrawal.transactionId = v["transactionid"].GetString();

    withdrawal.isSent = v["issent"].GetBool();
    withdrawal.sentAt = v["sentat"].GetInt64();

    withdrawal.isConfirmed = v["isconfirmed"].GetBool();
    withdrawal.requestedAt = v["requestedat"].GetInt64();
}

//--
template <>
void parseResponseValue_( rapidjson::Value &v ,Order::Market &orderMarket ) {
    orderMarket.id = v["id"].GetString();
    orderMarket.symbol = v["symbol"].GetString();
}

template <>
void parseResponseValue_( rapidjson::Value &v ,Order &order ) {
    order.id = v["id"].GetString();
    order.userProvidedId = v["userProvidedId"].GetString();

    rapidjson::Value &market = v["market"].GetObject();
    parseResponseValue_( market ,order.market );

    order.side = v["side"].GetString();
    order.type = v["type"].GetString();
    order.price = v["price"].GetString();
    order.quantity = v["quantity"].GetString();

    order.executedQuantity = v["executedQuantity"].GetString();
    order.remainQuantity = v["remainQuantity"].GetString();
    order.remainTotal = v["remainTotal"].GetString();
    order.remainTotalWithFee = v["remainTotalWithFee"].GetString();

    order.lastTradeAt = v["lastTradeAt"].GetInt64();
    order.status = v["status"].GetString();
    order.isActive = v["isActive"].GetBool();
    order.createdAt = v["createdAt"].GetInt64();
    order.updatedAt = v["updatedAt"].GetInt64();
}

//--
template <>
void parseResponseValue_( rapidjson::Value &v ,Trade::Market &tradeMarket ) {
    tradeMarket.id = v["id"].GetString();
    tradeMarket.symbol = v["symbol"].GetString();
}

template <>
void parseResponseValue_( rapidjson::Value &v ,Trade &trade ) {
    trade.id = v["id"].GetString();

    rapidjson::Value &market = v["market"].GetObject();
    parseResponseValue_( market ,trade.market );

    trade.orderId = v["orderid"].GetString();
    trade.side = v["side"].GetString();
    trade.triggeredBy = v["triggeredBy"].GetString();
    trade.price = v["price"].GetString();
    trade.quantity = v["quantity"].GetString();

    trade.fee = v["fee"].GetString();
    trade.totalWithFee = v["totalWithFee"].GetString();
    trade.alternateFeeAsset = getJsonString( v ,"alternateFeeAsset" );
    trade.alternateFee = getJsonString( v ,"alternateFee" );

    trade.createdAt = v["createdAt"].GetInt64();
    trade.updatedAt = v["updatedAt"].GetInt64();
}

//--
template <>
void parseResponseValue_( rapidjson::Value &v ,PoolTrade::Pool &tradePool ) {
    tradePool.id = v["id"].GetString();
    tradePool.symbol = v["symbol"].GetString();
}

template <>
void parseResponseValue_( rapidjson::Value &v ,PoolTrade &trade ) {
    trade.id = v["id"].GetString();

    rapidjson::Value &pool = v["pool"].GetObject();
    parseResponseValue_( pool ,trade.pool );

    trade.side = v["side"].GetString();
    trade.price = v["price"].GetString();
    trade.quantity = v["quantity"].GetString();
    trade.fee = v["fee"].GetString();
    trade.totalWithFee = v["totalWithFee"].GetString();

    trade.createdAt = v["createdAt"].GetInt64();
    trade.updatedAt = v["updatedAt"].GetInt64();
}

///-- POST

template <>
void generateBodyValue_( const OrderToCreate &order ,rapidjson::Value &v ,json_context_t &context ) {
    v.AddMember( "userProvidedId" ,toJsonString(order.userProvidedId) ,context );
    v.AddMember( "symbol" ,toJsonString(order.symbol) ,context );
    v.AddMember( "side" ,toJsonString(order.side) ,context );
    v.AddMember( "type" ,toJsonString(order.type) ,context );
    v.AddMember( "quantity" ,toJsonString(order.quantity) ,context );
    v.AddMember( "price" ,toJsonString(order.price) ,context );
    v.AddMember( "strictValidate" ,rapidjson::Value().SetBool(order.strictValidate) ,context );
}

template <>
void parseResponseValue_( rapidjson::Value &v ,OrderCreated &order ) {
    order.id = v["id"].GetString();
    order.userProvidedId = v["userProvidedId"].GetString();

    order.market = v["market"].GetString();
    order.user = v["user"].GetString();
    order.primaryAsset = v["primaryAsset"].GetString();
    order.secondaryAsset = v["secondaryAsset"].GetString();

    order.side = v["side"].GetString();
    order.type = v["type"].GetString();
    order.numberPrice = v["numberprice"].GetDouble();
    order.price = v["price"].GetString();
    order.quantity = v["quantity"].GetString();

    order.executedQuantity = v["executedQuantity"].GetString();
    order.remainQuantity = v["remainQuantity"].GetString();
    order.remainTotal = v["remainTotal"].GetString();
    order.remainTotalWithFee = v["remainTotalWithFee"].GetString();

    order.lastTradeAt = v["lastTradeAt"].GetInt64();
    order.status = v["status"].GetString();
    order.source = v["source"].GetString();
    order.isActive = v["isActive"].GetBool();
    order.isNew = v["isNew"].GetBool();
    order.feeRate = v["feeRate"].GetDouble();
    order.createdAt = v["createdAt"].GetInt64();
    order.updatedAt = v["updatedAt"].GetInt64();
}

//--
template <>
void generateBodyValue_( const OrderToCancel &order ,rapidjson::Value &v ,json_context_t &context ) {
    v.AddMember( "id" ,toJsonString(order.id) ,context );
}

template <>
void parseResponseValue_( rapidjson::Value &v ,OrderCancelled &order ) {
    order.success = v["success"].GetBool();
    order.id = v["id"].GetString();
}

//--
template <>
void generateBodyValue_( const OrdersToCancelAll &orders ,rapidjson::Value &v ,json_context_t &context ) {
    v.AddMember( "symbol" ,toJsonString(orders.symbol) ,context );
    v.AddMember( "side" ,toJsonString(orders.side) ,context );
}

template <>
void parseResponseValue_( rapidjson::Value &v ,OrdersCancelledAll &orders ) {
    orders.success = v["success"].GetBool();

    rapidjson::Value &ids = v["ids"].GetArray();
    parseResponseList_( ids ,orders.ids );
}

//--
template <>
void generateBodyValue_( const WithdrawalToCreate &withdrawal ,rapidjson::Value &v ,json_context_t &context ) {
    v.AddMember( "ticker" ,toJsonString(withdrawal.ticker) ,context );
    v.AddMember( "quantity" ,toJsonString(withdrawal.quantity) ,context );
    v.AddMember( "address" ,toJsonString(withdrawal.address) ,context );
    v.AddMember( "paymentid" ,toJsonString(withdrawal.paymentId) ,context );
}

template <>
void parseResponseValue_( rapidjson::Value &v ,WithdrawalCreated &withdrawal ) {
    withdrawal.id = v["id"].GetString();
    withdrawal.address = v["address"].GetString();
    withdrawal.paymentId = v["paymentid"].GetString();
    withdrawal.ticker = v["ticker"].GetString();
    withdrawal.childTicker = getJsonString( v ,"childticker" );
    withdrawal.quantity = v["quantity"].GetString();
    withdrawal.fee = v["fee"].GetString();
    withdrawal.feeCurrency = v["feecurrency"].GetString();
    withdrawal.status = v["status"].GetString();
    withdrawal.transactionId = v["transactionid"].GetString();

    withdrawal.isSent = v["issent"].GetBool();
    withdrawal.sentAt = getJsonInt64( v ,"sentat" );
    withdrawal.isConfirmed = v["isconfirmed"].GetBool();
    withdrawal.requestedAt = v["requestedat"].GetInt64();
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! ApiConfig

template <typename Iter>
static std::string EncodeHex( const Iter begin ,const Iter end ) {
    static const char hex[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

    std::string r;

    r.resize( (end - begin) * 2 );

    int i=0; for( auto it=begin; it<end; ++it, i+=2 ) {
        auto v = static_cast<uint8_t>(*it);

        r[i] = hex[v >> 4];
        r[i+1] = hex[v & 0x0f];
    }

    return r;
}

template <typename T>
inline std::string EncodeHex( const T &collection ) {
    return EncodeHex( std::begin(collection) ,std::end(collection) );
}

bool ApiSign( const ApiCredential &credential ,const char *url ,const char *body ,const char *nonce ,String &sign ) {
    // HexEncode( HmacSHA256( apiKey + url + body + nonce ,apiSecret ) );

    assert(url && nonce);

    String message;

    message += credential.apiKey;
    message += url;
    if( body ) message += body;
    message += nonce;

    const String &key = credential.apiSecret;

    using namespace CRYPTO_NAMESPACE;

    Data key_u8( key.begin() ,key.end() );
    Data msg_u8( message.begin() ,message.end() );

    // sign = EncodeHex( hmac256( key_u8 ,msg_u8 ) );
    assert(false); //TODO commented reference to TW

    return true;
}

//////////////////////////////////////////////////////////////////////////////
//! IHttp

void signApiBasic( const ApiCredential &credential ,String &userpass ) {
    String apiKey,apiSecret;

    solominer::globalDecypher( tocstr(credential.apiKey) ,apiKey );
    solominer::globalDecypher( tocstr(credential.apiSecret) ,apiSecret );

    userpass = apiKey + ":" + apiSecret;
}

bool signApiHmac( const ApiCredential &credential ,IHttp::Headers &headers ,const char *url ,const char *body ) {
    String nonce ,signature;

    nonce = std::to_string( (int) time( nullptr ) );
    ApiSign( credential ,url ,body ,nonce.c_str() ,signature );

    headers["X-API-KEY"] = credential.apiKey;
    headers["X-API-NONCE"] = nonce;
    headers["X-API-SIGN"] = signature;

    return true;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! CApi2

//////////////////////////////////////////////////////////////////////////////
///-- public

bool CApi2::assetGetList( ListOf<Asset> &assets ) {
    //TODO
        // => parse response List
        // => use list cache to fullfill request here below

    return false;
}

bool CApi2::assetGetById( const char *id ,Asset &asset ) {
    String response; long status;

    if( !HttpSend( "/asset/getbyid/" ,id ,nullptr ,response ,status ) )
        return( false );

    return parseResponseStruct_( response ,asset );
}

bool CApi2::assetGetByTicker( const char *ticker ,Asset &asset ) {
    String response; long status;

    if( !HttpSend( "/asset/getbyticker/" ,ticker ,nullptr ,response ,status ) )
        return( false );

    return parseResponseStruct_( response ,asset );
}

///-- market
bool CApi2::marketGetList( ListOf<MarketByList> &markets ) {
    String response; long status;

    if( !HttpSend( "/market/getlist" ,nullptr ,nullptr ,response ,status ) )
        return( false );

    return parseResponseList_( response ,markets );
}

bool CApi2::marketGetById( const char *id ,Market &market ) {
    return false;
}

bool CApi2::marketGetBySymbol( const char *symbol ,Market2 &market ) {
    static MapOf<String,bool> noMarket; //! @note avoid calling again if market doesn't exist (will not be cached because of error)

    if( noMarket.find(symbol) != noMarket.end() )
        return false;

    String response; long status;

    if( !HttpSend( "/market/getbysymbol/" ,symbol ,nullptr ,response ,status ,false ,false ) ) {
        return( false );
    }

    if( !parseResponseStruct_( response ,market ) ) {
        return noMarket[symbol] = false;
    }

    return true;
}

bool CApi2::marketGetOrderBookBySymbol( const char *symbol ,OrderBook &book ) {
    String response; long status;

    if( !HttpSend( "/market/getorderbookbysymbol/" ,symbol ,nullptr ,response ,status ,false ,m_noCache ) )
        return( false );

    m_noCache = false;

    return parseResponseStruct_( response ,book );
}

bool CApi2::marketGetOrderBookByMarketId( const char *marketId ,OrderBook &book ) {
    String response; long status;

    if( !HttpSend( "/market/getorderbookbymarketid/" ,marketId ,nullptr ,response ,status ,false ,m_noCache ) )
        return( false );

    m_noCache = false;

    return parseResponseStruct_( response ,book );
}

///-- pool

    //...

//////////////////////////////////////////////////////////////////////////////
///-- account

bool CApi2::balances( ListOf<Balance> &balances ) {
    String response; long status;

    if( !HttpSend( "/balances" ,nullptr ,nullptr ,response ,status ,true ,true ) )
        return( false );

    return parseResponseList_( response ,balances );
}

bool CApi2::getDepositAddress( const char *ticker ,DepositAddress &address ) {
    String response; long status;

    if( !HttpSend( "/getdepositaddress/" ,ticker ,nullptr ,response ,status ,true ) )
        return( false );

    return parseResponseStruct_( response ,address );
}

///-- POST
bool CApi2::CreateOrder( const OrderToCreate &createOrder ,OrderCreated &order ) {
    String body ,response; long status;

    generateBodyStruct_( createOrder ,body );

    if( !HttpSend( "/createorder" ,nullptr ,body.c_str() ,response ,status ,true ) )
        return( false );

    return parseResponseStruct_( response ,order );
}

bool CApi2::CancelOrder( const char *orderId ,OrderCancelled &order ) {
    String body ,response; long status;

    OrderToCancel cancelOrder;

    cancelOrder.id = orderId;

    generateBodyStruct_( cancelOrder ,body );

    if( !HttpSend( "/cancelorder" ,nullptr ,body.c_str() ,response ,status ,true ) )
        return( false );

    return parseResponseStruct_( response ,order );
}

bool CApi2::CancelAllOrders( const OrdersToCancelAll &cancelOrders ,OrdersCancelledAll &orders ) {
    String body ,response; long status;

    generateBodyStruct_( cancelOrders ,body );

    if( !HttpSend( "/cancelallorders" ,nullptr ,body.c_str() ,response ,status ,true ) )
        return( false );

    return parseResponseStruct_( response ,orders );
}

bool CApi2::CreateWithdrawal( const WithdrawalToCreate &createWithdrawal ,WithdrawalCreated &withdrawal ) {
    String body ,response; long status;

    generateBodyStruct_( createWithdrawal ,body );

    if( !HttpSend( "/createwithdrawal" ,nullptr ,body.c_str() ,response ,status ,true ) )
        return( false );

    return parseResponseStruct_( response ,withdrawal );
}

///--
bool CApi2::getDeposits( const char *ticker ,ListOf<Deposit> &deposits ,int limit ,int skip ) {
    String response; long status;

    std::stringstream ss;

    ss << "?ticker=" << ticker << "&limit=" << limit << "&skip=" << skip;

    if( !HttpSend( "/getdeposits" ,ss.str().c_str() ,nullptr ,response ,status ,true ,true ) )
        return( false );

    return parseResponseList_( response ,deposits );
}

bool CApi2::getWithdrawals( const char *ticker ,ListOf<Withdrawal> &withdrawals ,int limit ,int skip ) {
    String response; long status;

    std::stringstream ss;

    ss << "?ticker=" << ticker << "&limit=" << limit << "&skip=" << skip;

    if( !HttpSend( "/getwithdrawals" ,ss.str().c_str() ,nullptr ,response ,status ,true ,true ) )
        return( false );

    return parseResponseList_( response ,withdrawals );
}

bool CApi2::getOrder( const char *orderId ,Order &order ) {
    String response; long status;

    //! orderId = orderId | userProvidedId
    if( !HttpSend( "/getorder/" ,orderId ,nullptr ,response ,status ,true ,true ) )
        return( false );

    return parseResponseStruct_( response ,order );
}

bool CApi2::getOrders( const char *marketPair ,const char *orderStatus ,ListOf<Order> &orders ,int limit ,int skip ) {
    String response; long status;

    std::stringstream ss;

    ss << "?symbol=" << marketPair << "&status=" << orderStatus << "&limit=" << limit << "&skip=" << skip;

    if( !HttpSend( "/getorders" ,ss.str().c_str() ,nullptr ,response ,status ,true ,true ) )
        return( false );

    return parseResponseList_( response ,orders );
}

bool CApi2::getTrades( const char *symbol ,ListOf<Trade> &trades ,int limit ,int skip ) {
    String response; long status;

    std::stringstream ss;

    ss << "?symbol=" << symbol << "&limit=" << limit << "&skip=" << skip;

    if( !HttpSend( "/gettrades" ,ss.str().c_str() ,nullptr ,response ,status ,true ,true ) )
        return( false );

    return parseResponseList_( response ,trades );
}

bool CApi2::getTradesSince( const char *symbol ,ListOf<Trade> &trades ,const char *sinceTimestampMs ,int limit ,int skip ) {
    String response; long status;

    std::stringstream ss;

    ss << "?symbol=" << symbol << "&since=" << sinceTimestampMs << "&limit=" << limit << "&skip=" << skip;

    if( !HttpSend( "/gettradessince" ,ss.str().c_str() ,nullptr ,response ,status ,true ,true ) )
        return( false );

    return parseResponseList_( response ,trades );
}

bool CApi2::getPoolTrades( const char *symbol ,ListOf<PoolTrade> &trades ,int limit ,int skip ) {
    String response; long status;

    std::stringstream ss;

    ss << "?symbol=" << symbol << "&limit=" << limit << "&skip=" << skip;

    if( !HttpSend( "/getpolltrades" ,ss.str().c_str() ,nullptr ,response ,status ,true ,true ) )
        return( false );

    return parseResponseList_( response ,trades );
}

bool CApi2::getPoolTradesSince( const char *symbol ,ListOf<PoolTrade> &trades ,const char *sinceTimestampMs ,int limit ,int skip ) {
    String response; long status;

    std::stringstream ss;

    ss << "?symbol=" << symbol << "&since=" << sinceTimestampMs << "&limit=" << limit << "&skip=" << skip;

    if( !HttpSend( "/getpooltradessince" ,ss.str().c_str() ,nullptr ,response ,status ,true ,true ) )
        return( false );

    return parseResponseList_( response ,trades );
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! protected

bool CApi2::HttpSend( const char *path ,const char *param ,const char *body ,String &response ,long &status ,bool needAuth ,bool noCache ) {
    std::stringstream ss;

    ss << XEGGEX_HOSTNAME << XEGGEX_API2_PATH << path;
    if( param ) ss << param;

    const String &url = ss.str();

//-- headers
    IHttp::Headers headers;

    headers["accept"] = "application/json";

    if( body ) {
        headers["Content-type"] = "application/json";
    }

    if( noCache ) {
        headers["Cache-Control"] = "no-store";
    }

//-- auth
    String userpass;

    if( needAuth ) {
        switch( m_apiAuthorization ) {
            default:
            case authBasic:
                signApiBasic( m_config.credential ,userpass );
                break;
            case authHmac:
                signApiHmac( m_config.credential ,headers ,url.c_str() ,"" );
                break;
        }
    }

    return m_http.HttpSend( ss.str().c_str() ,headers ,userpass.c_str() ,body ,response ,status );
}

//////////////////////////////////////////////////////////////////////////////
} //namespace xeggex

//////////////////////////////////////////////////////////////////////////////
} //namespace nextwave

//////////////////////////////////////////////////////////////////////////////
//EOF