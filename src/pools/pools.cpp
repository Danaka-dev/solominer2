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
MiningMode &fromString( MiningMode &p ,const String &s ,size_t &size ) {
    String key = s; tolower(key);

    const char *str = s.c_str();
    size = ParseToken( str );

    if( strimatch( key.c_str() ,MININGMODE_NAME_SOLO_LOCAL )==0 ) {
        p = MiningMode::SoloLocal;
    } else if( strimatch( key.c_str() ,MININGMODE_NAME_SOLO_REMOTE )==0 ) {
        p = MiningMode::SoloRemote;
    } else if( strimatch( key.c_str() ,MININGMODE_NAME_POOL_SHARED )==0 ) {
        p = MiningMode::PoolSolo;
    } else if( strimatch( key.c_str() ,MININGMODE_NAME_POOL_SOLO )==0 ) {
        p = MiningMode::PoolShared;
    } else {
        p = MiningMode::SoloLocal;
        //? error
    }

    return p;
}

template <> String &toString( const MiningMode &p ,String &s ) {
    switch( p ) {
        default:
        case MiningMode::SoloLocal: s = MININGMODE_NAME_SOLO_LOCAL; break;
        case MiningMode::SoloRemote: s = MININGMODE_NAME_SOLO_REMOTE; break;
        case MiningMode::PoolSolo: s = MININGMODE_NAME_POOL_SHARED; break;
        case MiningMode::PoolShared: s = MININGMODE_NAME_POOL_SOLO; break;
    }

    return s;
}

///-- PoolConnectionInfo
template <>
PoolConnectionInfo &Zero( PoolConnectionInfo &p ) {
    p.coin = "";
    p.mode = MiningMode::SoloLocal;

    p.host = "";
    p.port = 0;
    p.user = "";
    p.password = "";

    p.options.isTls = false;
    p.options.isDaemon = false;
    p.options.isCore = false;

    p.args = "";

    return p;
}

template <>
PoolConnectionInfo &fromManifest( PoolConnectionInfo &p ,const Params &manifest ) {
    p.coin = getMember(manifest,"coin");
    fromString( p.mode ,getMember(manifest,"mode") );

    p.host = getMember(manifest,"host");
    fromString( p.port ,getMember(manifest,"port") );
    p.user = getMember(manifest,"user");
    p.password = getMember(manifest,"password");

    fromString( p.options ,getMember(manifest,"options") );
    p.args = getMember(manifest,"args");

    return p;
}

template <>
Params &toManifest( const PoolConnectionInfo &p ,Params &manifest ) {
    _TODO;

    return manifest;
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

bool CPool::findPoolConnections( ListOf<PoolConnectionInfo> &infos ,const char *coin ,MiningMode mode ) {
    infos.clear();

    for( auto &info : m_connections ) {
        if( (coin == NullPtr || info.coin == coin)
            && (mode == MiningMode::mmUnknown || info.mode == mode )
        ) {
            infos.emplace_back( info );
        }
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
            pool = new CPool( index.c_str() );
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