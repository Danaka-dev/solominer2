// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
// #include <common/stats.h>

#include "ui.h"
#include "ui-earnings.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
static const char *uiEarnings = {
    // "align=center; coords={0,0,100%,100%}"
    "controls={"
        "header:GuiTabBar = {"
            "align=top; anchor=vertical; coords={0,0,100%,6%} direction=right; titles={Earnings,Trades,Orders}"
        "}"
        "body:GuiTab = {"
            "align=top; anchor=vertical; coords={0,0,100%,88%}"
            "controls = {"
                "earnings:GuiNavGrid = { controls={ "
                    "grid:GuiGrid = {"
                        "cols={10%,6%,27%,12%,25%,10%,10%}"
                        "titles={Timestamp,Type,Transaction,Amount,Address,Traded,At}"
                        "fields={timestamp,type,txid,amount,toAddress,tradeAmount,tradePlacedAt}"
                        "rows=9"
                    "} }"
                "}"
                "trades:GuiNavGrid = { controls={ "
                    "grid:GuiGrid = {"
                        "cols={24%,10%,10%,6%,10%,10%,10%,10%,10%}"
                        "titles={Trade Id,Amount,Price,To,Status,RecordedAt,PlacedAt,ExecutedAt,CompletedAt}"
                        "fields={id,amount,price,toValue,status,timeRecorded,timePlaced,timeExecuted,timeCompleted}"
                        "rows=9"
                    "} }"
                "}"
                "orders:GuiNavGrid = { controls={ "
                    "grid:GuiGrid = {"
                        "cols={16%,8%,20%,20%,20%,8%,8%}"
                        "titles={Order Id,Type,Deposit,Order,Withdraw,Stage,Placed At}"
                        "fields={id,type,deposit,order,withdraw,stage,timePlaced}"
                        "rows=9"
                    "} }"
                "}"
            "}"
        "}"
    "}"
};

UiEarningsDialog::UiEarningsDialog( CEarningBook &book ) : GuiDialog("Accounts")
    ,m_earningData(book)
    ,m_tradeData( getTrader().getTradeBook() )
    ,m_orderData( getBroker().getOrderBook() )
{
    setPropertiesWithString( uiEarnings );

//-- tabs
    auto &tab = * getControlAs_<GuiTab>("body");

    GuiTabBar *tabbar = getControlAs_<GuiTabBar>("header");

    if( tabbar ) tabbar->Bind( tab );

//-- earnings
    m_earningData.Reversed() = true;
    m_earnings = tab.getControlAs_<GuiNavGrid>("earnings");
    m_earnings->Grid()->Bind( &m_earningData );

//-- trades
    m_tradeData.Reversed() = true;
    m_trades = tab.getControlAs_<GuiNavGrid>("trades");
    m_trades->Grid()->Bind( &m_tradeData );

//-- orders
    m_orderData.Reversed() = true;
    m_orders = tab.getControlAs_<GuiNavGrid>("orders");
    m_orders->Grid()->Bind( &m_orderData );
}

void UiEarningsDialog::Open() {
    GuiDialog::Open();

    m_earnings->updatePage(0);
    m_trades->updatePage(0);
    m_orders->updatePage(0);
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF