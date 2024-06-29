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
                "earnings:GuiGroup = { controls={ "
                "grid:GuiGrid = {"
                    "align=top; anchor=vertical; coords={0,0,100%,90%}"
                    "cols={10%,6%,27%,12%,25%,10%,10%}"
                    "titles={Timestamp,Type,Transaction,Amount,Address,Traded,At}"
                    "fields={timestamp,type,txid,amount,toAddress,tradeAmount,tradePlacedAt}"
                    "rows=9"
                    "}"
                // "nav:UiGridNavigate = { align=bottom; anchor=vertical; coords={0,0,100%,10%} }"
                "nav:GuiNavBar = { align=bottom; anchor=vertical; coords={0,0,100%,10%} }"
                "} }"
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

    GuiGroup *earnings = tab.getControlAs_<GuiGroup>("earnings");

    if( !earnings ) return;

    m_earnings = earnings->getControlAs_<GuiGrid>("grid");
    m_navbar = earnings->getControlAs_<GuiGroup>("nav");

    m_earnings->Bind( &m_datasource );

    earnings->getControlAs_<GuiNavBar>("nav")->Bind( *this );
}

void UiEarningsDialog::Open() {
    GuiDialog::Open();

    updateEarnings(0);
}

void UiEarningsDialog::updateEarnings( int pageId ) {
    if( m_earnings.isNull() ) return;

    int n = (int) m_book.getEntryCount();
    m_pageId = pageId;
    m_pageCount = (int) ( n / m_entryPerPage ) + 1;

    m_earnings->setRows( m_entryPerPage ,pageId * m_entryPerPage );
    m_earnings->UpdateData();

//-- update label
    String label = "Page ";

    Format( label ,"Page %d/%d" ,128 ,m_pageId+1 ,m_pageCount );
    m_navbar->getControlAs_<GuiLabel>("label")->text() = label;
}

void UiEarningsDialog::onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) {
    GuiDialog::onCommand( source ,commandId ,param ,params ,extra );

    switch( commandId ) {
        case GUI_MESSAGEID_PREV:
            m_pageId = MAX( m_pageId-1 ,0 ); break;
        case GUI_MESSAGEID_NEXT:
            m_pageId = MIN( m_pageId+1 ,m_pageCount-1 ); break;
        case GUI_MESSAGEID_FIRST:
            m_pageId = 0; break;
        case GUI_MESSAGEID_LAST:
            m_pageId = m_pageCount-1; break;
        default:
            return;
    }

    updateEarnings( m_pageId );

    Refresh();
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF