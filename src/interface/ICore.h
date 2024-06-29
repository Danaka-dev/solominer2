#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_ICORE_H
#define SOLOMINER_ICORE_H

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>

#include <interface/IService.h>
#include <interface/IChain.h>
#include <interface/IWallet.h>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
#define CORE_SERVICE_CATEGORY   "core"

//////////////////////////////////////////////////////////////////////////////
#define ICORE_PUID           0x033be36050f5f19b9
#define ICORESETUP_PUID      0x002d275d5001e8ec8
#define ICORESTORE_PUID      0x0b476c0de5c96e4a5

///--
class ICore;
class ICoreSetup;
class ICoreStore;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! ICore Service

class ICore : IOBJECT_PARENT
{
public: ///-- IBase
    static PUID getClassId() { return ICORE_PUID; };

public: ///-- ICore
    IAPI_DECL getName( String &name ) = 0;

    IAPI_DECL getCoin( String &coin ) = 0;

public:
    IAPI_DECL getWallet( IWalletRef &wallet ) = 0;

    IAPI_DECL getChain( IChainRef &chain ) = 0;
};

typedef RefOf<ICore> ICoreRef;

//////////////////////////////////////////////////////////////////////////////
//! ICoreSetup

class ICoreSetup : IOBJECT_PARENT
{
public: ///-- IBase
    static PUID getClassId() { return ICORESETUP_PUID; };

public: ///--- ICoreSetup
    //...
};

typedef RefOf<ICoreSetup> ICoreSetupRef;

//////////////////////////////////////////////////////////////////////////////
//! ICore Store

class ICoreStore : IOBJECT_PARENT
{
public: ///-- IBase
    static PUID getClassId() { return ICORESTORE_PUID; };

public: ///--- ICoreStore
    //...
};

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_ICORE_H