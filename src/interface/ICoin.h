#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_ICOIN_H
#define SOLOMINER_ICOIN_H

//////////////////////////////////////////////////////////////////////////////

/**
 * @brief ICoin interface and tools providing static information about supported \
    coin
 */

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
#define ICOIN_PUID           0x0fff3638dd108ff74

///--
class ICoin;

//////////////////////////////////////////////////////////////////////////////
// https://xmrig.com/docs/algorithms

enum PowAlgorithm {
    algoAuto=0
    ,Argon2
    ,GhostRider
    ,KawPow
    ,RandomWoW
    ,RandomX
};

//TODO use Enum_ instead
template <> PowAlgorithm &fromString( PowAlgorithm &p ,const String &s ,size_t &size );
template <> String &toString( const PowAlgorithm &p ,String &s );

//////////////////////////////////////////////////////////////////////////////
struct CoinInfo {
    String name; //! full coin name
    String ticker; //! coin ticker name

    PowAlgorithm algorithm;
};

//////////////////////////////////////////////////////////////////////////////
//TODO merge with chain service ?
    //IE remove chain and get information from chain or explorer in ICoin
    //=> NB see how that would work with core -> chain service,

class ICoin : IOBJECT_PARENT {
public:
    static PUID getClassId() { return ICOIN_PUID; };

public:

    /**
     * @brief get coin information
     * @param info
     * @return
     */
    virtual bool getCoinInfo( CoinInfo &info ) = 0;

    /**
     * @brief create a new address for the coin
     */
    // virtual bool makeAddress( const char *privkey ,std::string &address ) = 0;

    //NB need data struct -> in common
    // virtual bool signData( const char *data ,uint32_t size ) = 0; { return false; }

    // virtual bool getNetwork( INetworkPtr &network ) = 0;
    // virtual bool getWallet( IWalletPtr &wallet ) = 0;

    /**
     * @brief obtain dynamic coin info (from network)
     * @return
     */
    // virtual bool getNetworkInfo( NetworkInfo &info ) = 0;


    /**
     * @brief make a send transaction to another address
     */
    // virtual bool Send( double amount ,const char *toAddress ,const char *communication ) = 0;
    //NO, from wallet

    /**
     * @brief get spot trade rate from OWN DeX trade market
     * @param rate :
     * @param toCoin :
     * @return true if rate could be queried to
     */
    // virtual bool getTradeRate( const char *toCoin ,double &rate ) = 0;
};

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_ICOIN_H