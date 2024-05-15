//! Copyright (c) 2023-2024 Danaka developers
//! Distributed under the MIT software license, see the accompanying
//! file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include "algo/base58/base58.h"
#include "algo/sha/sha256.h"

#include "json.h"

#include <cstring>
#include <cassert>

//////////////////////////////////////////////////////////////////////////////
namespace bitcoin_blk {

//////////////////////////////////////////////////////////////////////////////
//! Definition

typedef uint8_t byte;

typedef std::vector<byte> ByteVector; //! binary
typedef std::vector<char> CharVector; //! hex

///-- type info
template <typename T>
struct type_info_ {
    static const bool is_vector = false;
};

template <> struct type_info_<std::string> { static const bool is_vector = true; };
template <> struct type_info_<ByteVector> { static const bool is_vector = true; };
template <> struct type_info_<CharVector> { static const bool is_vector = true; };

//////////////////////////////////////////////////////////////////////////////
//! Hex/Bin Convert

//--
template<typename T>
std::string &BinToHex( std::string &s ,const T itbegin ,const T itend ,bool fSpaces=false ) {
    //! @note s will not be clear, hex being append to s

    static const char hexmap[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

    s.reserve( (itend-itbegin) * (fSpaces ? 3 : 2) );

    for( T it=itbegin; it<itend; ++it ) {
        byte v = (byte)(*it);

        if( fSpaces && it != itbegin )
            s.push_back(' ');

        s.push_back(hexmap[v>>4]);
        s.push_back(hexmap[v&15]);
    }

    return s;
}

inline std::string &BinToHex( std::string &s ,const ByteVector &bin ) {
    return BinToHex( s ,bin.begin() ,bin.end() );
}

template<typename T>
std::string BinToHex( const T itbegin ,const T itend ,bool fSpaces=false ) {
    std::string s; BinToHex( s ,itbegin ,itend ,fSpaces ); return s;
}

inline std::string BinToHex( const ByteVector &bin ) {
    return BinToHex( bin.begin() ,bin.end() );
}

//--
static const signed char g_hexDigits[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        0,1,2,3,4,5,6,7,8,9,-1,-1,-1,-1,-1,-1,
        -1,0xa,0xb,0xc,0xd,0xe,0xf,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,0xa,0xb,0xc,0xd,0xe,0xf,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

static signed char HexDigit( char c ) {
    return g_hexDigits[ (unsigned char) c ];
}

static ByteVector &HexToBin( ByteVector &bin ,const char *hex ,size_t size ) {
    assert( hex != nullptr );

    if( size >= 2 &&  hex[0] == '0' && hex[1] == 'x' ) {
        hex += 2; size -= 2;
    }

    size_t n = size / 2;

    bin.reserve( n );

    size_t i=0; for( ; i<2*n; i+=2 ) {
        bin.push_back( HexDigit(hex[i]) * 16 + HexDigit(hex[i+1]) );
    }

    if( size % 2 ) {
        bin.push_back( HexDigit(hex[i]) );
    }

    return bin;
}

inline ByteVector HexToBin( const char *hex ,size_t size ) {
    ByteVector bin; HexToBin( bin ,hex ,size ); return bin;
}

inline ByteVector &HexToBin( ByteVector &bin ,const std::string &hex ) {
    return HexToBin( bin ,hex.c_str() ,hex.size() );
}

//////////////////////////////////////////////////////////////////////////////
//! Serialization (vector)

template <typename T ,bool is_vector> struct vector_fun_ {};

template <typename T> struct vector_fun_<T,false> { //! replacement fun for native types
    static byte *data( const T &v ) { return (byte*) &v; }
    static size_t size( const T &v ) { return sizeof(T); }
    static size_t put( byte *p ,const T &v );
    static size_t get( const byte *p ,T &v );
};

template <typename T> struct vector_fun_<T,true> {
    static byte *data( const T &v ) { return (byte*) v.data(); }
    static size_t size( const T &v ) { return v.size(); }
    static size_t put( byte *p ,const T &v );
    static size_t get( const byte *p ,T &v );
};

///-- data
template <typename T>
size_t data_( const T &v ) {
    return vector_fun_<T ,type_info_<T>::is_vector >::data(v);
}

///-- size
template <typename T>
size_t size_( const T &v ) {
    return vector_fun_<T ,type_info_<T>::is_vector >::size(v);
}

///-- copy
template <typename T>
size_t put_( byte *p ,const T &v ) {
    return vector_fun_<T ,type_info_<T>::is_vector >::put( p ,v );
}

///-- definition (NB here because we need size_)
template <typename T>
size_t vector_fun_<T,false>::put( byte *p ,const T &v ) {
    size_t sz=size_(v); * (T*) p = v; return sz;
}

template <typename T>
size_t vector_fun_<T,false>::get( const byte *p ,T &v ) {
    size_t sz=size(v); v = * (T*) p; return sz;
}

//--
template <typename T>
size_t vector_fun_<T,true>::put( byte *p ,const T &v ) {
    memcpy( p ,data(v) ,size(v) ); return size(v);
}

template <typename T>
size_t vector_fun_<T,true>::get( const byte *p ,T &v ) {
    memcpy( data(v) ,p ,size(v) ); return size(v);
}

//////////////////////////////////////////////////////////////////////////////
//! Serialization (stream)

struct ByteStream {
    ByteStream( ByteVector &bytes ) : bin(bytes) {}

    ByteVector &bin;
    size_t pos = 0;

    const byte *data() const { return bin.data() + pos; }
    byte *data() { return bin.data() + pos; }

    size_t size() const { return bin.size() - pos; }

    void resize( size_t size ) {
        bin.resize( bin.size() + size );
    }

    template <typename T>
    ByteStream &operator <<( const T &v ) {
        bin.resize( pos + size_(v) );
        pos += put_<T>( bin.data() + pos ,v );
        return *this;
    }

    ByteStream &push( const byte *p ,size_t size ) {
        resize( size );
        memcpy( data() ,p ,size ); pos += size;
        return *this;
    }
};

///-- convert
template <typename T>
ByteVector typeToByteVector( T in ) {
    ByteVector bin; const size_t n = sizeof(T);

    bin.resize(n);

    * (T*) bin.data() = in;

    return bin;
}

template <typename T>
ByteVector vectorToByteVector( const T &in ) {
    return ByteVector( in.begin() ,in.end() );
}

///--
template <typename T> //TODO remove
size_t push_( uint8_t *&p ,const T &v ) {
    size_t sz=sizeof(T); * (T*) p = v; p += sz; return sz;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! from bitcoin

//////////////////////////////////////////////////////////////////////////////
template<unsigned int BITS>
struct base_blob {
    static constexpr int WIDTH = BITS / 8;
    uint8_t data[WIDTH];

    base_blob() { SetNull(); }

    void SetNull() { memset(data, 0, sizeof(data)); }
    bool IsNull() const { for( int i=0; i<WIDTH; ++i ) if( data[i] != 0 ) return false; return true; }

    inline int Compare(const base_blob& other) const { return memcmp(data, other.data, sizeof(data)); }

    friend inline bool operator ==( const base_blob &a ,const base_blob &b ) { return a.Compare(b) == 0; }
    friend inline bool operator !=( const base_blob &a ,const base_blob &b ) { return a.Compare(b) != 0; }
    friend inline bool operator <( const base_blob &a ,const base_blob &b ) { return a.Compare(b) < 0; }

//--
    unsigned char *begin() { return &data[0]; }
    unsigned char *end() { return &data[WIDTH]; }

    const unsigned char *begin() const { return &data[0]; }
    const unsigned char *end() const { return &data[WIDTH]; }

    unsigned int size() const { return sizeof(data); }

//--
    std::string GetHex() const {
        return BinToHex( std::reverse_iterator<const uint8_t*>(data + sizeof(data)) ,std::reverse_iterator<const uint8_t*>(data) );
    }

    void SetHex( const char *psz ) {
        memset(data, 0, sizeof(data));

        // skip leading spaces
        while (isspace(*psz))
            psz++;

        // skip 0x
        if (psz[0] == '0' && tolower(psz[1]) == 'x')
            psz += 2;

        // hex string to uint
        const char* pbegin = psz;
        while (HexDigit(*psz) != -1)
            psz++;
        psz--;
        unsigned char* p1 = (unsigned char*)data;
        unsigned char* pend = p1 + WIDTH;
        while (psz >= pbegin && p1 < pend) {
            *p1 = HexDigit(*psz--);
            if (psz >= pbegin) {
                *p1 |= ((unsigned char)HexDigit(*psz--) << 4);
                p1++;
            }
        }
    }

    uint64_t GetUint64(int pos) const {
        const uint8_t* ptr = data + pos * 8;
        return ((uint64_t)ptr[0]) | \
               ((uint64_t)ptr[1]) << 8 | \
               ((uint64_t)ptr[2]) << 16 | \
               ((uint64_t)ptr[3]) << 24 | \
               ((uint64_t)ptr[4]) << 32 | \
               ((uint64_t)ptr[5]) << 40 | \
               ((uint64_t)ptr[6]) << 48 | \
               ((uint64_t)ptr[7]) << 56;
    }
};

struct uint256 : public base_blob<256> {
    uint256() {}
};

//////////////////////////////////////////////////////////////////////////////
//! @note p2pkh = RIPEMD160(SHA256(pubkey));
//! @note address = BASE58_ENCODE( 0x0 + p2pkh )

#define BITCOIN_P2PKH_SIZE  20

static bool keyAddressToP2PKH( const std::string &address ,ByteVector &pubkey ) {
    ByteVector decode;

    bool r = DecodeBase58Check( address.c_str() ,pubkey );

    //! @note first byte is prefix, LATER might want to check against well known chain prefixes (address, wif, etc...) ?
    pubkey.erase( pubkey.begin() );

    return r;
}

static uint256 getHashFromBin( const ByteVector &bin ) {
    uint256 hash1 ,hash2;

    CSHA256 sha ,sha2;

    //-- sha
    sha.Write( bin.data() ,bin.size() );
    sha.Finalize( hash1.data );

    //-- double sha
    sha2.Write( (const byte*) hash1.data ,CSHA256::OUTPUT_SIZE );
    sha2.Finalize( hash2.data );

    return hash2;
}

static uint256 getHashFromHex( const std::string &hex ) {

    //-- convert to binary
    ByteVector bin;

    HexToBin( bin ,hex );

    //-- hash
    return getHashFromBin( bin );
}

static uint256 ComputeMerkleRoot( std::vector<uint256> hashes ) {
    while( hashes.size() > 1 ) {
        if( hashes.size() & 1 ) { //! duplicate last hash for odd number of hashes
            hashes.push_back( hashes.back() );
        }

        //! compute in place multiple hash pair
        SHA256D64( hashes[0].begin() ,hashes[0].begin() ,hashes.size()/2 );

        hashes.resize( hashes.size() / 2 );
    }

    if( hashes.empty() )
        return {};

    return hashes[0];
}

//////////////////////////////////////////////////////////////////////////////
//! minimally encoded serialized CScript

struct MinEnc {
    explicit MinEnc( uint64_t v=0  ) { value = v; }

    uint64_t value;
};

template <>
inline ByteStream &ByteStream::operator <<( const uint256 &v ) {
    memcpy( bin.data() + pos ,v.data ,32 ); pos += 32; return *this;
}

template <>
inline ByteStream &ByteStream::operator <<( const MinEnc &v ) {
    if( v.value < 0xff ) {
        return *this << (uint8_t) 1 << (uint8_t) v.value;
    } else if( v.value < 0xffff ) {
        return *this << (uint8_t) 2 << (uint16_t) v.value;
    } else if( v.value < 0xffffff ) {
        return *this << (uint32_t) (0x03000000 | v.value);
    } else if( v.value < 0xffffffff ) {
        return *this << (uint8_t) 4 << (uint32_t) v.value;
    } else {
        throw; //! overflow
    }
}

//////////////////////////////////////////////////////////////////////////////
//! BlockHeader

struct BlockHeader {
    int32_t nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    uint32_t nTime;
    uint32_t nBits;
    uint32_t nNonce;

///--
    bool fromJSON( const rapidjson::Value &v ) {
        const char *previousblockhash = json::getString( v ,"previousblockhash" );
        const char *bits = json::getString( v ,"bits" );

        size_t idx = 0;

        nVersion = json::getInt( v ,"version" );
        hashPrevBlock.SetHex( previousblockhash );
        hashMerkleRoot.SetNull();
        nTime = json::getInt( v ,"curtime" );
        nBits = std::stoi( bits ,&idx ,16 );
        nNonce = 0;

        return true;
    }

///-- binary
    void toBin( ByteVector &bin ) const {
        bin.clear();
        bin.resize( 4 + 32 + 32 + 4 + 4 + 4 );

        byte *p = bin.data();

        //--
        push_( p ,nVersion );
        push_( p ,hashPrevBlock );
        push_( p ,hashMerkleRoot );
        push_( p ,nTime );
        push_( p ,nBits );
        push_( p ,nNonce );
    }

    ByteVector toBin() const {
        ByteVector bin;
        toBin( bin );
        return bin;
    }

///-- hex
    template<class T>
    void toHex( T &hex ) const {
        ByteVector bin = toBin();

        BinToHex( hex ,bin);
    }

    template<class T>
    T toHex_() const {
        T hex; toHex( hex ); return hex;
    }

    std::string toHex() const {
        return toHex_<std::string>();
    }
};

//////////////////////////////////////////////////////////////////////////////
//! Script

#define SCRIPT_OP_0             ( (uint8_t) 0x00)
#define SCRIPT_OP_DUP           ( (uint8_t) 0x76)
#define SCRIPT_OP_HASH160       ( (uint8_t) 0xa9)
#define SCRIPT_OP_EQUALVERIFY   ( (uint8_t) 0x88)
#define SCRIPT_OP_CHECKSIG      ( (uint8_t) 0xac)

struct Script {
    // uint8_t size; //! make sure not > 0x0fd //TODO use VarInt
    ByteVector ops;

    size_t getSize() {
        return ops.size() + 1; //TODO don't forget when/if using VarInt
    }
};

static Script getScriptCoinbaseIn_BIP0034( uint32_t height ) {
    Script s;

    ByteStream(s.ops)
        << height
        << SCRIPT_OP_0
    ;

    return s;
}

static Script getScriptForAddress( const std::string &address ) {
    ByteVector p2pkh;

    if( !keyAddressToP2PKH( address ,p2pkh ) ) {
        throw std::exception();
        return {};
    }

    Script s;

    ByteStream(s.ops)
        << SCRIPT_OP_DUP
        << SCRIPT_OP_HASH160
        << (uint8_t) BITCOIN_P2PKH_SIZE
        << p2pkh
        << SCRIPT_OP_EQUALVERIFY
        << SCRIPT_OP_CHECKSIG
    ;

    return s;
}

template <>
inline ByteStream &ByteStream::operator <<( const Script &v ) {
    size_t size = v.ops.size(); assert( size < 0xfd );

    *this << (uint8_t) size;
    this->push( v.ops.data() ,size );

    return *this;
}

//////////////////////////////////////////////////////////////////////////////
//! Input Transaction (TxIn)

#define TX_SEQUENCE_FINAL ((uint32_t)0xffffffff)

struct TxIn {
    uint8_t prevout[32] = {0}; //! => hash
    uint32_t previdx = 0x0ffffffff;
    Script script;
    uint32_t sequence = TX_SEQUENCE_FINAL;
};

static TxIn getTxInCoinbase( uint32_t blockHeight ) {
    TxIn tx;

    memset( tx.prevout ,0 ,sizeof(TxIn::prevout) );
    tx.previdx = 0x0ffffffff;
    tx.script = getScriptCoinbaseIn_BIP0034(blockHeight);
    tx.sequence = TX_SEQUENCE_FINAL;

    return tx;
}

template <>
inline ByteStream &ByteStream::operator <<( const TxIn &v ) {
    this->push( v.prevout ,32 );
    *this << v.previdx << v.script << v.sequence;
    return *this;
}

//////////////////////////////////////////////////////////////////////////////
//! Output Transaction (TxOut)

struct TxOut {
    int64_t amount = 0;
    Script script;
};

static TxOut getTxOutForAddress( const std::string &address ,int64_t amount ) {
    TxOut tx;

    tx.amount = amount;
    tx.script = getScriptForAddress(address);

    return tx;
}

static TxOut getTxOutWithScript( const std::string &script ,int64_t amount ) {
    TxOut tx;

    tx.amount = amount;
    tx.script.ops.clear();
    HexToBin( tx.script.ops ,script );

    return tx;
}

template <>
inline ByteStream &ByteStream::operator <<( const TxOut &v ) {
    *this << v.amount << v.script; return *this;
}

//////////////////////////////////////////////////////////////////////////////
//! Transaction

#define TRANSACTION_NORMAL      0
#define TRANSACTION_COINBASE    5

#define TRANSACTION_VERSION    3

struct Transaction {
    uint32_t version = TRANSACTION_VERSION;

    std::vector<TxIn> vin;
    std::vector<TxOut> vout;

    uint32_t lockTime;
    std::vector<uint8_t> extraPayload; //! only for special transaction types

///--
    static uint32_t makeVersion( uint16_t version=TRANSACTION_VERSION ,uint16_t type=TRANSACTION_NORMAL ) {
        return (version | (type << 16));
    }

    void makeCoinbase( uint32_t blockHeight ) {
        version = makeVersion( TRANSACTION_VERSION ,TRANSACTION_COINBASE );

        vin.emplace_back(); //! coinbase input
        vin[0] = getTxInCoinbase(blockHeight);
    }

    void addTxOut( const TxOut &txout ) {
        if( txout.amount > 0 ) {} else return; //TODO! log this

        int i = vout.size();

        vout.emplace_back();
        vout[i] = txout;
    }

///--
    std::string toHex( const std::string &anExtraPayload ) const;
};

template <>
inline ByteStream &ByteStream::operator <<( const Transaction &tx ) {
    *this << tx.version;

    { ///-- vin //TODO vector << to genereic
        size_t n = tx.vin.size(); assert( n < 0xfd );
        *this << (uint8_t) n;
        for( size_t i=0; i<n; ++i ) {
            *this << tx.vin[i];
        }
    }
    { ///-- vout
        size_t n = tx.vout.size(); assert( n < 0xfd );
        *this << (uint8_t) n;
        for( size_t i=0; i<n; ++i ) {
            *this << tx.vout[i];
        }
    }

    *this << tx.lockTime;

    return *this;
}

inline std::string Transaction::toHex( const std::string &anExtraPayload ) const {
    ByteVector cb;

    ByteStream(cb) << *this;

    ///-- done
    std::string s;

    BinToHex( s ,cb );

    size_t payloadSize = anExtraPayload.size() / 2;

    assert(payloadSize < 0x0fd ); //TODO a VarInt

    auto sz = typeToByteVector( (byte) payloadSize );

    s += BinToHex( sz );

    s += anExtraPayload;

    return s;
}

//////////////////////////////////////////////////////////////////////////////
//! Blockchain Block

struct Block {
    BlockHeader header;

    std::vector<std::string> vtx; //! transactions, as hex
    std::vector<uint256> hashes; //! transactions hashes, binary

    //! not part of serialization
    uint32_t height;

///--
    bool addCoinbaseTx( Transaction &txCoinbase ,const std::string &extraPayload ) {

        //-- make raw coinbase transaction
        std::string rawtx = txCoinbase.toHex(extraPayload);

        //-- hash
        uint256 hash = getHashFromHex( rawtx );

        //! TEST
        // std::string test_check = hash.GetHex();

        //-- add
        vtx.emplace_back( rawtx );
        hashes.emplace_back( hash );

        return true;
    }

    bool addTxFromJSON( const rapidjson::Value &gbt ) {
        auto jtx = gbt["transactions"].GetArray();

        std::string tx ,h;
        uint256 h256;

        for( const auto &it : jtx ) {
            tx = it["data"].GetString();
            vtx.emplace_back( tx );

            h = it["hash"].GetString();
            h256.SetHex( h.c_str() );

            //! TEST
            // uint256 hash = getHashFromHex( tx );
            // std::string chk = hash.GetHex();
            //!

            hashes.emplace_back( h256 );
        }

        return true;
    }
};

//////////////////////////////////////////////////////////////////////////////
static uint256 ComputeMerkleRoot( const Block &block ) {
    std::vector<uint256> leaves;

    leaves.resize( block.vtx.size());

    for( size_t i = 0; i < block.vtx.size(); ++i ) {
        leaves[i] = block.hashes[i];
    }

    return ComputeMerkleRoot( std::move( leaves ));
}

//////////////////////////////////////////////////////////////////////////////
#define HEADER_NONCE_OFFSET 76

class CMiningBlock : public Block {
public:
    void Clear() {
        *this = CMiningBlock();
    }

    void pokeNonce( uint64_t nonce ) {
        byte * p = m_headerBin.data();

        * (uint32_t*) (p + HEADER_NONCE_OFFSET) = (uint32_t) nonce;
    }

    ByteVector &makeHashingBlob() {
        m_headerBin = header.toBin();

        return m_headerBin;
    }

    ByteVector &hashingBlob() {
        return m_headerBin;
    }

    std::string toHex() {
        std::string s = BinToHex( m_headerBin );

        assert( vtx.size() < 0x0fd );

        auto sz = typeToByteVector( (byte) vtx.size() );

        s += BinToHex( sz );

        for( const auto &it : vtx ) {
            s += it;
        }

        return s;
    }

private:
    ByteVector m_headerBin; //! @note blob to be ashed == serialized header (binary, 80 bytes)
};

//////////////////////////////////////////////////////////////////////////////
} //namespace bitcoin-blk

//////////////////////////////////////////////////////////////////////////////
//EOF