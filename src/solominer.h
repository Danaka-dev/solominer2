#pragma one

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_H
#define SOLOMINER_H

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>
#include <pools/pools.h>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//! Config

Config &getConfig();
Config::Section &getGlobalConfig();
Config &getCredentialConfig();

//--
bool getConfigSectionValue( Config &config ,const char *section ,const char *entry ,const char *member ,String &s ,const char *defaultValue );
bool getGlobalSectionValue( Config &config ,const char *entry ,const char *member ,String &s ,const char *defaultValue );

//////////////////////////////////////////////////////////////////////////////
//! Login & Credential

const String &getGlobalLogin();
bool testGlobalPassword( const char *password );
//bool setGlobalPassword( const char *password ) {}

//--
bool globalLogin( const char *password );
bool globalCypher( const char *text ,String &s );
bool globalDecypher( const char *cypher ,String &s );
bool globalIsLogged();

//////////////////////////////////////////////////////////////////////////////
//! Pools

CPoolList &getPoolListInstance();

//////////////////////////////////////////////////////////////////////////////
//! Trading

bool isTraderEnabled();
bool isBrokerEnabled();

//////////////////////////////////////////////////////////////////////////////
} // namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_H