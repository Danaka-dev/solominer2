// Copyright (c) 2019      Howard Chu  <https://github.com/hyc>
// Copyright (c) 2018-2021 SChernykh   <https://github.com/SChernykh>
// Copyright (c) 2016-2021 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
// Copyright (c) 2023-2024 Solominer   <https://github.com/Danaka-dev/solominer2>
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef XMRIG_CORECLIENT_H
#define XMRIG_CORECLIENT_H

//////////////////////////////////////////////////////////////////////////////
#include "base/kernel/interfaces/IDnsListener.h"
#include "base/kernel/interfaces/IHttpListener.h"
#include "base/kernel/interfaces/ITimerListener.h"
#include "base/net/stratum/BaseClient.h"
#include "base/net/tools/Storage.h"
#include "base/tools/cryptonote/BlockTemplate.h"
#include "base/tools/cryptonote/WalletAddress.h"

#include <bitcoin-blk/bitcoin-blk.h>

#include <memory>

//////////////////////////////////////////////////////////////////////////////
using uv_buf_t      = struct uv_buf_t;
using uv_connect_t  = struct uv_connect_s;
using uv_handle_t   = struct uv_handle_s;
using uv_stream_t   = struct uv_stream_s;
using uv_tcp_t      = struct uv_tcp_s;

#ifdef XMRIG_FEATURE_TLS
using BIO           = struct bio_st;
using SSL           = struct ssl_st;
using SSL_CTX       = struct ssl_ctx_st;
#endif

//////////////////////////////////////////////////////////////////////////////
namespace xmrig {

//////////////////////////////////////////////////////////////////////////////
class DnsRequest;
// class CBlock;

class CoreClient : public BaseClient
    /*,public IDnsListener*/ ,public IHttpListener ,public ITimerListener
{
public:
    XMRIG_DISABLE_COPY_MOVE_DEFAULT(CoreClient)

    CoreClient(int id, IClientListener *listener);
    ~CoreClient() override;

protected:
    bool isTLS() const override;

    void setPool(const Pool &pool) override;
    void connect(const Pool &pool) override;

    void connect() override;
    bool disconnect() override;

    int64_t submit( const JobResult &result ) override;

    inline bool hasExtension(Extension) const noexcept override         { return false; }
    inline const char *mode() const override                            { return "daemon"; }
    inline const char *tlsFingerprint() const override                  { return m_tlsFingerprint; }
    inline const char *tlsVersion() const override                      { return m_tlsVersion; }
    inline int64_t send(const rapidjson::Value &, Callback) override    { return -1; }
    inline int64_t send(const rapidjson::Value &) override              { return -1; }
    inline void tick(uint64_t) override                                 {}
    void deleteLater() override;

    // void onResolved(const DnsRecords &records, int status, const char* error) override;
    void onHttpData(const HttpData &data) override;
    void onTimer(const Timer *timer) override;

private:
    bool isOutdated(uint64_t height, const char *hash) const;

    bool parseRpcResponse(int64_t id, const rapidjson::Value &result, const rapidjson::Value &error);
    bool parseJob(const rapidjson::Value &params, int *code);

    int64_t generateToAddress( int nblocks ,const char *address );
    int64_t getBlockTemplate();

    int64_t rpcAuthAndSend( const rapidjson::Document &doc );
    int64_t rpcAuthAndSend( const rapidjson::Document &doc ,std::map<std::string ,std::string> &headers );
    int64_t rpcSend( const rapidjson::Document &doc ,const std::map<std::string ,std::string> &headers={} );
    void httpGET( const char *path );

    void retry();
    void setState(SocketState state);

private:
    bitcoin_blk::CMiningBlock *m_block;

    /* enum {
        API_CRYPTONOTE_DEFAULT
        ,API_MONERO

    } m_apiVersion = API_CRYPTONOTE_DEFAULT; */
    // m_apiVersion = API_MONERO;

    BlockTemplate m_blocktemplate;
    Coin m_coin;

    std::shared_ptr<IHttpListener> m_httpListener;

    String m_blockhashingblob;
    String m_blocktemplateRequestHash;
    String m_blocktemplateStr;
    String m_currentJobId;
    String m_prevHash;
    uint64_t m_jobSteadyMs = 0;
    String m_tlsFingerprint;
    String m_tlsVersion;
    Timer *m_timer;
    uint64_t m_blocktemplateRequestHeight = 0;
    WalletAddress m_walletAddress;

    uint8_t m_jobTarget[32]; //! actual target for this job
};

//////////////////////////////////////////////////////////////////////////////
} //namespace xmrig

//////////////////////////////////////////////////////////////////////////////
#endif // XMRIG_CORECLIENT_H