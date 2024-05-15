#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_IMINER_H
#define SOLOMINER_IMINER_H

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
class IMinerListener;
class IMiner;

struct CConnection;

//////////////////////////////////////////////////////////////////////////////
//! Definitions

///--
struct MinerInfo {
    enum WorkIntensity {
        intensityNone=0 ,intensityIdle ,intensityLow ,intensityNormal ,intensityHigh ,intensityMax
    };

    enum WorkState {
        stateNone=0 ,stateIdle ,stateConnected ,stateMining ,statePaused
    };

//--
    String name;  //! miner name

    bool isAuto = false;      //! mining is automated
    uint32_t nWorkers = 0;    //! number of workers

    WorkIntensity workIntensity = intensityHigh;
    WorkState workState = stateIdle;
};

///--
struct MiningInfo {
    uint64_t hashes = 0;    //! hashes since last start

    uint32_t accepted = 0;  //! number of accepted result (shares|blocks, since last start)
    uint32_t stale = 0;     //! number of stale result
    uint32_t partial = 0;   //! number of partial result
    uint32_t rejected = 0;  //! number of rejected result

    double difficulty = 1.;  //! last result difficulty
    uint32_t elapsedMs = 0;  //! time since last result
};

//////////////////////////////////////////////////////////////////////////////
//! IMinerListener

class IMinerListener {
public:
    IAPI_DECL onStatus( IMiner &miner ,MinerInfo::WorkState state ,MinerInfo::WorkState oldState ) = 0;
    IAPI_DECL onJob( IMiner &miner ,const MiningInfo &info ) = 0;
    IAPI_DECL onResult( IMiner &miner ,const MiningInfo &info ) = 0;
};

typedef PtrOf<IMinerListener> IMinerListenerPtr;

//////////////////////////////////////////////////////////////////////////////
//! IMiner

class IMiner {
public:
    virtual ~IMiner() {}

public:
    IAPI_DECL Configure( CConnection &connection ) = 0;
    IAPI_DECL GetInfo( MinerInfo &info ) = 0;
    IAPI_DECL GetInfo( MiningInfo &info ) = 0;

    IAPI_DECL Start() = 0;
    IAPI_DECL Stop( int32_t msTimeout=-1 ) = 0;

    //? LATER Pause/Resume
};

typedef PtrOf<IMiner> IMinerPtr;

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_IMINER_H