#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_MINERS_H
#define SOLOMINER_MINERS_H

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>
#include <interface/IMiner.h>

#include <tiny-core.hpp>

#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
class CMinerListenerBase :  public IMinerListener {
public:
    IAPI_IMPL onStatus( IMiner &miner ,MinerInfo::WorkState state ,MinerInfo::WorkState oldState ) IOVERRIDE { return ENOEXEC; }
    IAPI_IMPL onJob( IMiner &miner ,const MiningInfo &info ) IOVERRIDE { return ENOEXEC; }
    IAPI_IMPL onResult( IMiner &miner ,const MiningInfo &info ) IOVERRIDE { return ENOEXEC; }
};

//////////////////////////////////////////////////////////////////////////////
class CMinerBase : public IMiner {
protected:
    CConnection *m_connection;
    IMinerListener *m_listener;

public:
    CMinerBase() : m_connection(NullPtr) ,m_listener(NullPtr)
    {}

    void setListener( IMinerListener *listener ) {
        m_listener = listener;
    }

    CConnection *getConnection() {
        return m_connection;
    }

public: ///-- IMiner interface
    IAPI_IMPL Configure( CConnection &connection ) IOVERRIDE {
        m_connection = &connection;
        return IOK;
    }

    //! @note required to be implemented by derived class
    /*
    IAPI_IMPL GetInfo( MinerInfo &info ) IOVERRIDE;
    IAPI_IMPL GetInfo( MiningInfo &info ) IOVERRIDE;
    IAPI_IMPL Start() IOVERRIDE;
    IAPI_IMPL Stop( int32_t msTimeout=-1 ) IOVERRIDE;
    */
};

///--
CMinerBase *makeMiner( CConnection &connection ,IMinerListener *minerListener ,bool configure=true );

void destroyMiner( IMiner **miner );

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_MINERS_H