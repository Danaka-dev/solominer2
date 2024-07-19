// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include "trader.h"

#include <solominer.h>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//! schedule

template <>
const char *Enum_<SchedulePeriod::Period>::names[] = {
    "immediate" ,"second" ,"minute" ,"hour" ,"day" ,"week"
};

template <>
const SchedulePeriod::Period Enum_<SchedulePeriod::Period>::values[] = {
    SchedulePeriod::immediate ,SchedulePeriod::second ,SchedulePeriod::minute ,SchedulePeriod::hour ,SchedulePeriod::day ,SchedulePeriod::week
};

///--
time_t getScheduleTime( const SchedulePeriod &period ) {

//-- duration
    time_t duration; //! in seconds

    switch( period.period ) {
        default:
        case SchedulePeriod::immediate:
            duration = 0; break;
        case SchedulePeriod::second:
            duration = 1; break;
        case SchedulePeriod::minute:
            duration = 60; break;
        case SchedulePeriod::hour:
            duration = 3600; break;
        case SchedulePeriod::day:
            duration = 24*3600; break;
        case SchedulePeriod::week:
            duration = 7*24*3600; break;
    }

    duration *= period.every;

//-- base
    time_t at = period.at % duration;
    time_t t = Now();

    return t - (t % duration) + duration + at;
}

time_t getConfigSchedule( const TraderConfig &config ,const char *market ) {

//-- market schedule
    //! @note if market is not specified trader market schedule is always bypassed

    if( market && market[0] && market[0]!='*' ) {
        const auto &markets = config.marketSchedule;

        const auto &it = markets.find( market );

        if( it != markets.end() ) { //! market schedule in use
            return getScheduleTime( it->second );
        }
    }

    //! using global schedule (default)
    return getScheduleTime( config.schedule );
}

time_t getTradeSchedule( const TradeInfo &trade ,time_t onScheduleTime ) {
    switch( trade.schedule ) {
        case TradeInfo::executeNow:
            return Now(); break;
        case TradeInfo::executeOnSchedule:
            return onScheduleTime; break;
        case TradeInfo::executeLater:
            return trade.timeToExecute; break;

        default:
            assert(false); //! should not happen
            return 0;
    }
}

//////////////////////////////////////////////////////////////////////////////
//! TradeInfo

template <>
const char *Enum_<TradeInfo::Schedule>::names[] = {
    "now" ,"onschedule" ,"later"
};

template <>
const TradeInfo::Schedule Enum_<TradeInfo::Schedule>::values[] = {
    TradeInfo::executeNow ,TradeInfo::executeOnSchedule ,TradeInfo::executeLater
};

template <>
const char *Enum_<TradeInfo::Status>::names[] = {
    "" ,"recorded" ,"placed" ,"executing" ,"completed" ,"abandoned" ,"cancelled"
};

template <>
const TradeInfo::Status Enum_<TradeInfo::Status>::values[] = {
    TradeInfo::noStatus ,TradeInfo::recorded ,TradeInfo::placed ,TradeInfo::executing ,TradeInfo::completed ,TradeInfo::abandoned ,TradeInfo::cancelled
};

//--
template <>
const Schema Schema_<TradeInfo>::schema = fromString( Schema::getStatic() ,String(
    "id:Guid"
    ",market:String"
    ",amount:AmountValue"
    ",toValue:String"
    ",price:double"
    ",depositFromAddress:string"
    ",withdrawToAddress:string"
    ",schedule:EnumSchedule"
    ",timeToExecute:timesec"
    ",orderId:guid"
    ",status:EnumStatus"
    ",timeRecorded:timesec"
    ",timePlaced:timesec"
    ",timeExecuted:timesec"
    ",timeCompleted:timesec"
) );

DEFINE_SETMEMBER(TradeInfo) {
    switch( m ) {
        case 0: fromString( p.id ,s ); return;
        case 1: fromString( p.market ,s ); return;
        case 2: fromString( p.amount ,s ); return;
        case 3: fromString( p.toValue ,s ); return;
        case 4: fromString( p.price ,s ); return;
        case 5: fromString( p.depositFromAddress ,s ); return;
        case 6: fromString( p.withdrawToAddress ,s ); return;
        case 7: enumFromString( p.schedule ,s ); return;
        case 8: fromString( p.timeToExecute ,s ); return;
        case 9: fromString( p.orderId ,s ); return;
        case 10: enumFromString( p.status ,s ); return;
        case 11: fromString( p.timeRecorded ,s ); return;
        case 12: fromString( p.timePlaced ,s ); return;
        case 13: fromString( p.timeExecuted ,s ); return;
        case 14: fromString( p.timeCompleted ,s ); return;
        default: break;
    }
}

DEFINE_GETMEMBER(TradeInfo) {
    switch( m ) {
        case 0: toString( p.id ,s ); return s;
        case 1: toString( p.market ,s ); return s;
        case 2: toString( p.amount ,s ); return s;
        case 3: toString( p.toValue ,s ); return s;
        case 4: toString( p.price ,s ); return s;
        case 5: toString( p.depositFromAddress ,s ); return s;
        case 6: toString( p.withdrawToAddress ,s ); return s;
        case 7: enumToString( p.schedule ,s ); return s;
        case 8: toString( p.timeToExecute ,s ); return s;
        case 9: toString( p.orderId ,s ); return s;
        case 10: enumToString( p.status ,s ); return s;
        case 11: toString( p.timeRecorded ,s ); return s;
        case 12: toString( p.timePlaced ,s ); return s;
        case 13: toString( p.timeExecuted ,s ); return s;
        case 14: toString( p.timeCompleted ,s ); return s;
        default: return s;
    }
}

//--
template <> TradeInfo &Zero( TradeInfo &p ) {
    p.sequence = 0;

//--
    Zero( p.id );
    Zero( p.market );
    Zero( p.amount );
    Zero( p.toValue );
    p.price = 0.;
    Zero( p.depositFromAddress );
    Zero( p.withdrawToAddress );
    p.schedule = TradeInfo::executeNow;
    p.timeToExecute = 0;

//-- book-keeping
    Zero( p.orderId );
    p.status = TradeInfo::noStatus;
    p.timeRecorded = 0;
    p.timePlaced = 0;
    p.timeExecuted = 0;
    p.timeCompleted = 0;

//--
    return p;
}

//////////////////////////////////////////////////////////////////////////////
//! TradeOrder

template <> TradeOrder &Zero( TradeOrder &p ) {
    p.withdrawPercent = 100.;
    p.timeToExecute = 0;

    Zero( p.market );
    Zero( p.amount );
    Zero( p.toValue );
    p.price = 0.;
    Zero( p.depositFromAddress );
    Zero( p.withdrawToAddress );
    p.trades.clear();
    return p;
}

//////////////////////////////////////////////////////////////////////////////
bool matchMarket( const char *requested ,const char *proposed ) {
    //! no request
    if( !requested || requested[0]==0 || requested[0]=='*' ) return true;

    //! universal proposal
    if( proposed[0]=='*' ) return true;

    //! else simple match (case insensitive)
    return stricmp( requested ,proposed ) == 0;
}

bool matchValue( const AmountValue &requested ,const String &proposed ) {
    return requested.value == proposed;
}

/* bool matchOrder( const MarketOrder &requested ,const MarketOrder &proposed ) {
    return
        requested.amount.value == proposed.amount.value
        && requested.toValue == proposed.toValue
    ;
} */

//////////////////////////////////////////////////////////////////////////////
//! CTrader

IAPI_DEF CTrader::Start( Config &config ,const char *path ) {

//-- open trade book
    if( IFAILED(m_tradeBook.Open( "trade" ,path ,true )) )
        return IERROR;

//-- set config
    auto &traderConfig = config.getSection("trader");

    //TODO set m_config with configfile

    //TEST
    m_config.schedule = {
        10 , SchedulePeriod::second ,0
    };

    m_config.minimumValues[ "RTC" ] = 5;

    m_nextScheduleTime = 0;

//-- create pending order list
    m_tradeBook.eachEntry( [this]( TradeInfo::Id id ,TradeInfo &trade ) -> bool {
        if( trade.status < TradeInfo::placed )
            addTradeToOrders( trade );

        return true;
    });

//-- subscribe to broker events
    getBroker().Subscribe(*this);

    m_started = true;

    return IOK;
}

IAPI_DEF CTrader::Stop() {
    //TODO update m_config with configfile + close

    m_started = false;

    m_tradeBook.Close();

    return IOK;
}

///--
IAPI_DEF CTrader::getTradeHistory( id_t begin ,id_t end ,ledger_t &trades ) {
    _TODO; return INOEXEC;
    // return m_tradeBook.getHistory( begin ,end ,entries ) ? IOK : INODATA;
}

IAPI_DEF CTrader::listOpenTrades( ListOf<TradeInfo::Id> &trades ) {
    trades.clear();

    m_tradeBook.eachEntry( [&trades]( TradeInfo::Id id ,TradeInfo &trade ) -> bool {
        if( !isCompleted(trade) ) trades.emplace_back(id);

        return true; //TODO id last open trade in file to avoid going thru all history
    });

    /* for( const auto &order : this->m_pendingOrders ) {
        for( const auto &tradeId : order.trades ) {
            trades.emplace_back( tradeId );
        }
    } */

    // return m_tradeBook.listEntries( orders ) ? IOK : INODATA;

    return IOK;
}

IAPI_DEF CTrader::findOpenTrades( ledger_t &trades ) {
    m_tradeBook.eachEntry( [&trades]( TradeInfo::Id id ,TradeInfo &trade ) -> bool {
        if( !isCompleted(trade) ) trades[id] = trade;

        return true; //TODO id last open trade in file to avoid going thru all history
    });

    return IOK;
}

IAPI_DEF CTrader::findTrade( const guid_t &id ,TradeInfo &trade ,bool openOnly ) {
    _TODO; return INOEXEC;
    // return INODATA;
}

///--
IAPI_DEF CTrader::getTrade( TradeInfo::Id sequence ,TradeInfo &trade ) {
    return m_tradeBook.getEntry( sequence ,trade ) ? IOK : INODATA;
}

IAPI_DEF CTrader::makeTrade( TradeInfo &info ) {
    Zero(info);

    Make( info.id );
    info.market = "*";

    return IOK;
}

IAPI_DEF CTrader::placeTrade( const TradeInfo &trade ,TradeInfo::Id &id ) {
    if( !isStarted() )
        return IBADENV;

    if( !isTraderEnabled() )
        return IREFUSED;

    if( !validateTrade(trade) )
        return IBADARGS;

//-- record entry
    TradeInfo entry = trade;

    entry.status = TradeInfo::recorded;
    entry.timeRecorded = Now();

    id = m_tradeBook.addEntry( entry );

    if( id == INVALID_ENTRY_VALUE )
        return IERROR;

//-- place trade in pending order
    addTradeToOrders( trade );

    return updateTrade(entry);
}

IAPI_DEF CTrader::cancelTrade( TradeInfo::Id sequence ) {
    //TODO find order, check order has only one trade, check order cancellable ,cancel order

    return ENOEXEC;
}

///--
IAPI_DEF CTrader::processTrades() {
    if( !isStarted() )
        return IBADENV;

    if( !isTraderEnabled() )
        return IREFUSED;

//--
    time_t now = Now();

    if( m_nextScheduleTime > now )
        return IOK;

    m_nextScheduleTime = 0;

//-- place orders
    for( auto it=m_pendingOrders.begin(); it!=m_pendingOrders.end(); ) {
        auto const &order = *it;

        if( isOrderReadyForBroker( order ,now ) ) {
            if( placeBrokerOrder( order ) )
                m_pendingOrders.erase(it);

        } else {
            time_t t = order.timeToExecute;

            if( m_nextScheduleTime < now || m_nextScheduleTime > t )
                m_nextScheduleTime = t;

            ++it;
        }
    }

    return IOK;
}

IAPI_DEF CTrader::updateTrade( const TradeInfo &trade ) {

//-- update book
    if( !m_tradeBook.updateEntry( trade.sequence ,trade ) ) {
        //! should not happen. NB if allowed would require to rollback
        assert(false);
        return IFATAL;
    }

//-- publish event
    CTradeEventSource::PostTradeUpdate( *this ,trade );

    return IOK;
}

///-- IBrokerEvents
IAPI_DEF CTrader::onOrderUpdate( CBroker &broker ,const BrokerOrder &order ) {

    m_tradeBook.eachEntry( [this,order]( BookEntry::Id id ,TradeInfo &trade ) -> bool {
        if( !Equals( trade.orderId ,order.id ) ) return true;

        TradeInfo::Status status = trade.status;

        //-- check for relevant update to trade
        if( status == TradeInfo::placed ) { //! @note any update from order mean execution is in progress
            status = TradeInfo::executing;
        }

        if( order.stage == BrokerOrder::completed) {
            switch( getStatus(order) ) {
                case BrokerOp::skipped :
                case BrokerOp::completed :
                    status = TradeInfo::completed; break;
                case BrokerOp::abandoned :
                    status = TradeInfo::abandoned; break;
                case BrokerOp::cancelled :
                    status = TradeInfo::cancelled; break;

                default:
                    break;
            }
        }

        //-- update trade on status changed
        if( trade.status != status ) {
            if( status == TradeInfo::executing )
                trade.timeExecuted = Now();
            else
                trade.timeCompleted = Now();

            trade.status = status;

            updateTrade( trade );
        }

        return false;
    });

    return IOK;
}

///--
bool CTrader::validateTrade( const TradeInfo &trade ) {
    return
        trade.amount.amount > 0.
        && !trade.amount.value.empty()
        && !trade.toValue.empty()
    ;
}

bool CTrader::checkOrderValue( const TradeOrder &order ) const {
    AmountValue amount = order.amount;

    const auto &minimums = m_config.minimumValues;

    const auto &it = minimums.find( amount.value );

    return it == minimums.end() || it->second <= amount.amount;
}

bool CTrader::isOrderReadyForBroker( const TradeOrder &order ,time_t now ) const {
    return order.timeToExecute < Now() && checkOrderValue( order );
}

//! @brief place trade in pending order
bool CTrader::addTradeToOrders( const TradeInfo &trade ) {

    if( !findOrderForTrade( trade ) && !makeOrderForTrade( trade ) )
        return false;

    return ISUCCESS( updateTrade( trade ) );
}

//--
bool CTrader::findOrderForTrade( const TradeInfo &trade ) {
    assert( trade.status < TradeInfo::placed );

    for( auto &it : m_pendingOrders ) {
        auto &order = it;

        if( addTradeToOrder(trade,order) ) {
            return true;
        }
    }

    return false;
}

bool CTrader::makeOrderForTrade( const TradeInfo &trade ) {
    assert( trade.status < TradeInfo::placed );

    time_t configSchedule = getConfigSchedule( m_config ,trade.market.c_str() );

    TradeOrder order;

    Zero(order);

    order.market = trade.market; //! NB cannot findOrderMarket, market will be resolved by broker at execution point
    order.amount = trade.amount;
    order.toValue = trade.toValue;
    order.price = trade.price;
    order.depositFromAddress = trade.depositFromAddress;
    order.withdrawToAddress = trade.withdrawToAddress;
    order.timeToExecute = getTradeSchedule( trade ,configSchedule );

    order.trades.emplace_back( trade.sequence );

//--
    m_pendingOrders.emplace_back( order );

    if( m_nextScheduleTime == 0 || m_nextScheduleTime > order.timeToExecute )
        m_nextScheduleTime = order.timeToExecute;

    return true;
}

bool CTrader::matchTradeToOrder( const TradeInfo &trade ,const TradeOrder &order ) {
    time_t configSchedule = getConfigSchedule( m_config ,trade.market.c_str() );
    time_t tradeSchedule = getTradeSchedule( trade ,configSchedule );

    switch( trade.schedule ) {
        case TradeInfo::executeNow :
            if( configSchedule > Now() + 5 ) return false;
            break;
        case TradeInfo::executeOnSchedule :
            break;
        case TradeInfo::executeLater :
            if( abs( (int) (configSchedule - tradeSchedule) ) > 5 ) return false;
            break;
    }

    //TODO find best schedule ! ... not a single one ... or order lookup by orders scheduleTime

    return
        matchMarket( trade.market.c_str() ,order.market.c_str() )
        && trade.amount.value == order.amount.value
        && trade.toValue == order.toValue
        && trade.price == order.price
        && trade.depositFromAddress == order.depositFromAddress
        && trade.withdrawToAddress == order.withdrawToAddress
        && tradeSchedule < order.timeToExecute
    ;
}

bool CTrader::addTradeToOrder( const TradeInfo &trade ,TradeOrder &order ) {
    if( !matchTradeToOrder( trade ,order ) )
        return false;

    order.amount.amount += trade.amount.amount;

    order.trades.emplace_back( trade.sequence );

    return true;
}

//--
bool CTrader::placeBrokerOrder( const TradeOrder &order ) {
    auto &broker = CBroker::getInstance();

    assert( order.timeToExecute < Now() );

//-- make order
    BrokerOrder o;

    broker.makeOrder(o);

    //!TODO use convert_
    o.market = order.market;
    o.deposit.fromAddress = order.depositFromAddress;
    o.deposit.amount = order.amount;

    o.order.amount = order.amount;
    o.order.toValue = order.toValue;
    o.order.price = order.price;

    if( !order.withdrawToAddress.empty() ) {
        o.withdraw.amount = { order.withdrawPercent ,order.toValue };
        o.withdraw.toAddress = order.withdrawToAddress;
    } else {
        o.withdraw.amount = { 0 ,"" };
    }

//-- place order
    time_t timePlaced = Now();

    if( IFAILED(broker.placeOrder( o )) )
        return false;

//-- update trades
    TradeInfo trade;

    for( auto it : order.trades ) {
        m_tradeBook.getEntry( it ,trade );

        Copy( trade.orderId ,o.id );
        trade.timePlaced = timePlaced;
        trade.status = TradeInfo::placed;

        updateTrade( trade );
    }

//--
    //! @note make sure caller will erase the pending order

    return true;
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF