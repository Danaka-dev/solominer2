#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_UI_DASHBOARD_H
#define SOLOMINER_UI_DASHBOARD_H

//////////////////////////////////////////////////////////////////////////////
#include <connections.h>

#include "ui.h"
#include "ui-dialogs.h"
#include "ui-settings.h"
#include "ui-earnings.h"
#include "ui-trade.h"
#include "ui-wizard.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
#define SOLOMINER_DASHBOARDWINDOW_PUID    0x0f794fe5b2cdc458a

//////////////////////////////////////////////////////////////////////////////
class UiHeader;
class UiFooter;
class UiConnection;
class UiConnectionList;

class CDashboardWindow;

//////////////////////////////////////////////////////////////////////////////
#define UI_HEADER_HEIGHT        122
#define UI_CONNECTION_HEIGHT    128
#define UI_FOOTER_HEIGHT        61

#define UI_COINICON_SIZE        75

#define UI_POOL_WIDTH           15.f
#define UI_ACTION_WIDTH         15.f
#define UI_BALANCE_WIDTH        30.f

//////////////////////////////////////////////////////////////////////////////
class UiHeader : public GuiGroup {
protected:
    CDashboardWindow &m_parent;

    GuiLink *m_totalEarnings; //! total income earnings from connection list

public:
    UiHeader( CDashboardWindow &parent );

    void updateTotalIncome();
};

//////////////////////////////////////////////////////////////////////////////
class UiFooter : public GuiGroup {
protected:
    CDashboardWindow &m_parent;

public:
    UiFooter( CDashboardWindow &parent );
};

//////////////////////////////////////////////////////////////////////////////
//TODO UiConnection here

//////////////////////////////////////////////////////////////////////////////
class UiConnectionList : public GuiGroup {
protected:
    CDashboardWindow &m_parent;
    CConnectionList &m_connections;

    UiConnectionWizard m_connectionWizard;

    int m_editIndex; //! index being edited/deleted

public:
    UiConnectionList( CDashboardWindow &parent ,CConnectionList &connections );

    void makeConnectionAdder();

//-- command
    void onAddConnection(); //! wants to add a connection => wizard dialog
    void onEditConnection( int index );
    void onMoveConnection( int previousIndex ,int newIndex );
    void onDeleteConnection( int index );

//-- callback
    void onAddConnectionOk( Params &settings ); //! new connection was setup (from wizard)
    void onEditConnectionOk( int index ,Params &settings ); //! a connection was updated (from wizard)
    void onDeleteConnectionOk( int index ); //! connection delete confirmed

public:
    void onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) override;
};

//////////////////////////////////////////////////////////////////////////////
class CDashboardWindow : public GuiControlWindow {

public:
    explicit CDashboardWindow( CConnectionList &connections );

    DECLARE_OBJECT_STD(GuiControlWindow,CDashboardWindow,SOLOMINER_DASHBOARDWINDOW_PUID);

    void showLoginDialog() {
        this->ShowModal( m_uiLoginDialog );
    }

    void showEarningsDialog() {
        this->ShowModal( m_uiEarningsDialog );
    }

    void showTradeDialog() {
        this->ShowModal( m_uiTradeDialog );
    }

    void showMainSettings() {
        this->ShowModal( m_mainSettings );
    }

    void showCoreSettings( const char *ticker ,CConnection &connection ) {
        m_coreSettings.setCoreByTicker( ticker ,connection );

        this->ShowModal( m_coreSettings );
    }

    void showConfirmDelete( int index ) {
        this->ShowMessageBox( m_confirmDelete );
    }

    CConnectionList &connections() { return m_connections; }

    UiHeader &header() { return m_uiHeader; }
    UiFooter &footer() { return m_uiFooter; }
    UiConnectionList &connectionList() { return m_uiConnectionList; }

    bool isLogged();

protected:
    void onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) override;
    void onNotify( IObject *source ,messageid_t notifyId ,long param ,Params *params ,void *extra ) override;

protected:
    CConnectionList &m_connections;

///--
    UiHeader m_uiHeader;
    UiFooter m_uiFooter;
    UiConnectionList m_uiConnectionList;

    UiLogin m_uiLoginDialog;
    UiEarningsDialog m_uiEarningsDialog;
    UiTradeDialog m_uiTradeDialog;
    UiMainSettings m_mainSettings;
    UiCoreSettings m_coreSettings;

///--
    GuiMessageBox m_confirmDelete;
};

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_UI_DASHBOARD_H