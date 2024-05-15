#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_OPTION_H
#define SOLOMINER_OPTION_H

//////////////////////////////////////////////////////////////////////////////
#include <string>

//////////////////////////////////////////////////////////////////////////////
/**
 * @brief global settings
 */
 
extern int g_optThreads; //! number of threads to use

void setOptThreads( int n );
int getOptThreads();

//////////////////////////////////////////////////////////////////////////////
/**
 * @brief log file path
 */
 
extern std::string g_optLogFile;

void setOptLogFile( const char *filename );
const std::string &getOptLogFile();

//////////////////////////////////////////////////////////////////////////////
/**
 * @brief configuration file path
 */

extern std::string g_optConfigFile; 

void setOptConfigFile( const char *filename );
const std::string &getOptConfigFile();

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_OPTION_H
