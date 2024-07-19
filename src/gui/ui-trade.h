#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_UI_TRADE_H
#define SOLOMINER_UI_TRADE_H

//////////////////////////////////////////////////////////////////////////////
#include <connections.h>
#include <markets/trader.h>
#include <markets/broker.h>

#include "ui.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
#define SOLOMINER_UITRADEDIALOG_PUID    0x0521c54f53eb22dcb

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! UiEarningsDialog

class UiTradeDialog : public GuiDialog {
public:
    UiTradeDialog( GuiControlWindow &parent );

    DECLARE_OBJECT_STD(GuiDialog,UiTradeDialog,SOLOMINER_UITRADEDIALOG_PUID);

public:
    void Open() override;

protected:
    void onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) override;

protected:
    GuiControlWindow &m_parent;

    GuiTab *m_body;
    GuiGroup *m_trade;

    CTradeDataSource m_tradeData;
    RefOf<GuiNavGrid> m_trades;
};

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_UI_TRADE_H