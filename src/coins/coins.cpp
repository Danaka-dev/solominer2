// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include "coins.h"

#include <wallets/wallets.h>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
template <>
PowAlgorithm &fromString( PowAlgorithm &p ,const String &s ,size_t &size ) {
    String key = s; tolower(key);

    const char *str = s.c_str();
    size = ParseToken( str );

    if( strimatch( key.c_str() ,"auto" )==0 ) {
        p = PowAlgorithm::algoAuto;
    } else if( strimatch( key.c_str() ,"argon2" )==0 ) {
        p = PowAlgorithm::Argon2;
    } else if( strimatch( key.c_str() ,"ghostrider" )==0 || strimatch( key.c_str() ,"gr" )==0 ) {
        p = PowAlgorithm::GhostRider;
    } else if( strimatch( key.c_str() ,"kawpow" )==0 || strimatch( key.c_str() ,"kp" )==0 ) {
        p = PowAlgorithm::KawPow;
    } else if( strimatch( key.c_str() ,"randomwow" )==0 || strimatch( key.c_str() ,"rw" )==0 ) {
        p = PowAlgorithm::RandomWoW;
    } else if( strimatch( key.c_str() ,"randomx" )==0 || strimatch( key.c_str() ,"rx" )==0 ) {
        p = PowAlgorithm::RandomX;
    } else {
        p = PowAlgorithm::algoAuto;
        //? error
    }

    return p;
}

template <>
String &toString( const PowAlgorithm &p ,String &s ) {
    switch( p ) {
        default:
        case PowAlgorithm::algoAuto: s = "Auto"; break;
        case PowAlgorithm::Argon2: s = "Argon2"; break;
        case PowAlgorithm::GhostRider: s = "GhostRider"; break;
        case PowAlgorithm::KawPow: s = "KawPow"; break;
        case PowAlgorithm::RandomWoW: s = "RandomWoW"; break;
        case PowAlgorithm::RandomX: s = "RandomX"; break;
    }

    return s;
}

//////////////////////////////////////////////////////////////////////////////
int secondsPerPeriod( ValuePeriod period ) {
    static const int seconds[] {
        1 ,60 ,3600 ,24*3600 ,7*24*3600 ,30*24*3600 ,365*24*3600
    };

    assert( period <= PerYear );

    return seconds[ CLAMP( period ,ValuePeriod::PerSecond ,ValuePeriod::PerYear ) ];
}

void scalePeriod( ValuePeriod &period ,double &amount ) {
    while( period < ValuePeriod::PerYear && amount < 1 ) {
        ValuePeriod last = period;

        period = (ValuePeriod) (period + 1);
        amount = amount * ( (double) secondsPerPeriod(period) / secondsPerPeriod(last) );
    }
}

//////////////////////////////////////////////////////////////////////////////
//! CCoin

IAPI_DEF CCoin::getInterface( PUID id ,void **ppv ) {
    if( !ppv || *ppv ) return IBADARGS;

    return
        honorInterface_<CCoin>(this,id,ppv) || honorInterface_<ICoin>(this,id,ppv) ? IOK
        : CObject::getInterface(id,ppv)
    ;
}

/*
double CCoinBase::getEstimateBlockPerDay( double hashPerSec ) {
    CoinNetworkInfo info;

    if( !getNetworkInfo( info ) )
        return 0;

    return ( 60 * 60 * 24 * hashPerSec / info.secondsPerBlock ) / info.networkHashRate;
}

double CCoinBase::getEstimateCoinPerDay( double hashPerSec ) {
    CoinNetworkInfo info;

    if( !getNetworkInfo( info ) )
        return 0;

    return info.blockReward * ( 60 * 60 * 24 * hashPerSec / info.secondsPerBlock ) / info.networkHashRate;
}
*/

bool CCoin::getCoinInfo( CoinInfo &info ) {
    info.name = m_name;
    info.ticker = m_ticker;
    info.algorithm = m_algorithm;

    return true;
}

//////////////////////////////////////////////////////////////////////////////
//! CCoinList

bool CCoinStore::findCoinByName( const char *name ,CCoinRef &p ) {
    return this->findItemWith( [name](CCoinRef &p) {
        return !p.isNull() && p->getName() == name;
    } ,p );
}

bool CCoinStore::findCoinByTicker( const char *name ,CCoinRef &p ) {
    return this->findItem( name ,p );
}

//////////////////////////////////////////////////////////////////////////////
//! Well-known coins

class CCoinBTC : public CCoin {
public:
    CCoinBTC() : CCoin( "bitcoin" ,"BTC" ,PowAlgorithm::algoAuto ,"" )
    {}
};

class CCoinUSDT : public CCoin {
public:
    CCoinUSDT() : CCoin( "tether" ,"USDT" ,PowAlgorithm::algoAuto ,"" )
    {}
};

///--
static bool g_hasCoin =
    CCoinStore::getInstance().RegisterItem( "BTC" ,RefOf<CCoin>( new CCoinBTC() ) )
    && CCoinStore::getInstance().RegisterItem( "USDT" ,RefOf<CCoin>( new CCoinUSDT() ) )
;

//////////////////////////////////////////////////////////////////////////////
bool coinHasWallet( const char *ticker ) {
    String name = ticker;

    name += "-core";
    tolower(name);

    CWalletServiceRef pwallet;

    return getWallet( tocstr(name) ,pwallet ) && !pwallet.isNull();
}

bool coinHasMarket( IMarket *market ,const char *ticker ,const char *peer ) {
    if( !market ) return false; //! no market

    if( ticker && *ticker && peer && *peer ) {} else return false; //! bad arguments

    if( stricmp(ticker,peer) == 0 ) return false; //! no market to self

    MarketPair pair;

    if( market->getMarketPair( ticker ,peer ,pair ) != IOK && market->getMarketPair( peer ,ticker ,pair ) != IOK )
        return false;

    return pair.hasMarket;
}

//--
ListOf<String> &getCoinList( ListOf<String> &coins ) {
    const auto &list = CCoinStore::getInstance().getList();

    for( const auto &it : list ) if( !it.isNull() ) {
        coins.emplace_back( it->getTicker() );
    }

    return coins;
}

ListOf<String> &getCoinsWithWallet( ListOf<String> &coins ) {
    const auto &list = CCoinStore::getInstance().getList();

    for( const auto &it : list ) if( !it.isNull() ) {
        const char *str = tocstr(it->getTicker());

        if( !coinHasWallet( str ) ) continue;

        coins.emplace_back( str );
    }

    return coins;
}

ListOf<String> &getCoinsWithTrade( IMarket *market ,const char *ticker ,ListOf<String> &coins ) {
    const auto &list = CCoinStore::getInstance().getList();

    for( const auto &it : list ) if( !it.isNull() ) {
        const char *str = tocstr(it->getTicker());

        if( !coinHasMarket( market ,ticker ,str ) ) continue;

        coins.emplace_back( str );
    }

    return coins;
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF