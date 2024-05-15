// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include "chains.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
template <>
ChainConfig &fromManifest( ChainConfig &p ,const Params &s ) {
    p.host = getMember(s,"host");

    return p;
}

template <>
Params &toManifest( const ChainConfig &p ,Params &s ) {
    s["host"] = p.host;

    return s;
}

//////////////////////////////////////////////////////////////////////////////
//! CChainBase

IAPI_DEF CChainService::getInterface( UUID id ,void **ppv ) {
    if( !ppv || *ppv ) return IBADARGS;

    if( id == classId() ) {
        *ppv = (CChainService*) this; return IOK;
    }

    return
        honorInterface_<IChain>(this,id,ppv) || honorInterface_<IService>(this,id,ppv) ? IOK
        : CService::getInterface(id,ppv)
    ;
}

//////////////////////////////////////////////////////////////////////////////
//! CChainSetupBase

IAPI_DEF CChainSetup::getInterface( UUID id ,void **ppv ) {
    if( !ppv || *ppv ) return IBADARGS;

    if( id == classId() ) {
        *ppv = (CChainSetup*) this; return IOK;
    }

    return
        honorInterface_<IChainSetup>(this,id,ppv) || honorInterface_<IServiceSetup>(this,id,ppv) ? IOK
        : CServiceSetup::getInterface(id,ppv)
    ;
}

//////////////////////////////////////////////////////////////////////////////
//! CChainStore

IAPI_DEF CChainStore::getInterface( UUID id ,void **ppv ) {
    if( !ppv || *ppv ) return IBADARGS;

    if( id == classId() ) {
        *ppv = (CChainStore*) this; return IOK;
    }

    return
        honorInterface_<IChainStore>(this,id,ppv) || honorInterface_<IServiceStore>(this,id,ppv) ? IOK
        : CServiceStore::getInterface(id,ppv)
    ;
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF