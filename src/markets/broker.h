#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_BROKER_H
#define SOLOMINER_BROKER_H

//////////////////////////////////////////////////////////////////////////////
//! BROKER

/**
 * Broker choose the best market to perform trade
 * Broker enable trade across markets
 * Verify and report on each trade steps
 *
 * @note Trader only schedule trades and pack them into broker order(s)
 */

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>
#include <common/book.h>
#include <common/service.h>

#include <interface/IMarket.h>
#include <interface/IWallet.h>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
#define BROKERORDER_UUID    0x07df933015eeddfb7
#define CBROKER_UUID        0x06584d3ad47e70ee8

class IBrokerEvents;
class CBrokerEventListener;
class CBroker;

//TODO
// class IBroker

//TODO
// MAYBE, delay between order per market ? //OR, in Market ?

//////////////////////////////////////////////////////////////////////////////
//! BrokerOp

struct BrokerOp {
    BookEntry::Id sequence; //! operation sequence (unique within BrokerOrder)

    enum Stage {
        skip=0      //! don't execute this stage
        ,check      //! check execution condition
        ,execute    //! execute
        ,verify     //! verify (check execution, sender side)
        ,confirm    //! confirm (get confirmation, recipient side)
        ,done
    } stage;

    enum Status {
        noStatus=0
        ,recorded       //! order was added, waiting for execution condition to be met
        ,processing     //! current stage is in execution
        ,cancelling     //! user requested cancel in progress
        ,completed      //! execution ended successfully
        ,skipped        //! stage execution not required and was skipped
        ,abandoned      //! execution was abandoned (trade expired, too many retries, unrecoverable error)
        ,cancelled      //! execution cancelled (at stage point)
    } status;

    time_t timeExecuted;    //! time execution started
    time_t timeVerified;    //! time of verification (sender side, eg. on chain...)
    time_t timeConcluded;   //! time of conclusion (be it: completed, abandoned ,cancelled)

    // Params values; //! values related to each steps (eg confirmation), not serialized
};

DEFINE_MEMBER_API(BrokerOp);
DEFINE_WITHSCHEMA_API(BrokerOp);

template <> BrokerOp &Zero( BrokerOp &p );

inline bool isCompleted( const BrokerOp &op ) {
    return op.status >= BrokerOp::completed || op.stage == BrokerOp::skip || op.stage == BrokerOp::done;
}

//////////////////////////////////////////////////////////////////////////////
//! BrokerOrder

#define BROKERORDER_STAGECOUNT  3

struct BrokerOrder : BookEntry {
    DECLARE_CLASSID(BROKERORDER_UUID)

    static size_t sizeofEntry() { return 2048; };

//-- info
    guid_t id; //! globally unique order id

    enum OrderType {
        marketOrder=0 ,limitOrder ,sellStopOrder ,buyStopOrder
    } type;

    double stopRate; //! exchange rate for stop orders

    String market; //! market name or '*' to select market with best price for the trade

    MarketDeposit deposit;      //-- input
    MarketOrder order;          //-- operation
    MarketWithdraw withdraw;    //-- output

    //TODO if pair doesn't exist on market, find route and split exchanges into order
        //NB => BrokerOrder => set of BrokerExchange/Trades ?

    BrokerOp stages[BROKERORDER_STAGECOUNT];

    ListOf<Id> intputOrders; //! wait for these order to complete before execution

//-- status
    enum Stage {
        makingDeposit=0 ,exchanging=1 ,withdrawing=2 ,completed=3
    } stage;

    bool cancellable; //! true if trade is cancellable (current state/status)

    time_t timePlaced;    //! time order was placed to broker

//-- out of payload
    IWalletRef pDepositWallet;
    IMarketRef pMarket;
    IWalletRef pWithdrawWallet;
};

DEFINE_MEMBER_API(BrokerOrder);
DEFINE_WITHSCHEMA_API(BrokerOrder);

template <> BrokerOrder &Zero( BrokerOrder &p );

//--
typedef CBookFile_<BrokerOrder> COrderBook;
typedef CBookDataSource_<BrokerOrder,BrokerOrder> COrderDataSource;

BrokerOp::Status getStatus( const BrokerOrder &order );

inline bool isCompleted( const BrokerOrder &order ) {
    // order.stage == BrokerOrder::withdrawing && getStatus(order) >= BrokerOp::completed
    return order.stage == BrokerOrder::Stage::completed;
}

//////////////////////////////////////////////////////////////////////////////
//! Trade events

class IBrokerEvents : IOBJECT_PARENT {
public:
    IAPI_DECL onOrderUpdate( CBroker &broker ,const BrokerOrder &order ) = 0;
};

class CBrokerEventSource : public CPublisher_<IBrokerEvents> {
public:
    IAPI_DECL PostOrderUpdate( CBroker &broker ,const BrokerOrder &order ) {
        for( auto &it : m_subscribers ) {
            it->onOrderUpdate(broker,order);
        }
        return IOK;
    }
};

class CBrokerEventListener : public IBrokerEvents {
public:
    IAPI_IMPL onOrderUpdate( CBroker &broker ,const BrokerOrder &order ) IOVERRIDE {
        return IOK;
    }
};

//////////////////////////////////////////////////////////////////////////////
//! CBroker

class CBroker : public CBrokerEventSource ,COBJECT_PARENT ,public Singleton_<CBroker>
{
public: //-- definitions
    typedef Map_<BrokerOrder::Id,BrokerOrder> ledger_t;

public: //-- instance
    CBroker() DEFAULT;

    DECLARE_OBJECT_STD(CObject,CBroker,CBROKER_UUID);

    COrderBook &getOrderBook() { return m_orderBook; }

public: ///-- IBroker interface
    IAPI_DECL Start( Config &config ,const char *path="" );
    IAPI_DECL Stop();

//--
    IAPI_DECL resolveMarket( const char *marketSpec ,IMarketRef &market );

    IAPI_DECL getOrderHistory( id_t begin ,id_t end ,ledger_t &orders );
    IAPI_DECL listOpenOrders( ListOf<BrokerOrder::Id> &orders );
    IAPI_DECL findOpenOrders( ledger_t &orders );
    IAPI_DECL findOrder( const guid_t &id ,BrokerOrder &order ,bool openOnly=true );

//--
    IAPI_DECL getOrder( BrokerOrder::Id sequence ,BrokerOrder &order );
    IAPI_DECL makeOrder( BrokerOrder &order );
    IAPI_DECL placeOrder( BrokerOrder &order );
    IAPI_DECL cancelOrder( BrokerOrder::Id sequence );

//--
    IAPI_DECL processOrders(); //! pulsing order processing

    IAPI_DECL updateOrder( const BrokerOrder &order ); //! order has update

protected: ///--
    static iresult_t checkMarketBalance( IMarket &market ,const AmountValue &amount );

    iresult_t addOpenOrder( BrokerOrder &order );

///--
    iresult_t cancelOrderDeposit( BrokerOrder &order );
    iresult_t processOrderDeposit( BrokerOrder &order );

    iresult_t cancelOrderExchange( BrokerOrder &order );
    iresult_t processOrderExchange( BrokerOrder &order );

    iresult_t cancelOrderWithdraw( BrokerOrder &order );
    iresult_t processOrderWithdraw( BrokerOrder &order );

///-- deposit
    iresult_t checkDeposit( BrokerOrder &order ,BrokerOp &op );
    iresult_t executeDeposit( BrokerOrder &order ,BrokerOp &op );
    iresult_t verifyDeposit( BrokerOrder &order ,BrokerOp &op );
    iresult_t confirmDeposit( BrokerOrder &order ,BrokerOp &op );

///-- exchange
    iresult_t checkExchange( BrokerOrder &order ,BrokerOp &op );
    iresult_t executeExchange( BrokerOrder &order ,BrokerOp &op );
    iresult_t verifyExchange( BrokerOrder &order ,BrokerOp &op );
    iresult_t confirmExchange( BrokerOrder &order ,BrokerOp &op );

///-- withdraw
    iresult_t checkWithdraw( BrokerOrder &order ,BrokerOp &op );
    iresult_t executeWithdraw( BrokerOrder &order ,BrokerOp &op );
    iresult_t verifyWithdraw( BrokerOrder &order ,BrokerOp &op );
    iresult_t confirmWithdraw( BrokerOrder &order ,BrokerOp &op );

protected: ///-- members
    COrderBook m_orderBook;

    ListOf<BrokerOrder> m_openOrders;

    time_t m_nextProcessTime;

    // MapOf<String,IMarketRef> m_markets; //! name => pMarket
};

//////////////////////////////////////////////////////////////////////////////
inline CBroker &getBroker() {
    return CBroker::getInstance();
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_BROKER_H
