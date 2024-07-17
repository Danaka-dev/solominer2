// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include "minerstat-chain.h"

#include <rapidjson/document.h>
#include <rapidjson/reader.h>
#include <rapidjson/stringbuffer.h>

#include <memory.h>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
static IServiceSetupRef g_setup( new CChainSetupMinerstat() );

static iresult_t g_hasMinerstat = getChainStore().registerServiceSupport( MINERSTAT_NAME ,g_setup );

//////////////////////////////////////////////////////////////////////////////
iresult_t CChainMinerstat::getInfo( const char *coin ,ChainInfo &info ,ChainInfoFlags flags ) {
    // https://api.minerstat.com/v2/coins?list=BTC,BCH,BSV

    if( coin[0]==0 || stricmp( coin ,"OWN" ) == 0 )
        return false;

    std::stringstream ss;
    ss << m_host << MINERSTAT_APIPATH << "/coins?list=" << coin;

    HttpMessage message;

    message.content_type = "";
    message.content = "";

    message.accept_type = "application/json";

    HttpResponse response;

// return IERROR; //! TEST
    CHttpRequest request;

    if( m_http.sendRequest( ss.str().c_str() ,HttpMethod::methodGET ,message ,request ,response ) != IOK || response.content.empty() )
        return IERROR;

    double reward1Hps1h = 0;
    String rewardCoin;

    try {
        rapidjson::Document jdoc;

        jdoc.Parse( tocstr(response.content) );

        if( jdoc.GetArray().Empty() )
            return INODATA;

        auto &v = jdoc[0];

        {
            info.networkDiff = (double) v["difficulty"].GetDouble();
            info.networkHPS = (double) v["network_hashrate"].GetDouble();
            info.blockReward = (double) v["reward_block"].GetDouble();
            info.price = (double) v["price"].GetDouble();

            reward1Hps1h = (double) v["reward"].GetDouble();
            rewardCoin = v["reward_unit"].GetString();
        }
    } catch(...) {
        return IERROR;
    }

    info.blockPerHour = info.networkHPS * reward1Hps1h / info.blockReward;

    /* if( info.networkDiff < 1 && rewardCoin == coin ) {
        //! TEST // not proper computation below //! TEST //

        info.networkDiff = (info.blockReward * 3600) / (reward1Hps1h * (2.*1024*1024*1024));

        double hostHps = 1;

        double blockPerHour = (hostHps*60*60) / (info.networkDiff * 2.*1024*1024*1024);

        double rewardPerHour = info.blockReward * blockPerHour;
        double _reward1Hph = info.blockReward * 3600 / (info.networkDiff * 4.*1024*1024*1024);

        info.networkDiff = (info.blockReward * 3600) / (reward1Hps1h * (2.*1024*1024*1024));
    } */

    m_http.adviseRequestValid( request );

    return IOK;
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF