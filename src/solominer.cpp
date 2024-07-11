// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include <common/console.h>
#include <common/logging.h>
#include <common/option.h>

#include <algo/crypth.h>
#include <markets/markets.h>
#include <markets/trader.h>
#include <markets/broker.h>
#include <wallets/wallets.h>
#include <pools/pools.h>
#include <coins/cores.h>
#include <gui/gui.h>

//! plog
#include <plog/Init.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Initializers/RollingFileInitializer.h>

//! clipp
#include <clipp/include/clipp.h>

//! c++
#include <iostream>
#include <fstream>

//////////////////////////////////////////////////////////////////////////////
//! PLOG application logging

namespace plog {

Record& operator<<( Record& record ,const LogCategory &category ) {
    return record << categoryToString(category) << " ";
}

class MinerConsoleFormatter {
public:
    //!  returns a header for a new file. In our case it is empty.
    static util::nstring header() {
        return util::nstring();
    }

    //!  returns a string from a record.
    static util::nstring format(const Record& record) {
        const util::nchar *msg = record.getMessage();

        util::nostringstream ss;

        tm t; util::localtime_s(&t, &record.getTime().time);

        // ss << t.tm_year + 1900 << "-" << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_mon + 1 << PLOG_NSTR("-") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_mday << PLOG_NSTR(" ");
        ss << PLOG_NSTR("[");
        ss << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_hour << PLOG_NSTR(":") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_min << PLOG_NSTR(":") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_sec << PLOG_NSTR(".") << std::setfill(PLOG_NSTR('0')) << std::setw(3) << static_cast<int> (record.getTime().millitm);
        ss << PLOG_NSTR("] ");

        if( record.getSeverity() != Severity::info ) {
            ss << std::setfill(PLOG_NSTR(' ')) << std::setw(5) << std::left << severityToString(record.getSeverity()) << PLOG_NSTR(" ");
        }

        // ss << PLOG_NSTR("[") << record.getTid() << PLOG_NSTR("] ");
        // ss << PLOG_NSTR("[") << record.getFunc() << PLOG_NSTR("@") << record.getLine() << PLOG_NSTR("] ");

        switch( categoryFromString(&msg) ) {
            case LogCategory::config:
                ss << CCOLOR_CYAN_BG << " CONFIG " << CCOLOR_NORMAL;
                break;
            case LogCategory::net:
                ss << CCOLOR_BLUE_BG << "  NET   " << CCOLOR_NORMAL;
                break;
            case LogCategory::PoW:
                ss << CCOLOR_MAGENTA_BG << "  POW   " << CCOLOR_NORMAL;
                break;
        }

        ss << msg << PLOG_NSTR("\n");

        return ss.str();
    }
};

} //! namespace plog

//////////////////////////////////////////////////////////////////////////////
//! CLIPP argument parsing

std::string g_progCall;

bool processArguments( int argc ,char *argv[] ) {
    using namespace clipp;

    plog::Severity g_optLogSeverity;

//-- arguments definition
    auto cli = (
            value( "" ,g_progCall )
                    //+ topology
                    ,(option( "-t" ,"--threads" ) & value( "threads" ,g_optThreads )) % "select number of threads, 0=auto"
                    ,(option( "-l" ,"--logfile" ) & value( "logfile" ,g_optLogFile )) % "log file name, default = 'solominer.log'"
                    ,(option( "-c" ,"--config" ) & value( "configFile" ,g_optConfigFile )) % "configuration file name, default = 'solominer.conf'"
            // ,( option("--log") & value("log", g_optLogSeverity )) % "log severity (verbose,debug...)"
    );

//-- parse arguments (including argv[0] => progCall )
    if( !parse( argv ,argv + argc ,cli )) {
        std::cout << "Program usage : \n" << usage_lines( cli ,argv[0] ) << '\n';

        return false;
    }

    std::cout << getOptThreads() << "\n";

    return true;
}

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//! Config

bool loadConfigFile( Config &config ,const char *filename ) {
    if( !config.LoadFile( filename ) || !config.file().isOpen() ) {
        LOG_ERROR << LogCategory::config << "Configuration file '" << filename << "' not found";
        return false;
    }

    return true;
}

//-- solominer.conf
static Config g_config;

Config &getConfig() {
    return g_config;
}

//-- global
Config::Section &getGlobalConfig() {
    return getConfig().getSection( "global" );
}

//-- credential config
static Config g_credentialConfig;

Config &getCredentialConfig() {
    return g_credentialConfig;
}

//-- service config
static Config g_serviceConfig;

Config &getServiceConfig() {
    return g_serviceConfig;
}

//-- pools config
static Config g_poolsConfig;

Config &getPoolsConfig() {
    return g_poolsConfig;
}

//////////////////////////////////////////////////////////////////////////////
//! Login

const String &getGlobalLogin() {
    static String login = getCredentialConfig().getSection("GLOBAL").params["password"];

    return login;
}

bool testGlobalPassword( const char *password ) {
    const String &s = getGlobalLogin();
    String r;

    DecryptH( password ,tocstr(s) ,r );

    return r == password;
}

//bool setGlobalPassword( const char *password ) {}

//--
static String g_password;

static bool g_logged = false;

bool globalLogin( const char *password ) {
    if( !testGlobalPassword( password ) ) return false;

    g_password = password;

    return g_logged = true;
}

bool globalCypher( const char *text ,String &s ) {
    return CryptH( tocstr(g_password) ,text ,s );
}

bool globalDecypher( const char *cypher ,String &s ) {
    return DecryptH( tocstr(g_password) ,cypher ,s );
}

bool globalIsLogged() {
    return g_logged;
}

//////////////////////////////////////////////////////////////////////////////
//! Pools

static CPoolList g_pools;

CPoolList &getPoolListInstance() {
    return g_pools;
}

bool initPools( const char *filename ) {
    Config config(filename);

    if( !config.file().isOpen() ) {
        LOG_ERROR << LogCategory::config << "Configuration file '" << filename << "' not found";
        return false;
    }

    g_pools.loadConfig( config );

    return true;
}

//////////////////////////////////////////////////////////////////////////////
//! Connections

CConnectionList g_connections;

CConnectionList &getConnectionList() {
    return g_connections;
}

///-- global config
bool initConnections( Config &config ) {
    return g_connections.loadConfig( config ) == IOK;
}

void cleanupConnections() {
    g_connections.saveConfig();
}

//////////////////////////////////////////////////////////////////////////////
//! tests

#ifdef TEST_SERVICES

void testServices() {

///-- xeggex api
    // test::xeggex::testBalances();
    // test::xeggex::testGetDepositAddress("RTM");
    // test::xeggex::testGetDeposits("RTM");
    // test::xeggex::testGetWithdrawals("RTM");
    // test::xeggex::testGetTrades( "RTM/USDT" ,NullPtr );

    //! POST
    nextwave::xeggex::OrderToCreate order = {
        "202401-000104" ,"RTM/USDT" ,"sell" ,"limit" ,"10" ,"0.001470" ,false
    };

    // test::xeggex::testCreateOrder( order );

    // test::xeggex::testGetOrders( "RTM/USDT" ,"active" );

    // test::xeggex::testCancelOrder( "65f73bd212e84e287abf46f3" ); // 202401-000101
    // test::xeggex::testCancelOrder( "202401-000102" );
    // test::xeggex::testCancelAllOrders( "RTM/USDT" ,"sell" );

    nextwave::xeggex::WithdrawalToCreate withdrawal = {
        "RTM" ,"100" ,"RRsoc2xrJgDFiWYGimKcY7AUqy9ghizMea" ,"202402-000101"
    };

    // test::xeggex::testCreateWithdrawal( withdrawal );
}

#endif // TEST_SERVICES

//////////////////////////////////////////////////////////////////////////////
//! services

//-- well known stores
CServiceStore *getStore( const char *category ) {
    if( stricmp(category,"core") ==0 ) {
        return &getStore_<CCoreStore>();
    }
    else if( stricmp(category,"chain") ==0 ) {
        return &getStore_<CChainStore>();
    }
    else if( stricmp(category,"market") ==0 ) {
        return &getStore_<CMarketStore>();
    }
    else if( stricmp(category,"wallet") ==0 ) {
        return &getStore_<CWalletStore>();
    }

    return NullPtr;
}

Params &getCredential( Config::Section &section ,const char *name ,Params &params ) {
    const char *credit = getMember( section.params ,name ,"" );

    params.clear();

    if( *credit )
        fromString( params ,credit );

    return params;
}

//-- service
bool initServices( Config &config ) {

#ifdef TEST_SERVICES
    // testServices();
#endif

    auto &credit = getCredentialConfig();

    Params params;

///-- chains
    StartChain( "minerstat" );

///-- cores
    if( getCoreStore().loadConfig( config ) == INOEXIST ) {
        LOG_ERROR << LogCategory::config << "Service section [" << "CORES" << "] not found";
        return false;
    }

    auto &coreCredit = credit.getSection("cores");

    StartCore( "maxe-core" ,&getCredential(coreCredit,"maxe-core",params) );
    StartCore( "rtc-core" ,&getCredential(coreCredit,"rtc-core",params) );
    StartCore( "rtm-core" ,&getCredential(coreCredit,"rtm-core",params) );

///-- markets
    // getMarketStore().loadConfig( config ); //TODO later

    auto &marketCredit = credit.getSection("markets");

    StartMarket( "xeggex" ,&getCredential(marketCredit,"xeggex",params) );

///-- done
    return true;
}

//////////////////////////////////////////////////////////////////////////////
//-- trading

bool initTrading( Config &config ) {
    bool result = true;

    initWalletAddresses();

    result &= ISUCCESS( getTrader().Start( config ) );
    result &= ISUCCESS( getBroker().Start( config ) );

    return result;
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! rpc interface ?

    //...

//////////////////////////////////////////////////////////////////////////////
//! http interface ?

    //...

//////////////////////////////////////////////////////////////////////////////
//! terminal interface

void onConnectionsListCommand() {
    //! from ui
        // start, stop ...

    //! from console

    //! forward to connection list
}

void onConnectionCommand() {
    // start ,stop ...

    //! create wallet ...
}

void onParamUpdate() {

}

void onStatsUpdate() {
    //! from timer

    //! report to console + update ui
}

///--
void terminalHandle() {
    //.. dispatch here above
}

//////////////////////////////////////////////////////////////////////////////
//! main loop

void mainAppLoop() {
    using namespace solominer;

    CTrader &trader = CTrader::getInstance();
    CBroker &broker = CBroker::getInstance();

//! tiny C loop
    time_t t0 = Now();

    while( OsSystemDoEvents() == ENOERROR ) {
        time_t now = Now();

        //TODO -> if option set, have a terminal command interface ?

        getConnectionList().updateConnections();

        if( now - t0 > 1 ) {
            trader.processTrades();
            broker.processOrders();
        }

        OsSleep(10);
    }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Main

// #include "test-graph.hpp"

#define ERROR_OK            0
#define ERROR_ARGS          -1
#define ERROR_CONFIG        -2

/**
 * @brief
 * @param argc
 * @param argv
 * @return 0 on success, negative value on error
 */
int main( int argc ,char *argv[] ) {
    using namespace solominer;

    //////////////////////////////////////////////////////////////////////////////
    //! Basics

    OsTimerGetResolution();

    //////////////////////////////////////////////////////////////////////////////
    //! TEST

    /* OsTimerGetResolution();

    // algo::test_graph();

    while( OsSystemDoEvents() == ENOERROR ) {
        OsSleep(10);
    }

    return 0; */

    //////////////////////////////////////////////////////////////////////////////
    //! prepare

///-- prepare arguments
    if( !processArguments( argc ,argv ) ) {
        return ERROR_ARGS;
    }

///-- prepare logging
    static plog::ColorConsoleAppender<plog::MinerConsoleFormatter> consoleAppender;

    plog::init(plog::Severity::debug, getOptLogFile().c_str() ); //! Initialize file logger
    plog::init(plog::Severity::verbose, &consoleAppender); //! initialize console logger

    PLOGI << LogCategory::net << "Starting network interface";
    PLOGI << LogCategory::PoW << "Starting miner threads";

    //////////////////////////////////////////////////////////////////////////////
    //! config

    Config &config = getConfig();

    //! @note keeping on even if config was not found
    loadConfigFile( config ,tocstr(getOptConfigFile()) );

    auto &globalSection = getConfig().getSection("config");

    const char *serviceConf = getMember( globalSection.params ,"service" ,"./credit.conf" );
    const char *credentialConf = getMember( globalSection.params ,"credential" ,"./service.conf" );
    const char *poolsConf = getMember( globalSection.params ,"pools" ,"./pools.conf" );

    //////////////////////////////////////////////////////////////////////////////
    //! credential

    loadConfigFile( getCredentialConfig() ,credentialConf );

    //////////////////////////////////////////////////////////////////////////////
    //! services

    loadConfigFile( getServiceConfig() ,serviceConf );

    PLOGI << LogCategory::config << "Starting services";

    if( !initServices( getServiceConfig() ) ) {
        PLOG_ERROR << LogCategory::config << "Error connecting services";
    }

    //////////////////////////////////////////////////////////////////////////////
    //! connections

    PLOGI << LogCategory::config << "Starting connections";

    //TODO return error if not continue on config error flag set

///-- load pools config
    if( !initPools( "pools.conf" ) ) {
        PLOG_ERROR << LogCategory::config << "Error loading pools configuration file";
    }

///-- load global config (parameters ,connection list ,...)
    if( !initConnections( getConfig() ) ) {
        PLOG_ERROR  << LogCategory::config << "Error loading configuration file";
    }

    //////////////////////////////////////////////////////////////////////////////
    //! Trading

    initTrading( config );

    //////////////////////////////////////////////////////////////////////////////
    //! gui

    PLOGI << LogCategory::config << "Initialize user interface";

    //TODO if not no_gui option (if so, care for login from ui)

    if( !uiInitialize( getConnectionList() ) ) {
        PLOG_ERROR << LogCategory::config << "Error initializing UI";
    }

    //////////////////////////////////////////////////////////////////////////////
    ///-- main loop

    PLOGI << LogCategory::config << "Running";

    PLOGI << LogCategory::net << "Starting network interface";
    PLOGI << LogCategory::PoW << "Starting miner threads";

    OsSystemSetGlobalTimer(1000);

    mainAppLoop();

    //////////////////////////////////////////////////////////////////////////////
    ///-- done

    PLOGD << "done, cleaning up";

    cleanupConnections();

    return ERROR_OK;
}

//////////////////////////////////////////////////////////////////////////////
//EOF