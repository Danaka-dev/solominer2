// Copyright (c) 2018-2023 The NExTWave developers      <http://www.nextwave-techs.com/>
// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include "markets.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//! MarketDeposit

template <>
const Schema Schema_<MarketDeposit>::schema = fromString( Schema::getStatic() ,String(
    "id:String"
    ",txid:String"
    ",amount:AmountValue"
    ",fromAddress:String"
    ",toAddress:String"
    ",seenAt:timesec"
    ",confirmations:int"
    ",requiredConfirmations:int"
) );

DEFINE_SETMEMBER(MarketDeposit) {
    switch( m ) {
        case 0: fromString( p.id ,s ); return;
        case 1: fromString( p.txid ,s ); return;
        case 2: fromString( p.amount ,s ); return;
        case 3: fromString( p.fromAddress ,s ); return;
        case 4: fromString( p.toAddress ,s ); return;
        case 5: fromString( p.seenAt ,s ); return;
        case 6: fromString( p.confirmations ,s ); return;
        case 7: fromString( p.requiredConfirmations ,s ); return;
        default: break;
    }
}

DEFINE_GETMEMBER(MarketDeposit) {
    switch( m ) {
        case 0: toString( p.id ,s ); return s;
        case 1: toString( p.txid ,s ); return s;
        case 2: toString( p.amount ,s ); return s;
        case 3: toString( p.fromAddress ,s ); return s;
        case 4: toString( p.toAddress ,s ); return s;
        case 5: toString( p.seenAt ,s ); return s;
        case 6: toString( p.confirmations ,s ); return s;
        case 7: toString( p.requiredConfirmations ,s ); return s;
        default: return s;
    }
}

//////////////////////////////////////////////////////////////////////////////
//! MarketWithdraw

template <>
const Schema Schema_<MarketWithdraw>::schema = fromString( Schema::getStatic() ,String(
    "id:String"
    ",txid:String"
    ",amount:AmountValue"
    ",fee:AmountValue"
    ",fromAddress:String"
    ",toAddress:String"
    ",status:String"
    ",postedAt:timesec"
    ",sentAt:timesec"
) );

DEFINE_SETMEMBER(MarketWithdraw) {
    switch( m ) {
        case 0: fromString( p.id ,s ); return;
        case 1: fromString( p.txid ,s ); return;
        case 2: fromString( p.amount ,s ); return;
        case 3: fromString( p.fee ,s ); return;
        case 4: fromString( p.fromAddress ,s ); return;
        case 5: fromString( p.toAddress ,s ); return;
        case 6: fromString( p.status ,s ); return;
        case 7: fromString( p.postedAt ,s ); return;
        case 8: fromString( p.sentAt ,s ); return;
        default: break;
    }
}

DEFINE_GETMEMBER(MarketWithdraw) {
    switch( m ) {
        case 0: toString( p.id ,s ); return s;
        case 1: toString( p.txid ,s ); return s;
        case 2: toString( p.amount ,s ); return s;
        case 3: toString( p.fee ,s ); return s;
        case 4: toString( p.fromAddress ,s ); return s;
        case 5: toString( p.toAddress ,s ); return s;
        case 6: toString( p.status ,s ); return s;
        case 7: toString( p.postedAt ,s ); return s;
        case 8: toString( p.sentAt ,s ); return s;
        default: return s;
    }
}

//////////////////////////////////////////////////////////////////////////////
//! MarketOrder

template <>
const Schema Schema_<MarketOrder>::schema = fromString( Schema::getStatic() ,String(
    "id:String"
    ",userId:String"
    ",amount:AmountValue"
    ",toValue:String"
    ",price:double"
    ",validity:timesec"
    ",quantityFilled:double"
    ",status:String"
    ",seenAt:timesec"
    ",createdAt:timesec"
    ",lastTradeAt:timesec"
    ",completedAt:imesec"
) );

DEFINE_SETMEMBER(MarketOrder) {
    switch( m ) {
        case 0: fromString( p.id ,s ); return;
        case 1: fromString( p.userId ,s ); return;
        case 2: fromString( p.amount ,s ); return;
        case 3: fromString( p.toValue ,s ); return;
        case 4: fromString( p.price ,s ); return;
        case 5: fromString( p.validity ,s ); return;
        case 6: fromString( p.quantityFilled ,s ); return;
        case 7: fromString( p.status ,s ); return;
        case 8: fromString( p.seenAt ,s ); return;
        case 9: fromString( p.createdAt ,s ); return;
        case 10: fromString( p.lastTradeAt ,s ); return;
        case 11: fromString( p.completedAt ,s ); return;
        default: break;
    }
}

DEFINE_GETMEMBER(MarketOrder) {
    switch( m ) {
        case 0: toString( p.id ,s ); return s;
        case 1: toString( p.userId ,s ); return s;
        case 2: toString( p.amount ,s ); return s;
        case 3: toString( p.toValue ,s ); return s;
        case 4: toString( p.price ,s ); return s;
        case 5: toString( p.validity ,s ); return s;
        case 6: toString( p.quantityFilled ,s ); return s;
        case 7: toString( p.status ,s ); return s;
        case 8: toString( p.seenAt ,s ); return s;
        case 9: toString( p.createdAt ,s ); return s;
        case 10: toString( p.lastTradeAt ,s ); return s;
        case 11: toString( p.completedAt ,s ); return s;
        default: return s;
    }
}

//////////////////////////////////////////////////////////////////////////////
void fromMarketSymbol( const String &s ,String &primary ,String &secondary ,const char sep ) {
    const char *c = s.c_str();

    auto pos = s.find(sep);

    primary = String( s.begin() ,s.begin()+pos );
    secondary = s.substr(pos+1);
}

String &toMarketSymbol( String &s ,const String &primary ,const String &secondary ,const char sep ) {
    s = primary; s += sep; s += secondary; return s;
}

//////////////////////////////////////////////////////////////////////////////
template <>
MarketConfig &fromString( MarketConfig &p ,const String &s ,size_t &size ) {
    //TODO

    return p;
}

//////////////////////////////////////////////////////////////////////////////
//! CMarketBase

IAPI_DEF CMarketService::getInterface( PUID id ,void **ppv ) {
    if( !ppv || *ppv ) return IBADARGS;

    if( id == classId() ) {
        *ppv = (CMarketService*) this; return IOK;
    }

    return
        honorInterface_<IMarket>(this,id,ppv) || honorInterface_<IService>(this,id,ppv) ? IOK
        : CService::getInterface(id,ppv)
    ;
}

//////////////////////////////////////////////////////////////////////////////
//! CMarketSetupBase

IAPI_DEF CMarketSetup::getInterface( PUID id ,void **ppv ) {
    if( !ppv || *ppv ) return IBADARGS;

    if( id == classId() ) {
        *ppv = (CMarketSetup*) this; return IOK;
    }

    return
        honorInterface_<IMarketSetup>(this,id,ppv) || honorInterface_<IServiceSetup>(this,id,ppv) ? IOK
        : CServiceSetup::getInterface(id,ppv)
    ;
}

//////////////////////////////////////////////////////////////////////////////
//! CMarketStore

IAPI_DEF CMarketStore::getInterface( PUID id ,void **ppv ) {
    if( !ppv || *ppv ) return IBADARGS;

    if( id == classId() ) {
        *ppv = (CMarketStore*) this; return IOK;
    }

    return
        honorInterface_<IMarketStore>(this,id,ppv) || honorInterface_<IServiceStore>(this,id,ppv) ? IOK
        : CServiceStore::getInterface(id,ppv)
    ;
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF