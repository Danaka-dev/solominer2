#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_UI_H
#define SOLOMINER_UI_H

//////////////////////////////////////////////////////////////////////////////
#include <tiny.h>

#include <pools/pools.h>

//////////////////////////////////////////////////////////////////////////////
#define SOLOMINER_UI_MAINNAME   "solominer-main"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////

    //TODO this in <solominer.h>

///--
const String &getGlobalLogin();
bool testGlobalPassword( const char *password );
//bool setGlobalPassword( const char *password ) {}

//--
bool globalLogin( const char *password );
bool globalCypher( const char *text ,String &s );
bool globalDecypher( const char *cypher ,String &s );
bool globalIsLogged();

///--
CPoolList &getPoolListInstance();

///--
Config::Section &getGlobalConfig();
Config &getCredentialConfig();

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_UI_H