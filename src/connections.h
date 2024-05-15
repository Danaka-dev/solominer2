#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_CONNECTIONS_H
#define SOLOMINER_CONNECTIONS_H

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>
#include <common/book.h>

#include <coins/coins.h>
#include <miners/miners.h>
#include <chains/chains.h>
#include <markets/markets.h>
#include <wallets/wallets.h>

#include <vector>
#include <string>
#include <memory>
#include <map>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//!TODO Stats, where ?

struct StatsHashRate {
    double minHash;
    double maxHash;
    double avgHash;
};

struct StatsEarnings {
    double amountPerDay;
};

//////////////////////////////////////////////////////////////////////////////
#define CONNECTIONINFO_UUID    0x0047ac054d8a00a6b
#define EARNING_UUID           0x07b92e7fa8dd5a535

#define CCONNECTIONLIST_UUID   0x00e10cbb7b698ee56
#define CCONNECTION_UUID       0x0aae57e3b443c7464

class CConnectionList;
class CConnection;

//////////////////////////////////////////////////////////////////////////////
enum PowDevice {
    deviceAuto=0 ,deviceCpu ,deviceGpu ,deviceFpga ,deviceAsic
};

/*
template <> PowDevice &fromString( PowDevice &p ,const String &s ,size_t &size );
template <> String &toString( const PowDevice &p ,String &s );
*/
//TODO getDevice ...

//////////////////////////////////////////////////////////////////////////////
enum PowTopology {
    topoAuto=0 ,topoIntel ,topoRyzen ,topoCuda ,topoOpencl //...
};

/*
template <> PowTopology &fromString( PowTopology &p ,const String &s ,size_t &size );
template <> String &toString( const PowTopology &p ,String &s );
*/

bool getNativeTopology( PowDevice device ,PowTopology &topology ); //! this pc topology

//////////////////////////////////////////////////////////////////////////////
//! Connection info

struct ConnectionInfo {
    DECLARE_CLASSID(CONNECTIONINFO_UUID)

    struct Status {
        bool isStarted;
        bool isAuto;
        int nThreads;
    } status;

    struct Coin {
        String coin;
        String address;
        String wallet;
    };

    Coin mineCoin;
    Coin tradeCoin;

    struct Trading {
        double percent; //! percent of income to trade on exchange
        bool withdraw; //! true if funds needs to be withdrawn from exchange after trade
    } trading;

    String market;
    String pool;

    struct Pow {
        PowAlgorithm algorithm;
        PowDevice device;
        PowTopology topology;
    } pow;

    UrlConnection connection;
    Credential credential;

    struct Options {
        bool isTls;
        bool isDaemon;
        bool isCore;
    } options;

    String args;

    DECLARE_SCHEMA
};

template <> ConnectionInfo &Zero( ConnectionInfo &p );
template <> ConnectionInfo &Init( ConnectionInfo &p );

//--
template <> ConnectionInfo::Status &fromString( ConnectionInfo::Status &p ,const String &s ,size_t &size );
template <> String &toString( const ConnectionInfo::Status &p ,String &s );

template <> ConnectionInfo::Coin &fromString( ConnectionInfo::Coin &p ,const String &s ,size_t &size );
template <> String &toString( const ConnectionInfo::Coin &p ,String &s );

template <> ConnectionInfo::Trading &fromString( ConnectionInfo::Trading &p ,const String &s ,size_t &size );
template <> String &toString( const ConnectionInfo::Trading &p ,String &s );

template <> ConnectionInfo::Pow &fromString( ConnectionInfo::Pow &p ,const String &s ,size_t &size );
template <> String &toString( const ConnectionInfo::Pow &p ,String &s );

template <> ConnectionInfo::Options &fromString( ConnectionInfo::Options &p ,const String &s ,size_t &size );
template <> String &toString( const ConnectionInfo::Options &p ,String &s );

//-- String/Manifest with schema
CLASS_SCHEMA(ConnectionInfo);

DEFINE_WITHSCHEMA_API(ConnectionInfo);

//-- Manifest
/* template <>
ConnectionInfo &fromManifest( ConnectionInfo &p ,const Params &manifest );

template <>
Params &toManifest( const ConnectionInfo &p ,Params &manifest );

//-- String
template <>
inline ConnectionInfo &fromString( ConnectionInfo &p ,const String &s ,size_t &size ) {
    Params params;
    fromString( params ,s ,size );
    return fromManifest( p ,params );
}

template <>
inline String &toString( const ConnectionInfo &p ,String &s ) {
    Params params;
    toManifest( p ,params );
    return toString( params ,s );
} */

//////////////////////////////////////////////////////////////////////////////
//! Earnings

struct Earning : BookEntry {
    DECLARE_CLASSID(EARNING_UUID)

    static size_t sizeofEntry() { return 1024; };

    TimeSec timestamp;

    enum Type  {
        earningIncome ,earningDeposit ,earningWithdraw
    } type;

    WalletTransaction transaction;

    double tradeAmount;

    TimeSec tradePlacedAt;

    DECLARE_SCHEMA
};

CLASS_SCHEMA(Earning);

DEFINE_WITHSCHEMA_API(Earning);

typedef CBookFile_<Earning> CEarningBook;

///--
struct EarningSums {
    double totalIncome; //! total income from earning book
};

//////////////////////////////////////////////////////////////////////////////
//! Connection

class CConnection : public IWalletEvents ,COBJECT_PARENT {
public:
    CConnection( CConnectionList &list ) :
        m_connectionList(list) ,m_hasEdit(false)
        ,m_miner(NullPtr) ,m_minerListener(NullPtr)
    {
        Init(m_info);
    }

    DECLARE_OBJECT_IOBJECT(CConnection,CCONNECTION_UUID)

    const ConnectionInfo &info() const { return m_info; }
    ConnectionInfo &info() { return m_info; }

    CChainServiceRef &chain() { return m_chain; }
    CMarketServiceRef &market() { return m_market; }

    CWalletServiceRef &coinWallet() { return m_coinWallet; }
    CWalletServiceRef &tradeWallet() { return m_tradeWallet; }

    CMinerBase *miner() { return m_miner; }

    CConnectionList &connectionList() { return m_connectionList; }

    void setMinerListener( IMinerListener *minerListener ) {
        m_minerListener = minerListener;
    }

public:
    bool hasTrade() const {
        return !m_info.tradeCoin.coin.empty();
    }

    bool isMining() const {
        return m_miner != NullPtr;
    }

    void adviseEdit( bool hasEdit=true ) {
        m_hasEdit = hasEdit;
    }

public: ///-- CConnection
    IAPI_DECL loadSettings( Params &settings );
    IAPI_DECL saveSettings( Params &settings );

public: ///-- IMiner
    IAPI_DECL Benchmark();

    IAPI_DECL Start();
    IAPI_DECL Pause();
    IAPI_DECL Resume();
    IAPI_DECL Stop();
    IAPI_DECL Halt();

public: ///-- IWallet
    void startWalletService();
    void stopWalletService();

    IAPI_IMPL onTransaction( IWallet &wallet ,const WalletTransaction &transaction ) IOVERRIDE;

public: //--
    double getCoinRatio( const ChainInfo &chainInfo ,const char *primary ,const char *secondary );
    double getCoinValue( const ChainInfo &chainInfo ,const char *coin ,const char *currency );
    double estimateEarnings( double hostHps ,const ValueOfReference &value ,ValuePeriod period );

protected:
    void tradeIncome( Earning &earning );

protected:
    CConnectionList &m_connectionList; //! back reference to connection list
    ConnectionInfo m_info;
    bool m_hasEdit;

//--
    CChainServiceRef m_chain;
    CMarketServiceRef m_market;
    CWalletServiceRef m_coinWallet;
    CWalletServiceRef m_tradeWallet;

//--
    IMinerListener *m_minerListener;
    CMinerBase *m_miner;
};

typedef RefOf<CConnection> CConnectionRef;

//////////////////////////////////////////////////////////////////////////////
//! Connection List

#define CCONNECTIONLIST_UPDATE_INTERVAL     (10) //! in seconds

/**
 * @brief list of configured connections
 */

class CConnectionList : COBJECT_PARENT {
public: //-- definitions
    typedef Map_<int,CConnectionRef> connections_t;

public: //-- instance
    CConnectionList() : m_config(NullPtr) ,m_updateTime(0) {
        loadHps();
    }

    ~CConnectionList() {
        saveHps();
    }

    DECLARE_OBJECT_IOBJECT(CConnectionList,CCONNECTIONLIST_UUID)

    NoDiscard size_t getCount() const {
        return m_connections.getCount();
    }

    NoDiscard const connections_t &getConnections() const {
        return m_connections;
    }

    connections_t &connections() {
        return m_connections;
    }

//-- earnings
    CEarningBook &earnings() {
        return m_earnings;
    }

    EarningSums &getEarningSums() {
        return earnings().getHeader<EarningSums>();
    }

    bool updateEarningSums() {
        return earnings().updateHeader();
    }

public: //-- interface
    IAPI_DECL loadConfig( Config &config );
    IAPI_DECL saveConfig();

    IAPI_DECL getConnection( int id ,CConnectionRef &connection );
    IAPI_DECL updateConnections();

//-- all connections
    IAPI_DECL Start();
    IAPI_DECL Pause();
    IAPI_DECL Resume();
    IAPI_DECL Stop();
    IAPI_DECL Halt();

public: //-- stats
    void loadHps();
    void saveHps();

    void registerHps( PowAlgorithm algorithm ,double hps );
    double getHostHps( PowAlgorithm algorithm );

protected: //-- members
    Config *m_config;

    connections_t m_connections; //! list of configured connection
    CEarningBook m_earnings; //! account of earnings

    struct Avg {
        double sum = 0.; int n = 0;
    };

    Map_<PowAlgorithm,Avg> m_hostHps;

    time_t m_updateTime;
};

//////////////////////////////////////////////////////////////////////////////
} // namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_CONNECTIONS_H