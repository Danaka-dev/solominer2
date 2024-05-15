#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_IWALLET_H
#define SOLOMINER_IWALLET_H

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>

#include <interface/ICommon.h>
#include <interface/IService.h>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
#define WALLET_SERVICE_CATEGORY   "wallet"

//////////////////////////////////////////////////////////////////////////////
#define IWALLET_UUID           0x011d7e32de9b2d514
#define IWALLETSETUP_UUID      0x0de2942a3e855889c
#define IWALLETSTORE_UUID      0x05b97e803e4b39ca2

//!
///--
/** IWallet
 * @note per coin interface, multi coin support via IWalletSetup ?
 */
class IWallet;

/** IWalletSetup interface
 *
 */
class IWalletSetup;

/**
 *
 */
class IWalletStore;

/**
 *
 */
class IWalletEvents;

//////////////////////////////////////////////////////////////////////////////
struct WalletInfo {
    String name;

    bool custodial;
    //...
};

//////////////////////////////////////////////////////////////////////////////
struct WalletCredential {
    String user;
    String password;
};

struct WalletConfig {
    WalletCredential credential;
    // ChainAddress address;
};

//////////////////////////////////////////////////////////////////////////////
/**
 * Account is a single value (fiat, coin, stock, ... ) holding within a wallet.
 * An account may hold any number of address(es)
 */
struct WalletAccount {
    String value;     //! value id (no tokens, those are held on addresses, not in accoutns)
    String name;      //! account name, default is sequence number within a wallet
};

typedef String AccountId;

//! @note AccountId, local identity
//! account_id = <value_id>#<account_name>
//! @exemple "RTM#1" ,"USDT;polygon#2"

template <>
String &fromManifest( AccountId &p ,const WalletAccount &s );

//TODO toManifest

//! maybe accountNumber, global identity ??
//! account_number = <value_id> : Base36( <market> | address ) <checksum>
//! exemple "RTM:XD14-DKFHF4-DK8DHFL-XA45 ...

// bool makeAccountNumber();

///--
struct WalletToken {
    //! @note token are held on an address

    String id;
    String data;
};

///--
struct WalletTransaction {
    String txid;             //! transaction id, globally unique

    AmountValue amount;    //! amount of value sent (negative amount) or received (positive amount)
    String fromAddress;    //! source address
    String toAddress;      //! destination address
    String comment;        //! sender comment for this transaction
    String communication;  //! communication sent to recipient

    TimeSec receivedAt;
    int confirmations;     //! number of on-market confirmations
};

inline bool isIncome( const AmountValue &amount ) {
    return amount.amount > 0.;
}

inline bool isExpense( const AmountValue &amount ) {
    return amount.amount < 0.;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! IWallet Events

class IWalletEvents : IOBJECT_PARENT {
public:
    IAPI_DECL onTransaction( IWallet &wallet ,const WalletTransaction &transaction ) = 0;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! IWallet Service

class IWallet : public virtual IPublisher_<IWalletEvents> ,IOBJECT_PARENT
{
public: ///-- IBase
    static UUID getClassId() { return IWALLET_UUID; };

public: ///-- IWallet
    IAPI_DECL getCoinList( ListOf<String> &coins ) = 0;
        // withAddress ,withBalance ...

    IAPI_DECL hasCoinSupport( const char *coin ) = 0;

public: ///-- manage
    // import( privkey )
    // export( privkey )

    IAPI_DECL unlock( const char *password ) = 0;

    IAPI_DECL lock() = 0;

    // encrypt( password )
    // changePass( old ,new );
    // get seed phrase

    // backup( toLocation )
    // restore( fromLocation );

public: ///-- accounts
    //TODO listValues // supported ,haveAccount ,zeroBalance

    IAPI_DECL listAccounts( ListOf<AccountId> &accounts ) = 0;

    IAPI_DECL CreateAccount( String &accountId ) = 0;

    // IAPI_DECL getAccount( AccountId &id ,IAccountRef &account ) = 0;

    // IAPI_DECL findAccountWithAddress( const char *address ,AccountID &account ) = 0;

    IAPI_DECL CloseAccount( AccountId &id ) = 0;

    IAPI_DECL getAccountBalance( const char *accountId ,AmountValue &balance ) = 0;

    IAPI_DECL getAccountTransactions( const char *accountId ,ListOf<WalletTransaction> &transactions ) = 0;

    IAPI_DECL sendFromAccount( const char *accountId ,WalletTransaction &transaction ) = 0;

public: ///-- addresses
    //! @note accountId can match many account, such as "BTC#*" or simply "BTC"
    //! when many account matched an operation requiring a specific account, first found account is used

    IAPI_DECL listAddresses( const char *accountId ,ListOf<String> &addresses ) = 0;

    IAPI_DECL getNewAddress( const char *accountId ,String &address ) = 0;

    IAPI_DECL ValidateAddress( const char *value ,String &address ) = 0;

    IAPI_DECL getAddressBalance( const char *address ,AmountValue &balance ) = 0;

    //+ getUnconfirmedBalance

    IAPI_DECL listTokens( const char *address ,ListOf<WalletToken> &tokens ) = 0;

    IAPI_DECL listTransactions( const char *address ,ListOf<WalletTransaction> &transactions ,int from=0 ,int count=0 ) = 0;

    IAPI_DECL getTransaction( const char *address ,const char *transactionId ,WalletTransaction &transaction ) = 0;

    IAPI_DECL sendToAddress( WalletTransaction &transaction ) = 0;

//--
    IAPI_DECL signMessage( const char *address ,const String &message ,String &signature ) = 0;

    IAPI_DECL verifyMessage( const char *address ,const String &message ,const String &signature ) = 0;

//...
};

typedef RefOf<IWallet> IWalletRef;

//////////////////////////////////////////////////////////////////////////////
//! IWallet Setup

class IWalletSetup : IOBJECT_PARENT
{
public: ///-- IBase
    static UUID getClassId() { return IWALLETSETUP_UUID; };

public: ///-- IWalletSetup
    //...
};

typedef RefOf<IWalletSetup> IWalletSetupRef;

//////////////////////////////////////////////////////////////////////////////
//! IWallet Store

class IWalletStore : IOBJECT_PARENT
{
public: ///-- IBase
    static UUID getClassId() { return IWALLETSTORE_UUID; };

public: ///-- IWalletStore
    //...
};

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_IWALLET_H