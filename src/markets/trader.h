#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_TRADER_H
#define SOLOMINER_TRADER_H

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>
#include <common/book.h>
#include <common/service.h>

#include <interface/IMarket.h>
#include <interface/IWallet.h>

#include "broker.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
#define TRADEINFO_UUID      0x04a5633b0860e9347
#define CTRADER_UUID        0x0251a286ac10b601f

class ITraderEvents;
class CTraderEventListener;
class CTrader;

//////////////////////////////////////////////////////////////////////////////
//! Schedule

struct SchedulePeriod {
    int every;

    enum Period {
        immediate ,second ,minute ,hour ,day ,week
    } period;

    TimeSec at;
};

    // "every 4 hour at 0

time_t getScheduleTime( const SchedulePeriod &period );

//--
INLINE_FROMSTRING(SchedulePeriod) {
    StringList list;

    fromString( list ,s );

    if( list.size() > 0 ) fromString( p.every ,list[0] ,size );
    if( list.size() > 1 ) enumFromString( p.period ,list[1] ,size );
    if( list.size() > 2 ) fromString( p.at ,list[2] ,size );

    return p;
}

INLINE_TOSTRING(SchedulePeriod) {
    StringList list;

    list.resize(3);

    toString( p.every ,list[0] );
    enumToString( p.period ,list[1] );
    toString( p.at ,list[2] );

    return toString( list ,s );
}

//////////////////////////////////////////////////////////////////////////////
//! Config

struct TraderConfig {
    SchedulePeriod schedule; //! default schedule for all markets
    AmountValue minimumTrade;

    //TODO LATER use below
    MapOf<String,SchedulePeriod> marketSchedule; //! schedule per market
    MapOf<String,double> minimumValues; //! minimum amount required to trade each value
};

// fromManifest( )

//--
time_t getConfigSchedule( const TraderConfig &config ,const char *market );

//////////////////////////////////////////////////////////////////////////////
//! TradeBook

struct TradeInfo : BookEntry {
    DECLARE_CLASSID(TRADEINFO_UUID)

    static size_t sizeofEntry() { return 1024; };

//-- info
    guid_t id; //! globally unique trade id

    String market; //! market name or '*' to select market with best price for the trade

    AmountValue amount;
    String toValue;
    double price;

    String depositFromAddress; //! empty if deposit shall not be executed by trader (deposit will still be verified before executing trade)
    String withdrawToAddress; //! empty if no withdraw (keep in exchange)

    enum Schedule {
        executeNow=0 ,executeOnSchedule ,executeLater
    } schedule;

    time_t timeToExecute;

//-- book-keeping
    guid_t orderId; //! order trade was affected to

    enum Status {
        noStatus=0
        ,recorded       //! first time order was recorded (added to pending orders)
        ,placed         //! order was placed with broker
        ,executing      //! execution started, in progress
        ,completed      //! execution ended successfully
        ,abandoned      //! execution was abandoned (trade expired, too many retries, unrecoverable error)
        ,cancelled      //! execution successfully cancelled
        //? refunded ?
    } status;

    time_t timeRecorded;  //! time trade order was added
    time_t timePlaced;    //! time trade was placed with broker
    time_t timeExecuted;  //! time execution started (by broker)
    time_t timeCompleted; //! time trade was completed
};

DEFINE_MEMBER_API(TradeInfo);
DEFINE_WITHSCHEMA_API(TradeInfo);

template <> TradeInfo &Zero( TradeInfo &p );

//--
typedef CBookFile_<TradeInfo> CTradeBook;
typedef CBookDataSource_<TradeInfo,TradeInfo> CTradeDataSource;

inline bool isCompleted( const TradeInfo &trade ) {
    return trade.status >= TradeInfo::completed;
}

time_t getTradeSchedule( const TradeInfo &trade ,time_t onScheduleTime );

//////////////////////////////////////////////////////////////////////////////
//! TradeOrder

    //! @brief trade(s) (TradeInfo) bundle to send as a broker order

struct TradeOrder {
    String market;

    String depositFromAddress;
    String withdrawToAddress;    //! zero if no withdraw (keep in exchange)

    double withdrawPercent;     //! percent of order output to withdraw

    // MarketOrder order;
    AmountValue amount;
    String toValue;
    double price;

    time_t timeToExecute;

    ListOf<TradeInfo::Id> trades;   //! trades regrouped in this order
};

//--
template <> TradeOrder &Zero( TradeOrder &a );

//////////////////////////////////////////////////////////////////////////////
//! Trade events

class ITradeEvents : IOBJECT_PARENT {
public:
    IAPI_DECL onTradeUpdate( CTrader &trader ,const TradeInfo &trade ) = 0;
};

class CTradeEventSource : public CPublisher_<ITradeEvents> {
public:
    IAPI_DECL PostTradeUpdate( CTrader &trader ,const TradeInfo &trade ) {
        for( auto &it : m_subscribers ) {
            it->onTradeUpdate(trader,trade);
        }
        return IOK;
    }
};

class CTradeEventListener : public ITradeEvents {
public:
    IAPI_IMPL onTradeUpdate( CTrader &trader ,const TradeInfo &trade ) IOVERRIDE {
        return IOK;
    }
};

//////////////////////////////////////////////////////////////////////////////
//! CTrader

class CTrader : public CTradeEventSource ,public CBrokerEventListener ,COBJECT_PARENT ,public Singleton_<CTrader>
{
public: //-- definitions
    typedef Map_<TradeInfo::Id,TradeInfo> ledger_t;

public: //-- instance
    CTrader() : m_started(false) {}

    DECLARE_OBJECT_STD(CObject,CTrader,CTRADER_UUID)

    CTradeBook &getTradeBook() { return m_tradeBook; }

    bool isStarted() { return m_started; }

public: ///-- ITrader
    IAPI_DECL Start( Config &config ,const char *path="" );
    IAPI_DECL Stop();

//--
    IAPI_DECL getTradeHistory( id_t begin ,id_t end ,ledger_t &trades );
    IAPI_DECL listOpenTrades( ListOf<TradeInfo::Id> &trades );
    IAPI_DECL findOpenTrades( ledger_t &trades );
    IAPI_DECL findTrade( const guid_t &id ,TradeInfo &trade ,bool openOnly=true );

//--
    IAPI_DECL getTrade( TradeInfo::Id sequence ,TradeInfo &trade );
    IAPI_DECL makeTrade( TradeInfo &info );
    IAPI_DECL placeTrade( const TradeInfo &info ,TradeInfo::Id &id );
    IAPI_DECL cancelTrade( TradeInfo::Id sequence );

//--
    IAPI_DECL processTrades(); //! pulsing trade processing
    IAPI_DECL updateTrade( const TradeInfo &trade ); //! trade has update

protected: ///-- IBrokerEvents
    IAPI_IMPL onOrderUpdate( CBroker &broker ,const BrokerOrder &order ) IOVERRIDE;

protected:
    static bool validateTrade( const TradeInfo &trade );
    NoDiscard bool checkOrderValue( const TradeOrder &order ) const;
    NoDiscard bool isOrderReadyForBroker( const TradeOrder &order ,time_t now ) const ;

    bool addTradeToOrders( const TradeInfo &trade );

    bool findOrderForTrade( const TradeInfo &trade );
    bool makeOrderForTrade( const TradeInfo &trade );

    bool matchTradeToOrder( const TradeInfo &trade ,const TradeOrder &order );
    bool addTradeToOrder( const TradeInfo &trade ,TradeOrder &order );

    //--
    bool placeBrokerOrder( const TradeOrder &order );

protected:
    bool m_started;

    TraderConfig m_config;

    CTradeBook m_tradeBook;

    ListOf<TradeOrder> m_pendingOrders; //! bundling trade into orders, to be send to broker when schedule condition are met

    time_t m_nextScheduleTime;
};

//////////////////////////////////////////////////////////////////////////////
inline CTrader &getTrader() {
    return CTrader::getInstance();
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_TRADER_H