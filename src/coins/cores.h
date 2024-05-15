#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_CORES_H
#define SOLOMINER_CORES_H

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>
#include <common/service.h>

#include <interface/ICore.h>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
#define CCORESERVICEBASE_UUID   0x090e9bc31cccc411c
#define CCORESETUPBASE_UUID     0x0ff943e5ed98505b5
#define CCORESTORE_UUID         0x06ac3b8047c44a563

///--
class CCoreService;
class CCoreSetup;
class CCoreStore;

//////////////////////////////////////////////////////////////////////////////
struct CoreConfig {
    struct Connection { //! IE connectionInfo
        String host;
        int port;
    } connection;

    struct Credential {
        String user;
        String password;
    } credential;

    String location;
    String filename;
};

template <>
CoreConfig &fromManifest( CoreConfig &p ,const Params &s );

template <>
Params &toManifest( const CoreConfig &p ,Params &s );

//--

//TODO OUT

template <>
inline CoreConfig &fromString( CoreConfig &p ,const String &s ,size_t &size ) {
    Params params; fromString( params ,s ,size ); return fromManifest( p ,params );
}

template <>
inline String &toString( const CoreConfig &p ,String &s ) {
    Params params; toManifest( p ,params ); return toString( params ,s );
}

//////////////////////////////////////////////////////////////////////////////
class CCoreService : public ICore ,public CService
{
protected:
    CoreConfig m_config;

public:
    CCoreService( IServiceSetupRef &setup ) :
        CService(setup)
    {
        this->info().category = category();
        // @note not getting the name from interface, we are in constructor
    }

    IMPORT_IOBJECT_API(CCORESERVICEBASE_UUID);

    static const char *category() { return CORE_SERVICE_CATEGORY; }

    const CoreConfig &config() const { return m_config; }

public: ///-- ICore
    IAPI_IMPL getWallet( IWalletRef &wallet ) IOVERRIDE {
        return ENOEXEC;
    }

    IAPI_IMPL getChain( IChainRef &chain ) IOVERRIDE {
        return ENOEXEC;
    }
};

typedef RefOf<CCoreService> CCoreServiceRef;

//////////////////////////////////////////////////////////////////////////////
//! Core Setup

class CCoreSetup : public ICoreSetup ,public CServiceSetup
{
public: ///-- IBase
    IMPORT_IOBJECT_API(CCORESETUPBASE_UUID);

public: ///-- IServiceSetup
    //...

public: ///-- ICoreSetup
    //...

public: ///-- CCoreSetup
    //...
};

typedef RefOf<CCoreSetup> CCoreSetupRef;

//////////////////////////////////////////////////////////////////////////////
//! Core Store

class CCoreStore :  public ICoreStore
    ,public CServiceStore
    ,public Singleton_<CCoreStore>
{
public: ///-- IBase
    IMPORT_IOBJECT_API(CCORESTORE_UUID);

public: ///-- IServiceStore
    //...

public: ///-- ICoreStore
    //...

public: ///-- CCoreStore
    //...
};

//////////////////////////////////////////////////////////////////////////////
inline CCoreStore &getCoreStore() {
    return getStore_<CCoreStore>();
}

//--
inline bool StartCore( const char *name ,const Params *params=NullPtr ) {
    return StartService( CCoreService::category() ,name ,params );
}

inline bool getCore( const char *name ,CCoreServiceRef &service ) {
    return getService_( name ,service );
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_CORES_H