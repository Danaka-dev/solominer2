#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#ifndef SOLOMINER_LOGGING_H
#define SOLOMINER_LOGGING_H

//////////////////////////////////////////////////////////////////////////////
#include <plog/Log.h>

#define APPLOG_VERBOSE          PLOG_VERBOSE
#define APPLOG_DEBUG            PLOG_DEBUG
#define APPLOG_INFO             PLOG_INFO
#define APPLOG_WARNING          PLOG_WARNING
#define APPLOG_ERROR            PLOG_ERROR
#define APPLOG_FATAL            PLOG_FATAL

#define APPLOG                  PLOG_INFO

//////////////////////////////////////////////////////////////////////////////
enum LogCategory  {
    none = 0
    ,config = 1
    ,net = 2
    ,PoW = 3
};

inline const char *categoryToString( LogCategory category ) {
    switch( category ) {
        case LogCategory::config : return "CONFIG";
        case LogCategory::net : return "NET";
        case LogCategory::PoW : return "POW";
        default: return "";
    }
}

inline LogCategory categoryFromString( const char **str ) {
    if( str == NULL || *str == NULL )
        return LogCategory::none;

    if( strncmp(*str,"CONFIG",6) == 0 ) {
        (*str) += 6; return LogCategory::config;
    }
    if( strncmp(*str,"NET",3) == 0 ) {
        (*str) += 3; return LogCategory::net;
    }
    if( strncmp(*str,"POW",3) == 0 ) {
        (*str) += 3; return LogCategory::PoW;
    }

    return LogCategory::none;
}

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_LOGGING_H