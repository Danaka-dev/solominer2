// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include "cores.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
template <>
CoreConfig &fromManifest( CoreConfig &p ,const Params &s ) {
    p.connection.host = getMember(s,"host");
    fromString( p.connection.port ,getMember(s,"port") );

    p.credential.user = getMember(s,"user");
    p.credential.password = getMember(s,"password");

    p.location = getMember(s,"location");
    p.filename = getMember(s,"filename");

    return p;
}

template <>
Params &toManifest( const CoreConfig &p ,Params &s ) {
    String to;

    s["host"] = p.connection.host;
    s["port"] = toString( p.connection.port ,to );

    s["user"] = p.credential.user;
    s["password"] = p.credential.password;

    s["location"] = p.location;
    s["filename"] = p.filename;

    return s;
}

//////////////////////////////////////////////////////////////////////////////
//! CCoreBase

IAPI_DEF CCoreService::getInterface( PUID id ,void **ppv ) {
    return
        (!ppv || *ppv) ? IBADARGS :
        honorInterface_<CCoreService>(this,id,ppv) || honorInterface_<ICore>(this,id,ppv)  ? IOK :
        CService::getInterface(id,ppv)
    ;
}

//////////////////////////////////////////////////////////////////////////////
//! CCoreSetupBase

IAPI_DEF CCoreSetup::getInterface( PUID id ,void **ppv ) {
    return
        (!ppv || *ppv) ? IBADARGS :
        honorInterface_<CCoreSetup>(this,id,ppv) || honorInterface_<ICoreSetup>(this,id,ppv)  ? IOK :
        CServiceSetup::getInterface(id,ppv)
    ;
}

//////////////////////////////////////////////////////////////////////////////
//! CCoreStore

IAPI_DEF CCoreStore::getInterface( PUID id ,void **ppv ) {
    return
        (!ppv || *ppv) ? IBADARGS :
        honorInterface_<CCoreStore>(this,id,ppv) || honorInterface_<ICoreStore>(this,id,ppv)  ? IOK :
        CServiceStore::getInterface(id,ppv)
    ;
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF