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

//////////////////////////////////////////////////////////////////////////////
#include "app-service.h"

//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! CService

IAPI_DEF CService::getInterface( PUID id ,void **ppv ) {
    if( !ppv || *ppv ) return IBADARGS;

    return
        honorInterface_<CService>(this,id,ppv) || honorInterface_<IService>(this,id,ppv) ? IOK
        : CObject::getInterface( id ,ppv )
    ;
}

//////////////////////////////////////////////////////////////////////////////
//! CServiceSetup

IAPI_DEF CServiceSetup::getInterface( PUID id ,void **ppv ) {
    if( !ppv || *ppv ) return IBADARGS;

    return
        honorInterface_<CServiceSetup>(this,id,ppv) || honorInterface_<IServiceSetup>(this,id,ppv) ? IOK
        : CObject::getInterface( id ,ppv )
    ;
}

void CServiceSetup::setConnectExisting( Params &params ,bool exist ) {
    params["instance"] = (exist ? "exist" : "new");
}

///-- IServiceSetup

///-- installed
IAPI_DEF CServiceSetup::addConfiguration( const Params &params ) {
    m_installed.emplace_back( params );
    return IOK;
}

IAPI_DEF CServiceSetup::listInstalled( ListOf<ServiceInfo> &info ) {
    //TODO
    return INOEXEC;
}

IAPI_DEF CServiceSetup::Install( ServiceInfo &info ,const char *source ,const char *settings ) {
    //TODO
    return INOEXEC;
}

IAPI_DEF CServiceSetup::Uninstall( ServiceInfo &info ) {
    //TODO
    return INOEXEC;
}

///-- running
IAPI_DEF CServiceSetup::listService( ListOf<IServiceRef> &services ) {
    for( const auto &it : m_services ) {
        services.emplace_back( it );
    }

    return IOK;
}

IAPI_DEF CServiceSetup::Connect( ServiceInfo &info ,const Params &params ,IServiceRef &service ) {
    String wantInstance = getMember( params ,"instance" ,"" );

    //-- find if service is running
    if( wantInstance != "new" ) //! wants only new instance
        for( auto &it : m_services ) {
            if( matchService( info ,it ) ) {
                service = it; return IOK;
            }
        }

    if( wantInstance == "exist" ) //! wants only existing instance
        return INOEXIST;

    //-- find if service is installed
    Params config;

    for( auto &it : m_installed ) {
        if( matchService( info ,it ) ) {
            config = it; break; //! get config if found .. //TODO check, if not found ?
        }
    }

    addMembers( config ,params ,false );

    //-- create a new service matching specification
    if( !connectNew( info ,config ,service ) )
        return IERROR;

    assert( !service.isNull() );

    //-- start the service
    return service ? service->Start( config ) : IBADENV;
}

IAPI_DEF CServiceSetup::adviseStarted( IService &service ) {
    m_services.emplace_back( service );

    return IOK;
}

IAPI_DEF CServiceSetup::adviseStopped( IService &service ) {
    for( auto it = m_services.begin(); it != m_services.end(); ++it ) {
        if( (IService *) it->ptr() == &service ) {
            m_services.erase( it );
            return IOK;
        }
    }

    return IOK;
}

///-- CService
inline bool matchStringSpecs( const char *value ,const char *specs ) {
    assert( value ); if( value == NullPtr ) return false;

    return !(specs && specs[0] && specs[0] != '*') || (value[0]=='*' || stricmp( value ,specs ) == 0);
}

bool CServiceSetup::matchService( const ServiceInfo &info ,IServiceRef &service ) {
    if( service.isNull() ) return false;

    ServiceInfo peer;

    if( service->getInfo( peer ) != IOK )
        return false;

    return
        matchStringSpecs( peer.category.c_str() ,info.category.c_str() )
        && matchStringSpecs( peer.name.c_str() ,info.name.c_str() )
        && matchStringSpecs( peer.taxonomy.c_str() ,info.taxonomy.c_str() )
        && matchStringSpecs( peer.version.c_str() ,info.version.c_str()  )
    ;
}

bool CServiceSetup::matchService( const ServiceInfo &info ,const Params &params ) {
    const char *taxonomy = getMember( params ,"taxonomy" ,"" );
    const char *version = getMember( params ,"version" ,"" );

    return
        matchStringSpecs( taxonomy ,info.taxonomy.c_str() )
        && matchStringSpecs( version ,info.version.c_str() )
    ;
}

//////////////////////////////////////////////////////////////////////////////
//! CServiceStore

IAPI_DEF CServiceStore::getInterface( PUID id ,void **ppv ) {
    if( !ppv || *ppv ) return IBADARGS;

    return
        honorInterface_<CServiceStore>(this,id,ppv) || honorInterface_<IServiceStore>(this,id,ppv) ? IOK
        : CObject::getInterface( id ,ppv )
    ;
}

///-- IServiceStore
IAPI_DEF CServiceStore::loadConfig( Config &config ) {
    const auto &section = config.getSection( "cores" ); //TODO section depends on service store

    IServiceSetupRef setup;

    for( const auto &it : section.params ) { //! for each setup
        IF_IFAILED( getServiceSetup( it.first.c_str() ,setup ) ) {
            //TODO log this as a config warning
            continue;
        }

        Params setupConfig;

        fromString( setupConfig ,it.second );

        setup->addConfiguration( setupConfig );
    }

    return IOK;
}

IAPI_DEF CServiceStore::listServiceSupport( ListOf<String> &supported ) {
    for( const auto &it : m_services ) {
        supported.emplace_back( it.first );
    }

    return IOK;
}

IAPI_DEF CServiceStore::getServiceSetup( const char *name ,IServiceSetupRef &setup ) {
    auto it = m_services.find(name);

    if( it == m_services.end() )
        return INODATA;

    setup = it->second;

    return IOK;
}

///-- CServiceStore

bool CServiceStore::Connect( ServiceInfo &info ,const Params &params ,IServiceRef &service ) {
    IServiceSetupRef setup;

    if( getServiceSetup( tocstr(info.name) ,setup ) != IOK )
        return false;

    return setup->Connect( info ,params ,service ) == IOK;
}

//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
//EOF