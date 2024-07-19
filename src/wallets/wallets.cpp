// Copyright (c) 2018-2023 The NExTWave developers      <http://www.nextwave-techs.com/>
// Copyright (c) 2023-2024  The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include "wallets.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//! WalletTransaction

template <>
const Schema Schema_<WalletTransaction>::schema = fromString( Schema::getStatic() ,String(
    "txid:String"
    ",amount:AmountValue"
    ",fromAddress:String"
    ",toAddress:String"
    ",comment:String"
    ",communication:String"
    ",receivedAt:TimeSec"
    ",confirmations:int"
) );

template <>
void setMember( WalletTransaction &p ,int m ,const String &s ) {
    switch( m ) {
        default:
        case 0: fromString( p.txid ,s ); return;
        case 1: fromString( p.amount ,s ); return;
        case 2: fromString( p.fromAddress ,s ); return;
        case 3: fromString( p.toAddress ,s ); return;
        case 4: fromString( p.comment ,s ); return;
        case 5: fromString( p.communication ,s ); return;
        case 6: fromString( p.receivedAt ,s ); return;
        case 7: fromString( p.confirmations ,s ); return;
    }
}

template <>
String &getMember( const WalletTransaction &p ,int m ,String &s ) {
    switch( m ) {
        default:
        case 0: return toString( p.txid ,s );
        case 1: return toString( p.amount ,s );
        case 2: return toString( p.fromAddress ,s );
        case 3: return toString( p.toAddress ,s );
        case 4: return toString( p.comment ,s );
        case 5: return toString( p.communication ,s );
        case 6: return toString( p.receivedAt ,s );
        case 7: return toString( p.confirmations ,s );
    }
}

template <> WalletTransaction &Zero( WalletTransaction &p ) {
    p.txid = "";

    Zero( p.amount );
    p.fromAddress = "";
    p.toAddress = "";
    p.comment = "";
    p.communication = "";

    p.receivedAt = 0;
    p.confirmations = 0;

    return p;
}

//////////////////////////////////////////////////////////////////////////////
template <>
WalletConfig &fromString( WalletConfig &p ,const String &s ,size_t &size ) {
    //TODO

    return p;
}

//////////////////////////////////////////////////////////////////////////////
//! CWalletBase

IAPI_DEF CWalletService::getInterface( PUID id ,void **ppv ) {
    if( !ppv || *ppv ) return IBADARGS;

    if( id == classId() ) {
        *ppv = (CWalletService*) this; return IOK;
    }

    return
        honorInterface_<IWallet>(this,id,ppv) || honorInterface_<IService>(this,id,ppv) ? IOK
        : CService::getInterface(id,ppv)
    ;
}

IAPI_DEF CWalletService::hasCoinSupport( const char *coin ) {
    ListOf<String> coins;

    IRESULT result = getCoinList( coins ); IF_IFAILED_RETURN(result);

    for( const auto &it : coins ) {
        if( it == coin ) return IOK;
    }

    return INODATA;
}

//////////////////////////////////////////////////////////////////////////////
//! CWalletSetupBase

IAPI_DEF CWalletSetupBase::getInterface( PUID id ,void **ppv ) {
    if( !ppv || *ppv ) return IBADARGS;

    if( id == classId() ) {
        *ppv = (CWalletSetupBase*) this; return IOK;
    }

    return
        honorInterface_<IWalletSetup>(this,id,ppv) || honorInterface_<IServiceSetup>(this,id,ppv) ? IOK
        : CServiceSetup::getInterface(id,ppv)
    ;
}

//////////////////////////////////////////////////////////////////////////////
//! CWalletStore

IAPI_DEF CWalletStore::getInterface( PUID id ,void **ppv ) {
    if( !ppv || *ppv ) return IBADARGS;

    if( id == classId() ) {
        *ppv = (CWalletStore*) this; return IOK;
    }

    return
        honorInterface_<IWalletStore>(this,id,ppv) || honorInterface_<IServiceStore>(this,id,ppv) ? IOK
        : CServiceStore::getInterface(id,ppv)
    ;
}

//////////////////////////////////////////////////////////////////////////////
//! Coin Book

struct CoinBookInfo {
    String ticker;
    String name;

    ListOf<String> wallets; //! known wallets with support for the coin
    MapOf<String,String> addresses; //! known addresses per wallet
    MapOf<String,double> balances; //! know balance per address

    double balance = 0.; //! last seen total balance
};

struct CoinBook : Singleton_<CoinBook> {
    String filename;

    CoinBook() : filename("coinbook.dat") {
        LoadBinary( filename.c_str() ,book );
    }

    ~CoinBook() {
        SaveBinary( filename.c_str() ,book );
    }

    static MapOf<String,CoinBookInfo> &Book() { return getInstance().book; }

    MapOf<String,CoinBookInfo> book;
};

//TODO complete and conform usage of these functions throughout codebase

// static MapOf<String,CoinBookInfo> &theCoinBook = CoinBook::Book(); //! to force loading file at load time

///--
bool listWalletForCoin( const char *ticker ,ListOf<String> &walletNames ) {
    auto it = CoinBook::Book().find(ticker);

    if( it == CoinBook::Book().end() ) return false;

    walletNames = it->second.wallets;

    return true;
}

void noteWalletHasCoin( const char *coin ,CWalletService &wallet ) {
    assert(coin);

    CoinBookInfo &info = CoinBook::Book()[coin];

    ServiceInfo serviceInfo;

    wallet.getInfo( serviceInfo );

    addUnique( info.wallets ,serviceInfo.name );

    //? list all addresses
    //? forward to noteWalletHasAddress
}

void noteWalletHasAddress( const char *coin ,const char *address ,CWalletService &wallet ) {

}

//////////////////////////////////////////////////////////////////////////////
static MapOf<String,IWalletRef> g_walletForAddress;

void initWalletAddresses() {
    auto &store = getWalletStore();

    ListOf<String> wallets ,addresses;

    store.listServiceSupport( wallets );

    CWalletServiceRef p;

    for( const auto &it : wallets ) {
        getWallet( tocstr(it) ,p );

        addresses.clear();
        p->listAddresses( "" ,addresses );

        for( auto &address : addresses ) {
            noteWalletHasAddress( tocstr(address) ,p );
        }
    }
}

void noteWalletHasAddress( const char *address ,IWalletRef &wallet ) {
    if( address && *address && !wallet.isNull() )
        g_walletForAddress[address] = wallet;
}

void listAddressForWallet( IWalletRef &wallet ,ListOf<String> &addresses ) {
    for( auto &it : g_walletForAddress ) {
        if( it.second != wallet ) return;

        addresses.emplace_back( it.first );
    }
}

bool findWalletForAddress( const char *address ,IWalletRef &wallet ) {
    auto it = g_walletForAddress.find(address);

    if( it == g_walletForAddress.end() )
        return false;

    wallet = it->second;

    return true;
}

//////////////////////////////////////////////////////////////////////////////
//! Controls
void makeAddressList( ListOf<String> &addresses ,const char *ticker ) {
    String name = ticker;

    name += "-core";
    tolower(name);

    CWalletServiceRef pWallet;

    getWallet( tocstr(name) ,pWallet );

    if( pWallet.isNull() ) return;

    pWallet->listAddresses( "" ,addresses );
}

void makeAddressList( GuiComboBox &combo ,const char *ticker ) {
    ListOf<String> addresses;

    makeAddressList( addresses ,ticker );

    //--
    auto &menu = combo.menu();

    menu.clear();
    combo.text() = ""; //TODO only if address not in list ?

    int i=0; for( auto &it : addresses ) {
        const char *address = tocstr(it);

        if( i==0 && *address ) { //! if auto
            combo.text() = address;
        }

        menu.addItem( i , address ,NullPtr );

        ++i;
    }
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF