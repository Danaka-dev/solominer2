// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include <common/stats.h>
#include <coins/cores.h>
#include <pools/pools.h>

#include "ui.h"
#include "ui-theme.h"
#include "ui-assets.h"
#include "ui-stats.h"
#include "ui-dashboard.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//! ui core

extern Config &getServiceConfig(); //! TODO find a proper place to put this declaration

UiCoreSettings::UiCoreSettings() : GuiDialog( "core-settings")
    ,m_data( getServiceConfig() ,"CORES" ) ,m_hasEdit(false)
{
    setPropertiesWithString(
            "background=#FF000000; textcolor=#FF808080; controls={"
            "ticker:GuiImageBox = { coords={0,0,100%,20%} align=top,vertical; image=RTC; text=RTC; textalign=center; textplacement=bottom; }"
            "body:GuiGroup = { coords={0,0,100%,60%} align=top,vertical; controls={"
                "lblCore:GuiLabel = { coords={0,0,48%,12%} align=left; text=Core; textalign=right+centerv; }"
                "comboCore:GuiComboBox = { coords={52%,1%,73%,10%} align=none; text=built-in; listonly=true; menu={ items=built-in,custom; } }"
                "start:GuiButton = { commandId=1006; coords={75%,1%,83%,10%} align=none; text=Start; } "
                "stop:GuiButton = { commandId=1005; coords={85%,1%,93%%,10%} align=none; text=Stop; colors={ hoover={#804040,#804040,#808080,0} } }"

                "lblFilename:GuiLabel = { coords={0,12%,48%,24%} align=left; text=Filename; textalign=right+centerv; }"
                "editFilename:GuiTextBox = { coords={52%,13%,73%,23%} align=none; text=; textalign=left+centerv; }"
                "lblLocation:GuiLabel = { coords={0,24%,48%,36%} align=left; text=Location; textalign=right+centerv; }"
                "editLocation:GuiTextBox = { coords={52%,25%,93%,35%} align=none; text=; textalign=left+centerv; }"
                "lblDatapath:GuiLabel = { coords={0,36%,48%,48%} align=left; text=Data Path; textalign=right+centerv; }"
                "editDatapath:GuiTextBox = { coords={52%,37%,93%,47%} align=none; text=; textalign=left+centerv; }"
                "lblArguments:GuiLabel = { coords={0,48%,48%,60%} align=left; text=Arguments; textalign=right+centerv; }"
                "editArguments:GuiTextBox = { coords={52%,49%,73%,59%} align=none; text=; textalign=left+centerv; }"

                "lblHostPort:GuiLabel = { coords={0,60%,48%,72%} align=left; text=Host:Port; textalign=right+centerv; }"
                "editHost:GuiTextBox = { coords={52%,61%,67%,71%} align=none; text=; textalign=left+centerv; }"
                "editPort:GuiTextBox = { coords={68%,61%,73%,71%} align=none; text=; textalign=left+centerv; }"

                "lblUserPass:GuiLabel = { coords={0,72%,48%,84%} align=left; text=User/Password; textalign=right+centerv; }"
                "editUser:GuiTextBox = { coords={52%,73%,62%,83%} align=none; text=; textalign=left+centerv; }"
                "editPass:GuiTextBox = { coords={63%,73%,73%,83%} align=none; text=; textalign=left+centerv; }"

                //-- connection
                "lblAddress:GuiLabel = { coords={0,84%,48%,96%} align=left; text=Address; textalign=right+centerv; }"
                "editAddress:GuiTextBox = { coords={52%,85%,83%,95%} align=none; text=; textalign=left+centerv; }"
                "new:GuiButton = { commandId=1009; coords={85%,85%,93%,95%} align=none; text=New; }"

            "} }"
            "footer:GuiGroup = { coords={0,0,100%,10%} align=bottom,vertical; controls={"
                "cancel:GuiButton = { commandId=1002; coords={36%,25%,48%,75%} align=none; text=Cancel; } "
                "confirm:GuiButton = { commandId=1001; coords={52%,25%,64%,75%} align=none; text=Confirm; } "
                "close:GuiButton = { commandId=1010; coords={52%,0,64%,50%} align=right,centerv; text=Close; } "
            "} }"
        "}"
    );

    auto *body = getControlAs_<GuiGroup>("body");
    auto *footer = getControlAs_<GuiGroup>("footer");

    //-- commands
    body->getControlAs_<GuiComboBox>("comboCore")->Subscribe( *this );
    body->getControlAs_<GuiButton>("stop")->Subscribe( *this );
    body->getControlAs_<GuiButton>("start")->Subscribe( *this );
    body->getControlAs_<GuiButton>("new")->Subscribe( *this );
    footer->getControlAs_<GuiButton>("confirm")->Subscribe( *this );
    footer->getControlAs_<GuiButton>("cancel")->Subscribe( *this );
    footer->getControlAs_<GuiButton>("close")->Subscribe( *this );

    //-- data source
    // m_data.Connect( "$service" ); //TODO reform store for this ...
    m_data.Bind( "CORES" );
    m_data.Subscribe( *this );

    //-- data edit
    //TODO bind from manifest ? .. IDataSource as GuiControl ?
    body->getControl("editFilename")->As_<GuiTextBox>()->Bind( "filename" ,m_data );
    body->getControl("editLocation")->As_<GuiTextBox>()->Bind( "location" ,m_data );
    body->getControl("editDatapath")->As_<GuiTextBox>()->Bind( "datapath" ,m_data );
    body->getControl("editArguments")->As_<GuiTextBox>()->Bind( "arguments" ,m_data );
    body->getControlAs_<GuiTextBox>("editHost")->Bind( "host" ,m_data );
    body->getControl("editPort")->As_<GuiTextBox>()->Bind( "port" ,m_data );
    body->getControl("editUser")->As_<GuiTextBox>()->Bind( "user" ,m_data );
    body->getControl("editPass")->As_<GuiTextBox>()->Bind( "password" ,m_data );

    body->getControl("editAddress")->As_<GuiTextBox>()->Bind( "address" ,*this );

    //-- state
    onCoreCombo(0);
}

void UiCoreSettings::setCoreByTicker( const char *ticker ,CConnection &connection ) {
    Params props = {
        { "image" ,ticker }
        ,{ "text" ,ticker }
    };

    m_coin = ticker;

    getControl("ticker")->setProperties( props );

    String table = ticker;

    table += "-core"; tolower(table);

    m_data.Seek( table.c_str() );
    updateEditState( false );

    m_connection = connection;

    String &address = m_connection->info().mineCoin.address;

    getControlAs_<GuiGroup>("body")->getControlAs_<GuiTextBox>("editAddress")->setValue( address.c_str() );
}

//--
static const Params ALL_DEFAULT = {
    { "arguments" ,"-listen -server" }
    ,{ "host" ,"localhost" }
    ,{ "user" ,"user" }
    ,{ "password" ,"pass" }
};

//TODO get default config from coins

static const Params MAXE_DEFAULT = {
     { "filename" ,"maxeter-qt" }
    ,{ "location" ,"./cores/maxeter/1.14.23.1/" }
    ,{ "datapath" ,"./cores/maxeter/blockchain/" }
    ,{ "port" ,"17086" }
};

static const Params RTC_DEFAULT = {
     { "filename" ,"reaction-qt" }
    ,{ "location" ,"./cores/reaction/1.1.15.0/" }
    ,{ "datapath" ,"./cores/reaction/blockchain/" }
    ,{ "port" ,"15075" }
};

static const Params RTM_DEFAULT = {
     { "filename" ,"raptoreum-qt" }
    ,{ "location" ,"./cores/raptoreum/1.3.17.05/" }
    ,{ "datapath" ,"./cores/raptoreum/blockchain/" }
    ,{ "port" ,"10225" }
};

void UiCoreSettings::updateEditState( bool edit ) {
    auto *footer = getControlAs_<GuiGroup>("footer");

    m_hasEdit = edit;

    footer->getControlAs_<GuiButton>("confirm")->setState( edit ? highlightNormal : highlightDisabled );
    footer->getControlAs_<GuiButton>("cancel")->setState( edit ? highlightNormal : highlightDisabled );

    Refresh();
}

void UiCoreSettings::loadBuiltInValue() {
    Params data;

    data = ALL_DEFAULT;
    m_data.onDataEdit( data );

    if( m_coin == "MAXE" ) {
        data = MAXE_DEFAULT;
    } else if( m_coin == "RTC" ) {
        data = RTC_DEFAULT;
    } else if( m_coin == "RTM" ) {
        data = RTM_DEFAULT;
    }

    m_data.onDataEdit( data );

    Refresh();
}

void UiCoreSettings::onCoreCombo( int id ,bool update ) {
    auto *body = getControlAs_<GuiGroup>("body");

    bool enable = true;

    switch( id ) {
        default:
        case 0: { //! build-in
            if( update ) loadBuiltInValue();

            enable = false;
        }
        break;

        case 1: {
            enable = true;
        }
        break;
    }

//--
    body->getControl("editFilename")->enabled() = enable;
    body->getControl("editLocation")->enabled() = enable;
    body->getControl("editDatapath")->enabled() = enable;
    body->getControl("editArguments")->enabled() = enable;
    body->getControl("editHost")->enabled() = enable;
    body->getControl("editPort")->enabled() = enable;
    body->getControl("editUser")->enabled() = enable;
    body->getControl("editPass")->enabled() = enable;

    Refresh();
}

void UiCoreSettings::onStart() {
    if( m_connection.isNull() ) return;

    m_connection->startWalletService();

    Refresh();
}

void UiCoreSettings::onStop() {
    if( m_connection.isNull() || m_connection->coinWallet().isNull() ) return;

    m_connection->stopWalletService();

    Refresh();
}

void UiCoreSettings::onNewAddress() {
    if( m_connection.isNull() || m_connection->coinWallet().isNull() ) return;

    CWalletService &wallet = m_connection->coinWallet().get();

    String newAddress;

    if( wallet.getNewAddress( "" ,newAddress ) != IOK ) return; //TODO log, update status text below ...

    String &address = m_connection->info().mineCoin.address;

    address = newAddress;

    m_connection->adviseEdit( true );
}

void UiCoreSettings::onConfirm() {
    if( !m_hasEdit ) {
        updateEditState(false);
        return;
    }

    IRESULT result = m_data.Commit();

    //-- report connection info
    auto *body = getControlAs_<GuiGroup>("body");

    assert( body );

    auto &info = m_connection->info();
    String port;

    if( result == IOK ) {
        body->getControlAs_<GuiTextBox>("editHost")->getValue( info.connection.host );
        body->getControlAs_<GuiTextBox>("editPort")->getValue( port );
        body->getControlAs_<GuiTextBox>("editUser")->getValue( info.credential.user );
        body->getControlAs_<GuiTextBox>("editPass")->getValue( info.credential.password );
        fromString( info.connection.port ,port );
    }

    body->getControlAs_<GuiTextBox>("editAddress")->getValue( info.mineCoin.address );

    m_connection->adviseEdit( true );
    m_connection->connectionList().saveConfig();

    updateEditState(false);
}

void UiCoreSettings::onCancel() {
    if( !m_hasEdit ) {
        updateEditState(false);
        return;
    }

    //! MAYBE message to confirm discard if edit in progress

//-- data
    m_data.Discard();

//-- address
    Params data;

    data["address"] = m_connection->info().mineCoin.address;

    adviseDataChanged( data );

//-- state
    updateEditState(false);
}

void UiCoreSettings::onClose() {
    if( m_hasEdit ) onConfirm();

    Close();
}

//--
void UiCoreSettings::onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) {
    GuiDialog::onCommand( source ,commandId ,param ,params ,extra );

    switch( commandId ) {
        case 1001: onConfirm(); return;
        case 1002: onCancel(); return;
        case 1005: onStop(); return;
        case 1006: onStart(); return;
        case 1009: onNewAddress(); return;
        case 1010: onClose(); return;

        case GUI_COMMANDID_MENU:
        case GUI_COMMANDID_MENU+1:
            onCoreCombo( (int) (commandId - GUI_COMMANDID_MENU) ,true );
    }
}

IAPI_DEF UiCoreSettings::onDataCommit( IDataSource &source ,Params &data ) {
    updateEditState(false); return IOK;
}

IAPI_DEF UiCoreSettings::onDataChanged( IDataSource &source ,const Params &data ) {
    updateEditState(true); return IOK;
}

IAPI_DEF UiCoreSettings::readData( Params &data ) {
    for( auto &it : data ) {
        if( it.first == "address" ) {
            it.second = m_connection->info().mineCoin.address;
        }
    }

    return IOK;
}

IAPI_DEF UiCoreSettings::onDataEdit( Params &data ) {
    updateEditState(true); return IOK;
}

//////////////////////////////////////////////////////////////////////////////
//! UiMainSeetings

#define SOLOMINER_INTRO_1     "Welcome to your best buddy, mate, bro, fellow"
#define SOLOMINER_INTRO_2     "software assistant to your solo mining adventures."

#define SOLOMINER_BODY_1      "This is BETA 1 version of SOLOMINER 2, with full"
#define SOLOMINER_BODY_2      "support for direct core mining and hopefully not too many bugs."
#define SOLOMINER_BODY_3      ""
#define SOLOMINER_BODY_4      "Look out for soon to come BETA 2 or most recent version at:"
#define SOLOMINER_BODY_5      "https://github.com/danaka-dev/solominer2"

#define SOLOMINER_DONATORS_1   "Top Donators"
#define SOLOMINER_DONATORS_2   "M. Ganapati   0.001754 BTC"
#define SOLOMINER_DONATORS_3   "Anonymous     0.2300 LTC"

#define SOLOMINER_FOOTER_1    "Thanks for using SOLOMINER, may the blocks be with you!"

UiMainSettings::UiMainSettings() : GuiDialog( "about solominer")
{
    setPropertiesWithString(
        "background=#FF000000; textcolor=#FF808080; controls={"
            "footer:GuiLabel = { coords={0,0,100%,40%} align=bottom; }"
            "heading:GuiLabel = { coords={0,0,100%,20%} font=huge; align=top,centerh,vertical; textalign=center; text=SOLOMINER v2 BETA 1; }"
            "intro1:GuiLabel = { coords={0,0,100%,4%} textcolor=#ffd700; align=top,centerh,vertical; textalign=top,centerh; text=" SOLOMINER_INTRO_1 "; }"
            "intro2:GuiLabel = { coords={0,0,100%,4%} textcolor=#ffd700; align=top,centerh,vertical; textalign=top,centerh; text=" SOLOMINER_INTRO_2 "; }"
            "inter1:GuiLabel = { coords={0,0,100%,4%} align=top,centerh,vertical; textalign=top,centerh; text=; }"
            "body1:GuiLabel = { coords={0,0,100%,4%} align=top,centerh,vertical; textalign=top,centerh; text=" SOLOMINER_BODY_1 "; }"
            "body2:GuiLabel = { coords={0,0,100%,4%} align=top,centerh,vertical; textalign=top,centerh; text=" SOLOMINER_BODY_2 "; }"
            "body3:GuiLabel = { coords={0,0,100%,4%} align=top,centerh,vertical; textalign=top,centerh; text=" SOLOMINER_BODY_3 "; }"
            "body4:GuiLabel = { coords={0,0,100%,4%} align=top,centerh,vertical; textalign=top,centerh; text=" SOLOMINER_BODY_4 "; }"
            "body5:GuiLabel = { coords={0,0,100%,4%} textcolor=#8080EE; align=top,centerh,vertical; textalign=top,centerh; text=" SOLOMINER_BODY_5 "; }"
            "inter2:GuiLabel = { coords={0,0,100%,5%} align=top,centerh,vertical; textalign=top,centerh; text=; }"
            "donators1:GuiLabel = { coords={0,0,100%,4%} textcolor=#ffd700; align=top,centerh,vertical; textalign=top,centerh; text=" SOLOMINER_DONATORS_1 "; }"
            "donators2:GuiLabel = { coords={0,0,100%,4%} align=top,centerh,vertical; textalign=top,centerh; text=" SOLOMINER_DONATORS_2 "; }"
            "donators3:GuiLabel = { coords={0,0,100%,4%} align=top,centerh,vertical; textalign=top,centerh; text=" SOLOMINER_DONATORS_3 "; }"
            "inter3:GuiLabel = { coords={0,0,100%,8%} align=top,centerh,vertical; textalign=top,centerh; text=; }"
            "footer1:GuiLabel = { coords={0,0,100%,6%} align=top,centerh,vertical; textalign=top,centerh; text=" SOLOMINER_FOOTER_1 "; }"
            "close:GuiButton = { commandId=19; coords={42%,90%,58%,95%} align=none; text=Keep mining; } "
        "}"
    );

    getControlAs_<GuiButton>("close")->Subscribe(*this);
}

void UiMainSettings::onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) {
    GuiDialog::onCommand( source ,commandId ,param ,params ,extra );
}

//////////////////////////////////////////////////////////////////////////////
//! Definitions

#define COMMANDID_EARNINGSDIALOG    1001
#define COMMANDID_TRADEDIALOG       1002
#define COMMANDID_SETTINGSDIALOG    1044
#define COMMANDID_DELETEOK          1045

//////////////////////////////////////////////////////////////////////////////
char periodLabelChar( ValuePeriod period ) {
    static const char labels[] { //TODO sync with internationalisation
        's' ,'m' ,'h' ,'d' ,'w' ,'M' ,'Y'
    };

    assert( period <= ValuePeriod::PerYear );

    return labels[ CLAMP( period ,ValuePeriod::PerSecond ,ValuePeriod::PerYear ) ];
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Header

UiHeader::UiHeader( CDashboardWindow &parent ) :
    m_parent(parent) ,m_totalEarnings(NullPtr)
{
    setRoot( parent );

    setPropertiesWithString(
        "align=top; anchor=vertical; coords={0,0,100%,122} background=#181818; controls={"
            "banner:GuiImageBox = { image=header; align=none; coords={48,0,628,100%} }"
            "settings:GuiImageBox = { commandId=1044; bind=root; image=icons; align=top,left; coords={0,0,48,48} thumbid=7; }"
            "version:GuiLabel = { text=version 2.0.1021 (BETA 2); align=none; textalign=top,left; coords={60,80%,600,100%} background=0; }"
            "earnlabel:GuiLabel = { text=earnings; textalign=right; align=top,right; anchor=vertical; textalign=center; coords={0,0,25%,20%} background=0; }"
            "currency:GuiLabel = { text=USD; font=huge; textalign=center; align=right,bottom; anchor=horizontal; coords={0,0,8%,80%} background=0; }"
            "incomes:GuiLink = { commandId=1001; bind=root; text=0.00; font=huge; textalign=right+centerv; align=right,bottom; anchor=horizontal; coords={0,0,25%,80%} background=0; }"
        "}"
    );

    m_totalEarnings = getControlAs_<GuiLink>("incomes");

    updateTotalIncome();
}

void UiHeader::updateTotalIncome() {
    double total = m_parent.connections().getEarningSums().totalIncome;

    String s;

    toString( total ,s );

    SAFECALL(m_totalEarnings)->text() = s;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Footer

UiFooter::UiFooter( CDashboardWindow &parent ) :
    m_parent(parent)
{
    setRoot( parent );

    setPropertiesWithString(
        "align=bottom,vertical; coords={0,0,100%,61} background=#181818; controls={"
            "trade:GuiLink = { commandId=1002; bind=root; align=right,centerv,horizontal; coords={0,0,25%,100%} background=0; text=trade...; font=large; textalign=right+centerv; }"
        "}"
    );
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Connection

#define COMMAND_BALANCE     1001
#define COMMAND_EARNINGS    1002
#define COMMAND_CONNECTION  1005
#define COMMAND_WALLETSTART 1006
#define COMMAND_MINING      1003
#define COMMAND_AUTO        1004

#define CONNECTION_REFRESH_DELAY    (10000) //TODO TEST // (10*1000)
#define CONNECTION_RETRY_DELAY      (60000)

struct UiConnection : public GuiGroup ,public IMinerListener {
///-- members
    CDashboardWindow &m_parent;
    CConnection &m_connection;
    uint32_t m_refreshTimer;
    uint32_t m_retryTimer;

///-- controls
    GuiImageBox coinThumbnail;
    GuiImageBox tradeIcon;
    GuiImageBox tradeThumbnail;

    // stats
    GuiLabel hpsLabel;
    GuiLabel luckLabel;
    UiStats stats;
    GuiLabel miningLabel;

    // pool
    GuiImageBox poolIcon;
    GuiLink poolLabel;

    // action
    GuiCheckBox actionAuto;
    GuiButton actionButton;

    // funds
    GuiLink balanceLabel;
    GuiLink earningLabel;

///-- MessageBox
    GuiMessageBox m_startWalletBox;
    GuiMessageBox m_noMiningAddressBox;

///-- instance
    UiConnection( CDashboardWindow &parent ,CConnection &connection ) :
        m_parent(parent) ,m_connection(connection)
        ,miningLabel( "" ,(TextAlign) (textalignCenterH | textalignCenterV) ,&getFontSmall() )
        ,poolLabel( "disconnected" ,this )
        ,poolIcon( NullPtr ,"POOL" )
        ,actionButton( "Start" ,this )
        ,balanceLabel( "..." ,this )
        ,earningLabel( "..." ,this )
        ,m_startWalletBox( *this ,"Core" ,"Would like to start core or configure it first?" ,{  {"Cancel","3"} ,{"Configure","1005"} ,{"Start","1006"} } )
        ,m_noMiningAddressBox( *this ,"Core" ,"No mining address was set for this coin, do you want to configure it?" ,{ {"No","3"} ,{"Configure","1005"} } )
    {
        setRoot(parent);
        // root().addBinding( "self" ,this );

        Params vars;

        toString( m_connection.getIndex() ,vars["index"] );

        setPropertiesWithString( "controls={"
                "tools:GuiGroup = { align=right,horizontal; coords={0,0,32,100%} controls= {"
                    "grab:GuiImageBox = { commandId=27,${index}; bind=list; image=minis; align=top,right,vertical; coords={0,0,32,32} thumbid=25; }"
                    "edit:GuiImageBox = { commandId=29,${index}; bind=list; image=minis; align=top,right,vertical; coords={0,0,32,32} thumbid=7; }"
                    "delete:GuiImageBox = { commandId=30,${index}; bind=list; image=minis; align=top,right,vertical; coords={0,0,32,32} thumbid=8; }"
                "}"
            "}"
            ,vars
        );

        connection.setMinerListener( this );
        m_refreshTimer = parent.addTimer( *this ,CONNECTION_REFRESH_DELAY );
        m_retryTimer  = parent.addTimer( *this ,CONNECTION_RETRY_DELAY );

        coords() = { 0 ,0 ,100.f ,UI_CONNECTION_HEIGHT  };
        align() = (GuiAlign) (GuiAlign::alignTop | GuiAlign::alignAnchor);
        // colors().fillColor = OS_COLOR_ORANGE; // get from theme

        const char *coinName = connection.info().mineCoin.coin.c_str();
        const char *tradeName = connection.info().tradeCoin.coin.c_str();
        const char *poolName = connection.info().pool.c_str();

        const float balanceLeftPos = 100.f - UI_BALANCE_WIDTH;
        const float actionLeftPos = balanceLeftPos - UI_ACTION_WIDTH;
        const float poolLeftPos = actionLeftPos - UI_POOL_WIDTH;
        int statLeftPos = (int) (UI_CONNECTION_HEIGHT*0.8f);

        { //! coin icon
            coinThumbnail.commandId() = GUI_MESSAGEID_EDIT;
            coinThumbnail.Subscribe( *this );
            coinThumbnail.setImage( getAssetCoinImage( coinName ) );
            coinThumbnail.text() = coinName;
            coinThumbnail.align() = (GuiAlign) (alignLeft | alignAnchor);
            coinThumbnail.coords() =  { 0 ,0 ,(int) (UI_CONNECTION_HEIGHT*0.8f) ,90.f  };
            addControl( coinThumbnail );

            if( m_connection.hasTrade() ) {
                tradeIcon.setImage( getAssetImage( "EXCHANGE" ) );
                tradeIcon.align() = (GuiAlign) (alignLeft | alignAnchor);
                tradeIcon.coords() = { 0 ,0 ,12 ,90.f  };
                addControl( tradeIcon );

                statLeftPos += 16;

                tradeThumbnail.setImage( getAssetCoinImage( tradeName ) );
                tradeThumbnail.text() = tradeName;
                tradeThumbnail.align() = (GuiAlign) (alignLeft | alignAnchor);
                tradeThumbnail.coords() =  { 0 ,0 ,(int) (UI_CONNECTION_HEIGHT*0.8f) ,90.f  };
                addControl( tradeThumbnail );

                statLeftPos += (int) (UI_CONNECTION_HEIGHT*0.8f);
            }
        }

        { //! stats
            stats.colors().fillColor = OS_RGB(24,24,80);
            stats.setBarImage( &getAssetImage( UIIMAGE_BAR_GREEN ) );
            stats.coords() = { statLeftPos ,15.f ,poolLeftPos ,75.f };
            addControl( stats );

            miningLabel.colors().textColor = OS_COLOR_GREEN;
            miningLabel.coords() = { statLeftPos ,75.f ,poolLeftPos ,100.f };
            addControl( miningLabel );

            luckLabel.coords() = { statLeftPos ,0.f ,poolLeftPos ,15.f };
            luckLabel.textAlign() = (TextAlign) (textalignRight | textalignCenterV);
            luckLabel.colors().textColor = OS_COLOR_OLIVE;
            luckLabel.text() = "luck";
            addControl( luckLabel );

            hpsLabel.coords() = { statLeftPos ,0.f ,poolLeftPos/2 ,15.f };
            hpsLabel.textAlign() = textalignCenterV;
            hpsLabel.colors().textColor = OS_COLOR_OLIVE;
            hpsLabel.text() = "hps";
            addControl( hpsLabel );
        }

        { //! pool
            poolIcon.setImage( getAssetPoolImage( poolName ) );
            poolIcon.setText( poolName );
            poolIcon.coords() = { poolLeftPos ,10.f ,actionLeftPos,70.f };
            addControl( poolIcon );

            poolLabel.setPropertiesWithString( "text=disconnected; textalign=center; font=normal;" );
            poolLabel.colors().textColor = OS_COLOR_GRAY;
            poolLabel.setCommandId( COMMAND_CONNECTION );
            poolLabel.coords() = { poolLeftPos ,70.f ,actionLeftPos ,100.f };
            addControl( poolLabel );
        }

        { //! actions
            miningCommand = miningStart;

            actionAuto.setCommandId( COMMAND_AUTO );
            actionAuto.Subscribe(*this);
            actionAuto.text() = "AUTO";
            actionAuto.checked() = connection.info().status.isAuto;
            actionAuto.coords() = { actionLeftPos ,0.f ,balanceLeftPos ,25.f };
            // actionAuto.align() = (GuiAlign) (GuiAlign::alignLeft | GuiAlign::alignTop);
            addControl( actionAuto );

            actionButton.setCommandId( COMMAND_MINING );
            actionButton.coords() = { actionLeftPos ,25.f ,balanceLeftPos ,75.f };
            actionButton.text() = getMiningCommandText();
            updateActionButton();

            addControl( actionButton );
        }

        { //! earnings

            ///-- balance
            balanceLabel.setCommandId( COMMAND_BALANCE );
            balanceLabel.textAlign() = (TextAlign) (textalignRight | textalignCenterV);
            balanceLabel.setFont( getFontLarge() );
            balanceLabel.coords() = { balanceLeftPos ,0 ,97.f ,80.f };
            balanceLabel.hooverColor() = OS_RGB(160,160,250);
            addControl( balanceLabel );

            ///-- earning
            defaultValueReference( m_earningReference );
            earningLabel.setCommandId( COMMAND_EARNINGS );
            earningLabel.textAlign() = (TextAlign) (textalignRight | textalignCenterV);
            earningLabel.setFont( getFontSmall() );
            earningLabel.coords() = { balanceLeftPos ,60.f ,97.f ,100.f };
            earningLabel.hooverColor() = OS_RGB(160,160,250);
            addControl( earningLabel );
        }
    }

    CDashboardWindow &parent() { return m_parent; }

///-- shorthands
    const char *mineCoinName() {
        return tocstr( m_connection.info().mineCoin.coin );
    }

    const char *mineCoinAddress() {
        return tocstr( m_connection.info().mineCoin.address );
    }

    const char *tradeCoinName() {
        return tocstr( m_connection.info().tradeCoin.coin );
    }

///-- functions
    enum ValueReference {
        MiningBlock=0 ,MiningCoin ,TradeCoin ,GlobalRef ,WalletCoin
    };

    void defaultValueReference( ValueReference &reference ) {
        reference = m_connection.hasTrade() ? ValueReference::TradeCoin : ValueReference::MiningCoin;
    }

    void cycleValueReference( ValueReference &reference ) {
        reference = (ValueReference) (reference+1);

        if( reference == TradeCoin && !m_connection.hasTrade() )
            reference = (ValueReference) (reference+1); //! skip trade if no trade

        reference = (ValueReference) (reference % (WalletCoin+1) );
    }

    void getValueReference( ValueReference reference ,ValueOfReference &value ) {
        switch( reference ) {
            case ValueReference::MiningBlock :
                value = { ValueOfReference::Type::Block ,"blocks" };
                break;
            case ValueReference::MiningCoin :
                value = { ValueOfReference::Type::Coin ,mineCoinName() };
                break;
            case ValueReference::TradeCoin :
                value = { ValueOfReference::Type::Coin ,tradeCoinName() };
                break;
            case ValueReference::GlobalRef :
                value = { ValueOfReference::Type::Fiat ,"usd" };
                break;
            case ValueReference::WalletCoin :
                value = { ValueOfReference::Type::Coin ,"BTC" };
                break;
        }
    }

///-- childs

    //-- stats
    uint64_t m_lastHashes = 0;

    void startStats() {
        time_t now; time ( &now );

        stats.stats().Start( (int) (now/60) );
    }

    void recordStats( uint64_t hashes ) {
        time_t now; time( &now );

        if( m_lastHashes >= hashes ) {
            hashes = m_lastHashes;
            return;
        }

        uint64_t d = hashes - m_lastHashes;
        stats.stats().Record( (double) d ,(int) (now/60) );
        m_lastHashes = hashes;

        m_connection.connectionList().registerHps( PowAlgorithm::GhostRider ,stats.stats().getAvg() / 60. );

        updateStatHpsLabel();
    }

    //-- command
    enum MiningCommand {
        miningNone=0 ,miningStart ,miningStop
    } miningCommand;

    const char *getMiningCommandText() const {
        switch( miningCommand ) {
            default:
            case miningStart: return "Start";
            case miningStop: return "Stop";
        }
    }

    bool getMiningCommandEnabled() const {
        return miningCommand != miningNone;
    }

    //-- balance
    String &makeProgressText( String &s ,const char *base ) const {
        s = progressText + base; return s;
    }

    void setWalletBalanceProgressText( const char *base ) {
        makeProgressText( balanceLabel.text() ,base );
    }

    void updateStatHpsLabel() {
        double hps = m_connection.connectionList().getHostHps( PowAlgorithm::GhostRider );

        if( hps > 10000. ) {
            hps = round( hps / 10. ) / 100.;
            Format( hpsLabel.text() ,"%.2f kp/s" ,64 ,(float) hps );
        } else if( hps > 100. ) {
            hps = round( hps );
            Format( hpsLabel.text() ,"%d p/s" ,64 ,(int) hps );
        } else {
            Format( hpsLabel.text() ,"%.2f p/s" ,64 ,(float) hps );
        }

        //-- luck
        double luck = getBlockLuck();

        Format( luckLabel.text() ,"luck %d%%" ,64 ,(int) round(luck) );
    }

    void updateWalletBalanceText() {
        auto pwallet = m_connection.coinWallet();

        if( !pwallet ) {
            balanceLabel.setText( "no wallet" );
            return;

        } else {
            String label;

            switch( pwallet->state() ) {
                default:
                case serviceStopped:
                    balanceLabel.setText( "not started" ); return;
                case serviceStarted:
                    balanceLabel.setText( "not connected" ); return;

                case serviceStarting:
                    setWalletBalanceProgressText( "starting" ); return;
                case serviceConnecting:
                    setWalletBalanceProgressText( "connecting" ); return;
                case serviceStopping:
                    setWalletBalanceProgressText( "stopping" ); return;

                case serviceConnected:
                    break; //! continues below
            }
        }

        if( poolLabel.text() == "disconnected" ) {
            poolLabel.setText( "idle" );
        }

        AmountValue balance = { 0 ,"" };

        pwallet->getAddressBalance( mineCoinAddress() ,balance );

        std::stringstream ss;
        ss << balance.amount << " " << balance.value;
        balanceLabel.text() = ss.str();
    }

    //-- earnings
    ValueReference m_earningReference;

    void updateEstimateEarningText() {
        const double hostHPS = m_connection.connectionList().getHostHps( PowAlgorithm::GhostRider );

        updateStatHpsLabel();

        ValueOfReference value;

        getValueReference( m_earningReference ,value );

        ValuePeriod period = ValuePeriod::PerDay;

        double earnings = m_connection.estimateEarnings( hostHPS ,value ,period ); //+ reference currency

        // scalePeriod( period ,earnings );
        char periodLabel = periodLabelChar( period );

        std::stringstream ss2;
        ss2 << earnings << " " << value.currency << "/" << periodLabel;
        earningLabel.text() = ss2.str();
    }

    void startWalletService() {
        m_connection.startWalletService();
    }

    void configureWalletService() {
        parent().showCoreSettings( m_connection.info().mineCoin.coin.c_str() ,m_connection );
    }

///-- controls
    void onClickEarningLabel() {
        cycleValueReference( m_earningReference );
        updateEstimateEarningText();
        root().Refresh();
    }

    void onClickBalanceLabel() {
        auto pwallet = m_connection.coinWallet();

        if( pwallet && pwallet->state() == serviceStarted ) {
            //! @note if wallet service not started propose the obvious start of service

            root().ShowMessageBox( m_startWalletBox );
        }
        else {
            //! @note else, show config options
            configureWalletService();
        }
    }

    void onClickConnectionLabel() {
        configureWalletService();
    }

///-- mining
    void onMiningAuto( bool isAuto ) {
        m_connection.info().status.isAuto = isAuto;
    }

    void updateActionButton() {
        if( miningCommand == miningStart ) {
            actionButton.colorSet().getColors( highlightHoover ).foreColor = OS_RGB(64,128,64);
            actionButton.colorSet().getColors( highlightHoover ).fillColor = OS_RGB(64,128,64);
            actionButton.colorSet().getColors( highlightHoover ).backColor = OS_RGB(64,128,64);
        } else if( miningCommand == miningStop ) {
            actionButton.colorSet().getColors( highlightHoover ).foreColor = OS_RGB(128,64,64);
            actionButton.colorSet().getColors( highlightHoover ).fillColor = OS_RGB(128,64,64);
            actionButton.colorSet().getColors( highlightHoover ).backColor = OS_RGB(128,64,64);
        }

        //-- text
        const char *text = getMiningCommandText();

        if( text && text[0] )
            actionButton.text() = text;

        actionButton.setState( getMiningCommandEnabled() ? highlightNormal : highlightDisabled );
    }

    void onMiningStatus( IMiner &miner ,MinerInfo::WorkState state ,MinerInfo::WorkState oldState ) {
        switch( state ) {
            default:
                poolLabel.colors().textColor = OS_COLOR_GRAY;
                poolLabel.setText( "waiting for miner..." );
                miningCommand = miningNone;
                break;
            case MinerInfo::WorkState::stateIdle:
                poolLabel.colors().textColor = OS_COLOR_GRAY;
                poolLabel.setText( "idle" );
                miningCommand = miningStart;
                break;
            case MinerInfo::WorkState::stateConnected:
                poolLabel.colors().textColor = OS_COLOR_WHITE;
                poolLabel.setText( "connected" );
                miningCommand = miningNone;
                break;
            case MinerInfo::WorkState::stateMining:
                poolLabel.colors().textColor = OS_COLOR_OLIVE;
                poolLabel.setText( "mining" );
                miningCommand = miningStop;
                break;
            case MinerInfo::WorkState::statePaused:
                poolLabel.colors().textColor = OS_COLOR_ORANGE;
                poolLabel.setText( "paused" );
                miningCommand = miningNone;
                break;
        }

        updateActionButton();
    }

    //TODO block or share function herebelow also called for rejected ... handle properly
    void onShareAccepted( const MiningInfo &info ) {
        std::stringstream ss;

        ss << "shares " << info.accepted << "/" << info.accepted+info.rejected << " (" << info.elapsedMs << "ms)";
        miningLabel.text() = ss.str();
        miningLabel.colors().textColor = OS_COLOR_GREEN;

        //-- pool status
        poolLabel.text() = "share accepted";
        poolLabel.colors().textColor = OS_COLOR_GREEN;
    }

    void onPartFound( const MiningInfo &info ) {
        float parts = 100.f * (info.partial - m_partial) / 256.f;

        parts = round( parts * 100.f) / 100.f;

        std::stringstream ss;

        ss << "part " << parts << "%" << " (blocks " << info.accepted << "/" << info.accepted+info.rejected << ")";
        miningLabel.text() = ss.str();
        miningLabel.colors().textColor = OS_COLOR_OLIVE;

        //-- pool status
        poolLabel.text() = "working";
        poolLabel.colors().textColor = OS_COLOR_OLIVE;
    }

    void onBlockFound( const MiningInfo &info ) {
        std::stringstream ss;

        ss << "blocks " << info.accepted << "/" << info.accepted+info.rejected << " (" << info.elapsedMs << "ms)";
        miningLabel.text() = ss.str();
        miningLabel.colors().textColor = OS_COLOR_GREEN;

        //-- pool status
        poolLabel.text() = "Block found!";
        poolLabel.colors().textColor = OS_COLOR_GREEN;
    }

    void onRejected( const MiningInfo &info ) {
        std::stringstream ss;

        ss << "blocks " << info.accepted << "/" << info.accepted+info.rejected << " (" << info.elapsedMs << "ms)";
        miningLabel.text() = ss.str();
        miningLabel.colors().textColor = OS_COLOR_RED;

        //-- pool status
        poolLabel.text() = "Block rejected";
        poolLabel.colors().textColor = OS_COLOR_RED;
    }

///-- commands
    void onMiningStart() {
        if( m_connection.info().mineCoin.address.empty() ) {
            root().ShowMessageBox( m_noMiningAddressBox );
            return;
        }

        m_connection.Start();
        startStats();
    }

    void onMiningStop() {
        m_connection.Stop();
    }

    void onMiningCommand() {
        switch( miningCommand ) {
            default:
                break;
            case miningStart:
                onMiningStart();
                break;
            case miningStop:
                onMiningStop();
                 break;
        }
    }

///-- timers
    String progressText;

    void makeProgressText( int n ) {
        progressText.clear();

        for( int i=0; i<(n%4); ++i )
            progressText += '.';
    }

    void updateTextLabels( OsEventTime now ) {
        updateWalletBalanceText();
        updateEstimateEarningText();
        m_parent.header().updateTotalIncome();

        m_parent.Refresh();
    }

    void onRefreshTimer( OsEventTime now ,OsEventTime last ) {
        updateTextLabels(now);
    }

    int nRetry = 0;

    void onRetryTimer( OsEventTime now ,OsEventTime last ) {
        auto pwallet = m_connection.coinWallet();

        if( !pwallet ) return;

        ServiceState state = pwallet->state();

        if( serviceInProgress(state) || state==serviceStarted ) {
            Params params;

            pwallet->Retry(params);

            makeProgressText(nRetry++);
            updateTextLabels(now);
        }
    }

///-- edit
    /* void onEditConnection( int page ) {
        const int *index = parent().connections().connections().digItem( m_connection );

        assert(index);

        parent().connectionList().onEditConnection( *index );
    }

    void onDeleteConnection() {
        parent().showConfirmDelete();
    } */

///-- ICommandListener
    void onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) override {
        switch( commandId ) {
            case COMMAND_BALANCE:
                onClickBalanceLabel(); break;
            case COMMAND_EARNINGS:
                onClickEarningLabel(); break;
            case COMMAND_WALLETSTART:
                startWalletService(); break;
            case COMMAND_CONNECTION:
                onClickConnectionLabel(); break;
            case COMMAND_MINING:
                onMiningCommand(); break;
            case COMMAND_AUTO:
                onMiningAuto( (bool) param ); break;

            /* case GUI_MESSAGEID_MOVE:
                //TODO drag & drop
                break;

            case GUI_MESSAGEID_EDIT:
                onEditConnection(0);
            case GUI_MESSAGEID_DELETE:
                onDeleteConnection(0);
                */

            default:
                break;
        }
    }

///-- IMinerListener interface
    IAPI_IMPL onStatus( IMiner &miner ,MinerInfo::WorkState state ,MinerInfo::WorkState oldState ) IOVERRIDE {
        onMiningStatus( miner ,state ,oldState );

        root().Refresh();

        return IOK;
    }

    IAPI_IMPL onJob( IMiner &miner ,const MiningInfo &info ) IOVERRIDE {
        poolLabel.colors().textColor = OS_COLOR_CYAN;

        if( isMiningOnCore() ) {
            poolLabel.setText( "new block" );
        } else {
            poolLabel.setText( "new job" );
        }

        recordStats( info.hashes );
        root().Refresh();

        return IOK;
    }

    int m_rejected = 0; //! since last accepted ..
    int m_accepted = 0;
    int m_partial = 0;
    int m_progress = 0; //! current partial
    double m_luck = 0.;

    bool isMiningOnCore() {
        return m_connection.info().pool == "SOLO";
    }

    double getBlockLuck() {
        double effort = ((double) (m_progress + m_partial) / 256.) / MAX(m_accepted ,1);

        return effort > (1./256.) ? 100. / effort : 100.;

        /* if( m_accepted > 0 )
            return 100. * m_accepted / m_luck;
        else
            return 100. / ( (m_progress+1) / 256. ); */
    }

    IAPI_IMPL onResult( IMiner &miner ,const MiningInfo &info ) IOVERRIDE {
        if( m_rejected < info.rejected ) {
            onRejected(info);

            m_rejected = info.rejected;
        }

        else if( isMiningOnCore() ) {
            if( m_accepted < info.accepted ) {
                onBlockFound( info );

                m_luck += (m_partial+1) / 256.;
                m_partial = (int) info.partial;
            }
            else {
                onPartFound( info );

                m_progress = info.partial - m_partial;
            }


            m_accepted = (int) info.accepted;
        }

        else {
            onShareAccepted( info );
        }

        recordStats( info.hashes );
        root().Refresh();

        return IOK;
    }

///-- ITimerListener
    void onTimer( OsTimerAction timeAction ,OsEventTime now ,OsEventTime last ) override {
        if( timeAction == m_refreshTimer ) {
            onRefreshTimer( now ,last );
        } else if( timeAction == m_retryTimer ) {
            onRetryTimer( now ,last );
        }
    }

///-- GuiWindow events
    void onDraw( const OsRect &updateArea ) override {
        GuiGroup::onDraw( updateArea );

        //-- draw bottom line
        const OsRect &r = this->area();

        root().SetColor( OS_SELECT_FORECOLOR ,OS_RGB(0,0,128) );
        root().DrawLine( r.left ,r.bottom ,r.right ,r.bottom );
    }
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! ConnectionList

UiConnectionList::UiConnectionList( CDashboardWindow &parent ,CConnectionList &connections ) : GuiGroup(true)
    ,m_parent(parent) ,m_connections(connections) ,m_connectionWizard(parent)
{
    setRoot( parent );
    parent.addBinding( "list" ,this );

//-- connection controls
    coords() = { 0 ,0 ,100.f ,100.f  };
    align() = GuiAlign::alignFill;
    colors().fillColor = OS_COLOR_BLACK;

    String name ,s;

    for( auto &it : m_connections.connections().map() ) {
        CConnection *p = it.second.ptr();

        assert(p); if( !p ) continue; //! LOG this ... should not happen

        auto *pui = new UiConnection(parent,*p);

        int index = it.first;
        s = ""; toString( index ,s );
        name = "connection"; name += s;

        addControl( tocstr(name) ,*pui );
    }

//-- connection adder
    makeConnectionAdder();
}

void UiConnectionList::makeConnectionAdder() {
    setPropertiesWithString(
        "controls={"
            "adder:GuiImageBox = { commandId=28; bind=list; image=icons; align=top,center; coords={0,0,48,48} thumbid=56; }"
        "}"
    );
}

//-- command
void UiConnectionList::onAddConnection() {
    m_connectionWizard.showAddConnection( m_parent );
}

void UiConnectionList::onEditConnection( int index ) {
    CConnectionRef connection;

    if( m_connections.getConnection( index ,connection ) != IOK || connection.isNull() ) {
        assert(false); return;
    }

    m_editIndex = index;
    m_connectionWizard.showEditConnection( m_parent ,index ,connection->info() );
}

void UiConnectionList::onMoveConnection( int previousIndex ,int newIndex ) {

}

void UiConnectionList::onDeleteConnection( int index ) {
    m_editIndex = index;
    m_parent.showConfirmDelete( index );
}

//-- callback
void UiConnectionList::onAddConnectionOk( Params &settings ) {
    CConnectionRef ref;

    m_connections.addConnection( settings ,ref );
    m_connections.saveConfig();

    assert( ref ); if( !ref ) return; //! LOG this ... should not happen

    auto *pui = new UiConnection( m_parent ,ref.get() );

    removeControl("adder");

    addControl( *pui );

    makeConnectionAdder();
}

void UiConnectionList::onEditConnectionOk( int index ,Params &settings ) {
    m_connections.editConnection( m_editIndex ,settings );
    m_connections.saveConfig();
}

void UiConnectionList::onDeleteConnectionOk( int index ) {
    index = m_editIndex; //! @note fix for missing index from MessageBox

    String name ,s;

    s = ""; toString( index ,s );
    name = "connection"; name += s;

    removeControl( tocstr(name) );

    m_connections.deleteConnection( index );
    m_connections.saveConfig(); //TODO HERE need to remove missing section in saveConfig
}

//--
void UiConnectionList::onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) {
    switch( commandId ) {
        case GUI_MESSAGEID_ADD:
            onAddConnection();
            break;
        case GUI_MESSAGEID_EDIT:
            onEditConnection( (int) param );
            break;
        case GUI_MESSAGEID_DELETE:
            onDeleteConnection( (int) param );
            break;

        case GUI_MESSAGEID_OK:
            if( param < 0 )
                onAddConnectionOk( *params );
            else
                onEditConnectionOk( (int) param ,*params );
            break;

        default:
            assert(false);
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Dashboard

CDashboardWindow::CDashboardWindow( CConnectionList &connections ) :
    GuiControlWindow( SOLOMINER_UI_MAINNAME ,"SOLOMINER2 - Dashboard" ,1280 ,800 )
    ,m_connections(connections)
    ,m_uiHeader(*this)
    ,m_uiFooter(*this)
    ,m_uiConnectionList(*this,connections)
    ,m_uiEarningsDialog( connections.earnings() )
    ,m_uiTradeDialog(*this)
    ,m_confirmDelete( *this ,"Delete Connection" ,"Are your sure you want to delete this connection?" ,{ {"No" ,"3"} ,{"Yes","1045"} } )
{
    foreground().addControl( m_uiHeader );
    foreground().addControl( m_uiFooter );
    foreground().addControl( m_uiConnectionList );
}

//--
void CDashboardWindow::onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) {

    //-- connection
    if( params && extra && strimatch( "connection" ,(const char*) extra ) == 0 ) {
        m_uiConnectionList.onCommand( source ,commandId ,param ,params ,extra );
        return;
    }

    //-- local
    switch( commandId ) {
        case COMMANDID_SETTINGSDIALOG:
            showMainSettings(); break;
        case COMMANDID_EARNINGSDIALOG:
            showEarningsDialog(); break;
        case COMMANDID_TRADEDIALOG:
            showTradeDialog(); break;

        case COMMANDID_DELETEOK:
            m_uiConnectionList.onDeleteConnectionOk( (int) param );
            break;

        default:
            GuiControlWindow::onCommand( source ,commandId ,param ,params ,extra );
            break;
    }
}

void CDashboardWindow::onNotify( IObject *source ,messageid_t notifyId ,long param ,Params *params ,void *extra ) {
    GuiControlWindow::onNotify( source ,notifyId ,param ,params ,extra );
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF