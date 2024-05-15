//! Copyright (c) 2023-2024 Danaka developers
//! Distributed under the MIT software license, see the accompanying
//! file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include "bitcoin-blk.h"

//////////////////////////////////////////////////////////////////////////////
using namespace bitcoin_blk;

//////////////////////////////////////////////////////////////////////////////
//! example

struct MiningJob {
    CMiningBlock block; //! mining block

    ByteVector hashingBlob; //! hash blob to solve
    //! @note nonce at HEADER_NONCE_OFFSET in hashing blob

    uint64_t height; //! blockchain current block height
    std::string target; //! target difficulty to achieve
};

void parseGetBlockTemplate( const rapidjson::Value &getBlockTemplate ,MiningJob &job ) {
    //! getBlockTemplate received from RPC getBlockTemplate daemon

    const rapidjson::Value &gbt = getBlockTemplate; //! shorthand alias

///-- make block header (hashing blob)
    CMiningBlock &block = job.block;

    block.Clear();
    block.header.fromJSON( getBlockTemplate ); //! getting block header info from json

///-- add block transactions
    block.height = json::getInt( gbt ,"height" );
    int64_t coinbaseValue = json::getInt64( gbt ,"coinbasevalue" );
    std::string coinbasePayload = json::getString( gbt ,"coinbase_payload" );

    Transaction txCoinbase;

    txCoinbase.makeCoinbase( block.height );

    //-- founder transaction (for bitcoin like blockchain requiring this)
    TxOut txFounder;
    int64_t founderAmount = 0;

    if( gbt.HasMember( "founder_payments_started" ) && gbt.HasMember( "founder" ) ) {
        auto &founder = json::getObject( gbt ,"founder" );

        founderAmount = json::getInt64( founder ,"amount" );
        std::string script = json::getString( founder ,"script" );

        txFounder = getTxOutWithScript( script ,founderAmount );

        txCoinbase.addTxOut( txFounder );
    }

    //-- smartnode transaction (for bitcoin like blockchain requiring this)
    TxOut txSmartnode;
    int64_t smartnodeAmount = 0;

    if( gbt.HasMember( "smartnode_payments_started" ) && gbt.HasMember( "smartnode" ) ) {
        auto smartnodes = gbt["smartnode"].GetArray();

        for( auto &smartnode : smartnodes ) {
            smartnodeAmount = json::getInt64( smartnode ,"amount" );
            std::string script = json::getString( smartnode ,"script" );

            txSmartnode = getTxOutWithScript( script ,smartnodeAmount );

            txCoinbase.addTxOut( txSmartnode );
        }
    }

    //-- miner transaction
    std::string miningAddress = "RRsoc2xrJgDFiWYGimKcY7AUqy9ghizMea"; //! example address

    int64_t minerAmount = coinbaseValue-founderAmount-smartnodeAmount;

    TxOut txMiner = getTxOutForAddress( miningAddress ,minerAmount );

    txCoinbase.addTxOut( txMiner );

    block.addCoinbaseTx( txCoinbase ,coinbasePayload );

    //-- add transactions from gbt
    block.addTxFromJSON( gbt );

///-- finalize block
    block.header.hashMerkleRoot = ComputeMerkleRoot( block );

    job.hashingBlob = block.makeHashingBlob();
    job.height = json::getUint64(gbt,"height");
    job.target = json::getString(gbt,"target");

//-- done
}

//////////////////////////////////////////////////////////////////////////////
//EOF