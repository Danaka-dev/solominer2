// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include "pools.h"

#include <common/logging.h>

#include <string>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

template <>
const char *Enum_<MiningMode>::names[] = {
    "none" ,"solo" ,"remote" ,"pool_solo" ,"pool_shared"
};

template <>
const MiningMode Enum_<MiningMode>::values[] = {
    MiningMode::mmUnknown ,MiningMode::SoloLocal ,MiningMode::SoloRemote ,MiningMode::PoolSolo ,MiningMode::PoolShared
};

///-- PoolConnectionInfo
template <>
PoolConnectionInfo &Zero( PoolConnectionInfo &p ) {
    p.coin = "";
    p.mode = MiningMode::SoloLocal;

    p.region = "*";
    p.ssl = false;

    p.host = "";
    // p.port = 0;
    p.user = "";
    p.password = "";

    p.options.isTls = false;
    p.options.isDaemon = false;
    p.options.isCore = false;

    p.args = "";

    return p;
}

//--
template <>
const Schema Schema_<PoolConnectionInfo>::schema = fromString( Schema::getStatic() ,String(
    "coin:string"
    ",mode:EnumMiningMode"
    ",region:string"
    ",ssl:bool"
    ",host:String"
    ",user:string"
    ",password:string"
    ",options:ConnectionInfoOptions"
    ",args:string"
) );

DEFINE_SETMEMBER(PoolConnectionInfo) {
    switch( m ) {
        case 0: fromString( p.coin ,s ); return;
        case 1: enumFromString( p.mode ,s ); return;
        case 2: fromString( p.region ,s ); return;
        case 3: fromString( p.ssl ,s ); return;
        case 4: fromString( p.host ,s ); return;
        case 5: fromString( p.user ,s ); return;
        case 6: fromString( p.password ,s ); return;
        case 7: fromString( p.options ,s ); return;
        case 8: fromString( p.args ,s ); return;
        default: break;
    }
}

DEFINE_GETMEMBER(PoolConnectionInfo) {
    switch( m ) {
        case 0: toString( p.coin ,s ); return s;
        case 1: enumToString( p.mode ,s ); return s;
        case 2: toString( p.region ,s ); return s;
        case 3: toString( p.ssl ,s ); return s;
        case 4: toString( p.host ,s ); return s;
        case 5: toString( p.user ,s ); return s;
        case 6: toString( p.password ,s ); return s;
        case 7: toString( p.options ,s ); return s;
        case 8: toString( p.args ,s ); return s;
        default: return s;
    }
}

//////////////////////////////////////////////////////////////////////////////
//! Pool

IAPI_DEF CPool::getInterface( PUID id ,void **ppv ) {
    if( !ppv || *ppv ) return IBADARGS;

    return
        honorInterface_<CPool>(this,id,ppv) ? IOK :
        CObject::getInterface(id,ppv)
    ;
}

IAPI_DEF CPool::loadSettings( StringList &settings ) {
    PoolConnectionInfo info;

    for( const auto &it : settings ) {
        Init(info);

        Params params;

        fromString( params ,it );

        fromManifest( info ,params );

        m_connections.emplace_back(info);
    }

    return IOK;
}

/* bool CPool::registerFromConfig( const char *configLine ) {
    using namespace std;

    stringstream ss(configLine);

    string s_connection;

    getline( ss ,s_connection ,'\n' );

        // check for comments

//-- parse
    PoolConnectionPtr connection = std::make_shared<PoolConnection>();

    PoolConnection *p = connection.get();

    if( !p ) {
        LOG_ERROR << LogCategory::config << "memory error while registering config";
        return false;
    }

    fromString( *p ,s_connection );

    m_connection.emplace_back( connection );

//-- done
    return true;
} */

//--
bool CPool::hasPoolConnections( const char *coin ,solominer::MiningMode mode ) {
    if( coin == nullptr && mode == MiningMode::mmUnknown )
        return !m_connections.empty();

    for( auto &info : m_connections ) {
        if( (coin == NullPtr || info.coin == coin)
            && (mode == MiningMode::mmUnknown || info.mode == mode )
        ) {
            return true;
        }
    }

    return false;
}

bool CPool::findPoolConnections( ListOf<PoolConnectionInfo> &infos ,const char *coin ,MiningMode mode ,const char *region ,const bool *ssl ) {
    infos.clear();

    for( auto &info : m_connections ) {
        if( coin && *coin && info.coin != coin ) continue;
        if( mode != MiningMode::mmUnknown && info.mode != mode ) continue;
        if( region && *region && info.region != region ) continue;
        if( ssl && info.ssl != *ssl ) continue;

        infos.emplace_back( info );
    }

    return !infos.empty();
}

//////////////////////////////////////////////////////////////////////////////
//! CPoolList

IAPI_DEF CPoolList::loadConfig( Config &config ) {
    Config::Section &section = config.getSection("pools");

    IServiceSetupRef setup;

    for( const auto &it : section.params ) { //! for each setup
        String index = it.first;

        if( index.empty() ) {
            // LGG CONFIG ERROR
            continue;
        }

        auto &pool = map()[ index ];

        if( pool.isNull() ) {
            pool = new CPool( tocstr(index) );
        }

        StringList settings;

        fromString( settings ,it.second );

        pool->loadSettings( settings ); //+ LOG error
    }

    return IOK;
}

bool CPoolList::findPoolByName( const char *name ,CPoolRef &pool ) {
    auto *p = findItem( name );

    if( !p ) {
        pool = NullPtr;
        return false;
    }

    pool = *p;
    return true;
}

bool CPoolList::listPools( ListOf<String> &pools ,const char *coin ,MiningMode mode ) {
    for( auto it : map() ) {
        if( it.second->hasPoolConnections( coin ,mode ) )
            pools.emplace_back( it.first );
    }

    return true;
}

bool CPoolList::getPoolRegionList( ListOf<String> &regions ) {
    MapOf<String,int> unique;

    for( auto pool : map() ) {
        if( pool.second.isNull() ) continue;

        for( const auto &it : pool.second->Connections() ) {
            if( it.region.empty() ) continue;

            unique[it.region] = 1;
        }
    }

    for( const auto &it : unique ) {
        regions.emplace_back( it.first );
    }

    return true;
}

/*
bool CPoolList::registerFromConfig( const char *configLine ) {
    using namespace std;

//-- parse
    stringstream ss( configLine );

    string s_name ,s_config;

    getline( ss ,s_name ,'=' );
    getline( ss ,s_config ,'\n' );

    trim( s_name );

//-- registe
    CPoolPtr pool = m_pools[s_name];

    if( pool.get() == nullptr ) {
        m_pools[s_name] = (pool = std::make_shared<CPool>( s_name.c_str()));
    }

    return pool->registerFromConfig( s_config.c_str());
}

//--
bool CPoolList::findPool( const char *name ,CPoolPtr &pool ) {
    m_pools.find( name );

    return false;
}

bool CPoolList::makePoolList( std::vector<CPoolPtr> &pools ,const char *coin ,MiningMode mode ) {
    for( auto it : m_pools ) {
        if( it.second->hasPoolConnections( coin ,mode ) )
            pools.emplace_back( it.second );
    }

    return true;
}
*/

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF