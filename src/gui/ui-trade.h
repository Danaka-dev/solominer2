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

    void readTrade( TradeInfo &info );
    void updateTrade( TradeInfo &info );

    void onUpdateTradeStatus();

protected:
    void onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) override;
    // void onTimer( OsTimerAction timeAction ,OsEventTime now ,OsEventTime last ) override;

protected:
    GuiControlWindow &m_parent;

    GuiTab *m_body;
    GuiGroup *m_trade;

    CTradeDataSource m_tradeData;
    RefOf<GuiNavGrid> m_trades;

    /* bool m_isTrading;
    TradeInfo::Id m_orderId;

//--
    GuiTextBox *m_fromAmount;
    GuiComboBox *m_fromValue;
    GuiComboBox *m_fromAddress;

    GuiTextBox *m_toAmount;
    GuiComboBox *m_toValue;
    GuiComboBox *m_toAddress;

//--
    GuiLabel *m_guid;
    GuiLabel *m_started;
    GuiLabel *m_placed;
    GuiLabel *m_executed;
    GuiLabel *m_completed; */
};

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_UI_TRADE_H