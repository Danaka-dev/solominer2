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

#ifndef TINY_ISERVICE_H
#define TINY_ISERVICE_H

//////////////////////////////////////////////////////////////////////////////
#include <tiny.h>

//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
#define ISERVICE_UUID           0x0b6278549264a959e
#define ISERVICESETUP_UUID      0x006865c663ef913ca
#define ISERVICESTORE_UUID      0x05e675c822509bef0

///--
class IService;
class IServiceSetup;
class IServiceStore;

//////////////////////////////////////////////////////////////////////////////
#define ISERVICE_VERSION_CURRENT    "current"   //! latest currently installed on the host
#define ISERVICE_VERSION_LATEST     "latest"    //! latest available (from some source)

//--
struct ServiceInfo {
    String category;    //! Service category (IE related to service store instance, unique globally)
    String name;        //! Service name (IE related to service setup instance, unique by store)
    String taxonomy;    //! Service specific class (EG a named network, a client or a server,...)
    String version;     //! Service version
};

//--
struct ServiceSetupInfo {
    String name;         //! service setup name (unique by store)
    bool installable;    //! service can be installed (true even if service is already installed)
};

//--
struct ServiceStoreInfo {
    String category;
};

//////////////////////////////////////////////////////////////////////////////
enum ServiceState {
    ssUnknown = 0
    ,serviceStopped     //! service not started or has been stopped
    ,serviceStarting    //! service start in progress
    ,serviceStarted     //! service is started (not connecting, use retry)
    ,serviceConnecting  //! service is connecting
    ,serviceConnected   //! service is up and running
    ,serviceStopping    //! service stop is in progress
};

//////////////////////////////////////////////////////////////////////////////
class IService : IOBJECT_PARENT
{
public: ///-- IObject
    DECLARE_ICLASS(IService,ISERVICE_UUID);

public: ///-- IService
    IAPI_DECL getInfo( ServiceInfo &info ) = 0;
    IAPI_DECL getSetup( RefOf<IServiceSetup> &setup ) = 0;
    IAPI_DECL getState( ServiceState &state ) = 0;

    IAPI_DECL Start( const Params &params ) = 0;
    IAPI_DECL Retry( const Params &params ) = 0;
    IAPI_DECL Stop( const Params &params ) = 0;
};

typedef RefOf<IService> IServiceRef;

//////////////////////////////////////////////////////////////////////////////
class IServiceSetup : IOBJECT_PARENT
{
public: ///-- IObject
    DECLARE_ICLASS(IServiceSetup,ISERVICESETUP_UUID);

public: ///-- IServiceSetup
    IAPI_DECL getInfo( ServiceSetupInfo &setupInfo ) = 0;

///-- installed
    IAPI_DECL addConfiguration( const Params &params ) = 0;
    IAPI_DECL listInstalled( ListOf<ServiceInfo> &info ) = 0;
    IAPI_DECL Install( ServiceInfo &info ,const char *source ,const char *settings ) = 0;
    IAPI_DECL Uninstall( ServiceInfo &info ) = 0;

///-- running
    IAPI_DECL listService( ListOf<IServiceRef> &services ) = 0;
    IAPI_DECL Connect( ServiceInfo &info ,const Params &params ,IServiceRef &service ) = 0;

///-- callback
    IAPI_DECL adviseStarted( IService &service ) = 0;
    IAPI_DECL adviseStopped( IService &service ) = 0;
};

typedef RefOf<IServiceSetup> IServiceSetupRef;

//////////////////////////////////////////////////////////////////////////////
class IServiceStore : IOBJECT_PARENT
{
public: ///-- IObject
    DECLARE_ICLASS(IServiceStore,ISERVICESTORE_UUID);

public:
    IAPI_DECL getInfo( ServiceStoreInfo &info ) = 0;
    IAPI_DECL loadConfig( Config &config ) = 0;

//--
    IAPI_DECL listServiceSupport( ListOf<String> &supported ) = 0;
    IAPI_DECL registerServiceSupport( const char *name ,IServiceSetupRef &setup ) = 0;
    IAPI_DECL unregisterServiceSupport( const char *name ) = 0;

//--
    IAPI_DECL getServiceSetup( const char *name ,IServiceSetupRef &setup ) = 0;
};

//! NO formal ptr, stores are singletons

//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMEPSACE

//////////////////////////////////////////////////////////////////////////////
#endif //TINY_ISERVICE_H