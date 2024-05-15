#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_MINERSTAT_CHAIN_H
#define SOLOMINER_MINERSTAT_CHAIN_H

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>
#include <common/http.h>

#include <chains/chains.h>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
#define MINERSTAT_NAME          "minerstat"
#define MINERSTAT_HOSTNAME      "https://api.minerstat.com"
#define MINERSTAT_APIPATH       "/v2"

//////////////////////////////////////////////////////////////////////////////
class CChainMinerstat : public CChainService {
protected:
    CHttpConnection m_http;

    ChainConfig m_config;

    String m_host;

public:
    CChainMinerstat( IServiceSetupRef &setup ) : CChainService(setup)
    {
        this->info().name = MINERSTAT_NAME;

        m_http.cache().Load( "minerstat-cache.dat" );
    }

    ~CChainMinerstat() {
        m_http.cache().Save();
    }

public: ///-- IChain
    IAPI_IMPL getInfo( const char *coin ,ChainInfo &info ,ChainInfoFlags flags ) IOVERRIDE;

public: ///-- IService
    IAPI_IMPL getInfo( ServiceInfo &info ) IOVERRIDE {
        info.category = CChainService::category();
        info.name = MINERSTAT_NAME;

        return IOK;
    }

    IAPI_IMPL Start( const Params &params ) IOVERRIDE {
        fromManifest( m_config ,params );

        m_host = !m_config.host.empty() ? m_config.host : MINERSTAT_HOSTNAME;

        m_http.quota().setLimit( 10 ,60 );

        return CChainService::Start( params );
    }
};

//////////////////////////////////////////////////////////////////////////////
class CChainSetupMinerstat : public CChainSetup {
public:
    API_IMPL(bool) connectNew( ServiceInfo &info ,const Params &params ,IServiceRef &service ) IOVERRIDE {
        IServiceSetupRef setup( *this );

        service = new CChainMinerstat( setup );

        return true;
    }
};

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_MINERSTAT_CHAIN_H