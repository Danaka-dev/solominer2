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
//! CoreDataSource

class CoreDataSource : public ConfigDataSource {
public:
    CoreDataSource( Config &config ,const char *section=NullPtr ) :
        ConfigDataSource(config,section)
    {}

protected:
    API_DECL(bool) readValues( const char *id );
    API_DECL(bool) readValues();
    API_DECL(bool) writeValues();
};

//////////////////////////////////////////////////////////////////////////////
//! MarketDataSource

class MarketDataSource : public ConfigDataSource {
public:
    MarketDataSource( Config &config ,const char *section=NullPtr ) :
        ConfigDataSource(config,section)
    {}

public:
    API_DECL(bool) readValues();
    API_DECL(bool) writeValues();
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

    void onEditConfirm();
    void onEditCancel();

    void updateControlState( bool editing );

public: ///-- Data
    IAPI_DECL onDataCommit( IDataSource &source ,Params &data );
    IAPI_DECL onDataChanged( IDataSource &source ,const Params &data );

public: ///-- control
    void onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) override;
    void onNotify( IObject *source ,messageid_t notifyId ,long param ,Params *params ,void *extra ) override;

protected:
    UiPassword m_passwordDialog;

    CoreDataSource m_coreCredential;
    MarketDataSource m_marketCredential;

    bool m_editing;
};

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_UI_SETTINGS_H