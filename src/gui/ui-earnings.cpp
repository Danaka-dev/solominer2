// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include <common/stats.h>

#include <tiny/x-gui/controls/gui-grid.h>

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
            "align=top; anchor=vertical; coords={0,0,100%,6%}"
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
                "trades:GuiGroup = { controls={ label:GuiLabel={ text=TRADES; textalign=center; } } }"
                "orders:GuiGroup = { controls={ label:GuiLabel={ text=ORDERS; textalign=center; } } }"
            "}"
        "}"
    "}"
};

UiEarningsDialog::UiEarningsDialog( CEarningBook &book ) : GuiDialog("Accounts")
    ,m_book(book) ,m_datasource(book)
{
    setPropertiesWithString( uiEarnings );

    auto &tab = * getControlAs_<GuiTab>("body");

    GuiTabBar *tabbar = getControlAs_<GuiTabBar>("header");

    if( tabbar ) tabbar->Bind( tab );

    m_earnings = tab.getControlAs_<GuiNavGrid>("earnings");

    m_earnings->Grid()->Bind( &m_datasource );
}

void UiEarningsDialog::Open() {
    GuiDialog::Open();

    m_earnings->updatePage(0);
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF