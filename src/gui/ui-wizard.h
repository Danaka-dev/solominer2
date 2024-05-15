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
#define SOLOMINER_WIZARDDIALOG_UUID    0x0c511ca221a26107b

//////////////////////////////////////////////////////////////////////////////
//! UiWizardDialog

class UiWizardDialog : public GuiCommandPublisher ,public GuiDialog  {
public:
    struct StepInfo {
        String title;
    };

protected:
    struct Header : GuiGroup {
        GuiImageBox m_cancel ,m_info;
    } m_header;

    GuiLabel m_title;
    GuiTab m_steps;

    struct Footer : GuiGroup {
        GuiButton m_prev ,m_next;
        GuiLabel m_status;
    } m_footer;

//--
    ListOf<StepInfo> m_stepInfo;

public:
    UiWizardDialog();

    IMPORT_IOBJECT_API(SOLOMINER_WIZARDDIALOG_UUID);

public:
    void addStep( GuiControl &step ,const StepInfo &info ) {
        m_steps.addControl( step );
        m_stepInfo.emplace_back( info );
    }

    void setStep( int index ) {
        GuiCommandPublisher::PostCommand( GUI_COMMANDID_CLOSE ,getCurrentStep() );

        m_steps.selectTab( index );

        index = m_steps.getCurrentTabIndex();

        m_title.text() = m_stepInfo[ index ].title;

        GuiCommandPublisher::PostCommand( GUI_COMMANDID_OPEN ,getCurrentStep() );
    }

    int getCurrentStep() {
        return m_steps.getCurrentTabIndex();
    }

//--
    virtual void onPrevious() {
        int i = m_steps.getCurrentTabIndex();

        if( i > 0 ) {

            setStep( i-1 );
        }

        root().Refresh();
    }

    virtual void onNext() {
        int i = m_steps.getCurrentTabIndex();
        size_t n = m_steps.getControlCount();

        if( i+1 < n ) setStep( i+1 );

        i = m_steps.getCurrentTabIndex();
        m_footer.m_next.text() = ( i+1 == n ) ? "Done" : "Next";

        root().Refresh();
    }

    virtual void onConfirm() { Close(); }
    virtual void onCancel() { Close(); }

public:
    void onCommand( GuiControl &source ,uint32_t commandId ,long param ,Params *params ,void *extra ) override;
};

//////////////////////////////////////////////////////////////////////////////
//! UiConnectionWizard

class UiConnectionWizard : public UiWizardDialog {
protected:
    int m_index = 0; //! Connection index in ConnectionList (0 = new)
    ConnectionInfo m_info;

public:
    UiConnectionWizard();

    ConnectionInfo &info() { return m_info; }

    void setConnectionAdd() {
        m_index = 0; Zero(m_info);
        setStep(0);
    }

    void setConnectionEdit( int index ,const ConnectionInfo &info ) {
        m_index = index; m_info = info;
        setStep(0);
    }

public: //-- UiWizardDialog
    void onConfirm() override {
        UiWizardDialog::onConfirm();
    }

    void onCancel() override {
        UiWizardDialog::onCancel();
    }

    // void onCommand( GuiControl &source ,uint32_t commandId ,long param ,Params *params ,void *extra ) override;
};

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_UI_WIZARD_H