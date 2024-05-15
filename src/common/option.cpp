// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include "option.h"

//////////////////////////////////////////////////////////////////////////////
int g_optThreads = 0;

void setOptThreads( int n ) {
    g_optThreads = n;
}

int getOptThreads() {
    return g_optThreads;
}

//////////////////////////////////////////////////////////////////////////////
std::string g_optLogFile( "solominer.log" );

void setOptLogFile( const char *filename ) {
    g_optLogFile = filename;
}

const std::string &getOptLogFile() {
    return g_optLogFile;
}

//////////////////////////////////////////////////////////////////////////////
std::string g_optConfigFile( "solominer.conf" );

void setOptConfigFile( const char *filename ) {
    g_optConfigFile = filename;
}

const std::string &getOptConfigFile() {
    return g_optConfigFile;
}

//////////////////////////////////////////////////////////////////////////////
//EOF