#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_COINS_H
#define SOLOMINER_COINS_H

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>

#include <interface/ICoin.h>
#include <interface/IMarket.h>

#include <vector>
#include <string>
#include <memory>
#include <map>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
#define CCOIN_PUID           0x0a35b6d2dfc7bbf24

///--
class CCoin;

//////////////////////////////////////////////////////////////////////////////
enum ValuePeriod {
    PerSecond=0 ,PerMinute ,PerHour ,PerDay ,PerWeek ,PerMonth ,PerYear
};

int secondsPerPeriod( ValuePeriod period );
void scalePeriod( ValuePeriod &period ,double &amount );

///--
struct ValueOfReference {
    enum Type {
        Block ,Coin ,Fiat
    } type;

    String currency;
};

///--
struct AmountInValue {
    double amount;
    int decimals; //! significant decimals in amount
};

void Ceil( AmountInValue &amount );
void Floor( AmountInValue &amount );
void Round( AmountInValue &amount );

//////////////////////////////////////////////////////////////////////////////
//! CCoinBase

class CCoin : COBJECT_PARENT ,public ICoin {
public:
    CCoin( const char *name ,const char *ticker ,PowAlgorithm algorithm ,const char *devAddress) :
        m_name(name) ,m_ticker(ticker) ,m_algorithm(algorithm) ,m_devAddress(devAddress)
    {}

    DECLARE_OBJECT(CCoin,CCOIN_PUID);

public:
    const String &getName() const { return m_name; } //! TODO return const char* ?
    const String &getTicker() const { return m_ticker; }
    PowAlgorithm getAlgorithm() const { return m_algorithm; }
    const String &getDevAdress() const { return m_devAddress; }

public:
    virtual bool getCoinInfo( CoinInfo &info );

    /**
     * @brief trade coin using OWN DeX trade market
     * @param amount : amount of coin to trade
     * @param toCoin : ticker name of coin to trade to
     * @param price : price per coin to pay for the trade; <= 0 for SPOT trade
     * @param nblocks : number of block to maintain the trade for
     *  (OWN trade will return no-trade or part-trade after nblocks are passed and full-trade could not be achieved)
     */
    // bool Trade( double amount ,const char *toCoin ,double price ,int nblocks=OWN_TRADE_MAXBLOCKS );

public:
    /* // this is probably not dynamic
    double getEstimateBlockPerDay( double hashPerSec );
    double getEstimateCoinPerDay( double hashPerSec );
    */

protected:
    String m_name;
    String m_ticker;

    PowAlgorithm m_algorithm;

    String m_devAddress;
};

typedef RefOf<CCoin> CCoinRef;

//////////////////////////////////////////////////////////////////////////////
/**
 * @brief List of supported coins
 * @note Stored coins are indexed by ticker
 */

class CCoinStore :
    public Store_<String,CCoinRef>
    ,public Singleton_<CCoinStore>
{
public:
    bool findCoinByName( const char *name ,CCoinRef &p );

    bool findCoinByTicker( const char *ticker ,CCoinRef &p );
};

inline const char *getDevAddress( const char *ticker ) {
    CCoinRef p;

    if( CCoinStore::getInstance().findCoinByTicker( ticker ,p ) && p ) {} else
        return "";

    return p->getDevAdress().c_str();
}

//////////////////////////////////////////////////////////////////////////////
//! Helper

bool coinHasWallet( const char *ticker );
bool coinHasMarket( IMarketRef &market ,const char *ticker ,const char *peer );

//--
ListOf<String> &getCoinList( ListOf<String> &coins );
ListOf<String> &getCoinsWithWallet( ListOf<String> &coins );
ListOf<String> &getCoinsWithTrade( IMarket *market ,const char *ticker ,ListOf<String> &coins );

//--
inline ListOf<String> getCoinList() {
    ListOf<String> coins; return getCoinList( coins );
}

inline ListOf<String> getCoinsWithWallet() {
    ListOf<String> coins; return getCoinsWithWallet( coins );
}

inline ListOf<String> getCoinsWithMarket( IMarket *market ,const char *ticker ) {
    ListOf<String> coins; return getCoinsWithTrade( market ,ticker ,coins );
}

//////////////////////////////////////////////////////////////////////////////
} // namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_COINS_H