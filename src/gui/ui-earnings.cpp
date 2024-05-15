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
static const char *uiNavigate = {
    "controls = {"
        "first:GuiLink = { commandId=1001; align=left,horizontal; font=large; text=|<; coords={0,0,6%,100%} }"
        "prev:GuiLink = { commandId=1002; align=left,horizontal; font=large; text=<<; coords={0,0,6%,100%} }"
        "last:GuiLink = { commandId=1004; align=right,horizontal; font=large; text=>|; textalign=right; coords={0,0,6%,100%} }"
        "next:GuiLink = { commandId=1003; align=right,horizontal; font=large; text=>>; textalign=right; coords={0,0,6%,100%} }"
        "label:GuiLabel = { align=center; font=normal; text=Page 1/1; textalign=center; coords={0,0,60%,100%} }"
    "}"
};

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
                        "titles={timestamp,type,txid,amount,address,traded,at}"
                    "}"
                    "nav:UiGridNavigate = { align=bottom; anchor=vertical; coords={0,0,100%,10%} }"
                "} }"
                // "trades:GuiGroup = { controls={ label:GuiLabel={ text=TRADES; textalign=center; } } }"
                // "orders:GuiGroup = { controls={ label:GuiLabel={ text=ORDERS; textalign=center; } } }"
            "}"
        "}"

    "}"
};

//////////////////////////////////////////////////////////////////////////////
#define GRIDNAVIGATE_UUID 0x073bf32ba24df0811

class UiGridNavigate : public GuiGroup {
public:
    UiGridNavigate() {
        setPropertiesWithString(uiNavigate);
    }

    void Bind( IGuiCommandEvent &listener ) {
        getControlAs_<GuiLink>("first")->Subscribe(listener);
        getControlAs_<GuiLink>("prev")->Subscribe(listener);
        getControlAs_<GuiLink>("next")->Subscribe(listener);
        getControlAs_<GuiLink>("last")->Subscribe(listener);
    }

    //TODO
    //BIND to grid
    //COMMAND action to data source

    DECLARE_GUICONTROL(GuiGroup,UiGridNavigate,GRIDNAVIGATE_UUID);
};

REGISTER_CLASS(UiGridNavigate);

//////////////////////////////////////////////////////////////////////////////
UiEarningsDialog::UiEarningsDialog( CEarningBook &book ) : GuiDialog("Accounts") ,m_book(book) {
    setPropertiesWithString( uiEarnings );

    auto &tab = * getControlAs_<GuiTab>("body");

    GuiTabBar *tabbar = getControlAs_<GuiTabBar>("header");

    if( tabbar ) tabbar->Bind( tab );

    GuiGroup *earnings = tab.getControlAs_<GuiGroup>("earnings");

    if( !earnings ) return;

    m_earnings = earnings->getControlAs_<GuiGrid>("grid");
    m_navbar = earnings->getControlAs_<GuiGroup>("nav");

    earnings->getControlAs_<UiGridNavigate>("nav")->Bind( *this );
}

void UiEarningsDialog::Open() {
    GuiDialog::Open();

    updateEarnings(0);
}

void UiEarningsDialog::updateEarnings( int pageId ) {
    //TODO use a datasource to do this

    if( m_earnings.isNull() ) return;

    m_earnings->Clear( false );

    int n = (int) m_book.getEntryCount();
    m_pageId = pageId;
    m_pageCount = (int) ( n / m_entryPerPage ) + 1;

    String s;

    int firstId = n - m_pageId * m_entryPerPage;
    int lastId = MAX( firstId - m_entryPerPage ,0 );

    m_book.eachEntry( [this,firstId,lastId,&s]( auto &id ,auto &p ) -> bool {
        if( id <= firstId ) {
            int y = m_earnings->addRow();

            int m=0; for( int x=0; x<7; ++x ,++m ) {
                p.getMember( m ,s );

                if( x == 2 ) { //! transaction
                    StringList list;

                    fromString( list ,s );

                    m_earnings->setCellText( y ,x++ ,list[0].c_str() );
                    m_earnings->setCellText( y ,x++ ,list[1].c_str() );
                    m_earnings->setCellText( y ,x ,list[3].c_str() );
                }
                else
                    m_earnings->setCellText( y ,x ,s.c_str() );
            }
        }

        return id > lastId;
    });

//-- update label
    String label = "Page ";

    Format( label ,"Page %d/%d" ,128 ,m_pageId+1 ,m_pageCount );
    m_navbar->getControlAs_<GuiLabel>("label")->text() = label;
}

void UiEarningsDialog::onCommand( GuiControl &source ,uint32_t commandId ,long param ,Params *params ,void *extra ) {
    GuiDialog::onCommand( source ,commandId ,param ,params ,extra );

    switch( commandId ) {
        case 1001:
            m_pageId = 0; break;
        case 1002:
            m_pageId = MAX( m_pageId-1 ,0 ); break;
        case 1003:
            m_pageId = MIN( m_pageId+1 ,m_pageCount-1 ); break;
        case 1004:
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