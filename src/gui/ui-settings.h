#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_UI_SETTINGS_H
#define SOLOMINER_UI_SETTINGS_H

//////////////////////////////////////////////////////////////////////////////
#include "ui.h"
#include "ui-dialogs.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#define SOLOMINER_UICORESETTING_PUID    0x064924fdb13ff734a
#define SOLOMINER_UIMAINSETTING_PUID    0x0e866e49e235a0c6a

//////////////////////////////////////////////////////////////////////////////
//! UiCoreSettings

class UiCoreSettings : public IDataEvents ,public CDataSource ,public GuiDialog {
public:
    UiCoreSettings();

    DECLARE_OBJECT_STD(GuiDialog,UiCoreSettings,SOLOMINER_UICORESETTING_PUID);

    void setCoreByTicker( const char *ticker ,CConnection &connection );

public:
    void updateEditState( bool edit );
    void loadBuiltInValue();

    void onCoreCombo( int id ,bool update=false );

    void onStart();
    void onStop();

    void onNewAddress();

    void onConfirm();
    void onCancel();
    void onClose();

    void onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) override;

public: //-- data event
    IAPI_IMPL onDataCommit( IDataSource &source ,Params &data ) IOVERRIDE;
    IAPI_IMPL onDataChanged( IDataSource &source ,const Params &data ) IOVERRIDE;

public: //-- data source
    IAPI_IMPL readData( Params &data ) IOVERRIDE;
    IAPI_IMPL onDataEdit( Params &data ) IOVERRIDE;

public:
    //! a data source to connect to each element
    ConfigDataSource m_data;
    bool m_hasEdit;

    String m_coin;
    CConnectionRef m_connection;
};

//////////////////////////////////////////////////////////////////////////////
//! TradeDataSource

class TradeDataSource : public ConfigDataSource {
public:
    TradeDataSource( Config &config ) :
        ConfigDataSource(config,"TRADER")
    {}

protected:
    // bool readValues( const char *id ) override;
    bool readValues() override;
    bool writeValues() override;
};

//////////////////////////////////////////////////////////////////////////////
//! CoreDataSource

class CoreDataSource : public ConfigDataSource {
public:
    CoreDataSource( Config &config ) :
        ConfigDataSource(config,"CORES")
    {}

protected:
    bool readValues( const char *id ) override;
    bool readValues() override;
    bool writeValues() override;

public:
    bool updatePassword( const char *password );
    bool updateAllPasswords( const char *password );
};

//////////////////////////////////////////////////////////////////////////////
//! MarketDataSource

class MarketDataSource : public ConfigDataSource {
public:
    MarketDataSource( Config &config ) :
        ConfigDataSource(config,"MARKETS")
    {}

public:
    bool readValues() override;
    bool writeValues() override;

public:
    bool updatePassword( const char *password );
    bool updateAllPasswords( const char *password );
};

//////////////////////////////////////////////////////////////////////////////
//! UiMainSettings

class UiMainSettings : public IDataEvents ,public GuiDialog {
public:
    UiMainSettings( GuiControlWindow *root=NullPtr );

    DECLARE_OBJECT_STD(GuiDialog,UiMainSettings,SOLOMINER_UIMAINSETTING_PUID);

public:
    virtual void Open();

    void showGlobalPassword();
    void showCorePassword( int i );

public:
    void onGlobalPasswordSet();
    void onCorePasswordSet( const char *id );
    void onPasswordSet();

    void onEditConfirm( int id );
    void onEditCancel( int id );

    void updateCorePassword();
    void updateControlState( IDataSource *source ,bool editing );

public: ///-- Data
    IAPI_DECL onDataCommit( IDataSource &source ,Params &data );
    IAPI_DECL onDataChanged( IDataSource &source ,const Params &data );

public: ///-- control
    void onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) override;
    void onNotify( IObject *source ,messageid_t notifyId ,long param ,Params *params ,void *extra ) override;

protected:
    UiPassword m_passwordDialog;

    ConfigDataSource m_globalConfig;
    TradeDataSource m_tradeConfig;
    CoreDataSource m_coreCredential;
    MarketDataSource m_marketCredential;

    bool m_editing;
};

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_UI_SETTINGS_H