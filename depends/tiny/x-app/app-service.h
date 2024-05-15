#pragma once

/****************************************************** //
//              tiny-for-c++ v3 library                 //
//              -----------------------                 //
//   Copyright (c) 2016-2024 | NEXTWave Technologies    //
//      <http://www.nextwave-techs.com/>                //
// ******************************************************/

//! Check if your project qualifies for a free license
//!   at http://nextwave-techs.com/license

//! THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//! IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//! FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//!        AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//! LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//! OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//! SOFTWARE.

#ifndef TINY_SERVICE_H
#define TINY_SERVICE_H

//////////////////////////////////////////////////////////////////////////////
#include <interface/IService.h>

//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
#define CSERVICEBASE_UUID           0x0f154115e4466683d
#define CSERVICESETUPBASE_UUID      0x09f5735d2796615b6
#define CSERVICESTOREBASE_UUID      0x0f09e23f2fdbcf91c

inline bool serviceInProgress( ServiceState state ) {
    return state==serviceStarting || state==serviceConnecting || state==serviceStopping;
}

//////////////////////////////////////////////////////////////////////////////
//! IService default base class

class CService : COBJECT_PARENT ,public IService {
protected:
    ServiceInfo m_info;
    ServiceState m_state;

    IServiceSetupRef m_setup;

public:
    CService( IServiceSetupRef &setup ) :
            m_state(serviceStopped) ,m_setup(setup)
    {}

    IMPORT_IOBJECT_API(CSERVICEBASE_UUID);

    //-- const
    const ServiceInfo &info() const { return m_info; }
    const ServiceState &state() const { return m_state; }

    IServiceSetupRef setup() const { return m_setup; }

    //-- non-const
    ServiceInfo &info() { return m_info; } //TODO, check visibility

public: ///-- IService
    IAPI_IMPL getInfo( ServiceInfo &info ) IOVERRIDE {
        info = m_info; return IOK;
    }

    IAPI_IMPL getSetup( IServiceSetupRef &setup ) IOVERRIDE {
        setup = m_setup; return IOK;
    }

    IAPI_IMPL getState( ServiceState &state ) IOVERRIDE {
        state = m_state; return IOK;
    }

    IAPI_IMPL Start( const Params &params ) IOVERRIDE {
        if( state()==serviceStarting || state()==serviceStopping ) return IPROGRESS;
        if( m_state >= serviceStarting ) return IALREADY;

        iresult_t result = m_setup ? m_setup->adviseStarted( *this ) : IBADENV;

        if( !IFAILED(result) ) {  // result==IOK || result==IALREADY || result==IEXIST) {
            setState(serviceConnected);
        }

        return result;
    }

    IAPI_IMPL Retry( const Params &params ) IOVERRIDE {
        return (m_state < serviceStarting) ? INOEXIST : INOEXEC;
    }

    IAPI_IMPL Stop( const Params &params ) IOVERRIDE {
        if( m_state==serviceStopped ) return IALREADY;

        IRESULT result = m_setup ? m_setup->adviseStopped( *this ) : IBADENV;

        if( !IFAILED(result) ) {
            setState(serviceStopped);
        }

        return result;
    }

public: ///-- CService
    //...

protected:
    API_DECL(void) setState( ServiceState state ) {
        m_state = state; //TODO later event
    }
};

//--
typedef RefOf<CService> CServiceRef;

//////////////////////////////////////////////////////////////////////////////
//! IServiceSetup default base class

class CServiceSetup : COBJECT_PARENT ,public IServiceSetup {
protected:
    ListOf<Params> m_installed;

    ListOf<IServiceRef> m_services;

public:
    CServiceSetup() {}

    IMPORT_IOBJECT_API(CSERVICESETUPBASE_UUID);

    static void setConnectExisting( Params &params ,bool exist=true );

public: ///-- IServiceSetup
    IAPI_IMPL getInfo( ServiceSetupInfo &info ) IOVERRIDE {
        return INOEXEC;
    }

///-- installed
    IAPI_IMPL addConfiguration( const Params &params ) IOVERRIDE;

    IAPI_IMPL listInstalled( ListOf<ServiceInfo> &info ) IOVERRIDE;

    IAPI_IMPL Install( ServiceInfo &info ,const char *source ,const char *settings );

    IAPI_IMPL Uninstall( ServiceInfo &info ) IOVERRIDE;

///-- running
    IAPI_IMPL listService( ListOf<IServiceRef> &services ) IOVERRIDE;

    IAPI_IMPL Connect( ServiceInfo &info ,const Params &params ,IServiceRef &service ) IOVERRIDE;

///-- connect
    IAPI_IMPL adviseStarted( IService &service ) IOVERRIDE;

    IAPI_IMPL adviseStopped( IService &service ) IOVERRIDE;

public: ///-- CServiceSetup
    API_DECL(bool) matchService( const ServiceInfo &info ,IServiceRef &service );

    API_DECL(bool) matchService( const ServiceInfo &info ,const Params &params );

    API_DECL(bool) connectNew( ServiceInfo &info ,const Params &params ,IServiceRef &service ) {
        return false;
    }
};

//--
typedef RefOf<CServiceSetup> CServiceSetupRef;

//////////////////////////////////////////////////////////////////////////////
//! IServiceStore default base class

class CServiceStore : COBJECT_PARENT ,public IServiceStore {
protected:
    MapOf<String,IServiceSetupRef> m_services;

public:
    CServiceStore() {}

    IMPORT_IOBJECT_API(CSERVICESTOREBASE_UUID);

public: ///-- IServiceStore
    IAPI_IMPL getInfo( ServiceStoreInfo &info ) IOVERRIDE {
        return INOEXEC;
    }

    IAPI_IMPL loadConfig( Config &config ) IOVERRIDE;

    IAPI_IMPL listServiceSupport( ListOf<String> &supported ) IOVERRIDE;

    IAPI_IMPL registerServiceSupport( const char *name ,IServiceSetupRef &setup ) IOVERRIDE {
        m_services[name] = setup; return IOK;
    }

    IAPI_IMPL unregisterServiceSupport( const char *name ) IOVERRIDE {
        m_services.erase( name ); return IOK;
    }

    ///--
    IAPI_IMPL getServiceSetup( const char *name ,IServiceSetupRef &setup ) IOVERRIDE;

public: ///-- CServiceStore

    ///-- pass thru
    bool Connect( ServiceInfo &info ,const Params &params ,IServiceRef &service );

    template <class TService>
    bool Connect_( ServiceInfo &info ,const Params &params ,RefOf<TService> &service ) {
        IServiceRef r;

        if( !Connect( info ,params ,r ) )
            return false;

        service = r;

        return true;
    }
};

///--
template <class TStore>
inline TStore &getStore_() { return TStore::getInstance(); }

//! to be implemented by client app
CServiceStore *getStore( const char *category );

///--
template <class TService>
inline bool ConnectService_( ServiceInfo &info ,const Params &params ,RefOf<TService> &service ) {
    CServiceStore *p = getStore( info.category.c_str() );

    return p && p->Connect_( info ,params ,service );
}

inline bool ConnectService( ServiceInfo &info ,const Params &params ,CServiceRef &service ) {
    return ConnectService_( info ,params ,service );
}

inline bool StartService( ServiceInfo &info ,const Params *params=NullPtr ) {
    Params p0; CServiceRef r0; return ConnectService( info ,params ? *params : p0 ,r0 );
}

inline bool StartService( const char *category ,const char *name ,const Params *params=NullPtr ) {
    ServiceInfo info={category,name}; return StartService( info ,params );
}

///--
template <class TService>
inline bool getService_( const char *category ,const char *name ,RefOf<TService> &service ) {
    ServiceInfo info={ category ,name}; Params p0={{"instance","exist"}};

    return ConnectService_( info ,p0 ,service );
}

template <class TService>
inline bool getService_( const char *name ,RefOf<TService> &service ) {
    return getService_( TService::category() ,name ,service );
}

inline bool getService( const char *category ,const char *name ,CServiceRef &service ) {
    return getService_( category ,name ,service );
}

template <class TService>
inline RefOf<TService> getService( const char *category ,const char *name ) {
    RefOf<TService> r0; getService_( category ,name ,r0 ); return r0;
}

template <class TService>
inline RefOf<TService> getService( const char *name ) {
    RefOf<TService> r0; getService_( name ,r0 ); return r0;
}

//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
#endif //TINY_SERVICE_H