// Copyright (c) 2018-2023 The NExTWave developers      <http://www.nextwave-techs.com/>
// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include "markets.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
void fromMarketSymbol( const String &s ,String &primary ,String &secondary ,const char sep ) {
    const char *c = s.c_str();

    auto pos = s.find(sep);

    primary = String( s.begin() ,s.begin()+pos );
    secondary = s.substr(pos);
}

String &toMarketSymbol( String &s ,const String &primary ,const String &secondary ,const char sep ) {
    s = primary; s += sep; s += secondary; return s;
}

//////////////////////////////////////////////////////////////////////////////
template <>
MarketConfig &fromString( MarketConfig &p ,const String &s ,size_t &size ) {
    //TODO

    return p;
}

//////////////////////////////////////////////////////////////////////////////
//! CMarketBase

IAPI_DEF CMarketService::getInterface( UUID id ,void **ppv ) {
    if( !ppv || *ppv ) return IBADARGS;

    if( id == classId() ) {
        *ppv = (CMarketService*) this; return IOK;
    }

    return
        honorInterface_<IMarket>(this,id,ppv) || honorInterface_<IService>(this,id,ppv) ? IOK
        : CService::getInterface(id,ppv)
    ;
}

//////////////////////////////////////////////////////////////////////////////
//! CMarketSetupBase

IAPI_DEF CMarketSetup::getInterface( UUID id ,void **ppv ) {
    if( !ppv || *ppv ) return IBADARGS;

    if( id == classId() ) {
        *ppv = (CMarketSetup*) this; return IOK;
    }

    return
        honorInterface_<IMarketSetup>(this,id,ppv) || honorInterface_<IServiceSetup>(this,id,ppv) ? IOK
        : CServiceSetup::getInterface(id,ppv)
    ;
}

//////////////////////////////////////////////////////////////////////////////
//! CMarketStore

IAPI_DEF CMarketStore::getInterface( UUID id ,void **ppv ) {
    if( !ppv || *ppv ) return IBADARGS;

    if( id == classId() ) {
        *ppv = (CMarketStore*) this; return IOK;
    }

    return
        honorInterface_<IMarketStore>(this,id,ppv) || honorInterface_<IServiceStore>(this,id,ppv) ? IOK
        : CServiceStore::getInterface(id,ppv)
    ;
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF