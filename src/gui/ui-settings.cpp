// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include <common/stats.h>
#include <coins/cores.h>
#include <pools/pools.h>
#include <algo/crypth.h>

#include "ui.h"
#include "ui-settings.h"

#include <algo/base58/base58.h>
#include <algo/sha/sha256.h>

#include <cmath>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

extern Config &getServiceConfig(); //! TODO find a proper place to put this declaration

void testCryptH() {
    const char *password = "123";
    const char *text = "Hello world! This is a long sentence :)";
    String s ,r;

    CryptH( password ,text ,s );
    printf( "%s\n" ,tocstr(s) );

    DecryptH( password ,tocstr(s) ,r );
    printf( "%s\n" ,tocstr(r) );
}

//////////////////////////////////////////////////////////////////////////////
//! Custom controls

#define TINY_UITITLE_PUID   0x03f8c3d023f411738

struct UiTitle : GuiGroup {
    UiTitle() {
        setPropertiesWithString(
            "controls={"
                "line:GuiShape = { align=centerv; coords={0,0,100%,1} shape=line; color=#e0e0e0; }"
                "label:GuiLabel = { align=center; coords={40%,0,60%,100%} background=#101010; text=title; textalign=center; }" // backcolor=#e01010; }"
            "}"
        );
    }

    DECLARE_GUICONTROL(GuiGroup,UiTitle,TINY_UITITLE_PUID);

protected:
    void onDraw( const OsRect &updateArea ) override;
};

REGISTER_CLASS(UiTitle)

void UiTitle::onDraw( const OsRect &updateArea ) {
    GuiGroup::onDraw( updateArea );
}

//////////////////////////////////////////////////////////////////////////////
//! UiCoreSettings

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
//! DataSource

bool CoreDataSource::readValues( const char *id ) {
    int x=0;

    fromString( x ,id );

    String entry = id;

    int i=0; for( auto &it : m_section->params ) {
        //! @note this is inefficient but we should only have a few entries

        if( i==x ) {
            entry = it.first; break;
        }

        ++i;
    }

    m_entry = entry;

    if( !readValues() ) return false;

    m_values["id"] = entry;

    return true;
}

bool CoreDataSource::readValues() {
    if( !ConfigDataSource::readValues() ) return false;

    if( !globalIsLogged() ) return false;

    String s;

    globalDecypher( tocstr(m_values["wallet_password"]) ,s );
    m_values["wallet_password"] = s;

    return true;
}

bool CoreDataSource::writeValues() {
    String s;

    Params cypher = m_values;

    globalCypher( tocstr(m_values["wallet_password"]) ,cypher["wallet_password"] );

    toString( cypher ,s );
    m_section->params[ m_entry ] = s;

    return m_config.commitSection( *m_section ) && m_config.SaveFile();
}

//////////////////////////////////////////////////////////////////////////////
//! MarketDataSource

bool MarketDataSource::readValues() {
    auto it = m_section->params.find( m_entry );
    if( it == m_section->params.end() ) return false;

    if( !globalIsLogged() ) return false;

    Params cypher;

    fromString( cypher ,it->second );

    String &api = m_values["api_key"];
    globalDecypher( tocstr(cypher["api_key"]) ,api );
    String &secret = m_values["api_secret"];
    globalDecypher( tocstr(cypher["api_secret"]) ,secret );

    m_haveEdit = false;

    return true;
}

bool MarketDataSource::writeValues() {
    String s;

    Params cypher = m_values;

    globalCypher( tocstr(m_values["api_key"]) ,cypher["api_key"] );
    globalCypher( tocstr(m_values["api_secret"]) ,cypher["api_secret"] );

    toString( cypher ,s );
    m_section->params[ m_entry ] = s;

    return m_config.commitSection( *m_section ) && m_config.SaveFile();
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

UiMainSettings::UiMainSettings( GuiControlWindow *root ) :
    GuiDialog( "Settings" )
    ,m_passwordDialog(root)
    ,m_coreCredential( getCredentialConfig() ,"CORES" )
    ,m_marketCredential( getCredentialConfig() ,"MARKETS" )
    ,m_editing(false)
{
    if( root ) {
        setRoot(*root);
        root->addBinding("dialog",this);
        root->addBinding("core",&m_coreCredential);
        root->addBinding("market",&m_marketCredential);
    }

    setPropertiesWithString(
        "controls={"
            "tabs:GuiTabBar = {"
                "align=top,left,horizontal; coords={0,0,12%,100%} background=#181818; direction=bottom; titles={About,Settings,Credential}"
            "}"
            "body:GuiTab = { align=top,left; coords={0,0,88%,100%} controls={"
                "about:GuiGroup = { background=#FF000000; textcolor=#FF808080; controls={"
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
                    "close:GuiButton = { commandId=19; bind=dialog; coords={42%,90%,58%,95%} align=none; text=Keep mining; } "
                "}}"
                "settings:GuiGroup={ controls={"
                    "globalTitle:UiTitle = { align=top,vertical; coords={15%,0,85%,10%} /label={text=Global;} }"
                    "workerLabel:GuiLabel = { align=top; coords={25%,0,35%,6%} text=Worker Name; textalign=left,centerv; }"
                    "worker:GuiTextBox = { align=top,vertical; coords={40%,0,65%,5%} text=; }"

                    "tradeTitle:UiTitle = { align=top,vertical; coords={15%,0,85%,10%} /label={text=Trade;} }"
                    "scheduleLabel:GuiLabel = { align=top; coords={25%,0,35%,6%} text=Trader schedule; textalign=left,centerv; }"
                    "schedule:GuiTextBox = { align=top,vertical; coords={40%,0,65%,5%} text=; }"
                    //+ minimum amount
                "}}"
                "credential:GuiGroup={ controls={"
                    "generalTitle:UiTitle = { align=top,vertical; coords={15%,0,85%,10%} /label={text=General;} }"
                    "lblPassword:GuiLabel = { align=top; coords={25%,0,35%,6%} text=Password; textalign=left,centerv; }"
                    "password:GuiTextBox = { align=top; enabled=false; coords={40%,0,65%,5%} text=; }"
                    "setpassword:GuiButton = { commandId=29,0; bind=dialog; align=top,vertical; coords={66%,0,70%,5%} text=set; }"

                    "coreTitle:UiTitle = { align=top,vertical; coords={15%,0,85%,10%} /label={text=Cores;} }"
                    "trades:GuiNavGrid = { align=top,vertical; coords={25%,0,75%,25%} controls={ "
                        "grid:GuiGrid = { datasource=core; "
                            "cols={30%,70%}"
                            "titles={Core,Wallet Password}"
                            "fields={id,wallet_password}"
                            "rows=3;"
                            "selectable=true;"
                        "}"
                        "nav = { visible=false; }"
                    "} }"

                    "marketTitle:UiTitle = { align=top,vertical; coords={15%,0,85%,10%} /label={text=Markets;} }"
                    "lblApiKey:GuiLabel = { align=top; coords={25%,0,50%,6%} text=api-key; textalign=left,centerv; }"
                    "apikey:GuiTextBox = { align=top,vertical; coords={35%,0,85%,5%} text=; datafield=api_key; datasource=market; }"
                    "lblApiSecret:GuiLabel = { align=top; coords={25%,0,35%,6%} text=api-secret; textalign=left,centerv; }"
                    "apisecret:GuiTextBox = { align=top,vertical; coords={35%,0,85%,5%} text=; datafield=api_secret; datasource=market; }"

                    "cancel:GuiButton = { commandId=3; bind=dialog; enabled=false; coords={40%,85%,49%,90%} align=none; text=Cancel; } "
                    "confirm:GuiButton = { commandId=2; bind=dialog; enabled=false; coords={51%,85%,60%,90%} align=none; text=Ok; } "
                "}}"
            "}}"
        "}"
    );

    //TODO .. can bind any control above in hierarchy ?

    m_passwordDialog.Subscribe(*this);

    // testCryptH();

//-- tab
    auto &tab = * getControlAs_<GuiTab>("body");

    GuiTabBar *tabbar = getControlAs_<GuiTabBar>("tabs");

    if( tabbar ) tabbar->Bind( tab );

//-- global
    auto &global = getCredentialConfig().getSection("GLOBAL");

    const char *current = getMember( global.params ,"password" ,"" );

    if( *current ) {
        setPropertiesWithString( "/body = { /credential = { /password = { text=*******; }}} " );
    }

    m_passwordDialog.values()["current"] = current;
    m_passwordDialog.values()["allow-empty"] = "true";

//-- cores
    auto *cores = getControlAs_<GuiNavGrid>("body/credential/trades");

    cores->Grid()->Subscribe(*this);

//-- market
    m_marketCredential.Seek("xeggex");
    m_marketCredential.Update();

//-- data
    m_coreCredential.Subscribe(*this);
    m_marketCredential.Subscribe(*this);

    updateControlState(false);
}

//--
void UiMainSettings::Open() {
    m_coreCredential.Update();
    m_marketCredential.Update();

    auto *cores = getControlAs_<GuiNavGrid>("body/credential/trades");

    cores->Grid()->UpdateData();
}

void UiMainSettings::showGlobalPassword() {
    auto &global = getCredentialConfig().getSection("GLOBAL");

    m_passwordDialog.values()["current"] = getMember( global.params ,"password" ,"" );
    m_passwordDialog.values()["confirm"] = "true";
    m_passwordDialog.values()["id"] = "global";

    root().ShowModal( m_passwordDialog );
}

void UiMainSettings::showCorePassword( int i ) {
    m_passwordDialog.values()["current"] = "";
    m_passwordDialog.values()["confirm"] = "false";

    toString( i ,m_passwordDialog.values()["id"] );

    root().ShowModal( m_passwordDialog );
}

//--
void UiMainSettings::onGlobalPasswordSet() {
    auto &global = getCredentialConfig().getSection("GLOBAL");
    auto &password = m_passwordDialog.values()["password"];

    String crypt;

    if( !password.empty() ) {
        CryptH( tocstr(password) ,tocstr(password) ,crypt );
        setPropertiesWithString( "/body = { /credential = { /password = { text=*******; }}} " ); //! visual
    } else {
        setPropertiesWithString( "/body = { /credential = { /password = { text=; }}} " ); //! visual
    }

    global.params["password"] = crypt;
    getCredentialConfig().commitSection( global );
    getCredentialConfig().SaveFile();

//-- rewrite all entries (new password)
    Params data;

    m_coreCredential.onDataEdit(data);
    m_marketCredential.onDataEdit(data);

    onEditConfirm(); //! update
}

void UiMainSettings::onCorePasswordSet( const char *id ) {
    if( m_coreCredential.Seek( id ) != IOK ) {
        return; //TODO log this
    }

    Params data = {{"wallet_password",m_passwordDialog.values()["password"]}};

    m_coreCredential.onDataEdit( data );
    m_coreCredential.Commit();

    auto *cores = getControlAs_<GuiNavGrid>("body/credential/trades");

    SAFECALL(cores)->updatePage(0);
}

void UiMainSettings::onPasswordSet() {
    bool set = false;

    if( !fromString( set ,m_passwordDialog.values()["set"]) ) return;

    if( m_passwordDialog.values()["id"] == "global" ) {
        onGlobalPasswordSet();
    } else {
        onCorePasswordSet( tocstr(m_passwordDialog.values()["id"]) );
    }
}

void UiMainSettings::onEditConfirm() {
    m_coreCredential.Commit();
    m_marketCredential.Commit();

    updateControlState( m_editing=false );

    Refresh();
}

void UiMainSettings::onEditCancel() {
    m_coreCredential.Update();
    m_marketCredential.Update();

    updateControlState( m_editing=false );

    Refresh();
}

void UiMainSettings::updateControlState( bool editing ) {
    getControlAs_<GuiButton>("body/credential/cancel")->setState( editing ? highlightNormal : highlightDisabled );
    getControlAs_<GuiButton>("body/credential/confirm")->setState( editing ? highlightNormal : highlightDisabled );
}

//--
IAPI_DEF UiMainSettings::onDataCommit( IDataSource &source ,Params &data ) {
    if( !m_editing ) return IOK;

    updateControlState( m_editing=false );
    Refresh();

    return IOK;
}

IAPI_DEF UiMainSettings::onDataChanged( IDataSource &source ,const Params &data ) {
    if( m_editing ) return IOK;

    Params info = {{"haveedit","false"}};

    source.getInfo(info);

    if( fromString(m_editing,getMember(info,"haveedit")) ) {
        updateControlState( m_editing=true );
        Refresh();
    }

    return IOK;
}

//--
void UiMainSettings::onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) {
    switch( commandId ) {
        case GUI_MESSAGEID_EDIT:
            showGlobalPassword();
            break;

        case GUI_MESSAGEID_OK:
            onEditConfirm();
            break;
        case GUI_MESSAGEID_CANCEL:
            onEditCancel();
            break;


        default:
            GuiDialog::onCommand( source ,commandId ,param ,params ,extra );
            break;
    }
}

void UiMainSettings::onNotify( IObject *source ,messageid_t notifyId ,long param ,Params *params ,void *extra ) {
    switch( notifyId ) {
        case GUI_MESSAGEID_CLOSE:
            if( *source == m_passwordDialog ) {
                onPasswordSet();
            }
            break;
        case GUI_MESSAGEID_EDIT:
            showCorePassword( (int) param );
            break;

        default:
            GuiControl::onNotify( source ,notifyId ,param ,params ,extra );
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF