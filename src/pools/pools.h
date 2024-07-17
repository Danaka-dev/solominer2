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
#define SOLOMINER_CPOOLCONNECTIONINFO_PUID      0x0d5c80c5d1a4b3a77

#define SOLOMINER_CPOOL_PUID      0x05daa7ce872c20c1e

struct PoolConnectionInfo; //! Rename to pool info

///--
class CPoolList;
class CPool;

//////////////////////////////////////////////////////////////////////////////
//! PoolConnectionInfo

enum MiningMode {
    mmUnknown = 0 ,SoloLocal ,SoloRemote ,PoolSolo ,PoolShared
};

//--
struct PoolConnectionInfo {
    String coin;
    MiningMode mode;

    String region;
    bool ssl;

    String host;
    String user;
    String password;

    ConnectionInfo::Options options;
    String args;
};

DECLARE_STRUCT(PoolConnectionInfo,SOLOMINER_CPOOLCONNECTIONINFO_PUID);

template <>
PoolConnectionInfo &Zero( PoolConnectionInfo &p );

DEFINE_MEMBER_API(PoolConnectionInfo);
DEFINE_WITHSCHEMA_API(PoolConnectionInfo);

//////////////////////////////////////////////////////////////////////////////
//! CPool

class CPool : COBJECT_PARENT {
public:
    CPool( const char *name ) : m_name(name)
    {}

    DECLARE_OBJECT(CPool,SOLOMINER_CPOOL_PUID);

    const String &getName() {
        return m_name;
    }

    ListOf<PoolConnectionInfo> &Connections() { return m_connections; }

public:
    /**
     *
     * @param configLine
     * @return
     */

    IAPI_DECL loadSettings( StringList &settings );

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
    bool findPoolConnections( ListOf<PoolConnectionInfo> &infos ,const char *coin=NullPtr ,MiningMode mode=MiningMode::mmUnknown ,const char *region=NullPtr ,const bool *ssl=NullPtr );

    /**
     * @brief configure a connection info
     * @param coin
     * @param connectionInfo
     * @return
     */
    // bool makeConnectionInfo( const char *coin ,ConnectionInfo &connectionInfo );
    //! script replace with relevant info

protected:
    String m_name;

    ListOf<PoolConnectionInfo> m_connections;
};

typedef RefOf<CPool> CPoolRef;

//////////////////////////////////////////////////////////////////////////////
//! CPoolList

    //! @brief list of configured pools

class CPoolList : public Store_<String,CPoolRef>
    ,public Singleton_<CPoolList>
{
public:
    //! @brief build pool list from config
    IAPI_DECL loadConfig( Config &config );

    //! event ... from IPFS

public:
    bool findPoolByName( const char *name ,CPoolRef &pool );

    bool listPools( ListOf<String> &pools ,const char *coin=NullPtr ,MiningMode mode=MiningMode::mmUnknown );

    bool getPoolRegionList( ListOf<String> &regions );
};

//////////////////////////////////////////////////////////////////////////////
} // namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_POOLS_H