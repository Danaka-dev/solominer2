#pragma once

// Copyright (c) 2018-2023 The NExTWave developers      <http://www.nextwave-techs.com/>
// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_WALLETS_H
#define SOLOMINER_WALLETS_H

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>
#include <common/service.h>

#include <interface/IWallet.h>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
#define CWALLETSERVICEBASE_PUID 0x00236b0c954ecfd79
#define CWALLETSETUPBASE_PUID   0x0d7273d720086e132
#define CWALLETSTORE_PUID       0x08defd967b95050c0

///--
class CWalletService;
class CWalletSetupBase;
class CWalletStore;

//////////////////////////////////////////////////////////////////////////////
//! WalletTransaction

DEFINE_MEMBER_API(WalletTransaction);
DEFINE_WITHSCHEMA_API(WalletTransaction);

//////////////////////////////////////////////////////////////////////////////
//! IWalletEvent Source

class CWalletEventSource : public CPublisher_<IWalletEvents> {
public:
    IAPI_DECL PostTransaction( IWallet &wallet ,const WalletTransaction &transaction ) {
        for( auto &it : m_subscribers ) {
            it->onTransaction(wallet,transaction);
        }
        return IOK;
    }
};

class CWalletEventListener : public IWalletEvents {
public:
    IAPI_IMPL onTransaction( IWallet &wallet ,const WalletTransaction &transaction ) IOVERRIDE {
        return IOK;
    }
};

//////////////////////////////////////////////////////////////////////////////
//! IWallet Service

class CWalletService : public IWallet ,public CService
    ,public CWalletEventSource
{
public:
    CWalletService( IServiceSetupRef &setup ) : CService(setup) ,m_unlocked(false)
    {
        this->info().category = WALLET_SERVICE_CATEGORY;
    }

    DECLARE_OBJECT(CWalletService,CWALLETSERVICEBASE_PUID);

    static const char *category() { return "wallet"; }

public: ///-- IWallet
    IAPI_IMPL hasCoinSupport( const char *coin );

public: ///-- manage
    IAPI_IMPL unlock( const char *password ) IOVERRIDE { return INOEXEC; }

    IAPI_IMPL lock() IOVERRIDE { return INOEXEC; }

public: ///-- accounts
    IAPI_IMPL listAccounts( ListOf<AccountId> &accounts ) IOVERRIDE { return INOEXEC; }

    IAPI_IMPL CreateAccount( String &accountId ) IOVERRIDE { return INOEXEC; }

    IAPI_IMPL CloseAccount( AccountId &id ) IOVERRIDE { return INOEXEC; }

    IAPI_IMPL getAccountBalance( const char *accountId ,AmountValue &balance ) IOVERRIDE { return INOEXEC; }

    IAPI_IMPL getAccountTransactions( const char *accountId ,ListOf<WalletTransaction> &transactions ) IOVERRIDE { return INOEXEC; }

    IAPI_IMPL sendFromAccount( const char *accountId ,WalletTransaction &transaction ) IOVERRIDE { return INOEXEC; }

public: ///-- addresses
    IAPI_IMPL listAddresses( const char *accountId ,ListOf<String> &addresses ) IOVERRIDE { return INOEXEC; }

    IAPI_IMPL getNewAddress( const char *accountId ,String &address ) IOVERRIDE { return INOEXEC; }

    IAPI_IMPL ValidateAddress( const char *value ,String &address ) IOVERRIDE { return INOEXEC; }

    IAPI_IMPL getAddressBalance( const char *address ,AmountValue &balance ) IOVERRIDE { return INOEXEC; }

    IAPI_IMPL listTokens( const char *address ,ListOf<WalletToken> &tokens ) IOVERRIDE { return INOEXEC; }

    IAPI_IMPL listTransactions( const char *address ,ListOf<WalletTransaction> &transactions ,int from=0 ,int count=0 ) IOVERRIDE { return INOEXEC; }

    IAPI_IMPL getTransaction( const char *address ,const char *transactionId ,WalletTransaction &transaction ) IOVERRIDE { return INOEXEC; }

    IAPI_IMPL sendToAddress( WalletTransaction &transaction ) IOVERRIDE { return INOEXEC; }

//--
    IAPI_IMPL signMessage( const char *address ,const String &message ,String &signature ) IOVERRIDE { return INOEXEC; }

    IAPI_IMPL verifyMessage( const char *address ,const String &message ,const String &signature ) IOVERRIDE { return INOEXEC; }

protected:
    bool m_unlocked;
};

typedef RefOf<CWalletService> CWalletServiceRef;

//////////////////////////////////////////////////////////////////////////////
//! IWallet Setup

class CWalletSetupBase : public IWalletSetup ,public CServiceSetup
{
public: ///-- IBase
    DECLARE_OBJECT(CWalletSetupBase,CWALLETSETUPBASE_PUID);

public: ///-- IWalletSetup
    //...
};

typedef RefOf<CWalletSetupBase> CWalletSetupRef;

//////////////////////////////////////////////////////////////////////////////
//! IWalletStore

class CWalletStore : public IWalletStore
    ,public CServiceStore
    ,public Singleton_<CWalletStore>
{
public: ///-- IBase
    DECLARE_OBJECT(CWalletStore,CWALLETSTORE_PUID);

public: ///-- IServiceStore
    //..

public: ///-- IChainStore
    //..
};

//////////////////////////////////////////////////////////////////////////////
inline CWalletStore &getWalletStore() {
    return getStore_<CWalletStore>();
}

//--
inline bool StartWallet( const char *name ,const Params *params=NullPtr ) {
    return StartService( CWalletService::category() ,name ,params );
}

inline bool getWallet( const char *name ,CWalletServiceRef &service ) {
    return getService_( name ,service );
}

//////////////////////////////////////////////////////////////////////////////
//! coin book

bool listWalletForCoin( const char *ticker ,ListOf<String> &walletNames );
void listAddressForWallet( IWalletRef &wallet ,ListOf<String> &addresses );

bool findWalletForAddress( const char *address ,IWalletRef &wallet );
void noteWalletHasAddress( const char *address ,IWalletRef &wallet );

inline void noteWalletHasAddress( const char *address ,CWalletServiceRef &walletService ) {
    IWalletRef wallet; wallet = walletService; noteWalletHasAddress(address,wallet);
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_WALLETS_H