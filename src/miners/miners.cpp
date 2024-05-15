// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include "miners.h"
#include "connections.h"

//////////////////////////////////////////////////////////////////////////////
//! Embedded xmrig

#include <App.h>
#include <base/kernel/Process.h>
#include <base/kernel/interfaces/IStrategy.h>
#include <base/kernel/interfaces/IStrategyListener.h>
#include <base/net/stratum/SubmitResult.h>

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
typedef MinerInfo::WorkIntensity WorkIntensity;
typedef MinerInfo::WorkState WorkState;

//////////////////////////////////////////////////////////////////////////////
//! Mine thread base class

class CMinerThreadedBase : public CMinerBase ,protected Thread {
protected:
    MinerInfo m_minerInfo;
    MiningInfo m_miningInfo;

public:
    CMinerThreadedBase() {}

public: ///-- IMiner interface
    IAPI_IMPL GetInfo( MinerInfo &info ) IOVERRIDE {
        info = m_minerInfo; return IOK;
    }

    IAPI_IMPL GetInfo( MiningInfo &info ) IOVERRIDE {
        info = m_miningInfo; return IOK;
    }

    IAPI_IMPL Start() IOVERRIDE {
        return (Thread::Start() == ENOERROR) ? IOK : IERROR;
    }

    IAPI_IMPL Stop( int32_t msTimeout=-1 ) IOVERRIDE {
        return (Thread::Stop(msTimeout) == ENOERROR) ? IOK : IERROR;
    }
};

//////////////////////////////////////////////////////////////////////////////
//! Utils

void vArgPack( int argc ,const char *argv[] ,Memory_<char> &mem ,const char *pack[] ) {
    int n = 0;

    for( int i=0; i<argc; ++ i ) {
        n += strlen( argv[i] ) + 1;
    }

    mem.Alloc( n ); n = 0;

    for( int i=0; i<argc; ++ i ) {
        int len = strlen(argv[i])+1;

        memcpy( mem.ptr()+n ,argv[i] ,len );

        pack[i] = mem.ptr() + n;

        n += len;
    }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Embedded xmrig

#define	SIGTERM		15

class CAppXmrig {
protected:
    CriticalSection m_cs;

    xmrig::App *m_app = NullPtr;

    bool m_running = false;

public:
    virtual void Start( xmrig::App &app ) {
        m_cs.Enter();
        {
            m_app = &app; m_running = true;
        }
        m_cs.Leave();
    }

    virtual void Report() {
        m_cs.Enter();
        {
            if( m_app ) m_app->doCommand('h');
        }
        m_cs.Leave();
    }

    virtual void Pause() {
        m_cs.Enter();
        {
            if( m_app ) m_app->doCommand('p');
        }
        m_cs.Leave();
    }

    virtual void Resume() {
        m_cs.Enter();
        {
            if( m_app ) m_app->doCommand('r');
        }
        m_cs.Leave();
    }

    //! @note returns true if app was stopped
    virtual bool Stop( uint32_t msTimeout=-1 ) {
        bool stopped = false;

        m_cs.Enter(); if( m_app && m_running )
        {
            m_running = false;

            m_app->Stop();

            stopped = true;
        }
        m_cs.Leave();

        return stopped;
    }

    virtual void Quit() {
        m_cs.Enter();
        {
            if( m_app ) m_app->Quit();
        }
        m_cs.Leave();
    }
};

class CMainXmrig : public Thread {
public:
    OsError Main() override {
        xmrig::App::Main();

        return ENOERROR;
    }
};

template <typename T>
static std::string makeOption_( const char *option ,T value ) {
    std::stringstream ss;
    ss << "--" << option << "=" << value;

    return ss.str();
}

String makeMiningAddress( const String &coin ,const String &miningAddress) {
    const char *devAddress = getDevAddress( coin.c_str() );

    String s;

    s = coin; s += "/";
    s += devAddress ? devAddress : ""; s += ":";
    s += miningAddress;

    return s;
}

///--
class CMinerXmrig : public CMinerThreadedBase , public xmrig::IStrategyListener {
public:
    struct WorkStateX { //! Work state transition
        bool isTransition;
        WorkState state;
        WorkState oldState;
    };

    WorkStateX SetWorkState( WorkState state ) {
        WorkStateX wsx;

        wsx.isTransition = (m_minerInfo.workState != state);
        wsx.oldState = m_minerInfo.workState;
        wsx.state = m_minerInfo.workState = state;

        return wsx;
    }

    void UpdateMiningInfo( xmrig::IStrategy *strategy ) {
        xmrig::INetworkState *netState = strategy->network()->state();

        if( netState == NullPtr ) {
            assert(false); return; //! should not happen
        }

        m_miningInfo.accepted = (uint32_t) netState->acceptedShare();
        m_miningInfo.partial = (uint32_t) netState->partialShare();
        m_miningInfo.rejected = (uint32_t) netState->rejectedShare();
        //TODO stale ?

        m_miningInfo.hashes = (uint64_t) netState->hashes();
        m_miningInfo.difficulty = (double) netState->diff();
    }

    CAppXmrig m_app;

public: ///-- xmrig::IClientListener interface

    ///-- events
    virtual void onActive( xmrig::IStrategy *strategy ,xmrig::IClient *client ) {
        WorkStateX wsx = SetWorkState( WorkState::stateConnected );

        if( m_listener && wsx.isTransition )
            m_listener->onStatus( *this ,wsx.state ,wsx.oldState );
    }

    virtual void onVerifyAlgorithm( xmrig::IStrategy *strategy ,const xmrig::IClient *client ,const xmrig::Algorithm &algorithm ,bool *ok ) {
        //! NOP
    }

    virtual void onJob( xmrig::IStrategy *strategy ,xmrig::IClient *client ,const xmrig::Job &job ,const rapidjson::Value &params ) {
        WorkStateX wsx = SetWorkState( WorkState::stateMining );

        if( !m_listener ) return;

        if( wsx.isTransition )
            m_listener->onStatus( *this ,wsx.state ,wsx.oldState );

        UpdateMiningInfo( strategy );

        m_listener->onJob( *this ,this->m_miningInfo );
    }

    virtual void onResultAccepted( xmrig::IStrategy *strategy ,xmrig::IClient *client ,const xmrig::SubmitResult &result ,const char *error ) {
        WorkStateX wsx = SetWorkState( WorkState::stateMining );

        if( !m_listener ) return;

        if( wsx.isTransition )
            m_listener->onStatus( *this ,wsx.state ,wsx.oldState );

        UpdateMiningInfo( strategy );

        m_miningInfo.elapsedMs = (uint32_t) result.elapsed;

        m_listener->onResult( *this ,this->m_miningInfo );
    }

    virtual void onLogin( xmrig::IStrategy *strategy ,xmrig::IClient *client ,rapidjson::Document &doc ,rapidjson::Value &params ) {
        WorkStateX wsx = SetWorkState( WorkState::stateConnected );

        if( m_listener && wsx.isTransition )
            m_listener->onStatus( *this ,wsx.state ,wsx.oldState );
    }

    virtual void onPause( xmrig::IStrategy *strategy ) {
        WorkStateX wsx = SetWorkState( WorkState::statePaused );

        if( m_listener && wsx.isTransition )
            m_listener->onStatus( *this ,wsx.state ,wsx.oldState );
    }

public: //! IMiner interface
    IAPI_IMPL Stop( int32_t msTimeout=-1 ) IOVERRIDE {
        if( !m_app.Stop(msTimeout) )
            return IOK; //! no app or already stopped

        if( WaitFor(msTimeout) == EFAILED ) {
            m_app.Quit();

            Thread::Stop(msTimeout);
        }

        //!-- @note state update here, not in app thread, making sure it's called
        WorkStateX wsx = SetWorkState( WorkState::stateIdle );

        if( m_listener && wsx.isTransition )
            m_listener->onStatus( *this ,wsx.state ,wsx.oldState );

        //--
        return IOK;
    }

public: //! CMinerThreadedBase

    int AppMain() {
        static CMainXmrig *main = nullptr;

        if( main == nullptr ) {
            main = new CMainXmrig();

            main->Start();
        }

        return 0;
    }

    int AppExec( int argc ,char **argv ,xmrig::IStrategyListener *strategyListener ) {
        using namespace xmrig;

        Process process( argc ,argv );

        App app( &process );

        // AppMain();

        m_app.Start( app );

        int rc = app.Exec( strategyListener );

        return rc;
    }

    OsError Main() override {
        if( m_connection == nullptr )
            return EBADE;

        ConnectionInfo &info  = m_connection->info();

        std::string nThreads = makeOption_( "threads" ,info.status.nThreads );
        std::string coin = makeOption_( "coin" ,info.mineCoin.coin );
        std::string address = makeMiningAddress( info.mineCoin.coin ,info.mineCoin.address );
        String host = info.connection.host;

        if( info.connection.port > 0 ) {
            String port;

            toString( info.connection.port ,port );
            host += ':'; host += port;
        }

        const char *argv[] = {
            "--asm=ryzen" //TODO topology
            ,nThreads.c_str()
            ,coin.c_str()
            ,"-a" ,"ghostrider" //TODO algo
            ,"-o" ,host.c_str()
            ,"-u" ,info.credential.user.c_str()
            ,"-p" ,info.credential.password.c_str()
            ,"-d" ,address.c_str() // info.mineCoin.address.c_str()
            ,"--daemon-job-timeout=2000" //TODO from config
            ,"" // --core / --daemon
            ,"" // --tls
        };

        int argc = 14;

        if( info.options.isDaemon || info.options.isCore ) { //TODO split daemon/core
            argv[argc++] = "--daemon";
        }
        if( info.options.isTls ) {
            argv[argc++] = "--tls";
        }

        //-- UV lib require argv memory to be adjacent, packing it here
        Memory_<char> packMem;

        const char *packVArg[16];

        vArgPack( argc ,argv ,packMem ,packVArg );

        //-- go
        AppExec( argc ,(char**) packVArg ,this );

        return ENOERROR;
    }
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
CMinerBase *makeMiner( CConnection &connection ,IMinerListener *minerListener ,bool configure ) {
    //TODO LATER choose miner for the device/algo/coin

    CMinerBase *miner = new CMinerXmrig();

    if( minerListener )
        miner->setListener( minerListener );

    if( configure )
        miner->Configure( connection );

    return miner;
}

void destroyMiner( IMiner **miner ) {
    if( miner && *miner ) {
        (*miner)->Stop();

        delete *miner;

        *miner = NullPtr;
    }
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF