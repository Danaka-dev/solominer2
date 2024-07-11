#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_UI_DIALOGS_H
#define SOLOMINER_UI_DIALOGS_H

//////////////////////////////////////////////////////////////////////////////
#include "ui.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//! UiLogin

class UiLogin : public GuiDialog {
public:
    UiLogin( GuiControlWindow *root );

    void Open() override;

    virtual bool onConfirm();

public:
    void onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra );

protected:
    int m_retries;
    int m_tries;
};

//////////////////////////////////////////////////////////////////////////////
//! UiPassword (settings a password)

class UiPassword : public GuiDialog {
public:
    UiPassword( GuiControlWindow *root );

    void Open() override;

    virtual bool onConfirm();

public:
    void onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra );

protected:
    bool m_changePassword;
    bool m_confirmPassword;
};

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_UI_DIALOGS_H