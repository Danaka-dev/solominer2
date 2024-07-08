// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include "broker.h"

#include <markets/markets.h>
#include <wallets/wallets.h>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! BrokerOp

template <>
const char *Enum_<BrokerOp::Stage>::names[] = {
    "skip" ,"check" ,"execute" ,"verify" ,"confirm" ,"done"
};

template <>
const BrokerOp::Stage Enum_<BrokerOp::Stage>::values[] = {
    BrokerOp::skip ,BrokerOp::check ,BrokerOp::execute ,BrokerOp::verify ,BrokerOp::confirm ,BrokerOp::done
};

template <>
const char *Enum_<BrokerOp::Status>::names[] = {
    "" ,"recorded" ,"processing" ,"cancelling" ,"completed" ,"skipped" ,"abandoned" ,"cancelled"
};

template <>
const BrokerOp::Status Enum_<BrokerOp::Status>::values[] = {
    BrokerOp::noStatus
    ,BrokerOp::recorded
    ,BrokerOp::processing
    ,BrokerOp::cancelling
    ,BrokerOp::completed
    ,BrokerOp::skipped
    ,BrokerOp::abandoned
    ,BrokerOp::cancelled
};

//--
template <>
const Schema Schema_<BrokerOp>::schema = fromString( Schema::getStatic() ,String(
    "sequence:int"
    ",stage:EnumStage"
    ",status:EnumStatus"
    ",timeExecuted:timesec"
    ",timeVerified:timesec"
    ",timeConcluded:timesec"
) );

DEFINE_SETMEMBER(BrokerOp) {
    switch( m ) {
        case 0: fromString( p.sequence ,s ); return;
        case 1: enumFromString( p.stage ,s ); return;
        case 2: enumFromString( p.status ,s ); return;
        case 3: fromString( p.timeExecuted ,s ); return;
        case 4: fromString( p.timeVerified ,s ); return;
        case 5: fromString( p.timeConcluded ,s ); return;
        default: break;
    }
}

DEFINE_GETMEMBER(BrokerOp) {
    switch( m ) {
        case 0: toString( p.sequence ,s ); return s;
        case 1: enumToString( p.stage ,s ); return s;
        case 2: enumToString( p.status ,s ); return s;
        case 3: toString( p.timeExecuted ,s ); return s;
        case 4: toString( p.timeVerified ,s ); return s;
        case 5: toString( p.timeConcluded ,s ); return s;
        default: return s;
    }
}

//--
template <> BrokerOp &Zero( BrokerOp &p ) {
    p.sequence = 0;

    p.stage = BrokerOp::check;
    p.status = BrokerOp::noStatus;

    p.timeExecuted = 0;
    p.timeVerified = 0;
    p.timeConcluded = 0;

    return p;
}

//////////////////////////////////////////////////////////////////////////////
//! BrokerOrder

template <>
const char *Enum_<BrokerOrder::OrderType>::names[] = {
    "market" ,"limit" ,"sellstop" ,"buystop"
};

template <>
const BrokerOrder::OrderType Enum_<BrokerOrder::OrderType>::values[] = {
    BrokerOrder::marketOrder ,BrokerOrder::limitOrder ,BrokerOrder::sellStopOrder ,BrokerOrder::buyStopOrder
};

template <>
const char *Enum_<BrokerOrder::Stage>::names[] = {
    "deposit" ,"exchanging" ,"withdrawing" ,"completed"
};

template <>
const BrokerOrder::Stage Enum_<BrokerOrder::Stage>::values[] = {
    BrokerOrder::makingDeposit ,BrokerOrder::exchanging ,BrokerOrder::withdrawing ,BrokerOrder::completed
};

//--
template <>
const Schema Schema_<BrokerOrder>::schema = fromString( Schema::getStatic() ,String(
    "id:Guid"
    ",type:EnumOrderType"
    ",stopRate:double"
    ",market:String"
    ",deposit:MarketDeposit"
    ",order:MarketOrder"
    ",withdraw:MarketWithdraw"
    ",stage1:BrokerOp"
    ",stage2:BrokerOp"
    ",stage3:BrokerOp"
    ",intputOrders:ListId"
    ",stage:EnumStage"
    ",cancellable:bool"
    ",timePlaced:time_t"
) );

DEFINE_SETMEMBER(BrokerOrder) {
    switch( m ) {
        case 0: fromString( p.id ,s ); return;
        case 1: enumFromString( p.type ,s ); return;
        case 2: fromString( p.stopRate ,s ); return;
        case 3: fromString( p.market ,s ); return;
        case 4: fromString( p.deposit ,s ); return;
        case 5: fromString( p.order ,s ); return;
        case 6: fromString( p.withdraw ,s ); return;
        case 7: fromString( p.stages[0] ,s ); return;
        case 8: fromString( p.stages[1] ,s ); return;
        case 9: fromString( p.stages[2] ,s ); return;
        case 10: fromString( p.intputOrders ,s ); return;
        case 11: enumFromString( p.stage ,s ); return;
        case 12: fromString( p.cancellable ,s ); return;
        case 13: fromString( p.timePlaced ,s ); return;
        default: break;
    }
}

DEFINE_GETMEMBER(BrokerOrder) {
    switch( m ) {
        case 0: toString( p.id ,s ); return s;
        case 1: enumToString( p.type ,s ); return s;
        case 2: toString( p.stopRate ,s ); return s;
        case 3: toString( p.market ,s ); return s;
        case 4: toString( p.deposit ,s ); return s;
        case 5: toString( p.order ,s ); return s;
        case 6: toString( p.withdraw ,s ); return s;
        case 7: toString( p.stages[0] ,s ); return s;
        case 8: toString( p.stages[1] ,s ); return s;
        case 9: toString( p.stages[2] ,s ); return s;
        case 10: toString( p.intputOrders ,s ); return s;
        case 11: enumToString( p.stage ,s ); return s;
        case 12: toString( p.cancellable ,s ); return s;
        case 13: toString( p.timePlaced ,s ); return s;
        default: return s;
    }
}

template <> BrokerOrder &Zero( BrokerOrder &p ) {
    p.sequence = 0;
    p.type = BrokerOrder::marketOrder;
    p.stopRate = 0.;
    p.stage = BrokerOrder::makingDeposit;
    p.cancellable = false;
    p.timePlaced = 0;

    Zero(p.id);
    Zero(p.deposit);
    Zero(p.order);
    Zero(p.withdraw);

    for( int i=0; i<3; ++i ) {
        Zero(p.stages[i]);
    }

    p.intputOrders.clear();

    p.pDepositWallet.Release();
    p.pMarket.Release();
    p.pWithdrawWallet.Release();

    return p;
}

///--
BrokerOp::Status getStatus( const BrokerOrder &order ) {
    int stage = MIN( order.stage ,BROKERORDER_STAGECOUNT-1 ); //! current/last stage

    auto status = order.stages[stage].status;

    switch( status ) {
        case BrokerOp::recorded:
            if( stage == 0 ) return status;

        default:
            return BrokerOp::processing;

    //! final for all
        case BrokerOp::cancelling:
        case BrokerOp::completed:
        case BrokerOp::skipped:
        case BrokerOp::abandoned:
        case BrokerOp::cancelled:
            return status;
    }
}

//////////////////////////////////////////////////////////////////////////////
//! CBroker

IAPI_DEF CBroker::Start( Config &config ,const char *path ) {
    if( IFAILED(m_orderBook.Open( "order" ,path ,true )) )
        return IERROR;

//-- set config
    config.getSection("broker");

//-- create open order list
    m_orderBook.eachEntry( [this]( BrokerOrder::Id id ,BrokerOrder &order ) -> bool {
        if( !isCompleted(order) )
            addOpenOrder( order );

        return true;
    });

//-- done
    m_nextProcessTime = Now();

    return IOK;
}

IAPI_DEF CBroker::Stop() {
    m_orderBook.Close();

    return IOK;
}

///--
IAPI_DEF CBroker::resolveMarket( const char *marketSpec ,IMarketRef &market ) {
    String marketName;

    if( stricmp( marketSpec ,"*" ) == 0 ) {
        //TODO find best priced market for exchange

        marketName = "xeggex"; //TEMP
    } else {
        marketName = marketSpec;
    }

    CMarketServiceRef marketService;

    if( !getMarket( marketName.c_str() ,marketService ) )
        return INODATA;

    market = marketService;

    return IOK;
}

IAPI_DEF CBroker::getOrderHistory( id_t begin ,id_t end ,CBroker::ledger_t &orders ) {
    _TODO; return ENOEXEC;
    // return m_orderBook.getHistory( begin ,end ,entries ) ? IOK : INODATA;
}

IAPI_DEF CBroker::listOpenOrders( ListOf<BrokerOrder::Id> &orders ) {
    _TODO; return ENOEXEC;
    // return m_orderBook.listEntries( orders ) ? IOK : INODATA;
}

IAPI_DEF CBroker::findOpenOrders( ledger_t &orders ) {
    //TODO id last open order in file to avoid going thru all history

    m_orderBook.eachEntry( [&orders]( BrokerOrder::Id id ,BrokerOrder &order ) -> bool {
        if( !isCompleted(order) ) orders[id] = order;

        return true;
    });

    return IOK;
}

IAPI_DEF CBroker::findOrder( const guid_t &id ,BrokerOrder &order ,bool openOnly ) {
    BrokerOrder::Id sequence = INVALID_ENTRY_VALUE;

    m_orderBook.eachEntry( [&id,&sequence]( BrokerOrder::Id orderId ,BrokerOrder &p ) -> bool {
        if( Equals( p.id ,id ) ) {
            sequence = orderId; return false;
        }

        return true;
    });

    if( sequence == INVALID_ENTRY_VALUE )
        return INODATA;

    return m_orderBook.getEntry( sequence ,order ) ? IOK : INODATA;
}

///--
IAPI_DEF CBroker::getOrder( BrokerOrder::Id sequence ,BrokerOrder &order ) {
    return m_orderBook.getEntry( sequence ,order ) ? IOK : INODATA;
}

IAPI_DEF CBroker::makeOrder( BrokerOrder &order ) {
    Zero(order);

    Make( order.id );

    return IOK;
}

IAPI_DEF CBroker::placeOrder( BrokerOrder &order ) {
    //TODO do due diligence on the order

    //! deposit
    if( hasAmount(order.deposit.amount) ) {
        if( !order.deposit.fromAddress.empty() ) //! broker will make the deposit
            order.stages[BrokerOrder::makingDeposit].stage = BrokerOp::check;
        else //! not making the deposit, verify only
            order.stages[BrokerOrder::makingDeposit].stage = BrokerOp::verify;
    } else {
        order.stages[BrokerOrder::makingDeposit].stage = BrokerOp::skip;
    }

    //! withdraw
    if( hasAmount(order.withdraw.amount) ) {
        order.stages[BrokerOrder::withdrawing].stage = BrokerOp::check;
    } else {
        order.stages[BrokerOrder::withdrawing].stage = BrokerOp::skip;
    }

    //! order
    order.stages[BrokerOrder::exchanging].stage = BrokerOp::check;

    //! market
    iresult_t result = resolveMarket( tocstr(order.market) ,order.pMarket ); IF_IFAILED_RETURN(result);

//-- place
    order.timePlaced = Now();

    auto sequence = m_orderBook.addEntry( order );

    return addOpenOrder( order );
}

IAPI_DEF CBroker::cancelOrder( BrokerOrder::Id sequence ) {
    _TODO; return INOEXEC;
}

///--
IAPI_DEF CBroker::processOrders() {
    IRESULT result;

    time_t now = Now();

    if( m_nextProcessTime > now )
        return IOK;

    m_nextProcessTime = now + 1; //! default

    //-- orders
    for( auto it=m_openOrders.begin(); it!=m_openOrders.end(); ) {
        auto &order = *it;

        switch( order.stage ) {
            case BrokerOrder::makingDeposit:
                result = processOrderDeposit( order );
                break;
            case BrokerOrder::exchanging:
                result = processOrderExchange( order );
                break;
            case BrokerOrder::withdrawing:
                result = processOrderWithdraw( order );
                break;

            default:
                assert(false);
                return IERROR;
        }

        if( result==IOK ) {
            if( updateOrder( order ) != IOK ) {
                //TODO Log this
            }
        }

        if( isCompleted(order) ) {
            m_openOrders.erase(it);
        } else {
            ++it;
        }
    }

    return IOK;
}

IAPI_DEF CBroker::updateOrder( const BrokerOrder &order ) {

//-- update book
    if( !m_orderBook.updateEntry( order.sequence ,order ,true ) ) {
        //! should not happen. NB if allowed would require to rollback

        assert(false);
        return IFATAL;
    }

//-- publish event
    CBrokerEventSource::PostOrderUpdate( *this ,order );

    return IOK;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Broker stages & steps

iresult_t CBroker::checkMarketBalance( IMarket &market ,const AmountValue &amount ) {
    MarketBalance balance;

    iresult_t result = market.getBalance( amount.value.c_str() ,balance );
    IF_IFAILED_RETURN(result);

    if( balance.amount.amount < amount.amount )
        return IAGAIN;

    return IOK;
}

iresult_t CBroker::addOpenOrder( BrokerOrder &order ) {

    //! deposit
    if( !isCompleted( order.stages[BrokerOrder::makingDeposit] ) ) {
        if( !findWalletForAddress( tocstr(order.deposit.fromAddress) ,order.pDepositWallet ) )
            return IBADENV;
    }

    //! withdraw
    if( !isCompleted( order.stages[BrokerOrder::withdrawing] ) ) {
        findWalletForAddress( tocstr(order.withdraw.toAddress) ,order.pWithdrawWallet );

        //TODO warning if withdraw without a wallet, will not be able to confirm
    }

    //! market
    iresult_t result = resolveMarket( tocstr(order.market) ,order.pMarket ); IF_IFAILED_RETURN(result);

//-- add
    m_openOrders.emplace_back(order);

    return IOK;
}

///--
iresult_t CBroker::cancelOrderDeposit( BrokerOrder &order ) {
    BrokerOp &op = order.stages[ BrokerOrder::makingDeposit ];

    assert( op.status == BrokerOp::cancelling );

    switch( op.stage ) {
        case BrokerOp::skip:
        case BrokerOp::check:
            op.status = BrokerOp::cancelled;
            break;

        case BrokerOp::execute:
        case BrokerOp::verify:
        case BrokerOp::confirm:
            //TODO MAYBE rollback ?
            op.status = BrokerOp::cancelled;
            break;
    }

    return IOK;
}

iresult_t CBroker::processOrderDeposit( BrokerOrder &order ) {
    BrokerOp &op = order.stages[ BrokerOrder::makingDeposit ];

    if( op.status == BrokerOp::cancelling )
        return cancelOrderDeposit( order );

    iresult_t result = INOEXEC;

    switch( op.stage ) {
        case BrokerOp::skip:
            op.status = BrokerOp::skipped;
            result = IOK;
            break;

        case BrokerOp::check:
            result = checkDeposit( order ,op );
            if( result != IOK ) break;

        case BrokerOp::execute:
            result = executeDeposit( order ,op );
            break; //! @note no flow, want to write this status

        case BrokerOp::verify:
            result = verifyDeposit( order ,op );
            if( result != IOK ) break;

        case BrokerOp::confirm:
            result = confirmDeposit( order ,op );
            break;
    }

    if( op.status >= BrokerOp::completed ) {
        order.stage = BrokerOrder::exchanging;
    } else if( op.status < BrokerOp::processing ) {
        op.status = BrokerOp::processing;
    }

    return result;
}

///--
iresult_t CBroker::cancelOrderExchange( BrokerOrder &order ) {
    BrokerOp &op = order.stages[ BrokerOrder::exchanging ];

    assert( op.status == BrokerOp::cancelling );

    switch( op.stage ) {
        case BrokerOp::skip:
        case BrokerOp::check:
            op.status = BrokerOp::cancelled;
            break;

        case BrokerOp::execute:
        case BrokerOp::verify:
        case BrokerOp::confirm:
            //TODO MAYBE rollback ?
            op.status = BrokerOp::cancelled;
            break;
    }

    return IOK;
}

iresult_t CBroker::processOrderExchange( BrokerOrder &order ) {
    BrokerOp &op = order.stages[ BrokerOrder::exchanging ];

    if( op.status == BrokerOp::cancelling )
        return cancelOrderExchange( order );

    iresult_t result = INOEXEC;

    switch( op.stage ) {
        case BrokerOp::skip:
            op.status = BrokerOp::skipped;
            result = IOK;
            break;

        case BrokerOp::check:
            result = checkExchange( order ,op );
            if( result != IOK ) break;

        case BrokerOp::execute:
            result = executeExchange( order ,op );
            break; //! @note no flow, want to write this status

        case BrokerOp::verify:
            result = verifyExchange( order ,op );
            if( result != IOK ) break;

        case BrokerOp::confirm:
            result = confirmExchange( order ,op );
            break;
    }

    if( op.status >= BrokerOp::completed ) {
        order.stage = BrokerOrder::withdrawing;
    } else if( op.status < BrokerOp::processing ) {
        op.status = BrokerOp::processing;
    }

    return result;
}

///--
iresult_t CBroker::cancelOrderWithdraw( BrokerOrder &order ) {
    BrokerOp &op = order.stages[ BrokerOrder::withdrawing ];

    assert( op.status == BrokerOp::cancelling );

    switch( op.stage ) {
        case BrokerOp::skip:
        case BrokerOp::check:
            op.status = BrokerOp::cancelled;
            break;

        case BrokerOp::execute:
        case BrokerOp::verify:
        case BrokerOp::confirm:
            //TODO MAYBE rollback ?
            op.status = BrokerOp::cancelled;
            break;
    }

    return IOK;
}

iresult_t CBroker::processOrderWithdraw( BrokerOrder &order ) {
    BrokerOp &op = order.stages[ BrokerOrder::withdrawing ];

    if( op.status == BrokerOp::cancelling )
        return cancelOrderWithdraw( order );

    iresult_t result = INOEXEC;

    switch( op.stage ) {
        case BrokerOp::skip:
            op.status = BrokerOp::skipped;
            result = IOK;
            break;

        case BrokerOp::check:
            result = checkWithdraw( order ,op );
            if( result != IOK ) break; //! @note flowing, its ok if we do this again before execution

        case BrokerOp::execute:
            result = executeWithdraw( order ,op );
            break;

        case BrokerOp::verify:
            result = verifyWithdraw( order ,op );
            break;

        case BrokerOp::confirm:
            result = confirmWithdraw( order ,op );
            break;
    }

    if( op.status >= BrokerOp::completed ) {
        order.stage = BrokerOrder::completed;
    } else if( op.status < BrokerOp::processing ) {
        op.status = BrokerOp::processing;
    }

    return result;
}

//////////////////////////////////////////////////////////////////////////////
//! order deposit steps

iresult_t CBroker::checkDeposit( BrokerOrder &order ,BrokerOp &op ) {
    assert( order.pDepositWallet ); //TODO ? correct

    AmountValue balance;

    iresult_t r = order.pDepositWallet->getAddressBalance( order.deposit.fromAddress.c_str() ,balance );
    IF_IFAILED_RETURN(r);

    if( balance.amount < order.order.amount.amount )
        return IAGAIN;

    op.stage = BrokerOp::execute;

    return IOK;
}

iresult_t CBroker::executeDeposit( BrokerOrder &order ,BrokerOp &op ) {
    assert( order.pDepositWallet && order.pMarket );

    String toAddress;

    if( order.deposit.toAddress.empty() ) {
        order.pMarket->getDepositAddress( order.order.amount.value.c_str() ,toAddress );
    } else {
        toAddress = order.deposit.toAddress;
    }

    if( *tocstr(toAddress) == 0 )
        return INODATA;

    WalletTransaction tx;

    tx.fromAddress = order.deposit.fromAddress;
    tx.toAddress = toAddress;
    tx.amount = order.order.amount;
    tx.comment = "exchange deposit"; //TODO configurable
    tx.communication = "";

    iresult_t r = order.pDepositWallet->sendToAddress( tx ); IF_IFAILED_RETURN(r);

    //TODO TEST remove
    // iresult_t r = IOK;
    // tx.txid = "b6d35383d8abc756a0cbb444bd9732a58909fe4cdacce698c290a995865310bc";
    //

    //! wallet sendToAddress
    order.deposit.txid = tx.txid;

    op.timeExecuted = Now();
    op.stage = BrokerOp::verify;

    return IOK;
}

iresult_t CBroker::verifyDeposit( BrokerOrder &order ,BrokerOp &op ) {
    assert( order.pDepositWallet );

    //! check transaction on chain
    WalletTransaction tx;

    iresult_t result = order.pDepositWallet->getTransaction( tocstr(order.deposit.fromAddress) ,tocstr(order.deposit.txid) ,tx );
    IF_IFAILED_RETURN(result);

    if( tx.txid != order.deposit.txid )
        return INODATA;

    op.timeVerified = Now();
    op.stage = BrokerOp::confirm;

    order.deposit.confirmations = tx.confirmations;

    m_nextProcessTime = Now() + 5; //! @note leave market decent window before confirming deposit

    return IOK;
}

iresult_t CBroker::confirmDeposit( BrokerOrder &order ,BrokerOp &op ) {
    assert( order.pMarket );

    String fromAddress = order.deposit.fromAddress; //! @note preserve from address, getDeposit might reset it

    int confirmations = order.deposit.confirmations;

    iresult_t result = order.pMarket->getDeposit( tocstr(order.deposit.amount.value) ,tocstr(order.deposit.txid) ,order.deposit );

    if( order.deposit.fromAddress.empty() )
        order.deposit.fromAddress = fromAddress;

    m_nextProcessTime = Now() + 60; //! @note 60 seconds, most blockchain's block rate, and we don't want to spam market

    if( IFAILED(result) ) {
        verifyDeposit( order ,op );
        return result;
    }

    if( !isConfirmed( order.deposit ) ) {
        if( confirmations != order.deposit.confirmations )
            return IOK; //! @note register confirmation update

        return IAGAIN; //! no changes
    }

    op.timeConcluded = Now();
    op.status = BrokerOp::completed;

    m_nextProcessTime = Now() + 1; //! default if moving on from deposit

    return IOK;
}

//////////////////////////////////////////////////////////////////////////////
//! order steps

iresult_t CBroker::checkExchange( BrokerOrder &order ,BrokerOp &op ) {
    assert( order.pMarket );

    iresult_t result = checkMarketBalance( *order.pMarket.ptr() ,order.order.amount );
    IF_IFAILED_RETURN(result);

    op.stage = BrokerOp::execute;

    return IOK;
}

iresult_t CBroker::executeExchange( BrokerOrder &order ,BrokerOp &op ) {
    assert( order.pMarket );

    order.order.userId = toString( order.id );

    iresult_t result = order.pMarket->CreateOrder( order.order );
    IF_IFAILED_RETURN(result);

    op.timeExecuted = Now();
    op.stage = BrokerOp::verify;

    return IOK;
}

iresult_t CBroker::verifyExchange( BrokerOrder &order ,BrokerOp &op ) {
    assert( order.pMarket );

    iresult_t result = order.pMarket->getOrder( order.order.id.c_str() ,order.order );
    IF_IFAILED_RETURN(result);

    if( order.order.status != "Filled" )
        return IAGAIN;

    op.timeVerified = Now();
    op.stage = BrokerOp::confirm;

    return IOK;
}

iresult_t CBroker::confirmExchange( BrokerOrder &order ,BrokerOp &op ) {
    //! NOP
    op.timeConcluded = Now();
    op.stage = BrokerOp::done;

    op.status = BrokerOp::completed;

    return IOK;
}

//////////////////////////////////////////////////////////////////////////////
//! order withdraw steps

iresult_t CBroker::checkWithdraw( BrokerOrder &order ,BrokerOp &op ) {
    assert( order.pMarket );

    iresult_t result = checkMarketBalance( *order.pMarket.ptr() ,order.withdraw.amount );
    IF_IFAILED_RETURN(result);

    double withdrawPercent = order.withdraw.amount.amount;
    order.withdraw.amount.amount = withdrawPercent * order.order.quantityFilled;

    op.stage = BrokerOp::execute;

    return IOK;
}

iresult_t CBroker::executeWithdraw( BrokerOrder &order ,BrokerOp &op ) {
    assert( order.pMarket );

    iresult_t result = order.pMarket->CreateWithdraw( order.withdraw );
    IF_IFAILED_RETURN(result);

    op.timeExecuted = Now();
    op.stage = BrokerOp::verify;

    return IOK;
}

iresult_t CBroker::verifyWithdraw( BrokerOrder &order ,BrokerOp &op ) {
    assert( order.pMarket );

    iresult_t result = order.pMarket->getWithdraw( order.withdraw.amount.value.c_str() ,order.withdraw.id.c_str() ,order.withdraw );
    IF_IFAILED_RETURN(result);

    if( order.order.status != "completed" )
        return IAGAIN;

    op.timeVerified = Now();
    op.stage = BrokerOp::confirm;

    return IOK;
}

iresult_t CBroker::confirmWithdraw( BrokerOrder &order ,BrokerOp &op ) {
    IWalletRef &wallet = order.pWithdrawWallet;

    if( !wallet.isNull() ) {
        WalletTransaction tx;

        if( wallet->getTransaction( order.withdraw.toAddress.c_str() ,order.withdraw.txid.c_str() ,tx ) != IOK )
            return IAGAIN;

        //TODO check tx.confirmations, how many required ?
    } else {
        //TODO warning, cannot confirm reception side
    }

    op.timeConcluded = Now();
    op.stage = BrokerOp::done;

    op.status = BrokerOp::completed;

    return IOK;
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF