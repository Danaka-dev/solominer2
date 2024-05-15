Bitcoin BLK
===========

A minimalist C++ header to build bitcoin and bitcoin like blocks for mining and other purposes.

Using bitcoin-blk
-----------------

[![badge](https://img.shields.io/badge/Language-C++-green)]()
[![badge](https://img.shields.io/badge/release-v1.0.0-blue)]()
[![badge](https://img.shields.io/badge/License-MIT-blue)](https://github.com/Danaka-dev/bitcoin-rpc/blob/master/LICENSE.md)

**bitcoin-blk** consist of one main header <bitcoin-blk.h> and a few header and source files related to sha256, base58 and json operation.

The code is designed to be simply added in your project as your own header and source files.

**Dependencies**

Out of the box **bitcoin-blk** uses [rapidjson](https://github.com/Tencent/rapidjson) for json operation which should
be available from include paths. If you wish to use another json library, file <json.h> in **bitcoin-blk** root may
be simply adapted as required.

Using the code
--------------
Include <bitcoin-blk.h> and link with .cpp files from directory "algo/*" that you may required.

"sample.cpp" provides a base example of creating a hashing blob from a getBlockTemplate json result retrieved from a daemon:  

```
void parseGetBlockTemplate( const rapidjson::Value &getBlockTemplate ,MiningJob &job ) {
    //! getBlockTemplate received from RPC getBlockTemplate daemon

    const rapidjson::Value &gbt = getBlockTemplate; //! shorthand alias

    //-- make block header (hashing blob)
    CMiningBlock &block = job.block;

    block.Clear();
    block.header.fromJSON( getBlockTemplate ); //! getting block header info from json

    //-- add block transactions
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

    //-- finalize block
    block.header.hashMerkleRoot = ComputeMerkleRoot( block );

    job.hashingBlob = block.makeHashingBlob();
    job.height = json::getUint64(gbt,"height");
    job.target = json::getString(gbt,"target");

//-- done
}
```

License
-------

The bitcoin-blk library is released under the terms of [MIT](http://en.wikipedia.org/wiki/MIT_License).

```
Copyright (c) 2024 Danaka developers

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in the 
Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of 
the Software, and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE 
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE 
OR OTHER DEALINGS IN THE SOFTWARE.
```

Acknowledgements
----------------
**bitcoin-blk** was developed for [solominer](https://github.com/Danaka-dev/solominer2) by Danaka and the Solominer developers.    