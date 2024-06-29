// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include <coins/cores.h>
#include <wallets/wallets.h>
// #include <bitcoinapi/bitcoinapi.h>
#include <bitcoin-rpc.h>

#include "bitcoin-core.h"

//////////////////////////////////////////////////////////////////////////////
//TODO in tiny-os

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

const char *getUserHomeDirectory() {
    const char *homedir;

    if ((homedir = getenv("HOME")) == NullPtr) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    return homedir;
}

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//! Daemon service

template <class TCore ,typename TLambda>
iresult_t CallDaemon( TCore &core ,TLambda &&method ) {
    const String &username = core.config().credential.user;
    const String &password = core.config().credential.password;

    const String &url = core.config().connection.host;
    int port = core.config().connection.port;

    try {
        //! Constructor to connect daemon
        BitcoinRPC cored( username ,password ,url ,port ,1000 );

        //! bitcoin API method call
        method( core ,cored );
    }
    catch( BitcoinRpcException &e ) {
        std::cerr << e.getMessage() << std::endl;
        return IERROR;
    }

    return IOK;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Wallet

ref_t CWalletBitcoinBase::AddRef() { return m_core.AddRef(); }
ref_t CWalletBitcoinBase::Release() { return m_core.Release(); }

//--
IAPI_DEF CWalletBitcoinBase::Start( const Params &params ) {
    if( m_core.state() != serviceStarted )
        return m_core.Start(params);

    return CWalletService::Start( params );
}

IAPI_DEF CWalletBitcoinBase::Retry( const Params &params ) {
    return m_core.Retry(params);
}

IAPI_DEF CWalletBitcoinBase::Stop( const Params &params ) {
    if( m_core.state() == serviceConnected )
        return m_core.Stop(params);

    return CWalletService::Stop( params );
}

IAPI_DEF CWalletBitcoinBase::getCoinList( ListOf<String> &coins ) {
    String coin; getCoin(coin);

    coins.clear();
    coins.emplace_back( coin.c_str() );

    return IOK;
}

//////////////////////////////////////////////////////////////////////////////
//! manage

IAPI_DEF CWalletBitcoinBase::unlock( const char *password ) {
    const String passphrase = password;

    auto lambda = [&passphrase]( CCoreBitcoinBase &core ,BitcoinRPC &api ){
        api.walletpassphrase( passphrase ,1000 );
    };

    return CallDaemon( core() ,lambda );
}

IAPI_DEF CWalletBitcoinBase::lock() {
    auto lambda = []( CCoreBitcoinBase &core ,BitcoinRPC &api ){
        api.walletlock();
    };

    return CallDaemon( core() ,lambda );
}

//////////////////////////////////////////////////////////////////////////////
//! accounts

    ///...

//////////////////////////////////////////////////////////////////////////////
//! addresses

void setTransaction( const char *coin ,WalletTransaction &t ,const transactioninfo_t &tx ) {
    t.txid = tx.txid;
    t.amount.amount = tx.amount;
    t.amount.value = coin;
    t.fromAddress = (tx.category == "generate") ? "mined" : "";
    t.toAddress = tx.address;
    t.receivedAt = tx.timereceived;
    t.confirmations = tx.confirmations;
}

int getLastTransactionTime( std::vector<transactioninfo_t> &txs ) {
    int last = 0;

    for( auto &it : txs ) {
        if( last < it.timereceived )
            last = it.timereceived;
    }

    return last;
}

///--
IAPI_DEF CWalletBitcoinBase::listAddresses( const char *accountId ,ListOf<String> &addresses ) {
    IRESULT result = CallDaemon( core() ,[accountId,&addresses]( CCoreBitcoinBase &core ,BitcoinRPC &api ) {
        // addresses = api.getaddressesbyaccount( accountId ? accountId : "" ); //! @note deprecated

        auto grouping = api.listaddressgroupings();

        for( auto &it : grouping ) {
            for( auto &jt : it ) {
                addresses.emplace_back( jt.address );
            }
        }
    });

    return result;
}

IAPI_DEF CWalletBitcoinBase::getNewAddress( const char *accountId ,String &address ) {
    IRESULT result = CallDaemon( core() ,[accountId,&address]( CCoreBitcoinBase &core ,BitcoinRPC &api ) {
        address = api.getnewaddress( accountId ? accountId : "" );
    });

    return result;
}

IAPI_DEF CWalletBitcoinBase::getAddressBalance( const char *address ,AmountValue &balance ) {
    double amount;

    auto lambda = [&amount]( CCoreBitcoinBase &core ,BitcoinRPC &api ){
        amount = api.getbalance();
    };

    if( CallDaemon( core() ,lambda ) != IOK )
        return IERROR;

    String coin; getCoin(coin);

    balance = { amount ,coin };

///-- check for incoming transaction
    std::vector<transactioninfo_t> txs;

    IRESULT result = CallDaemon( core() ,[&txs]( CCoreBitcoinBase &core ,BitcoinRPC &api ){
        txs = api.listtransactions();
    });

    IF_IFAILED_RETURN(result);

    int lastTxTime = getLastTransactionTime( txs );

    if( m_initialTxTime == 0 ) { //! first balance check
        m_initialTxTime = lastTxTime;

    } else if( lastTxTime > m_initialTxTime ) { //! new transaction detected
         for( auto &it : txs ) {
            if( it.timereceived <= m_initialTxTime ) continue;

            WalletTransaction t;

            setTransaction( coin.c_str() ,t ,it );

            CWalletEventSource::PostTransaction( *this ,t );
        }

        m_initialTxTime = lastTxTime;
    }

    return IOK;
}

IAPI_DEF CWalletBitcoinBase::listTransactions( const char *address ,ListOf<WalletTransaction> &transactions ,int from ,int count ) {
    std::vector<transactioninfo_t> txs;

    //TOOD listtransaction only returns partial list
    IRESULT result = CallDaemon( core() ,[&txs]( CCoreBitcoinBase &core ,BitcoinRPC &api ){
        txs = api.listtransactions();
    });

    IF_IFAILED_RETURN(result);

    String coin; getCoin(coin);

    int i=0; for( auto &it : txs ) {
        if( it.address == address ) {
            if( i >= from && i < from+count ) {} else continue;

            WalletTransaction t;

            setTransaction( coin.c_str() ,t ,it );

            transactions.emplace_back( t );
        }

        ++i;
    }

    return IOK;
}

IAPI_DEF CWalletBitcoinBase::getTransaction( const char *address ,const char *transactionId ,WalletTransaction &transaction ) {
    const String txid = transactionId;
    gettransaction_t tx;

    iresult_t result = CallDaemon( core() ,[&txid,&tx]( CCoreBitcoinBase &core ,BitcoinRPC &api ){
        tx = api.gettransaction( txid ,false );
    });

    IF_IFAILED_RETURN(result);

    transaction.txid = tx.txid;
    transaction.amount.amount = tx.amount;
    getCoin( transaction.amount.value );

    const auto &it = tx.details.begin();

    if( it != tx.details.end() ) {
        transaction.toAddress = it->address;
    }

    return IOK;
}

IAPI_DEF CWalletBitcoinBase::sendToAddress( WalletTransaction &transaction ) {
    WalletTransaction &tx = transaction;

    String coin; getCoin(coin);

    if( tx.amount.value != coin || tx.fromAddress == tx.toAddress )
        return IBADARGS;

    iresult_t result;

    AmountValue balance;

    if( !transaction.fromAddress.empty() && transaction.fromAddress != "*" ) {
        //! verifying fromAddress belongs to this wallet, and balance is sufficient

        //! @note no guarantee that fromAddress will be used though
        result = getAddressBalance( transaction.fromAddress.c_str() ,balance ); IF_IFAILED_RETURN(result);

        if( balance.value < tx.amount.value )
            return IREFUSED;
    }

    //-- unlock
    if( !m_unlocked ) {
        String walletPassword = getMember( core().params() ,"wallet-password" );

        if( !walletPassword.empty() ) {
            m_unlocked = (unlock( walletPassword.c_str() ) == IOK);
        }
    }

    //-- send
    String txid;

    auto lambda = [&txid,&tx]( CCoreBitcoinBase &core ,BitcoinRPC &api ){
        txid = api.sendtoaddress( tx.toAddress ,tx.amount.amount ,tx.comment ,tx.communication );
    };

    result = CallDaemon( core() ,lambda ); IF_IFAILED_RETURN(result);

    transaction.txid = txid;

    return IOK;
}

//////////////////////////////////////////////////////////////////////////////
///-- CWalletBitcoinBase

IAPI_DEF CWalletBitcoinBase::getCoin( String &coin ) {
    return m_core.getCoin(coin);
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Chain

const char *getDaemonFilepath( const Params &params ,String &filepath ) {
    StringStream ss;

    const char *location = getMember( params ,"location" ,"./" );
    const char *filename = getMember( params ,"filename" ,"" );

    if( !location || !filename || !filename[0] )
        return NullPtr;

    //-- translate location as required
    String path;

    const char *home = getUserHomeDirectory();
    //TODO replaceStringParams( location ,params ,path );
    path = location;

    //--
    ss << path << filename;

#ifdef PLATFORM_WINDOWS
    ss << ".exe";
#endif

    filepath = ss.str();

    return filepath.c_str();
}

String &makeDaemonArgSwitch( String &s ,const char *arg ,const char *value=NullPtr ) {
    s = "-"; s += arg; if( value ) s = s + '=' + value; return s;
}

void makeDaemonArguments( const Params &params ,ListOf<String> &argv ) {

    //-- get fields
    String filename; getDaemonFilepath(params,filename);
    const char *datadir = getMember( params ,"datapath" ,"./data/" );
    const char *rpcport = getMember( params ,"port" ,"15075" ); //TODO default port for each core
    const char *rpcuser = getMember( params ,"user" ,"" );
    const char *rpcpass = getMember( params ,"password" ,"" );
    const char *args = getMember( params ,"arguments" ,"" );

    //-- add fields
    String s;

    argv.emplace_back( filename );
    argv.emplace_back( makeDaemonArgSwitch(s,"datadir",datadir) );
    argv.emplace_back( makeDaemonArgSwitch(s,"rpcport",rpcport) );
    argv.emplace_back( makeDaemonArgSwitch(s,"rpcuser",rpcuser) );
    argv.emplace_back( makeDaemonArgSwitch(s,"rpcpassword",rpcpass) );

    //-- make args
    StringStream ss(args);

    while( std::getline(ss,s,' ') ) {
        argv.emplace_back( s.c_str() );
    }

    //-- done
    return;
}

///--
IAPI_DEF CCoreBitcoinBase::ConnectDaemon( const char *name ,const Params &params ) {
    Json::Value jin ,result;

    CallDaemon( *this ,[jin ,&result]( CCoreBitcoinBase &core ,BitcoinRPC &api ) {
        api.sendcommand( "getpeerinfo" ,jin ,result );
    } );

    bool hasResult = !result.empty();

    return hasResult ? IOK : IERROR;
}

#define MAX_ARGS 64

IAPI_DEF CCoreBitcoinBase::StartDaemon( const char *name ,const Params &params ) {
    //! requiring forced manual entry to start a daemon core
    const char *start = getMember( params ,"daemon" ,"" ) ;

    if( stricmp(start,"start") != 0 )
        return IREFUSED;

    //! don't restart if process is still alive
    if( m_hprocess != OS_INVALID_HANDLE )
        return IALREADY;

    ///-- new
    this->info().category = "core";
    this->info().name = name;
    this->info().taxonomy = "*"; // getMember( params ,"netname" ,"mainnet,qt" );
    this->info().version = getMember( params ,"version" ,"?" );

    ListOf<String> args;

    makeDaemonArguments( params ,args ); if( args.size() > MAX_ARGS ) return IBADARGS;

    //TODO proper check of args from file
    const char *argv[MAX_ARGS]; // = (const char**) malloc( args.size() * sizeof(char*) );
    memset( argv ,0 ,MAX_ARGS*sizeof(char*) );

    for( int i=0; i<args.size(); ++i ) {
        argv[i] = args[i].c_str();
    }

    if( OsProcessRun( &m_hprocess ,argv[0] ,argv ,NullPtr ) != ENOERROR )
        return IERROR;

    return IOK;
}

IAPI_DEF CCoreBitcoinBase::StopDaemon( const Params &params ) {
    //! require forced manual entry to stop a daemon core
    const char *stop = getMember( params ,"daemon" ,"" ) ;

    if( stricmp(stop,"stop") != 0 )
        return IREFUSED;

    ///-- stopping
    auto lambda = []( CCoreBitcoinBase &core ,BitcoinRPC &api ){
        api.stop();
    };

    iresult_t result = CallDaemon( *this ,lambda ); IF_IFAILED_RETURN(result);

    OsHandleDestroy( &m_hprocess ); //TODO, or get process info instead to decide if we need to kill

    return IOK;
}

///-- helpers
IAPI_DEF CCoreBitcoinBase::ConnectAndStartDaemon( const char *name ,const Params &params ) {
    iresult_t result = ConnectDaemon( name ,params );

    if( result != IOK ) {
        if( state()==serviceConnecting && m_startTime + m_startTimeout > time(NULL) )
            return IPROGRESS;

        //! start daemon
        m_startTime = time(NULL);

        ///? //! @note ignoring daemon start result, service is trying to connect either way
        result = StartDaemon( name ,params ); IF_IFAILED_RETURN(result);

        setState(serviceConnecting);

        //! retry
        result = ConnectDaemon( name ,params ); IF_IFAILED_RETURN(result);
    }

    setState(serviceConnected);

    return IOK;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Core

//...

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF