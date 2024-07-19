// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
// #include <common/stats.h>

#include "ui.h"
#include "ui-trade.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#include <ctime>

String &tradeTimeToString( const time_t &t ,String &s ) {
    if( t == 0 ) { s = ""; return s; }

    std::time_t now = t; // std::time(NULL);
    std::tm *ptm = std::localtime(&now);

    char buffer[64];
    std::strftime( buffer ,64 ,"%d/%m/%Y %H:%M:%S" ,ptm );

    s = buffer;

    return s;
}

//////////////////////////////////////////////////////////////////////////////
//! UiTradePlacer

#define SOLOMINER_UITRADEPLACER_PUID    0x07f18698a1b27b822

static const char *uiPlacer = {
    "controls = {"
        "trade:GuiLabel={ align=top,vertical; coords={0,0,100%,10%} text=Trade; textalign=center; font=large; }"
        "fromAmount:GuiTextBox = { align=top; coords={15%,0,60%,6%} text=0.0; }"
        "fromValue:GuiComboBox = { align=top,vertical; coords={61%,0,80%,6%} listonly=true; menu={ background=#101010; } }"
        "ma:GuiMargin={ align=top,vertical; coords={0,0,100%,2%} }"
        "lblFrom:GuiLabel={ align=top; coords={15%,0,25%,6%} text=Address; textalign=left+centerv; }"
        "fromAddress:GuiComboBox = { align=top,vertical; coords={30%,0,80%,6%} listonly=true; menu={ background=#101010; } }"
        "fromIcon:GuiImageBox = { align=none; coords={82%,5%,98%,24%} image=; text=; }"

        "to:GuiLabel={ align=top,vertical; coords={0,0,100%,10%} text=To; textalign=center; font=large; }"
        "toAmount:GuiTextBox = { text=0.0; align=top; coords={15%,0,60%,6%} enabled=false; }"
        "toValue:GuiComboBox = { align=top,vertical; coords={61%,0,80%,6%} listonly=true; menu={ background=#101010; } }"
        "mb:GuiMargin={ align=top,vertical; coords={0,0,100%,2%} }"
        "lblTo:GuiLabel={ align=top; coords={15%,0,30%,6%} text=To Address; textalign=left+centerv; }"
        "toAddress:GuiComboBox = { align=top,vertical; coords={30%,0,80%,6%} menu={ background=#101010; } }"
        "toIcon:GuiImageBox = { align=none; coords={82%,29%,98%,48%} image=; text=; }"

        "ok:GuiButton ={ commandId=2; bind=set; align=bottom,vertical; coords={40%,0,60%,8%} text=Trade; textalign=center; }"
        "info:GuiLabel ={ align=bottom,vertical; coords={40%,0,60%,40%} text=; textalign=center; font=large; textcolor=#00e000; }"
    "}"
};

struct UiTradePlacer : GuiGroup ,CGuiTabControl {
    UiTradePlacer() {
        setPropertiesWithString(uiPlacer);

        getMarket( "xeggex" ,m_market );

        //-- from
        m_fromAmount = getControlAs_<GuiTextBox>("fromAmount");
        m_fromValue = getControlAs_<GuiComboBox>("fromValue");
        m_fromAddress = getControlAs_<GuiComboBox>("fromAddress");

        if( m_fromValue ) {
            m_fromValue->Subscribe(*this);
            m_fromValue->menu().clear();
            m_fromValue->makeMenu( getCoinList() );

            if( m_fromAddress ) makeAddressList( *m_fromAddress ,m_fromValue->getText() );
        }

        //-- to
        m_toAmount = getControlAs_<GuiTextBox>("toAmount");
        m_toValue = getControlAs_<GuiComboBox>("toValue");
        m_toAddress = getControlAs_<GuiComboBox>("toAddress");

        if( m_toValue ) {
            m_toValue->Subscribe(*this);
            m_toValue->menu().clear();
            m_toValue->makeMenu( getCoinsWithMarket( m_market.ptr() ,tocstr(m_fromValue->text()) ) );

            if( m_toAddress ) makeAddressList( *m_toAddress ,m_toValue->getText() );
        }

        //-- ok
        getControlAs_<GuiButton>("ok")->Subscribe(*this);

        updateImages();
    }

    DECLARE_GUICONTROL(GuiGroup,UiTradePlacer,SOLOMINER_UITRADEPLACER_PUID);

///-- events
    void updateImages() {
        setPropertiesWithString( "controls={ fromIcon={ image=${a}; text=${a} } }" ,{{"a",m_fromValue->getText()}} );
        setPropertiesWithString( "controls={ toIcon={ image=${a}; text=${a} } }" ,{{"a",m_toValue->getText()}} );
    }

    void onFromSelect() {
        if( m_fromValue && m_fromAddress ) {} else return;

        makeAddressList( *m_fromAddress ,m_fromValue->getText() );

        m_toValue->menu().clear();
        m_toValue->makeMenu( getCoinsWithMarket( m_market.ptr() ,tocstr(m_fromValue->getText())) );

        updateImages();
    }

    void onToSelect() {
        if( m_toValue && m_toAddress ) {} else return;

        makeAddressList( *m_toAddress ,m_toValue->getText() );

        updateImages();
    }

    void startTrade() {
        auto &trader = getTrader();

//-- get info
        TradeInfo info;

        trader.makeTrade( info );

        // info.id;
        info.market = "*";

        fromString( info.amount.amount ,m_fromAmount->text() );
        fromString( info.amount.value ,m_fromValue->text() );

        fromString( info.price ,m_toAmount->text() );
        fromString( info.toValue ,m_toValue->text() );

        info.depositFromAddress = m_fromAddress->text();
        info.withdrawToAddress = m_toAddress->text();

        info.schedule = TradeInfo::executeNow;
        info.timeToExecute = 0;

//-- start trade
        TradeInfo::Id tradeId;

        if( IFAILED(trader.placeTrade( info ,tradeId )) ) {
            setInfo("Error occurred while placing trade","#e00000");
            return;
        }

        String msg;

        Format( msg ,"Trade %d placed successfully" ,128 ,(int) tradeId );
        setInfo( tocstr(msg) ,"#00e000" );

        m_fromAmount->text() = "";
        m_fromValue->text() = "";
        m_fromAddress->text() = "";
        m_fromAddress->menu().clear();

        //-- to
        m_toAmount->text() = "";
        m_toValue->text() = "";
        m_toAddress->text() = "";
        m_toAddress->menu().clear();

        auto *dialog = root().getBinding("dialog")->As_<GuiGroup>();

        SAFECALL(dialog)->onCommand( this ,GUI_MESSAGEID_NEXT ,0 ,NullPtr ,NullPtr );
    }

    void setInfo( const char *text ,const char *color="#e0e0e0" ) {
        setPropertiesWithString( "controls={ info={ text=${a}; textcolor=${b} } }" ,{{"a",text},{"b",color}} );
    }

//--
    void onTabEnter( int fromTabIndex ) override {}

//--
    void onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) override {
        if( commandId >= GUI_COMMANDID_MENU && commandId <= GUI_COMMANDID_MENUMAX ) {
            if( *source == m_fromValue->menu() )
                onFromSelect();
            else if( *source == m_toValue->menu() )
                onToSelect();
        }

        switch( commandId ) {
            case GUI_MESSAGEID_OK:
                startTrade();
                break;
            default:
                GuiGroup::onCommand( source ,commandId ,param ,params ,extra );
                return;
        }
    }

//--
    CMarketServiceRef m_market;

    GuiTextBox *m_fromAmount;
    GuiComboBox *m_fromValue;
    GuiComboBox *m_fromAddress;

    GuiTextBox *m_toAmount;
    GuiComboBox *m_toValue;
    GuiComboBox *m_toAddress;
};

REGISTER_CLASS(UiTradePlacer);

//////////////////////////////////////////////////////////////////////////////
//! UiTradeMonitor

#define SOLOMINER_UITRADEMONITOR_PUID    0x03f7abc98f7ca25e3

static const char *uiMonitor = {
    "controls={"
        "header:GuiGroup = { align=top,vertical; coords={0,0,100%,25%} controls={"
            "fromValue:GuiImageBox ={ align=left,horizontal; coords={0,0,20%,100%} image=RTM; text=; }"
            "fromAmount:GuiLabel ={ align=left,horizontal; coords={0,0,30%,100%} text=0.0; textalign=left,centerv; font=large; }"
            "toValue:GuiImageBox ={ align=left,horizontal; coords={0,0,20%,100%} image=RTM; text=; }"
            "toAmount:GuiLabel ={ align=left,horizontal; coords={0,0,30%,100%} text=0.0; textalign=left,centerv; font=large; }"
        "} }"
        "footer:GuiGroup ={ align=bottom,vertical; coords={0,0,100%,10%} controls={"
            "prev:GuiButton ={ commandId=22; bind=dialog; align=left,centerv; coords={0,0,10%,50%} text=<<; textalign=center; }"
            "next:GuiButton ={ commandId=23; bind=dialog; align=right,centerv; coords={0,0,10%,50%} text=>>; textalign=center; }"
            "info:GuiLabel ={ align=bottom; coords={20%,0,80%,100%} text=; textalign=center; font=large; }"
        "} }"

        "body:GuiGroup = { align=top,vertical; coords={0,0,100%,65%} controls={"
            "tradeId:GuiLabel ={ align=top,vertical; coords={0,0,100%,8%} text=TRADE ID; textalign=center; font=medium; }"
            "started:GuiLabel ={ align=top,vertical; coords={0,0,100%,6%} text=-; textalign=center; }"
            "placed:GuiLabel ={ align=top,vertical; coords={0,0,100%,6%} text=-; textalign=center; }"
            "executed:GuiLabel ={ align=top,vertical; coords={0,0,100%,6%} text=-; textalign=center; }"
            "completed:GuiLabel ={ align=top,vertical; coords={0,0,100%,6%} text=-; textalign=center; }"

            "ma:GuiMargin ={ align=top,vertical; coords={0,0,100%,4%} }"
            "orderId:GuiLabel ={ align=top,vertical; coords={0,0,100%,8%} text=ORDER ID : 0x04654235468754; textalign=center; font=medium; }"

            "mb:GuiMargin ={ align=top,vertical; coords={0,0,100%,2%} }"
            "hdeposit:GuiLabel ={ align=top,vertical; coords={0,0,100%,6%} text=Deposit; textalign=center; }"
            "deposit:GuiLabel ={ align=top,vertical; coords={0,0,100%,6%} text=.; textalign=center; }"

            "mc:GuiMargin ={ align=top,vertical; coords={0,0,100%,2%} }"
            "hexchange:GuiLabel ={ align=top,vertical; coords={0,0,100%,6%} text=Exchange; textalign=center; }"
            "exchange:GuiLabel ={ align=top,vertical; coords={0,0,100%,6%} text=.; textalign=center; }"

            "md:GuiMargin ={ align=top,vertical; coords={0,0,100%,2%} }"
            "hwithdraw:GuiLabel ={ align=top,vertical; coords={0,0,100%,6%} text=Withdraw; textalign=center; }"
            "withdraw:GuiLabel ={ align=top,vertical; coords={0,0,100%,6%} text=.; textalign=center; }"
        "} }"
    "}"
};

struct UiTradeMonitor : GuiGroup {
    UiTradeMonitor() : m_tradeCount(0) ,m_tradeId(-1) {
        // root().addBinding("self",this);

        setPropertiesWithString(uiMonitor);

        m_header = getControlAs_<GuiGroup>("header");
        assert(m_header);

        m_body = getControlAs_<GuiGroup>("body");
        assert(m_body);

        auto *footer = getControlAs_<GuiGroup>("footer");
        assert(footer);

        footer->getControlAs_<GuiButton>("prev")->Subscribe(*this);
        footer->getControlAs_<GuiButton>("next")->Subscribe(*this);
    }

    DECLARE_GUICONTROL(GuiGroup,UiTradeMonitor,SOLOMINER_UITRADEMONITOR_PUID);

//--
    void formatProgress( String &s ,int t ,const char *label ) {
        s = "";

        for( int i=0;i<t;++i ) s += ".";

        s += label;

        for( int j=0;j<t;++j ) s += ".";
    }

    void formatTradeTiming( int i ,int &x ,const char *control ,const char *label ,const char *done ,time_t timing ) {
        static int nPoints = -1; if( i==0 ) ++nPoints;

        auto *pControl = m_body->getControlAs_<GuiLabel>(control);
        assert(pControl);

        if( timing == 0 ) {
            if( x < i ) {
                pControl->text() = "-";
                return;
            }

            formatProgress( pControl->text() ,nPoints%4 ,label );
            return;

        } else {
            ++x;
        }

        String t;

        if( timing > 0 ) {
            tradeTimeToString( timing ,t );

            Format( pControl->text() ,"%s - %s" ,128 ,tocstr( t ) ,(const char *) done );
        } else {
            Format( pControl->text() ,"%s" ,128 ,(const char *) done );
        }
    }

    void formatOrderString( const char *name ,const BrokerOp &op ) {
        auto *pControl = m_body->getControlAs_<GuiLabel>(name);
        assert(pControl);

        String s ,t;

        enumToString( op.status ,s );

        if( op.status >= BrokerOp::completed && op.timeConcluded > 0 ) {
            tradeTimeToString( op.timeConcluded ,t );
            Format( pControl->text() ,"%s - %s" ,128 ,tocstr(t) ,tocstr(s) );
        } else {
            Format( pControl->text() ,"%s" ,128 ,tocstr(s) );
        }
    }

    void formatTradeValue( const char *name ,double amount ,double price=1. ,const char *marketLabel=NullPtr ) {
        double value = amount * price;

        if( value > DBL_EPSILON )
            toString( value ,m_header->getControlAs_<GuiLabel>(name)->text() );
        else
            m_header->getControlAs_<GuiLabel>(name)->text() = marketLabel;
    }

    void updateTrade( TradeInfo &info ) {
        String guid;

    //-- header
        double amount = info.amount.amount;

        formatTradeValue( "fromAmount" ,amount );
        formatTradeValue( "toAmount" ,amount ,info.price ,"(spot)" );

        setPropertiesWithString( "/header={ /fromValue={ image=${a}; text=${a}; } } }" ,{{"a",info.amount.value}} );
        setPropertiesWithString( "/header={ /toValue={ image=${a}; text=${a}; } } }" ,{{"a",info.toValue}} );

    //-- trade
        toString( info.id ,guid );
        Format( m_body->getControlAs_<GuiLabel>("tradeId")->text() ,"TRADE ID : %s" ,128 ,(const char*) tocstr(guid) );

        int x = 0;
        formatTradeTiming( 0 ,x ,"started" ,"recording" ,"Recorded" ,info.timeRecorded );
        formatTradeTiming( 1 ,x ,"placed" ,"placing" ,"Placed" ,info.timePlaced );
        formatTradeTiming( 2 ,x ,"executed" ,"executing" ,"Executed" ,info.timeExecuted );
        formatTradeTiming( 3 ,x ,"completed" ,"completing" ,"Completed" ,info.timeCompleted );

    //-- order
        BrokerOrder order;

        if( IFAILED(getBroker().findOrder( info.orderId ,order )) )  {
            setPropertiesWithString( "/body={ /deposit={text=.;} /exchange={text=.;} /withdraw={text=.;} }}" );
            return;
        }

        toString( order.id ,guid );
        Format( m_body->getControlAs_<GuiLabel>("orderId")->text() ,"ORDER ID : %s" ,128 ,(const char*) tocstr(guid) );

        setPropertiesWithString( "/header={ /toValue={ image=${a}; text=${a};} }}" ,{{"a",info.toValue}} );

        formatTradeValue( "toAmount" ,order.order.amount.amount ,order.order.price ,"(spot)" ); //! @note override with order value

        //--
        formatOrderString( "deposit" ,order.stages[0] );
        formatOrderString( "exchange" ,order.stages[1] );
        formatOrderString( "withdraw" ,order.stages[2] );

        auto *pDeposit = m_body->getControlAs_<GuiLabel>("deposit");
        assert(pDeposit);

        if( order.stage == BrokerOrder::makingDeposit && order.stages[0].stage == BrokerOp::confirm ) {
            Format( pDeposit->text() ,"Deposit confirmation %d" ,128 ,(int) order.deposit.confirmations );
        }
    }

    void updateTrades() {
        auto &trader = getTrader();

    //-- list trades
        ListOf<TradeInfo::Id> trades;

        trader.listOpenTrades( trades );

        for( auto it : trades ) {
            if( std::find( m_trades.begin() ,m_trades.end() ,it ) != m_trades.end() ) continue;

            m_trades.emplace_back(it);
        }

        int n = (int) m_trades.size();

        if( m_tradeCount != n ) {
            m_tradeId = 0;
            m_tradeCount = n;
        }

    //-- update pages
        String page ,count;

        toString( m_tradeId+1 ,page );
        toString( n ,count );

        if( m_tradeId < 0 ) {
            setPropertiesWithString( "/footer={/info={ text=; }}" );
            return;
        }

    //-- update current
        setPropertiesWithString( "/footer={/info={ text=Trades ${a}/${b};}}" ,{{"a",tocstr(page)},{"b",tocstr(count)}} );

        TradeInfo info;

        trader.getTrade( m_trades[m_tradeId] ,info );

        updateTrade( info );

        Refresh();
    }

    void onPrevTrade() {
        m_tradeId = MAX( m_tradeId-1 ,0 );
        Refresh();
    }

    void onNextTrade() {
        int n = (int) m_trades.size();

        m_tradeId = MIN( m_tradeId+1 ,n-1 );
        Refresh();
    }

//--
    void onDraw( const OsRect &uptadeArea ) override {
        if( m_trades.empty() ) {
            root().DrawTextAlign( "No trade in progress" ,area() ,textalignCenter );
            return;
        }

        GuiGroup::onDraw(uptadeArea);
    }

    void onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) override {
        switch( commandId ) {
            case GUI_MESSAGEID_PREV:
                onPrevTrade();
                break;
            case GUI_MESSAGEID_NEXT:
                onNextTrade();
                break;
            default:
                GuiGroup::onCommand( source ,commandId ,param ,params ,extra );
                return;
        }
    }

    void onTimer( OsTimerAction timeAction ,OsEventTime now ,OsEventTime last ) override {
        GuiSet::onTimer( timeAction ,now ,last );

        static OsEventTime lastUpdate = now;

        if( now - lastUpdate < 2000 ) return;

        updateTrades();

        lastUpdate = now;
    }

//--
    GuiGroup *m_header;
    GuiGroup *m_body;

    int m_tradeCount; //! @note (last) know count
    int m_tradeId; //! @note current monitored id

    ListOf<TradeInfo::Id> m_trades;
};

REGISTER_CLASS(UiTradeMonitor);

//////////////////////////////////////////////////////////////////////////////
static const char *uiTrade = {
    "controls={"
        "header:GuiTabBar = {"
            "align=top,vertical; coords={0,0,100%,6%} direction=right; titles={Trade,In progress,History}"
        "}"

        "warning1:GuiLabel={ align=top,vertical; coords={0,0,100%,5%} background=#101010; text=Warning: this is a BETA version, bug involving double trades and; textcolor=#ff8000; textalign=center; font=medium; }"
        "warning2:GuiLabel={ align=top,vertical; coords={0,0,100%,5%} background=#101010; text=ther costly annoyance might remain, only use as a developer for testing!; textcolor=#ff8000; textalign=center; font=medium; }"

        "body:GuiTab = {"
            "align=top; anchor=vertical; coords={0,0,100%,78%}" // 88
            "controls = {"
                "trade:UiTradePlacer = { align=top,vertical; coords={25%,0,75%,100%} }"
                "progress:UiTradeMonitor = { align=top,vertical; coords={25%,0,75%,100%} }"
                "history:GuiNavGrid = { controls={ "
                    "grid:GuiGrid = {"
                    "cols={24%,10%,10%,6%,10%,10%,10%,10%,10%}"
                    "titles={Trade Id,Amount,Price,To,Status,RecordedAt,PlacedAt,ExecutedAt,CompletedAt}"
                    "fields={id,amount,price,toValue,status,timeRecorded,timePlaced,timeExecuted,timeCompleted}"
                    "rows=10"
                    "} }"
                "}"
            "}"
        "}"
        "footer:GuiLabel = {"
            "align=top; anchor=vertical; coords={0,0,100%,6%} textalign=center; text=info;"
        "}"
    "}"
};

UiTradeDialog::UiTradeDialog( GuiControlWindow &parent ) : GuiDialog("Trade")
    ,m_parent(parent)
    ,m_tradeData( getTrader().getTradeBook() )
{
    setRoot( parent );
    parent.addBinding( "dialog" ,this );

    setPropertiesWithString( uiTrade );

    m_body = getControlAs_<GuiTab>("body");
    assert(m_body);

    GuiTabBar *tabbar = getControlAs_<GuiTabBar>("header");
    if( tabbar ) tabbar->Bind( *m_body );

    //-- orders
    m_trades = m_body->getControlAs_<GuiNavGrid>("history");
    m_trades->Grid()->Bind( &m_tradeData );
}

void UiTradeDialog::Open() {
    GuiDialog::Open();

    m_trades->updatePage(0);
}

//--
void UiTradeDialog::onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) {
    switch( commandId ) {
        case GUI_MESSAGEID_NEXT:
            m_body->nextTab();
            Refresh();
            break;
        default:
            GuiDialog::onCommand( source ,commandId ,param ,params ,extra );
            return;
    }
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF