#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_UI_EARNINGS_H
#define SOLOMINER_UI_EARNINGS_H

//////////////////////////////////////////////////////////////////////////////
#include <connections.h>

#include <tiny/x-gui/controls/gui-grid.h>

#include "ui.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
#define SOLOMINER_UIEARNINGDIALOG_UUID    0x08eb254b6b77d0b04

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! UiEarningsDialog

class UiEarningsDialog : public GuiCommandPublisher ,public GuiDialog {
public:
    UiEarningsDialog( CEarningBook &book );

    DECLARE_OBJECT_STD(GuiDialog,UiEarningsDialog,SOLOMINER_UIEARNINGDIALOG_UUID);

    // int firstId() const { return m_firstId; }
    // int lastId() const { return firstId() + m_entryPerPage; }

public:
    void Open() override;

    void updateEarnings( int pageId );

protected:
    void onCommand( GuiControl &source ,uint32_t commandId ,long param ,Params *params ,void *extra ) override;

protected:
    CEarningBook &m_book;

    RefOf<GuiGrid> m_earnings;
    RefOf<GuiGroup> m_navbar;
    // GuiGrid m_body;

    int m_pageId; //! active page in grid
    int m_pageCount; //! count of page in book

    int m_entryPerPage = 9;
};

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_UI_EARNINGS_H