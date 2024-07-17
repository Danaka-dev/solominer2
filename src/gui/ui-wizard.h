#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_UI_WIZARD_H
#define SOLOMINER_UI_WIZARD_H

//////////////////////////////////////////////////////////////////////////////
#include "ui.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#define SOLOMINER_WIZARDDIALOG_PUID    0x0c511ca221a26107b

//////////////////////////////////////////////////////////////////////////////
//! UiWizardDialog

class UiWizardDialog : public GuiDialog  {
public:
    struct PageInfo {
        String title;

        //! LATER steps within a page
    };
//--

public:
    UiWizardDialog( GuiControlWindow &parent );

    DECLARE_OBJECT_STD(GuiDialog,UiWizardDialog,SOLOMINER_WIZARDDIALOG_PUID);

public:
    int getPageCount() {
        return (int) m_body.getControlCount();
    }

    int getCurrentPage() {
        return m_body.getCurrentTabIndex();
    }

    bool isFirstPage() {
        return getCurrentPage() == 0;
    }

    bool isLastPage() {
        return getCurrentPage() == getPageCount();
    }

    GuiControl *getPage( int index ) {
        return m_body.getControl( index );
    }

    void addPage( GuiControl &page ,const PageInfo &info );

    bool selectPage( int index );

//--
    bool PreviousPage() {
        return selectPage( getCurrentPage()-1 );
    }

    bool NextPage() {
        return selectPage( getCurrentPage()+1 );
    }

public: ///-- interface
    virtual bool onStepLeave( int step ,int toStep ) { return true; }
    virtual void onStepEnter( int step ,int fromStep ) {}

    virtual void onConfirm() { Close(); }
    virtual void onCancel() { Close(); }

public: ///-- IGuiEvents
    void onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) override;

protected:
    GuiLabel m_title;
    GuiTab m_body;

    ListOf<PageInfo> m_pages;
};

//////////////////////////////////////////////////////////////////////////////
//! CDataConnectionInfo2

    //! @note all fields flat

struct CDataConnectionInfo2 : CDataConnectionInfo {
    CDataConnectionInfo2( ConnectionInfo &a_info ) : CDataConnectionInfo(a_info)
    {}

    IAPI_IMPL readHeader( Params &data ,bool requireValues=false ) IOVERRIDE {
        //! @note all fields flat

        return readData( data );
    }

    IAPI_IMPL readData( Params &data ) IOVERRIDE {
        toString( info.status.nThreads ,data["Threads"] );
        toString( info.mineCoin.coin ,data["mineCoin"] );
        toString( info.mineCoin.address ,data["mineAddress"] );
        toString( info.tradeCoin.coin ,data["tradeCoin"] );
        toString( info.tradeCoin.address ,data["tradeAddress"] );
        toString( info.trading.percent ,data["tradePercent"] );
        toString( info.trading.withdraw ,data["tradeWithdraw:bool"] );
        toString( info.market ,data["Market"] );
        toString( info.pool ,data["Pool"] );
        toString( info.connection ,data["Host"] );
        toString( info.credential.user ,data["User"] );
        toString( info.credential.password ,data["Password"] );
        toString( info.options ,data["Options"] );
        toString( info.args ,data["Args"] );

        return IOK;
    }

    IAPI_IMPL onDataEdit( Params &data ) IOVERRIDE {
        fromParamsWithSchema( info ,data );

        //! sub schema
        fromMember( info.status.nThreads ,data ,"Threads" );
        fromMember( info.mineCoin.coin ,data ,"mineCoin" );
        fromMember( info.mineCoin.address ,data ,"mineAddress" );
        fromMember( info.tradeCoin.coin ,data ,"tradeCoin" );
        fromMember( info.tradeCoin.address ,data ,"tradeAddress" );
        fromMember( info.trading.percent ,data ,"tradePercent" );
        fromMember( info.trading.withdraw ,data ,"tradeWithdraw" );
        fromMember( info.credential.user ,data ,"User" );
        fromMember( info.credential.password ,data ,"Password" );

        return IOK;
    }
};

//////////////////////////////////////////////////////////////////////////////
//! UiConnectionWizard

extern CConnectionList &getConnectionList();

class UiConnectionWizard : public CDataConnectionInfo2 ,public UiWizardDialog {

public:
    UiConnectionWizard( GuiControlWindow &parent );

    DECLARE_OBJECT(UiConnectionWizard,SOLOMINER_WIZARDDIALOG_PUID) { \
        return (!ppv || *ppv) ? IBADARGS : CDataConnectionInfo2::getInterface( id ,ppv ) == IOK ? IOK : UiWizardDialog::getInterface( id ,ppv );
    }

    ConnectionInfo &info() { return m_info; }

    void showAddConnection( GuiControlWindow &parent ) {
        m_index = -1; Zero(m_info);
        selectPage(0);

        parent.ShowModal( *this );
    }

    void showEditConnection( GuiControlWindow &parent ,int index ,const ConnectionInfo &info ) {
        m_index = index; m_info = info;
        selectPage(0);

        parent.ShowModal( *this );
    }

    //--
    void confirmAddConnection() {
        Params settings;

        toManifest( m_info ,settings );

        root().onCommand( this ,GUI_MESSAGEID_OK ,-1 ,&settings ,(void*) "connection" );
    }

    void confirmEditConnection() {
        Params settings;

        toManifest( m_info ,settings );

        root().onCommand( this ,GUI_MESSAGEID_OK ,m_index ,&settings ,(void*) "connection" );
    }

    //--
    void updateData() {
        Params data;

        readData( data );

        CDataConnectionInfo::adviseDataChanged( data );
    }

public: ///-- UiWizardDialog
    void onStepEnter( int step ,int fromStep ) override;

    void onConfirm() override;
    void onCancel() override;

protected:
    int m_index = 0; //! Connection index in ConnectionList (-1 = new)
    ConnectionInfo m_info;
};

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_UI_WIZARD_H