#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_POOLS_H
#define SOLOMINER_POOLS_H

//////////////////////////////////////////////////////////////////////////////
/**
 * Classes to manage list of pools (i.e coin mining providers)
 *
 * constructed with a config file
 *
 * @note all coin members and reference in this file refer to 'coin ticker'
 */

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>

#include "connections.h"

#include <vector>
#include <string>
#include <memory>
#include <map>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
#define CPOOL_PUID      0x05daa7ce872c20c1e

struct PoolInfo;
struct PoolConnectionInfo; //! Rename to pool info

class CPoolList;
class CPool;

//////////////////////////////////////////////////////////////////////////////
//! PoolInfo

struct PoolInfo {

};

//////////////////////////////////////////////////////////////////////////////
//! PoolConnectionInfo

///-- mining mode
#define MININGMODE_NAME_SOLO_LOCAL      "SOLO"
#define MININGMODE_NAME_SOLO_REMOTE     "SOLO_REMOTE"
#define MININGMODE_NAME_POOL_SHARED     "POOL"
#define MININGMODE_NAME_POOL_SOLO       "POOL_SOLO"

enum MiningMode {
    mmUnknown = 0 ,SoloLocal ,SoloRemote ,PoolSolo ,PoolShared
};

struct PoolConnectionInfo {
    String coin;
    MiningMode mode;

    String host;
    int port;
    String user;
    String password;

    ConnectionInfo::Options options;
    String args;
};

template <>
PoolConnectionInfo &Zero( PoolConnectionInfo &p );

//-- Manifest
template <>
PoolConnectionInfo &fromManifest( PoolConnectionInfo &p ,const Params &manifest );

template <>
Params &toManifest( const PoolConnectionInfo &p ,Params &manifest );

//-- String
template <>
inline PoolConnectionInfo &fromString( PoolConnectionInfo &p ,const String &s ,size_t &size ) {
    Params params;
    fromString( params ,s ,size );
    return fromManifest( p ,params );
}

template <>
inline String &toString( const PoolConnectionInfo &p ,String &s ) {
    Params params;
    toManifest( p ,params );
    return toString( params ,s );
}

//////////////////////////////////////////////////////////////////////////////
//! CPool

class CPool : COBJECT_PARENT {
public:
    CPool( const char *name ) : m_name(name)
    {}

    DECLARE_OBJECT(CPool,CPOOL_PUID);

    const String &getName() {
        return m_name;
    }

public:
    /**
     *
     * @param configLine
     * @return
     */

    IAPI_DECL loadSettings( StringList &settings );

    // IAPI_DECL getInfo( PoolInfo &info ); //TODO

public:
    /**
     *
     * @param coinTicker
     * @param mode
     * @return
     */
    bool hasPoolConnections( const char *coin=NullPtr ,MiningMode mode=MiningMode::mmUnknown );
        //TODO return index

    /**
     *
     * @param connections
     * @param coinTicker
     * @param mode
     * @return
     */
    bool findPoolConnections( ListOf<PoolConnectionInfo> &infos ,const char *coin=NullPtr ,MiningMode mode=MiningMode::mmUnknown );

    /**
     * @brief configure a connection info
     * @param coin
     * @param connectionInfo
     * @return
     */
    // bool makeConnectionInfo( const char *coin ,ConnectionInfo &connectionInfo );
    //! script replace with relevant info

protected:
    String m_name; //! TODO replace with PoolInfo

    ListOf<PoolConnectionInfo> m_connections;
};

typedef RefOf<CPool> CPoolRef;

//////////////////////////////////////////////////////////////////////////////
/**
 * @brief list of configured pools
 */

class CPoolList : public Store_<String,CPoolRef>
    ,public Singleton_<CPoolList>
{
public:
    /**
     * @brief register a pool configuration using configLine text
     * @param config
     * @return
     */
    IAPI_DECL loadConfig( Config &config );

    //! event ... from IPFS

public:
    bool findPoolByName( const char *name ,CPoolRef &pool );

    bool listPools( ListOf<String> &pools ,const char *coin=NullPtr ,MiningMode mode=MiningMode::mmUnknown );
};

//////////////////////////////////////////////////////////////////////////////
} // namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_POOLS_H