#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_UI_EARNINGS_H
#define SOLOMINER_UI_EARNINGS_H

//////////////////////////////////////////////////////////////////////////////
#include <connections.h>
#include <markets/trader.h>
#include <markets/broker.h>

#include <tiny/x-gui/controls/gui-grid.h>

#include "ui.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
#define SOLOMINER_UIEARNINGDIALOG_PUID    0x08eb254b6b77d0b04

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! UiEarningsDialog

class UiEarningsDialog : public GuiDialog {
public:
    UiEarningsDialog( CEarningBook &book );

    DECLARE_OBJECT_STD(GuiDialog,UiEarningsDialog,SOLOMINER_UIEARNINGDIALOG_PUID);

    // int firstId() const { return m_firstId; }
    // int lastId() const { return firstId() + m_entryPerPage; }

public:
    void Open() override;

    void updateEarnings( int pageId );

protected:
    // void onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) override;

protected:
    CEarning2DataSource m_earningData;
    RefOf<GuiNavGrid> m_earnings;

    CTradeDataSource m_tradeData;
    RefOf<GuiNavGrid> m_trades;

    COrderDataSource m_orderData;
    RefOf<GuiNavGrid> m_orders;
};

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_UI_EARNINGS_H