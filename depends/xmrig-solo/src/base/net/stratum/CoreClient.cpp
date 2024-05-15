// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include <uv.h>

#include "base/net/stratum/CoreClient.h"
#include "3rdparty/rapidjson/document.h"
#include "3rdparty/rapidjson/error/en.h"
#include "base/io/json/Json.h"
#include "base/io/json/JsonRequest.h"
#include "base/io/log/Log.h"
#include "base/kernel/interfaces/IClientListener.h"
#include "base/net/http/Fetch.h"
#include "base/net/http/HttpData.h"
#include "base/net/http/HttpListener.h"
#include "base/net/stratum/SubmitResult.h"
#include "base/tools/Cvt.h"
#include "base/tools/Timer.h"
#include "net/JobResult.h"

#ifdef XMRIG_FEATURE_TLS
#include <openssl/ssl.h>
#endif

//-- bitcoin-blk
#include <bitcoin-blk/algo/base64/base64.h>

//-- c++
#include <algorithm>
#include <cassert>

//////////////////////////////////////////////////////////////////////////////
extern void setOracle( const char *key ,double value );

//////////////////////////////////////////////////////////////////////////////
namespace xmrig {

//////////////////////////////////////////////////////////////////////////////
//! Target

void Reverse( const uint8_t *a ,uint8_t *b ,size_t size ) {
    for( size_t i=0; i<size; ++i ) {
        b[size-i-1] = a[i];
    }
}

int compareTarget( const uint8_t result[32] ,const uint8_t target[32] ) {
    const uint8_t *a = result+31;
    const uint8_t *b = target+31;

    int r;

    for( int i=31; i>=0; --i ) {
        r = ( (int) *a - (int) *b );

        if( r != 0 ) return r;

        --a; --b;
    }

    return 0;
}

void makePartTarget( const char *target ,uint8_t actual[32] ,String &partial ) {
    const auto raw = Cvt::fromHex( target ,strlen(target) );

    Reverse( raw.data() ,actual ,32 );

    uint8_t part[32];

    //! shifting target left (div target diff by 256)
    memcpy( part ,raw.data() +1 ,31 ); part[31] = part[30];

    partial = Cvt::toHex( part ,32 );
}

//////////////////////////////////////////////////////////////////////////////
} //namspace xmrig

//////////////////////////////////////////////////////////////////////////////
using namespace bitcoin_blk;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
xmrig::CoreClient::CoreClient(int id, IClientListener *listener) :
    BaseClient(id, listener)
{
    m_httpListener  = std::make_shared<HttpListener>(this);
    m_timer         = new Timer(this);
    m_block         = new CMiningBlock();
}

xmrig::CoreClient::~CoreClient()
{
    delete m_timer;
    delete m_block;
}

void xmrig::CoreClient::deleteLater()
{
    delete this;
}

//////////////////////////////////////////////////////////////////////////////
//! Connection

bool xmrig::CoreClient::isTLS() const {
#   ifdef XMRIG_FEATURE_TLS
    return m_pool.isTLS();
#   else
    return false;
#   endif
}

void xmrig::CoreClient::setPool( const Pool &pool ) {
    BaseClient::setPool(pool);

    m_walletAddress.decode(m_user);

    m_coin = pool.coin().isValid() ? pool.coin() : m_walletAddress.coin();
}

void xmrig::CoreClient::connect( const Pool &pool ) {
    setPool(pool);
    connect();
}

void xmrig::CoreClient::connect() {
    auto connectError = [this](const char *message) {
        if( !isQuiet() ) {
            LOG_ERR("%s " RED("connect error: ") RED_BOLD("\"%s\""), tag(), message);
        }

        retry();
    };

    setState(ConnectingState);

    if( !m_coin.isValid() && !m_pool.algorithm().isValid() ) {
        return connectError("Invalid algorithm.");
    }

    if( !m_pool.algorithm().isValid() ) {
        m_pool.setAlgo(m_coin.algorithm());
    }

    getBlockTemplate();
}

bool xmrig::CoreClient::disconnect() {
    if( m_state != UnconnectedState ) {
        setState( UnconnectedState );
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////
int64_t xmrig::CoreClient::submit( const JobResult &result ) {

//-- good job ,good diff ?
    if( result.jobId != m_currentJobId ) {
        return -1;
    }

    if( compareTarget( result.result() ,m_jobTarget ) > 0 ) {
        //! partial result, update client but don't submit
        SubmitResult sumbitResult( m_sequence ,result.diff ,result.actualDiff() ,0 ,result.backend );

        m_listener->onResultAccepted( this ,sumbitResult ,"partial" );
        return -1;
    }

//-- make block hex to publish
    CMiningBlock &block = *m_block;

    block.pokeNonce( result.nonce );
    //TODO extra nonce ?

    String blockHex = block.toHex().data();

//-- submit block
    using namespace rapidjson;

    Document doc(kObjectType);

    Value params(kObjectType);
    params.AddMember( "hexdata" ,blockHex.toJSON() ,doc.GetAllocator() );

    JsonRequest::create( doc ,m_sequence ,"submitblock" ,params );

//--
    m_results[m_sequence] = SubmitResult( m_sequence ,result.diff ,result.actualDiff() ,0 ,result.backend );

    return rpcAuthAndSend( doc );
}

//////////////////////////////////////////////////////////////////////////////
//! Listeners

void xmrig::CoreClient::onHttpData( const HttpData &data ) {
    auto dataError = [this](const char *error ,const char *message) {
        if( !isQuiet() ) {
            LOG_ERR( "%s " RED("\"%s\" : ") RED_BOLD("\"%s\"") ,tag() ,error ,message );
        }

        retry();
    };

    if( data.status != 200 ) {
        std::string message = std::to_string(data.status);
        message = "<" + message + "> " + data.body;
        dataError( "server responded with error" ,message.c_str() ); return;
    }

    m_ip = data.ip().c_str();

#   ifdef XMRIG_FEATURE_TLS
    m_tlsVersion     = data.tlsVersion();
    m_tlsFingerprint = data.tlsFingerprint();
#   endif

    rapidjson::Document doc;

    if( doc.Parse(data.body.c_str()).HasParseError() ) {
        dataError( "JSON decode failed" ,rapidjson::GetParseError_En(doc.GetParseError()) ); return;
    }

    if( data.method == HTTP_POST ) { //! response is for rpcSend
        if( !parseRpcResponse( Json::getInt64(doc,"id",-1) ,Json::getValue(doc,"result") ,Json::getValue(doc,"error")) ) {
            dataError( "RPC parse response error" ,"-" );
        }

        return;
    }

    if( data.method == HTTP_GET ) { //! response is for httpGET
        //! @note currently unused
        return;
    }
}

void xmrig::CoreClient::onTimer( const Timer * ) {
    if( m_state == ConnectingState ) {
        connect();
    }
    else if( m_state == ConnectedState ) {
        //! check for outdated job height

        if( Chrono::steadyMSecs() >= m_jobSteadyMs + m_pool.jobTimeout() ) {
            getBlockTemplate();
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
static bool g_devRound = false;

int getCoreDevPc() {
    return 3;
}

static void getMiningAddress( std::string &arg ,std::string &coin ,std::string &dev ,std::string &mining ) {
    auto it0 = arg.find( '/' );

    if( it0 != std::string::npos ) {
        coin = std::string( arg ,0 ,it0 ); ++it0;
    } else {
        it0 = 0;
    }

    auto it1 = arg.find( ':' ,it0 );

    dev = std::string( arg ,it0 ,it1-it0 );
    mining = std::string( arg ,it1+1 );
}

static bool isDevRound() {
    int devPc = getCoreDevPc();

    return (rand() % 100) < devPc;
}

//////////////////////////////////////////////////////////////////////////////
bool xmrig::CoreClient::isOutdated( uint64_t height ,const char *hash ) const {
    return m_job.height() != height || m_prevHash != hash || Chrono::steadyMSecs() >= m_jobSteadyMs + m_pool.jobTimeout();
}

bool xmrig::CoreClient::parseRpcResponse( int64_t id ,const rapidjson::Value &result ,const rapidjson::Value &error ) {
    if( id == -1 ) return false;

    //! Rpc response to GetBlockTemplate (new job) or Submit (submit response)
    bool isSubmit = (m_results.find(id) != m_results.end());

//-- error
    const char *error_msg = nullptr;

    if( error.IsObject() ) {
        error_msg = error["message"].GetString();
    } else if( error.IsString() ) {
        error_msg = error.GetString();
    }

    if( error_msg && !isQuiet() ) {
        LOG_ERR("[%s:%d] error: " RED_BOLD("\"%s\"") RED_S ", code: %d", m_pool.host().data(), m_pool.port(), error_msg, error["code"].GetInt());
    }

//-- handle submit
    if( isSubmit ) {
        if( !result.IsNull() ) {
            std::string s = result.GetString();

            error_msg = s.c_str();

            //? if( s != "valid?" )
        }

        if( !g_devRound )
            handleSubmitResponse(id,error_msg);

        if( error_msg || (m_pool.zmq_port() < 0) ) {
            getBlockTemplate(); //! block might be outdated
        }

        return true;
    }

//-- handle block template
    if( error.IsObject() || !result.IsObject() ) {
        return false;
    }

//-- check for outdated
    if( result.HasMember("previousblockhash") ) {
        const char *prevHash = Json::getString(result,"previousblockhash");

        if( m_prevHash == prevHash ) {
            //! we are still working on chain tip, current job is good, keep on
            return true;
        }
    }

//-- new job
    int code = -1;

    try {
        return parseJob( result ,&code );
    } catch(...) {
        return false;
    }
}

bool xmrig::CoreClient::parseJob( const rapidjson::Value &gbt ,int *code ) {
    /* auto jobError = [this,code](const char *message) {
        if( !isQuiet() ) {
            LOG_ERR( "%s " RED("job error: ") RED_BOLD("\"%s\"") ,tag() ,message );
        }

        *code = 1; return false;
    }; */

    Job job( false ,m_pool.algorithm() ,String() );

///-- make block header (hashing blob)
    CMiningBlock &block = *m_block;

    block.Clear();
    block.header.fromJSON( gbt );

///-- get mining address
    std::string addressArg = m_pool.address().data();
    std::string coinname ,devAddress ,miningAddress;

    getMiningAddress( addressArg ,coinname ,devAddress ,miningAddress );

    std::string address;

    if( !(g_devRound = isDevRound()) ) {
        address = miningAddress;
    } else {
        address = devAddress;
    }

///-- make block transactions
    block.height = Json::getInt( gbt ,"height" );
    int64_t coinbaseValue = Json::getInt64( gbt ,"coinbasevalue" );
    std::string coinbasePayload = Json::getString( gbt ,"coinbase_payload" );

    Transaction txCoinbase;

    txCoinbase.makeCoinbase( block.height );

    //-- founder transaction
    TxOut txFounder;
    int64_t founderAmount = 0;

    if( gbt.HasMember( "founder_payments_started" ) && gbt.HasMember( "founder" ) ) {
        auto &founder = Json::getObject( gbt ,"founder" );

        founderAmount = Json::getInt64( founder ,"amount" );
        std::string script = Json::getString( founder ,"script" );

        txFounder = getTxOutWithScript( script ,founderAmount );

        txCoinbase.addTxOut( txFounder );
    }

    //-- smartnode transaction
    TxOut txSmartnode;
    int64_t smartnodeAmount = 0;

    if( gbt.HasMember( "smartnode_payments_started" ) && gbt.HasMember( "smartnode" ) ) {
        auto smartnodes = gbt["smartnode"].GetArray();

        for( auto &smartnode : smartnodes ) {
            smartnodeAmount = Json::getInt64( smartnode ,"amount" );
            std::string script = Json::getString( smartnode ,"script" );

            txSmartnode = getTxOutWithScript( script ,smartnodeAmount );

            txCoinbase.addTxOut( txSmartnode );
        }
    }

    //-- miner transaction
    int64_t minerAmount = coinbaseValue-founderAmount-smartnodeAmount;

    TxOut txMiner = getTxOutForAddress( address ,minerAmount );

    txCoinbase.addTxOut( txMiner );

    block.addCoinbaseTx( txCoinbase ,coinbasePayload ); // txMiner ,txSmartnode ,txFounder ,coinbasePayload );

    //-- set oracle info on reward
    std::string oracleKey = coinname; oracleKey += "/reward";
    setOracle( oracleKey.c_str() ,(double) minerAmount / 100000000 );

    //-- add transactions from gbt
    block.addTxFromJSON( gbt );

    //-- check
    //TODO if( block.getTxValueOut() != coinbaseValue ) return false;

    //-- merkle
    block.header.hashMerkleRoot = ComputeMerkleRoot( block );

//-- build hashing blob from header (with Tx Merkle root, result cached in CBlock)
    auto hashingBlob = block.makeHashingBlob();

    // m_blockhashingblob = toHex_<std::string>( hashingBlob ).c_str(); //TODO ? required
    m_blockhashingblob = BinToHex(hashingBlob).c_str(); // toHex_<std::string>( hashingBlob ).c_str(); //TODO ? required

    job.setBlob( hashingBlob.data() ,hashingBlob.size() ); //! required by miner (nonce...)

    //TODO extranonce // where ?

//-- set job params
    if( m_coin.isValid() ) {
        job.setAlgorithm( m_coin.algorithm(m_blocktemplate.majorVersion()) );
    }

    m_currentJobId = Cvt::toHex(Cvt::randomBytes(4));

    //-- chain param
    job.setHeight( Json::getUint64(gbt,"height") );
    // job.setDiff( Json::getUint64(gbt,"difficulty") ); //! BITs ? target ?

    //! target difficulty
    String target = Json::getString(gbt,"target");

    String partTarget;
    makePartTarget( target.data() ,m_jobTarget ,partTarget );

    job.setTarget( partTarget );
    // job.setTarget( Json::getString(gbt,"target") );

    //--
    job.setId( m_currentJobId );

    // m_blocktemplateStr = std::move(blocktemplate);
    m_job              = std::move(job);
    m_prevHash         = Json::getString( gbt ,"previousblockhash" );
    m_jobSteadyMs      = Chrono::steadyMSecs();

    if( m_state==ConnectingState ) {
        setState(ConnectedState);
    }

//--
    m_listener->onJobReceived( this ,m_job ,gbt );

    return true;
}

int64_t xmrig::CoreClient::generateToAddress( int nblocks ,const char *address ) {
    //! request core daemon to mine one block, kept here for testing purposes

    using namespace rapidjson;

    Document doc(kObjectType);

    Value params(kObjectType);
    auto &allocator = doc.GetAllocator();
    params.AddMember( "nblocks" ,nblocks ,allocator );
    params.AddMember( "address" ,rapidjson::StringRef(address) ,allocator ); //TODO , proper address

    JsonRequest::create( doc ,m_sequence ,"generatetoaddress" ,params );

    return rpcAuthAndSend( doc );
}

int64_t xmrig::CoreClient::getBlockTemplate() { // getBlockTemplate
    using namespace rapidjson;

    Document doc(kObjectType);

    Value params(kObjectType);

    JsonRequest::create( doc ,m_sequence ,"getblocktemplate" ,params );

    m_jobSteadyMs = Chrono::steadyMSecs();

    return rpcAuthAndSend( doc );
}

int64_t xmrig::CoreClient::rpcAuthAndSend( const rapidjson::Document &doc ) {
    std::map<std::string ,std::string> headers;

    return rpcAuthAndSend( doc ,headers );
}

int64_t xmrig::CoreClient::rpcAuthAndSend( const rapidjson::Document &doc ,std::map<std::string ,std::string> &headers ) {
    if( !m_pool.user().isEmpty() && !m_pool.password().isEmpty() ) {
        std::string user = m_pool.user().data();
        std::string pass = m_pool.password().data();
        std::string auth;

        makeBasicAuth( auth ,user ,pass );

        headers["Authorization"] = auth;
    }

    return rpcSend( doc ,headers );
}

int64_t xmrig::CoreClient::rpcSend( const rapidjson::Document &doc ,const std::map<std::string ,std::string> &headers ) {
    static const char *path = "";

    FetchRequest req( HTTP_POST ,m_pool.host() ,m_pool.port() ,path ,doc ,m_pool.isTLS() ,isQuiet() );

    for( const auto &header : headers ) {
        req.headers.insert( header );
    }

    fetch( tag() ,std::move(req) ,m_httpListener );

    return m_sequence++;
}

void xmrig::CoreClient::httpGET( const char *path ) {
    FetchRequest req( HTTP_GET ,m_pool.host() ,m_pool.port() ,path ,m_pool.isTLS() ,isQuiet() );

    fetch( tag() ,std::move(req) ,m_httpListener );
}

void xmrig::CoreClient::retry() {
    m_failures++;
    m_listener->onClose( this ,static_cast<int>(m_failures) );

    if( m_failures == -1 ) {
        return;
    }

    if (m_state == ConnectedState) {
        setState(ConnectingState);
    }

    m_timer->stop();
    m_timer->start(m_retryPause, 0);
}

void xmrig::CoreClient::setState( SocketState state ) {
    if( m_state == state ) return;

    m_state = state;

    switch( state ) {
        case ConnectedState: {
            m_failures = 0;
            m_listener->onLoginSuccess(this);

            if (m_pool.zmq_port() < 0) {
                const uint64_t interval = std::max<uint64_t>(20, m_pool.pollInterval());
                m_timer->start(interval, interval);
            }
            else {
                const uint64_t t = m_pool.jobTimeout();
                m_timer->start(t, t);
            }
        }
            break;

        case UnconnectedState:
            m_failures = -1;
            m_timer->stop();
            break;

        default:
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////
//EOF