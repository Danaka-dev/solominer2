// Copyright (c) 2018-2023 The NExTWave developers      <http://www.nextwave-techs.com/>
// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include "xeggex-market.h"

#include <solominer.h>

#include <rapidjson/document.h>
#include <rapidjson/reader.h>
#include <rapidjson/stringbuffer.h>

#include <memory.h>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
static IServiceSetupRef g_setup( new CMarketSetupXeggex() );

static iresult_t g_hasMarket = getMarketStore().registerServiceSupport( XEGGEX_NAME ,g_setup );

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! IMarket <=> xeggex

template <class Ta ,class Tb>
ListOf<Ta> &fromManifest( ListOf<Ta> &p ,const ListOf<Tb> &manifest ) {

    try {
        for( auto &it : manifest ) {
            Ta a;

            fromManifest( a ,it );

            p.emplace_back( a );
        }

    } catch( ... ) {
        return p;
    }

    return p;
}

//////////////////////////////////////////////////////////////////////////////
template <>
MarketPair &fromManifest( MarketPair &p ,const xeggex::MarketByList &manifest ) {
    p.primaryCoin = manifest.primaryTicker;
    p.secondaryCoin = String( manifest.symbol ,p.primaryCoin.size()+1 );

    p.lastPrice = manifest.lastPrice;
    p.highPrice24h = manifest.highPrice;
    p.lowPrice24h = manifest.lowPrice;
    p.volume24h = manifest.volume;

    // p.chart24h;
    p.lastTradeTime = (int) manifest.lastTradeAt;
    p.priceDecimal = (int) manifest.priceDecimals;

    p.hasMarket = manifest.isActive;

    return p;
}

template <>
MarketPair &fromManifest( MarketPair &p ,const xeggex::Market2 &manifest ) {
    StringList list;
    Split( tocstr(manifest.symbol) ,list ,'/' );

    p.primaryCoin = (list.size() > 0) ? list[0] : "";
    p.secondaryCoin = (list.size() > 1) ? list[1] : "";

    p.lastPrice = manifest.lastPrice;
    p.highPrice24h = manifest.highPrice;
    p.lowPrice24h = manifest.lowPrice;
    p.volume24h = manifest.volume;

    p.chart24h = manifest.lineChart;
    p.lastTradeTime = (int) manifest.lastTradeAt;
    p.priceDecimal = (int) manifest.priceDecimals;

    p.hasMarket = manifest.isActive;

    return p;
}

template <>
MarketOrderBook::Offer &fromManifest( MarketOrderBook::Offer &p ,const xeggex::OrderBook::Offer &manifest ) {
    p.price = manifest.price;
    p.quantity = manifest.quantity;

    return p;
}

template <>
MarketOrderBook &fromManifest( MarketOrderBook &p ,const xeggex::OrderBook &manifest ) {
    p.marketId = manifest.marketid;
    fromMarketSymbol( manifest.symbol ,p.primaryAsset ,p.secondaryAsset );
    p.timestamp = manifest.timestamp;

    //-- bids
    for( const auto &it : manifest.bids ) {
        MarketOrderBook::Offer bid;

        fromManifest( bid ,it );

        p.bids.emplace_back(bid);
    }

    //-- asks
    for( const auto &it : manifest.asks ) {
        MarketOrderBook::Offer ask;

        fromManifest( ask ,it );

        p.asks.emplace_back(ask);
    }

//--
    return p;
}

//////////////////////////////////////////////////////////////////////////////
//! account

///-- balance
template <>
MarketBalance &fromManifest( MarketBalance &p ,const xeggex::Balance &manifest ) {
    fromString( p.amount.amount ,manifest.available );
    p.amount.value = manifest.asset;
    fromString( p.pending ,manifest.pending );
    p.id = manifest.assetId; //TODO is this right ?

    return p;
}

///-- deposit
template <>
MarketDeposit &fromManifest( MarketDeposit &p ,const xeggex::Deposit &manifest ) {
    p.id = manifest.id;
    p.txid = manifest.transactionId;

    fromString( p.amount.amount ,manifest.quantity );
    p.amount.value = manifest.ticker;

    p.fromAddress = "";
    p.toAddress = manifest.address;

    p.seenAt  = manifest.firstSeenAt;
    p.confirmations = manifest.confirmations;

    if( manifest.status == "Pending" ) {
        p.requiredConfirmations = p.confirmations + 1; //TODO find out how many from market instead
    }

    return p;
}

///-- withdraw
template <>
xeggex::WithdrawalToCreate &toManifest( const MarketWithdraw &p ,xeggex::WithdrawalToCreate &manifest ) {
    if( !p.fromAddress.empty() ) throw;

    manifest.ticker = p.amount.value;
    manifest.quantity = toString( p.amount.amount );
    manifest.address = p.toAddress;
    manifest.paymentId = "";

    return manifest;
}

template <>
MarketWithdraw &fromManifest( MarketWithdraw &p ,const xeggex::WithdrawalCreated &manifest ) {
    p.id = manifest.id;
    p.txid = manifest.transactionId;

    fromString( p.amount.amount ,manifest.quantity );
    p.amount.value = manifest.ticker;
    fromString( p.fee.amount ,manifest.fee );
    p.amount.value = manifest.feeCurrency;

    p.fromAddress = manifest.address; //TODO check if to or from
    // p.toAddress

    p.status = manifest.status; //TODO translate
    p.postedAt = manifest.requestedAt;
    p.sentAt = manifest.sentAt;

    return p;
}

template <>
MarketWithdraw &fromManifest( MarketWithdraw &p ,const xeggex::Withdrawal &manifest ) {
    String primaryAsset ,secondaryAsset;

    p.id = manifest.id;
    p.txid = manifest.transactionId;

    p.amount.amount = fromString<double>( manifest.quantity );
    p.amount.value = manifest.ticker;
    p.fee.amount = fromString<double>( manifest.fee );
    p.fee.value = manifest.feeCurrency;

    p.fromAddress = "";
    p.toAddress = manifest.address;

    p.status = manifest.status;
    p.postedAt = manifest.requestedAt;
    p.sentAt = manifest.sentAt;

    return p;
}

///-- exchange order

template <>
xeggex::OrderToCreate &toManifest( const MarketOrder &p ,xeggex::OrderToCreate &manifest ) {
    String symbol;

    manifest.userProvidedId = p.userId;
    manifest.symbol = toMarketSymbol( symbol ,p.amount.value ,p.toValue );
    manifest.side = (p.side == MarketOrder::Sell) ? "sell" : "buy";
    manifest.type = (p.price < 0.) ? "market" : "limit";
    manifest.quantity = toString( p.amount.amount );
    manifest.price = toString( MAX( p.price ,0. ) );
    manifest.strictValidate = false;

    return manifest;
}

template <>
MarketOrder &fromManifest( MarketOrder &p ,const xeggex::OrderCreated &manifest ) {
    p.id = manifest.id;

    p.amount.amount = fromString<double>( manifest.quantity );
    p.amount.value = manifest.primaryAsset ;

    p.toValue = manifest.secondaryAsset;
    p.price = fromString<double>( manifest.price );
    // p.validity;

    p.quantityFilled = fromString<double>( manifest.executedQuantity );

    p.status = manifest.status;
    p.seenAt = Now(); //! market doesn't know
    p.createdAt = manifest.createdAt;
    p.lastTradeAt = manifest.lastTradeAt;
    p.completedAt = manifest.updatedAt;

    return p;
}

template <>
MarketOrder &fromManifest( MarketOrder &p ,const xeggex::Order &manifest ) {
    String primaryAsset ,secondaryAsset;

    fromMarketSymbol( manifest.market.symbol ,primaryAsset ,secondaryAsset );

    if( manifest.side != "sell" )
        swap( primaryAsset ,secondaryAsset );

    p.id = manifest.id;
    p.userId = manifest.userProvidedId;

    p.amount.amount = fromString<double>( manifest.quantity );
    p.amount.value = primaryAsset;
    p.toValue = secondaryAsset;
    p.price = fromString<double>( manifest.price );
    p.side = (manifest.side == "buy") ? MarketOrder::Buy : MarketOrder::Sell;

    // p.validity; //TODO
    p.quantityFilled = fromString<double>( manifest.executedQuantity );

    p.status = manifest.status;
    p.seenAt = 0; //! market doesn't know
    p.createdAt = manifest.createdAt;
    p.lastTradeAt = manifest.lastTradeAt;
    p.completedAt = manifest.updatedAt;

    return p;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! IMarket

IAPI_DEF CMarketXeggex::getAssetList( ListOf<String> &assets ) {
    return ENOEXEC;
}

IAPI_DEF CMarketXeggex::getAssetInfo( const char *coin ,MarketAsset &info ) {
    xeggex::Asset asset;

    if( !api().assetGetByTicker( coin ,asset ) )
        return IERROR;

///-- transpose asset info to MarketAsset
    info.ticker = asset.ticker;
    info.name = asset.name;
    info.value = std::stod( asset.usdValue );
    info.currency = "usd";

    return IOK;
}

IAPI_DEF CMarketXeggex::listMarkets( ListOf<MarketPair> &markets ) {
    ListOf<xeggex::MarketByList> listOfMarketByList;

    if( !api().marketGetList( listOfMarketByList ) )
        return IERROR;

    fromManifest( markets ,listOfMarketByList );

    return IOK;
}

IAPI_DEF CMarketXeggex::getMarketPair( const char *primary ,const char *secondary ,MarketPair &pair ) {
    xeggex::Market2 market;

    std::stringstream ss;

    ss << primary << "_" << secondary;

    if( !api().marketGetBySymbol( tocstr(ss.str()) ,market ) )
        return IERROR;

    fromManifest( pair ,market );

    return IOK;
}

IAPI_DEF CMarketXeggex::getOrderBook( const char *primary ,const char *secondary ,MarketOrderBook &book ) {
    xeggex::OrderBook xbook;

    std::stringstream ss;

    ss << primary << "_" << secondary;

    if( !api().marketGetOrderBookBySymbol( ss.str().c_str() ,xbook ) )
        return IERROR;

    fromManifest( book ,xbook );

    return IOK;
}

//////////////////////////////////////////////////////////////////////////////
//! account

IAPI_DEF CMarketXeggex::getBalance( const char *value ,MarketBalance &balance ) {
    ListOf<xeggex::Balance> xbalances;

    if( !api().balances( xbalances ) )
        return IERROR;

    for( const auto &it : xbalances ) {
        if( it.asset == value ) {
            fromManifest( balance ,it );
            return IOK;
        }
    }

    return INODATA;
}

IAPI_DEF CMarketXeggex::getDepositAddress( const char *coin ,String &address ) {
    xeggex::DepositAddress xaddress;

    if( !api().getDepositAddress( coin ,xaddress ) )
        return IERROR;

    address = xaddress.address;

    return IOK;
}

IAPI_DEF CMarketXeggex::getDeposit( const char *value ,const char *transactionId ,MarketDeposit &deposit ) {
    //! NB XeggeX doesn't support retrieving deposit from address, listing and filtering manually

    ListOf<xeggex::Deposit> xdeposits;

    const int nlist = 50;

    int i=0; while(true) {
        if( !api().getDeposits( value ,xdeposits ,nlist ,i ) )
            return INODATA;

        if( xdeposits.empty() )
            return INODATA;

        for( const auto &it :xdeposits ) {
            if( it.transactionId == transactionId ) {
                fromManifest( deposit ,it );
                return IOK;
            }
        }

        i += nlist;
    };

    return INODATA;
}

IAPI_DEF CMarketXeggex::CreateWithdraw( MarketWithdraw &withdraw ) {
    xeggex::WithdrawalToCreate withdrawalToCreate;
    xeggex::WithdrawalCreated withdrawalCreate;

    try {
        toManifest( withdraw ,withdrawalToCreate );
    } catch( ... ) {
        return IBADARGS;
    }

    if( !api().CreateWithdrawal( withdrawalToCreate ,withdrawalCreate ) )
        return IERROR;

    fromManifest( withdraw ,withdrawalCreate );

    return IOK;
}

IAPI_DEF CMarketXeggex::getWithdraw( const char *value ,const char *transactionId ,MarketWithdraw &withdraw ) {

    ListOf<xeggex::Withdrawal> xwithdrawwals;

    const int nlist = 50;

    int i=0; while(true) {
        if( !api().getWithdrawals( value ,xwithdrawwals ,nlist ,i ) )
            return INODATA;

        if( xwithdrawwals.empty() )
            return INODATA;

        for( const auto &it :xwithdrawwals ) {
            if( it.transactionId == transactionId ) {
                fromManifest( withdraw ,it );
                return IOK;
            }
        }

        i += nlist;
    };

    return INODATA;
}

IAPI_DEF CMarketXeggex::CancelWithdraw( const char *id ) {
    //TODO here ... and other IMarket to XeggeX
    return INOEXEC;
}

IAPI_DEF CMarketXeggex::CreateOrder( MarketOrder &order ) {
    xeggex::OrderToCreate orderToCreate;
    xeggex::OrderCreated orderCreated;

    try {
        toManifest( order ,orderToCreate );
    } catch( ... ) {
        return IBADARGS;
    }

    if( !api().CreateOrder( orderToCreate ,orderCreated ) )
        return IERROR;

    fromManifest( order ,orderCreated );

    return IOK;
}

IAPI_IMPL CMarketXeggex::getOrder( const char *id ,MarketOrder &order ) {
    xeggex::Order xorder;

    if( !api().getOrder( id ,xorder ) )
        return IERROR;

    fromManifest( order ,xorder );

    return IOK;
}

IAPI_IMPL CMarketXeggex::CancelOrder( const char *id ,MarketOrder &order ) {
    //TODO here ... and other IMarket to XeggeX
    return INOEXEC;
}

//--
IAPI_DEF CMarketXeggex::listBalances( ListOf<MarketBalance> &balances ,int from ,int count ,bool includeZeroBalance ) {
    ListOf<xeggex::Balance> xbalances;

    if( !api().balances( xbalances ) )
        return IERROR;

    //! @note xeggex doesn't have an option to not include zero balance
    double available;

    for( auto it = xbalances.begin(); it != xbalances.end(); ) {
        fromString( available ,it->available );

        if( Equals( available ,0. ) ) {
            xbalances.erase(it);
        } else {
            ++it;
        }
    }

    fromManifest( balances ,xbalances );

    return IOK;
}

IAPI_DEF CMarketXeggex::listDeposits( const char *value ,std::vector<MarketDeposit> &deposits ,int from ,int count ) {
    ListOf<xeggex::Deposit> xdeposits;

    if( !api().getDeposits( value ,xdeposits ,count ,from ) )
        return IERROR;

    MarketDeposit deposit;

    for( const auto &it : xdeposits ) {
        fromManifest( deposit ,it );

        deposits.emplace_back( deposit );
    }

    return IOK;
}

IAPI_DEF CMarketXeggex::listWithdrawals( const char *value ,std::vector<MarketWithdraw> &deposits ,int from ,int count ) {
    //TODO here ... and other IMarket to XeggeX
    return INOEXEC;
}

IAPI_DEF CMarketXeggex::listOrders( const char *pair ,std::vector<MarketOrder> &orders ,int from ,int count ) {
    ListOf<xeggex::Order> xorders;

    if( !api().getOrders( pair ,"active" ,xorders ) ) //! TODO .. limit vs since ?
        return IERROR;

    MarketOrder order;

    for( const auto &it : xorders ) {
        fromManifest( order ,it );

        orders.emplace_back( order );
    }

    return IOK;
}

//////////////////////////////////////////////////////////////////////////////
bool CMarketXeggex::HttpSend( const char *url ,const Headers &headers ,const char *userpass ,const char *body ,String &response ,long &status ) {

//-- content
    HttpMessage message;

    message.content_type = "application/json";
    message.content = body ? body : "";
    message.accept_type = "application/json";

//-- headers
    CHttpRequest request;

    request.headers() = headers;
    request.userpass() = userpass;

//-- method
    HttpResponse httpResponse;

    iresult_t result = m_http.sendRequest( url ,body ? HttpMethod::methodPOST : HttpMethod::methodGET ,message ,request ,httpResponse );

    response = httpResponse.content;
    status = (long) httpResponse.status;

    IF_IFAILED( result ) {
        return false;
    }

    m_http.adviseRequestValid( request );

    return true;
};

//////////////////////////////////////////////////////////////////////////////
//! Setup

bool CMarketSetupXeggex::connectNew( ServiceInfo &info ,const Params &params ,IServiceRef &service ) {
    IServiceSetupRef setup( *this );

    auto *xeggex = new CMarketXeggex( setup );

    xeggex->api().config().credential = {
        getMember( params ,"api_key" ,"" )
        ,getMember( params ,"api_secret" ,"" )
    };

    service = xeggex;

    return true;
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF