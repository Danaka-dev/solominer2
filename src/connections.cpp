// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include "connections.h"

#include <common/logging.h>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! from/to Manifest

//////////////////////////////////////////////////////////////////////////////
//! PowDevice

template <>
const char *Enum_<PowDevice>::names[] = {
    "auto" ,"cpu" ,"gpu" ,"fpga" ,"asic"
};

template <>
const PowDevice Enum_<PowDevice>::values[] = {
    PowDevice::deviceAuto ,PowDevice::deviceCpu ,PowDevice::deviceGpu ,PowDevice::deviceFpga ,PowDevice::deviceAsic
};

//////////////////////////////////////////////////////////////////////////////
//! PowTopology

template <>
const char *Enum_<PowTopology>::names[] = {
    "auto" ,"intel" ,"ryzen" ,"cuda" ,"opencl"
};

template <>
const PowTopology Enum_<PowTopology>::values[] = {
    PowTopology::topoAuto ,PowTopology::topoIntel ,PowTopology::topoRyzen ,PowTopology::topoCuda ,PowTopology::topoOpencl //...
};

//////////////////////////////////////////////////////////////////////////////
//! ConnectionInfo

///-- Zero
template <>
ConnectionInfo::Status &Zero( ConnectionInfo::Status &p ) {
    p.isStarted = false;
    p.isAuto = false;
    p.nThreads = 0;
    return p;
}

template <>
ConnectionInfo::Coin &Zero( ConnectionInfo::Coin &p ) {
    p.coin = "";
    p.wallet = "";
    p.address = "";
    return p;
}

template <>
ConnectionInfo::Trading &Zero( ConnectionInfo::Trading &p ) {
    p.percent = 0.;
    p.withdraw = false;
    return p;
}

template <>
ConnectionInfo::Pow &Zero( ConnectionInfo::Pow &p ) {
    p.algorithm = PowAlgorithm::algoAuto;
    p.device = PowDevice::deviceAuto;
    p.topology = PowTopology::topoAuto;
    return p;
}

template <>
ConnectionInfo::Options &Zero( ConnectionInfo::Options &p ) {
    p.isTls = false;
    p.isDaemon = false;
    p.isCore = false;
    return p;
}

//--
template <>
ConnectionInfo &Zero( ConnectionInfo &p ) {
    Zero( p.status );
    Zero( p.mineCoin );
    Zero( p.tradeCoin );
    Zero( p.trading );

    p.market = "";
    p.pool = "";

    Zero( p.pow );

    Zero( p.connection );
    Zero( p.credential );

    Zero(p.options);

    p.args = "";

    return p;
}

///-- default
template <>
ConnectionInfo::Status &Init( ConnectionInfo::Status &p ) {
    OsSystemInfo sysinfo;

    OsSystemGetInfo( &sysinfo );

    p.isStarted = false;
    p.isAuto = false;
    p.nThreads = (int) MAX(sysinfo._logicalCoreCount,1) - 1;
    return p;
}

template <>
ConnectionInfo &Init( ConnectionInfo &p ) {
    Init( p.status );
    Init( p.mineCoin );
    Init( p.tradeCoin );
    Init( p.trading );

    p.market = "";
    p.pool = "";

    Init( p.pow );

    Init( p.connection );
    Init( p.credential );

    Init(p.options);

    p.args = "";
    return p;
}

///-- string
template <>
ConnectionInfo::Status &fromString( ConnectionInfo::Status &p ,const String &s ,size_t &size ) {
    StringList list;

    fromString( list ,s ,size );

    for( auto &it : list ) {
        if( it.find('=') != std::string::npos ) {
            KeyValue kv; fromString( kv ,it );

            if( strimatch( kv.key.c_str() ,"threads" ) == 0 ) {
                fromString( p.nThreads ,kv.value );
            }

            continue;
        }

        if( it == "stop" ) {
            p.isStarted = false;
        } else if( it == "start" ) {
            p.isStarted = true;
        } else if( it == "auto" ) {
            p.isAuto = true;
        } else if( it == "manual" ) {
            p.isAuto = false;
        } else {
            fromString( p.nThreads ,it );
            //? errror ?
        }
    }

    return p;
}

template <>
String &toString( const ConnectionInfo::Status &p ,String &s ) {
    String si;

    s.clear();
    s += p.isStarted ? "start" : "stop"; s += ",";
    s += p.isAuto ? "auto" : "manual"; s += ",";
    toString( p.nThreads ,si ); /* s += "threads=";*/ s += si;

    return s;
}

//--
template <> ConnectionInfo::Coin &fromString( ConnectionInfo::Coin &p ,const String &s ,size_t &size ) {
    StringList list;

    fromString( list ,s ,size );

    size_t n = list.size();

    if( n > 0 ) p.coin = list[0];
    if( n > 1 ) p.address = list[1];

    //-- wallet name
    String name = (n > 2) ? list[2] : "";

    if( name.empty() || strimatch( name.c_str() ,"core" ) == 0 ) {
        name = p.coin; name += "-core"; //! @note allow lazy wallet naming in manifest, prefix with proper coin
    }

    p.wallet = tolower(name);

    return p;
}

template <> String &toString( const ConnectionInfo::Coin &p ,String &s ) {
    size_t maxsize = p.coin.size() +1 + p.address.size() +1 + p.wallet.size() +1;

    if( p.coin.empty() && p.address.empty() && p.wallet.empty() ) {
        s.clear(); return s;
    }

    String wallet;

    wallet = p.coin; wallet += "-core"; tolower(wallet);

    if( wallet == p.wallet ) {
        wallet = "core";
    } else {
        wallet = p.wallet;
    }

    Format( s ,"%s,%s,%s" ,maxsize
        ,p.coin.c_str() ,p.address.c_str() ,wallet.c_str()
    );

    return s;
}

//--
template <> ConnectionInfo::Trading &fromString( ConnectionInfo::Trading &p ,const String &s ,size_t &size ) {
    StringList list;

    fromString( list ,s ,size );

    for( auto &it : list ) {
        //TODO key tolower

        if( it.find('=') != std::string::npos ) {
            KeyValue kv; fromString( kv ,it );

            if( strimatch( kv.key.c_str() ,"percent" ) == 0 ) {
                fromString( p.percent ,kv.value );
            } else if( strimatch( kv.key.c_str() ,"withdraw" ) == 0 ) {
                fromString( p.withdraw ,kv.value );
            }

            continue;
        }

        if( it == "withdraw" ) {
            p.withdraw = true;
        } else if( it == "nowidthdraw" ) {
            p.withdraw = false;
        } else if( it.find('%') != std::string::npos ) {
            fromString( p.percent ,it );
        } else {
            //? error
        }
    }

    return p;
}

template <> String &toString( const ConnectionInfo::Trading &p ,String &s ) {
    StringList list;
    String si;

    double pc = RoundDecimal( p.percent ,2 );

    if( (int) pc == 0 ) {
        s.clear(); return s;
    }

    toString( p.percent ,s ); s += "%";
    if( p.withdraw ) s += ",withdraw";

    return s;
}

//--
template <> ConnectionInfo::Pow &fromString( ConnectionInfo::Pow &p ,const String &s ,size_t &size ) {
    StringList list;

    fromString( list ,s ,size );

    size_t n = list.size();

    if( n > 0 ) fromString( p.algorithm ,list[0] );
    if( n > 1 ) enumFromString( p.device ,list[1] );
    if( n > 2 ) enumFromString( p.topology ,list[2] );

    return p;
}

template <> String &toString( const ConnectionInfo::Pow &p ,String &s ) {
    StringList list;
    String si;

    toString( p.algorithm ,si ); list.emplace_back(si);
    enumToString( p.device ,si ); list.emplace_back(si);
    enumToString( p.topology ,si ); list.emplace_back(si);

    toString( list ,s );
    return s;
}

//--
template <> ConnectionInfo::Options &fromString( ConnectionInfo::Options &p ,const String &s ,size_t &size ) {
    StringList list;

    fromString( list ,s ,size );

    for( const auto &it : list ) {
        String key = it; tolower(key);

        if( strimatch( key.c_str() ,"tls" ) == 0 ) {
            p.isTls = true;
        } else if( strimatch( key.c_str() ,"daemon" ) == 0 ) {
            p.isDaemon = true;
        } else if( strimatch( key.c_str() ,"core" ) == 0 ) {
            p.isCore = true;
        }
    }

    return p;
}

template <> String &toString( const ConnectionInfo::Options &p ,String &s ) {
    StringList list;

    if( p.isTls ) list.emplace_back("tls");
    if( p.isDaemon ) list.emplace_back("daemon");
    if( p.isCore ) list.emplace_back("core");

    toString( list ,s );

    return s;
}

///-- manifest
template <>
const Schema Schema_<ConnectionInfo>::schema = fromString( Schema::getStatic() ,String(
    "status:ConnectionInfoStatus"
    ",coin:ConnectionInfoCoin"
    ",trade:ConnectionInfoCoin"
    ",trading:ConnectionInfoTrading"
    ",market:String"
    ",pool:String"
    ",pow:TimeSec"
    ",host:UrlConnection"
    ",user:String"
    ",password:String"
    ",options:String"
    ",args:String"
) );

void ConnectionInfo::setMember( int m ,const String &s ) {
    switch( m ) {
        default:
        case 0: fromString( status ,s ); return;
        case 1: fromString( mineCoin ,s ); return;
        case 2: fromString( tradeCoin ,s ); return;
        case 3: fromString( trading ,s ); return;
        case 4: fromString( market ,s ); return;
        case 5: fromString( pool ,s ); return;
        case 6: fromString( pow ,s ); return;
        case 7: fromString( connection ,s ); return;
        case 8: fromString( credential.user ,s ); return;
        case 9: fromString( credential.password ,s ); return;
        case 10: fromString( options ,s ); return;
        case 11: fromString( args ,s ); return;
    }
}

String &ConnectionInfo::getMember( int m ,String &s ) const {
    switch( m ) {
        default:
        case 0: return toString( status ,s );
        case 1: return toString( mineCoin ,s );
        case 2: return toString( tradeCoin ,s );
        case 3: return toString( trading ,s );
        case 4: return toString( market ,s );
        case 5: return toString( pool ,s );
        case 6: return toString( pow ,s );
        case 7: return toString( connection ,s );
        case 8: return toString( credential.user ,s );
        case 9: return toString( credential.password ,s );
        case 10: return toString( options ,s );
        case 11: return toString( args ,s );
    }
}

//////////////////////////////////////////////////////////////////////////////
//! Earning

template <>
const char *Enum_<Earning::Type>::names[] = {
    "income" ,"deposit" ,"withdraw"
};

template <>
const Earning::Type Enum_<Earning::Type>::values[] = {
    Earning::earningIncome ,Earning::earningDeposit ,Earning::earningWithdraw
};

template <>
const Schema Schema_<Earning>::schema = fromString( Schema::getStatic() ,String(
    "timestamp:TimeSec"
    ",type:EnumType"
    ",transaction:WalletTransaction"
    ",tradeAmount:double"
    ",tradePlacedAt:TimeSec"
) );

void Earning::setMember( int m ,const String &s ) {
    switch( m ) {
        default:
        case 0: fromString( timestamp ,s ); return;
        case 1: enumFromString( type ,s ); return;
        case 2: fromString( transaction ,s ); return;
        case 3: fromString( tradeAmount ,s ); return;
        case 4: fromString( tradePlacedAt ,s ); return;
    }
}

String &Earning::getMember( int m ,String &s ) const {
    switch( m ) {
        default:
        case 0: return toString( timestamp ,s );
        case 1: return enumToString( type ,s );
        case 2: return toString( transaction ,s );
        case 3: return toString( tradeAmount ,s );
        case 4: return toString( tradePlacedAt ,s );
    }
}

template <>
Earning &Zero( Earning &p ) {
    p.timestamp = 0;
    p.type = Earning::earningIncome;
    Zero( p.transaction );
    p.tradeAmount = 0;
    p.tradePlacedAt = 0;

    return p;
}

///-- Earning2
template <>
const Schema Schema_<Earning2>::schema = fromString( Schema::getStatic() ,String(
    "timestamp:TimeSec"
    ",type:EnumType"
    ",tradeAmount:double"
    ",tradePlacedAt:TimeSec"

    //-- from WalletTransaction
    ",txid:String"
    ",amount:AmountValue"
    ",fromAddress:String"
    ",toAddress:String"
    ",comment:String"
    ",communication:String"
    ",receivedAt:TimeSec"
    ",confirmations:int"
) );

void Earning2::setMember( int m ,const String &s ) {
    switch( m ) {
        case 0: fromString( timestamp ,s ); return;
        case 1: enumFromString( type ,s ); return;
        case 2: fromString( tradeAmount ,s ); return;
        case 3: fromString( tradePlacedAt ,s ); return;
        default: break;
    }

    solominer::setMember( transaction ,m-4 ,s );
}

String &Earning2::getMember( int m ,String &s ) const {
    switch( m ) {
        case 0: return toString( timestamp ,s );
        case 1: return enumToString( type ,s );
        case 2: return toString( tradeAmount ,s );
        case 3: return toString( tradePlacedAt ,s );
        default: break;
    }

    return solominer::getMember( transaction ,m-4 ,s );
}

//////////////////////////////////////////////////////////////////////////////
//! Connection

IAPI_DEF CConnection::loadSettings( Params &settings ) {
    if( isMining() ) return IBADENV;

    fromManifest( m_info ,settings );
    m_hasEdit = false;

//-- get relatives
    getChain( "minerstat" ,m_chain );
    getMarket( info().market.c_str() ,m_market );

    getWallet( info().mineCoin.wallet.c_str() ,m_coinWallet );
    getWallet( info().tradeCoin.wallet.c_str() ,m_tradeWallet );

    //TODO auto in wallet on setup using ListAddresses
    if( m_coinWallet ) {
        noteWalletHasAddress( info().mineCoin.address.c_str() ,m_coinWallet );
        m_coinWallet->Subscribe( *this );
    }

    if( m_tradeWallet ) {
        noteWalletHasAddress( info().tradeCoin.address.c_str() ,m_tradeWallet );
    }

//-- done
    return IOK;
}

IAPI_DEF CConnection::saveSettings( Params &settings ) {
    if( isMining() ) return IBADENV;
    if( !m_hasEdit ) return IALREADY; //! no change made, should get settings from files instead

    toManifest( m_info ,settings );
    m_hasEdit = false;

    return IOK;
}

IAPI_DEF CConnection::Benchmark() {
    return INOEXEC;
}

IAPI_DEF CConnection::Start() {
    if( info().status.isStarted ) return IALREADY;

    m_miner = makeMiner( *this ,m_minerListener );

    if( !m_miner ) {
        LOG_ERROR << LogCategory::PoW << "Could not make miner for this connection";
        return false;
    }

    IRESULT result = m_miner->Start();

    if( result == IOK ) {
        info().status.isStarted = true;
    }

    return result;
}

IAPI_DEF CConnection::Pause() {
    return INOEXEC;
}

IAPI_DEF CConnection::Resume() {
    return INOEXEC;
}

IAPI_DEF CConnection::Stop() { //TODO delay ?
    if( !m_miner || !info().status.isStarted ) return IALREADY;

    m_miner->Stop();

    IMiner *miner = (IMiner*) m_miner;

    destroyMiner( &miner ); //! @note or use miner ref/ptr

    m_miner = NullPtr;

    info().status.isStarted = false;

    return IOK;
}

IAPI_DEF CConnection::Halt() {
    return INOEXEC;
}

///-- IWallet
void CConnection::startWalletService() {
    if( coinWallet().isNull() ) return;

    CWalletService &wallet = coinWallet().get();

    ServiceState state = wallet.state();

    Params params ,retryParams = {{"daemon","start"}};

    switch( state ) {
        case serviceStopped:
            wallet.Start(params);
            break;
        case serviceConnected:
            break; //! OK

        default: {
            wallet.Retry( state==serviceStarted ? retryParams : params );
            break;
        }
    }
}

void CConnection::stopWalletService() {
    if( coinWallet().isNull() ) return;

    CWalletService &wallet = coinWallet().get();

    ServiceState state = wallet.state();

    Params params = {{"daemon","stop"}};

    switch( state ) {
        case serviceStopped:
            break;
        case serviceConnected:
            wallet.Stop(params);
            break;

        default: {
            wallet.Retry( params );
            break;
        }
    }
}

IAPI_DEF CConnection::onTransaction( IWallet &wallet ,const WalletTransaction &transaction ) {
    auto &book = connectionList().earnings();

    Earning earning;

    earning.timestamp = transaction.receivedAt;
    earning.type = isIncome(transaction.amount) ? Earning::earningIncome : Earning::earningWithdraw;
    earning.transaction = transaction;
    earning.tradeAmount = 0.;
    earning.tradePlacedAt = book.getEntryCount(); // TEST // 0

//-- processing income
    if( earning.type != Earning::earningIncome )
        return IOK;

//-- add transaction to earning book

    //! checking if transaction is already in book (should not happen, making sure)
    auto txid = transaction.txid;

    auto *pTransaction = book.findEntry( [txid]( auto id ,auto &p ) -> bool {
        return p.transaction.txid == txid;
    } );

    if( pTransaction == NullPtr ) {
        book.addEntry( earning ,true );
    } else {
        assert( false ); //! should not happen
    }

//-- add income to total
    ChainInfo chainInfo;

    const char *coin = info().mineCoin.coin.c_str();

    m_chain->getInfo( coin ,chainInfo ,ChainInfoFlags::noFlags );

    double exchangeValue = getCoinValue( chainInfo ,coin ,"usd" );

    auto &sums = connectionList().getEarningSums();

    sums.totalIncome += earning.transaction.amount.amount * exchangeValue;

    connectionList().updateEarningSums();

//-- processing trade
    if( !hasTrade() )
        return IOK;

    //TODO
    //! look out for BETA 2 featuring automated trading !

    return IOK;
}

//--
double priceToDouble( const String &s ) {
    if( s.empty() ) return 1;

    double price = std::stod( s );

    return price > 0 ? price : 1;
}

double CConnection::getCoinRatio( const ChainInfo &chainInfo ,const char *primary ,const char *secondary ) {
    if( !primary || !secondary || strcmp(primary,secondary) == 0 )
        return 1;

    CMarketServiceRef &p = market();

    if( !p )
        return 1;

    MarketPair marketPair;

    MarketAsset marketAsset;

    double price = 1;

    if( p->getMarketPair( primary ,secondary ,marketPair ) == IOK ) {
        price = priceToDouble( marketPair.lastPrice );
    }
    else {
        double price1 = getCoinValue( chainInfo ,primary ,"usd" );
        double price2 = getCoinValue( chainInfo ,secondary ,"usd" );

        price = price1 / price2;
    } /* else {
        // findTradeRoute(); //TODO
    } */

    return price;
}

double CConnection::getCoinValue( const ChainInfo &chainInfo ,const char *coin ,const char *currency ) {
    if( !market() ) return 0;

    const char *mineCoin = info().mineCoin.coin.c_str();

    if( stricmp( coin ,mineCoin ) == 0 && chainInfo.price > 0 )
        return chainInfo.price;

    MarketAsset assetInfo;

    market()->getAssetInfo( coin ,assetInfo );

    return assetInfo.value;
}

double getMiningReward( const char *coin ,ChainInfo &info ) {
    String entry = coin;

    toupper(entry);

    entry += "/reward";
    String s = ::getOracle( entry.c_str() );

    double reward = 0.;

    if( !s.empty() ) {
        fromString( reward ,s );
    } else {
        reward = info.blockReward;
    }

    return reward;
}

double CConnection::estimateEarnings( double hostHps ,const ValueOfReference &value ,ValuePeriod period ) {
    ChainInfo chainInfo;

    const char *mineCoin = info().mineCoin.coin.c_str();
    const char *currency = value.currency.c_str();

    //TODO if wallet is core we can get info there (networkHps ,difficulty ...
    if( m_chain.isNull() ) return 0.;

    m_chain->getInfo( mineCoin ,chainInfo ,ChainInfoFlags::noFlags );

///-- reward
    double blockReward = (value.type == ValueOfReference::Type::Block) ? 1 : getMiningReward( mineCoin ,chainInfo );
    double networkDiff = chainInfo.networkDiff;
    double networkHps = chainInfo.networkHPS;

    if( networkDiff < 1 ) {
        //! if wallet is core, get networkdiff from there
    }

    double blockPerSecond = (chainInfo.blockPerHour / 3600) * (hostHps / networkHps);

///-- period
    double periodInSec = secondsPerPeriod(period); //! day

    double blockPerPeriod = blockPerSecond * periodInSec;

///-- currency
    double currencyRatio = 1;

    if( value.type == ValueOfReference::Type::Coin ) {
        currencyRatio = getCoinRatio( chainInfo ,mineCoin ,currency );
    }
    else if( value.type == ValueOfReference::Type::Fiat ) {
        currencyRatio = getCoinValue( chainInfo ,mineCoin ,currency );
    }

///-- done
    return blockPerPeriod * blockReward * currencyRatio;
}

void CConnection::tradeIncome( Earning &earning ) {
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Connection List

IAPI_DEF CConnectionList::loadConfig( Config &config ) {

    //TODO use proper logging

    ///-- earnings
    IRESULT result = m_earnings.Open( "earning" );
    IF_IFAILED_RETURN(result);

    ///-- connections
    m_config = &config;

    Config::Section &section = config.getSection("connections");

    for( const auto &it : section.params ) { //! for each setup
        int index = fromString<int>( it.first );

        if( index == 0 || index > getCount()+1 ) {
            continue;
        }

        auto &connection = m_connections[ index-1 ];

        if( connection.isNull() ) {
            connection = new CConnection( *this ,index-1 );
        }

        Params settings;

        fromString( settings ,it.second );

        connection->loadSettings( settings );
    }

    return IOK;
}

IAPI_DEF CConnectionList::saveConfig() {
    if( !m_config ) return IBADENV;

    Config::Section &section = m_config->getSection("connections");

    bool hasEdit = m_hasEdit;

    for( auto &it : m_connections.map() ) {
        Params settings;

        if( it.second->saveSettings( settings ) != IOK ) continue;

        String id ,value;

        toString( it.first+1 ,id );
        toString( settings ,value );

        section.params[ id ] = value;
        hasEdit = true;
    }

    //-- removed
    /* int n = (int) section.params.size();

    for( int i=0; i<n; ++i ) {
        if( m_connections.findItem(i) != NullPtr ) continue;

        const int index = i+1;

        section.params.erase(index);
    } */

    // for( auto &it : section.params ) {
    for( auto it = section.params.cbegin(); it != section.params.cend(); ) {
        int id;

        fromString( id ,it->first );

        if( m_connections.findItem( id-1 ) == NullPtr ) {
            section.params.erase(it++);
        } else {
            ++it;
        }
    }

    //-- commit
    if( hasEdit ) {
        m_config->commitSection( section );
        m_config->SaveFile();
    }

    m_hasEdit = false;

    return IOK;
}

//--
IAPI_DEF CConnectionList::Start() {
    for( auto &it : connections().map() ) if( it.second ) {
        auto &connection = it.second.get();

        auto &info = connection.info();

        if( info.status.isAuto && !info.status.isStarted ) {
            connection.Start();
        }
    }

    return INOEXEC;
}

IAPI_DEF CConnectionList::Pause() {
    return INOEXEC;
}

IAPI_DEF CConnectionList::Resume() {
    return INOEXEC;
}

IAPI_DEF CConnectionList::Stop() {
    for( auto &it : connections().map() ) if( it.second ) {
        auto &connection = it.second.get();

        auto &info = connection.info();

        if( info.status.isStarted ) {
            connection.Stop();
        }
    }

    return IOK;
}

IAPI_DEF CConnectionList::Halt() {
    return INOEXEC;
}

///--
void CConnectionList::loadHps() {
    StringList list;

    String s = ::getOracle( "hps" );

    fromString( list ,s );

    for( auto &it : list ) {
        KeyValue kv;

        fromString( kv ,it );

        PowAlgorithm pow;
        double hps;

        fromString( pow ,kv.key );
        fromString( hps ,kv.value );

        registerHps( pow ,hps );
    }
}

void CConnectionList::saveHps() {
    StringList list;

    String s;

    for( auto &it : m_hostHps.map() ) if( it.second.n > 0 ) {
            KeyValue kv;

            toString( it.first ,kv.key );
            toString( (uint32_t) (it.second.sum / it.second.n) ,kv.value );

            toString( kv ,s );

            list.emplace_back( s );
        }

    toString( list ,s );

    ::setOracle( "hps" ,s.c_str() );
}

void CConnectionList::registerHps( PowAlgorithm algorithm ,double hps ) {
    auto &avg = m_hostHps[ algorithm ];

    avg.sum += hps;
    avg.n++;
}

double CConnectionList::getHostHps( PowAlgorithm algorithm ) {
    const auto *avg = m_hostHps.findItem( algorithm );

    if( !avg || avg->n == 0 ) return 1000.; //! @note arbitrary base

    return avg->sum / avg->n;
}

///--
IAPI_DEF CConnectionList::getConnection( int id ,CConnectionRef &connection ) {
    return m_connections.findItem( id ,connection ) ? IOK : INODATA;
}

IAPI_DEF CConnectionList::addConnection( Params &settings ,CConnectionRef &connection ) {
    int index = (int) getCount();

    auto &p  = m_connections[ index ];

    if( p.isNull() ) {
        p = new CConnection( *this ,index );
    }

    connection = p;

    IRESULT result = connection->loadSettings( settings );

    p->adviseEdit();

    return result;
}

IAPI_DEF CConnectionList::editConnection( int index ,Params &settings ) {
    if( index < 0 || index >= getCount() ) return IBADARGS;

    auto &p  = m_connections[ index ];

    IRESULT result = p->loadSettings( settings );

    p->adviseEdit();

    return result;
}

IAPI_DEF CConnectionList::deleteConnection( int index ) {
    if( index < 0 || index >= getCount() ) return IBADARGS;

    m_connections.delItem( index );

    //-- re-order
    int n = m_connections.getCount();

    auto &m = m_connections.map();

    for( int i=index; i<n; ++i ) {
        auto node = m.extract(i+1);

        node.key() = i;
        node.mapped()->setIndex(i);
        node.mapped()->adviseEdit();

        m.insert( std::move(node) );
    }

    //--
    adviseEdit();

    return IOK;
}

IAPI_DEF CConnectionList::updateConnections() {
    time_t now = Now();

    if( /*m_updateTime == 0 ||*/ m_updateTime > now ) return IOK;

///-- process pending trade if any
    //TODO retry pending earning

///-- auto select best earning connection
    ListOf<CConnectionRef> listAuto;

    m_updateTime = now + CCONNECTIONLIST_UPDATE_INTERVAL;

    //-- all auto
    m_connections.listItemsWith( listAuto
        ,[]( CConnectionRef &p ) { return p && p->info().status.isAuto == true; }
    );

    if( listAuto.size() < 2 ) return IOK;

    //-- best earning
    ValueOfReference value = { ValueOfReference::Fiat ,"usd" };

    int ibest = -1 ,icurrent = -1; double vbest ,vcurrent;

    int i = 0; for( auto &it : listAuto ) {
        double hostHps = getHostHps( it->info().pow.algorithm );
        double earning = it->estimateEarnings( hostHps ,value ,ValuePeriod::PerDay );

        if( ibest < 0 || earning > vbest ) {
            ibest = i; vbest = earning;
        }

        if( it->info().status.isStarted ) { //! should only have one
            icurrent = i; vcurrent = earning;
        }

        ++i;
    }

    if( icurrent < 0 ) return IOK; //! none started

    if( vbest > vcurrent * 1.02f ) { //! 2%
        if( icurrent >= 0 ) {
            Stop();

            OsSleep(1000);
        }

        listAuto[ibest]->Start();
    }

//-- done
    return IOK;
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF