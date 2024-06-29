#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_CHAINS_H
#define SOLOMINER_CHAINS_H

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>
#include <common/service.h>

#include <interface/IChain.h>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
#define CCHAINSERVICEBASE_PUID  0x09121d9603c5f26bb
#define CCHAINSETUPBASE_PUID    0x06677a3201d3aa63f
#define CCHAINSTORE_PUID        0x00641c1db7e92b418

///--
class CChainService;
class CChainSetup;
class CChainStore;

//////////////////////////////////////////////////////////////////////////////
template <> ChainConfig &fromManifest( ChainConfig &p ,const Params &s );
template <> Params &toManifest( const ChainConfig &p ,Params &s );

//--
template <>
inline ChainConfig &fromString( ChainConfig &p ,const String &s ,size_t &size ) {
    Params params; fromString( params ,s ,size ); return fromManifest( p ,params );
}

template <>
inline String &toString( const ChainConfig &p ,String &s ) {
    Params params; toManifest( p ,params ); return toString( params ,s );
}

//////////////////////////////////////////////////////////////////////////////
class CChainService : public IChain ,public CService
{
public:
    CChainService( IServiceSetupRef &setup ) :
        CService(setup)
    {
        this->info().category = CHAIN_SERVICE_CATEGORY;
    }

    DECLARE_OBJECT(CChainService,CCHAINSERVICEBASE_PUID);

    static const char *category() { return "chain"; }

public: ///-- IService
    //..

public: ///-- IChain
    IAPI_IMPL getInfo( const char *coin ,ChainInfo &info ,ChainInfoFlags flags ) IOVERRIDE {
        return INOEXEC;
    }
};

typedef RefOf<CChainService> CChainServiceRef;

//////////////////////////////////////////////////////////////////////////////
class CChainSetup : public IChainSetup ,public CServiceSetup
{
public: ///-- IBase
    DECLARE_OBJECT(CChainSetup,CCHAINSETUPBASE_PUID);

public: ///-- IServiceSetup
    //...

public: ///-- IChainSetup
    //...

public: ///-- CChainSetup
    //...
};

typedef RefOf<CChainSetup> CChainSetupRef;

//////////////////////////////////////////////////////////////////////////////
class CChainStore : public IChainStore
    ,public CServiceStore
    ,public Singleton_<CChainStore>
{
public: ///-- IBase
    DECLARE_OBJECT(CChainStore,CCHAINSTORE_PUID);

public: ///-- IServiceStore
    //..

public: ///-- IChainStore
    //..

public: ///-- CChainStore
    //..
};

//////////////////////////////////////////////////////////////////////////////
inline CChainStore &getChainStore() {
    return getStore_<CChainStore>();
}

//--
inline bool StartChain( const char *name ,const Params *params=NullPtr ) {
    return StartService( CChainService::category() ,name ,params );
}

inline bool getChain( const char *name ,CChainServiceRef &service ) {
    return getService_( name ,service );
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_CHAINS_H