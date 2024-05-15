#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_BITCOIN_H
#define SOLOMINER_BITCOIN_H

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>
#include <coins/cores.h>

#include <wallets/wallets.h>
#include <chains/chains.h>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
class CCoreBitcoinBase;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Wallet

class CWalletBitcoinBase : public CWalletService {

    template <class TWallet ,class TChain>
    friend class CCoreBitcoinBase_;

protected:
    CCoreBitcoinBase &m_core; //! parent daemon core service

    int m_initialTxTime; //! latest transaction time as first seen when running the wallet

public:
    CWalletBitcoinBase( CCoreBitcoinBase &core ,IServiceSetupRef &setup ) : CWalletService(setup)
        ,m_core(core) ,m_initialTxTime(0)
    {}

    API_IMPL(ref_t) AddRef() IOVERRIDE;
    API_IMPL(ref_t) Release() IOVERRIDE;

    const CCoreBitcoinBase &core() const { return m_core; }

    CCoreBitcoinBase &core() { return m_core; }

public: ///-- CWalletBitcoinBase
    IAPI_DECL getCoin( String &coin ); //TODO ?? here ?

public: ///-- IService
    IAPI_IMPL Start( const Params &params ) IOVERRIDE;
    IAPI_IMPL Retry( const Params &params ) IOVERRIDE;
    IAPI_IMPL Stop( const Params &params ) IOVERRIDE;

public: ///-- IWallet
    IAPI_IMPL getCoinList( ListOf<String> &coins ) IOVERRIDE;

///-- manage
    IAPI_IMPL unlock( const char *password ) IOVERRIDE;
    IAPI_IMPL lock() IOVERRIDE;

///-- accounts
    //...

///-- addresses
    IAPI_IMPL getNewAddress( const char *accountId ,String &address ) IOVERRIDE;
    IAPI_IMPL getAddressBalance( const char *address ,AmountValue &balance ) IOVERRIDE;

    IAPI_IMPL listTransactions( const char *address ,ListOf<WalletTransaction> &transactions ,int from=0 ,int count=0 ) IOVERRIDE;
    IAPI_IMPL getTransaction( const char *address ,const char *transactionId ,WalletTransaction &transaction ) IOVERRIDE;
    IAPI_IMPL sendToAddress( WalletTransaction &transaction ) IOVERRIDE;

public: ///-- bitcoin api

};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Chain

class CChainBitcoinBase : public CChainService {

    template <class TWallet ,class TChain>
    friend class CCoreBitcoinBase_;

protected:
    CCoreService &m_core;
    // CoreConfig m_config;

public:
    CChainBitcoinBase( CCoreService &core ,IServiceSetupRef &setup ) :
        m_core(core) ,CChainService(setup)
    {}

    API_IMPL(ref_t) AddRef() IOVERRIDE { return m_core.AddRef(); }
    API_IMPL(ref_t) Release() IOVERRIDE { return m_core.Release(); }

    const CCoreService &core() const { return m_core; }
    // const CoreConfig &config() const { return m_config; }

    CCoreService &core() { return m_core; }

public: ///-- CChainBitcoinBase
    IAPI_DECL getCoin( String &coin ) {
        return m_core.getCoin(coin);
    }
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Core

#define CORE_TIMEOUT_DEFAULT    10000

class CCoreBitcoinBase : public CCoreService {
protected:
    Params m_params;

    time_t m_startTime = 0;
    time_t m_startTimeout = CORE_TIMEOUT_DEFAULT;

    OsHandle m_hprocess = OS_INVALID_HANDLE;

public:
    CCoreBitcoinBase( IServiceSetupRef &coreSetup ) :
        CCoreService(coreSetup)
    {}

    const Params &params() const { return m_params; }

public: ///-- CCoreBitcoinBase
    IAPI_DECL ConnectDaemon( const char *name ,const Params &params );

    IAPI_DECL StartDaemon( const char *name ,const Params &params );

    IAPI_DECL StopDaemon( const Params &params );

    //-- helper
    IAPI_DECL ConnectAndStartDaemon( const char *name ,const Params &params );
};

///--
template <class TWallet ,class TChain>
class CCoreBitcoinBase_ : public CCoreBitcoinBase {
protected:
    TWallet m_wallet;
    TChain m_chain;

public:
    CCoreBitcoinBase_( IServiceSetupRef &coreSetup ) :
        CCoreBitcoinBase(coreSetup) ,m_wallet(*this,coreSetup) ,m_chain(*this,coreSetup)
    {}

    API_IMPL(ref_t) AddRef() IOVERRIDE { return CCoreBitcoinBase::AddRef(); }
    API_IMPL(ref_t) Release() IOVERRIDE { return CCoreBitcoinBase::Release(); }

    static const char *getName();
    static const char *getCoin();

    const CoreConfig &config() const { return m_config; }

    TWallet &wallet() { return m_wallet; }
    TChain &chain() { return m_chain; }

public: ///-- IService
    IAPI_IMPL getInfo( ServiceInfo &info ) IOVERRIDE {
        info.category = CORE_SERVICE_CATEGORY;
        info.name = getName();

        return IOK;
    }

    IAPI_IMPL Start( const Params &params ) IOVERRIDE {
        if( m_setup.isNull() ) return IBADENV;
        if( serviceInProgress(state()) ) return IPROGRESS;
        if( state()>=serviceStarted ) return IALREADY;

        IRESULT result = IERROR;

        //-- config/params
        fromManifest( m_config ,m_params = params );

        const char *timeout = getMember( m_params ,"timeout" ,TINY_STR(CORE_TIMEOUT_DEFAULT) ) ;

        m_startTimeout = (time_t) atoi(timeout);

        //-- start
        result = m_setup->adviseStarted( *this ); IF_IFAILED_RETURN(result);

        setState(serviceStarted);

        m_wallet.Start( params );
        m_chain.Start( params );

        //-- connect to daemon
        //! @note return success from here even if connection fails (the service has been successfully started)
        result = ConnectAndStartDaemon( getName() ,params );

        IF_IFAILED(result) return IAGAIN;

        return IOK;
    }

    IAPI_IMPL Retry( const Params &params ) IOVERRIDE {
        if( state()==serviceConnected ) return IOK;

        //-- retry connection
        if( state()==serviceStarted || state()==serviceConnecting ) {
            Params methodParams = m_params;

            addMembers( methodParams ,params );

            return ConnectAndStartDaemon( getName() ,methodParams );
        }

        //-- no other retry state
        return INOEXEC;
    }

    IAPI_IMPL Stop( const Params &params ) IOVERRIDE {
        if( state()==serviceStarting || state()==serviceStopping ) return IPROGRESS;
        if( state()==serviceStopped ) return IALREADY;

        Params methodParams = m_params;

        addMembers( methodParams ,params );

        StopDaemon( methodParams );

        setState(serviceStarted); //! IE not connected

        m_wallet.Stop(params);
        m_chain.Stop(params);

        return CService::Stop(params);
    }

public: ///-- ICore
    IAPI_IMPL getName( String &name ) IOVERRIDE {
        name = getName(); return IOK;
    }

    IAPI_IMPL getCoin( String &coin ) IOVERRIDE {
        coin = getCoin(); return IOK;
    }

    IAPI_IMPL getWallet( IWalletRef &wallet ) IOVERRIDE {
        wallet = m_wallet; return IOK;
    }

    IAPI_IMPL getChain( IChainRef &chain ) IOVERRIDE {
        chain = m_chain; return IOK;
    }

public: ///-- CService
    //...

protected:
    API_IMPL(void) setState( ServiceState state ) IOVERRIDE {
        CService::setState(state);
        m_wallet.setState(state);
        m_chain.setState(state);
    }
};

//////////////////////////////////////////////////////////////////////////////
template <class TCore>
class CCoreBitcoinSetup_ : public CCoreSetup
{
public:
    CCoreBitcoinSetup_() {
        IServiceSetupRef r( *this );

        getWalletStore().registerServiceSupport( TCore::getName() ,r );
        getChainStore().registerServiceSupport( TCore::getName() ,r );
    }

    ~CCoreBitcoinSetup_() {
        getWalletStore().unregisterServiceSupport( TCore::getName() );
        getChainStore().unregisterServiceSupport( TCore::getName() );
    }

public: ///-- IServiceSetup
    IAPI_IMPL Install( ServiceInfo &info ,const char *source ,const char *settings ) IOVERRIDE {
        return INOEXEC;
    }

    IAPI_IMPL Uninstall( ServiceInfo &info ) IOVERRIDE {
        return INOEXEC;
    }

    ///-- running
    IAPI_IMPL Connect( ServiceInfo &info ,const Params &params ,IServiceRef &service ) IOVERRIDE {
        ServiceInfo coreInfo = info; IServiceRef core;

        //! get or start core
        coreInfo.category = CCoreService::category();

        IRESULT result = CServiceSetup::Connect( coreInfo ,params ,core ); IF_IFAILED_RETURN(result);

        assert( !core.isNull() );

        if( info.category == "core" ) {
            service = core; return IOK;
        }

        CCoreServiceRef ccore; ccore = core;

        if( info.category == "wallet" ) {
            IWalletRef wallet;
            ccore->getWallet( wallet );
            service = wallet;

        } else if( info.category == "chain" ) {
            IChainRef chain;
            ccore->getChain( chain );
            service = chain;

        } else {
            return INOEXEC;
        }

        return IOK;
    }

public: ///-- CServiceSetupBase_
    API_IMPL(bool) connectNew( ServiceInfo &info ,const Params &params ,IServiceRef &service ) IOVERRIDE {
        if( info.category != "core" )
            return false;

        IServiceSetupRef setup( *this );

        auto *core = new TCore( setup );

    //-- done
        service = core;

        return true;
    }
};

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_BITCOIN_H