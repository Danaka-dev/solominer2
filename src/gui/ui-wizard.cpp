// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include "ui.h"
#include "ui-theme.h"
#include "ui-assets.h"
#include "ui-wizard.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! UiWizardDialog

UiWizardDialog::UiWizardDialog( GuiControlWindow &parent ) :
    GuiDialog( "Connection Wizard" )
{
    setRoot( parent );
    parent.addBinding( "dialog" ,this );

    addControl( "header" ,m_header );
    addControl( "footer" ,m_footer );
    addControl( "body" ,m_body );

    setPropertiesWithString(
        "controls = {"
            "header = { background=#101010; coords={0,0,100%,10%} align=top,vertical; }"
            "footer = { background=#101010; coords={0,0,100%,5%} align=bottom,vertical; controls={"
                "prev:GuiLink = { bind=dialog; commandId=22; align=left,horizontal; coords={0,5%,5%,95%} text=<<; textalign=center; font=large; }"
                "next:GuiLink = { bind=dialog; commandId=23; align=right,horizontal; coords={0,5%,5%,95%} text=>>; textalign=center; font=large; }"
            "} }"
            "body = { background=#0; coords={0,0,100%,50%} align=fill; }"
        "}"
    );
}

//--
void UiWizardDialog::onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) {
    switch( commandId ) {
        case GUI_MESSAGEID_PREV:
            PreviousPage();
            return;

        case GUI_MESSAGEID_NEXT:
            NextPage();
            return;

        case GUI_MESSAGEID_CANCEL:
            onCancel();
            return;

        case GUI_MESSAGEID_OK:
            onConfirm();
            return;

        default:
            GuiDialog::onCommand( source ,commandId ,param ,params ,extra );
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! UiConnectionWizard

#define UIWALL_THUMBSIZE {100,100}

class UiCoinList : public GuiList {
public:
    UiCoinList() {
        itemSize() = UIWALL_THUMBSIZE;
    }

    void buildList() {
        const auto &coins = CCoinStore::getInstance().getList();

        for( const auto &it : coins ) if( !it.isNull() ) {
            const char *ticker = it->getTicker().c_str();

            if( !filter(ticker) ) continue;

            auto *image = Assets::Image().get( ticker );
            auto &thumb = * new GuiImageBox( image );

            thumb.backgroundColor() = 0;
            thumb.text() = ticker;

            addControl( thumb );
        }
    }

    virtual bool filter( const char *ticker ) {
        return true;
    }

    bool filterWallet( const char *ticker ) {
        String name = ticker;

        name += "-core";
        tolower(name);

        CWalletServiceRef pwallet;

        getWallet( tocstr(name) ,pwallet );

        return !pwallet.isNull();
    }

    bool filterPool() {
        /* auto &store = getWalletStore();

        ListOf<String> wallets;
        store.listServiceSupport( wallets );

        //...
        info.wallet->hasCoinSupport(coin);
        */
        return true;
    }

protected:
    // bool m_hasWallet = false;
    // bool m_hasPool = false;
    // hasMarket
};

class UiAddressList : public GuiGroup {
public:
    UiAddressList( GuiControlWindow &parent ) {
        setRoot(parent);
        root().addBinding("self",this);

        addControl( "coin" ,m_coinbox );
        addControl( "address" ,m_addressbox );

        setPropertiesWithString(
            "controls={"
                "coin:GuiImageBox = { align=top,vertical; coords={0,0,100%,25%} }"
                "address:GuiComboBox = { align=top,vertical; coords={25%,0,75%,10%} font=large; menu={ background=#101010; } }"
                "ma:GuiMargin = { align=top,vertical; coords={25%,0,75%,2%} background=0; }"
                "new:GuiButton = { commandId=28; bind=self; align=top,vertical; coords={45%,0,55%,6%} text=New; }"
                "mb:GuiMargin = { align=top,vertical; coords={25%,0,75%,2%} background=0; }"
                "info:GuiLabel = { align=top,vertical; coords={0,0,100%,10%} background=0; textalign=center; font=large; }"
                "ok:GuiButton = { commandId=2; bind=self; coords={45%,80%,55%,86%} text=Ok; }"
            "}"
        );

        m_info = getControl("info")->As_<GuiLabel>(); assert( m_info );
    }

    bool getWalletRef( CWalletServiceRef &wallet ) {
        wallet = m_wallet; return !wallet.isNull();
    }

    String &getAddress() {
        return m_addressbox.text();
    }

public:
    void updateCoin( const char *ticker ) {
        auto *image = Assets::Image().get( ticker );

        if( image )
            m_coinbox.setImage( *image );

        m_coinbox.backgroundColor() = 0;
        m_coinbox.text() = ticker;

        m_info->text() = "";

        Update( false ,refreshResized );
    }

    void buildList( const char *ticker ) {
        if( m_ticker == ticker ) return; //! no change since last call

        //-- gui
        updateCoin(ticker);

        //-- address list
        String name = ticker;

        name += "-core";
        tolower(name);

        CWalletServiceRef pwallet;

        getWallet( tocstr(name) ,m_wallet );

        if( m_wallet.isNull() ) return;

        m_addresses.clear();

        m_wallet->listAddresses( "" ,m_addresses );

        m_addressbox.menu().clear();
        m_addressbox.text() = "";

        int i=0; for( auto &it : m_addresses ) {
            const char *address = tocstr(it);

            if( i==0 && *address ) { //! if auto
                getAddress() = address;
            }

            m_addressbox.menu().addItem( i , address ,NullPtr );

            ++i;
        }

        m_ticker = ticker;
    }

    void onNewAddress() {
        if( m_wallet.isNull() )
            return;

        String address;

        if( ISUCCESS(m_wallet->getNewAddress( "" ,address )) ) {
            m_addressbox.text() = address;
            m_info->text() = "New address generated";
            m_info->textColor() = OS_COLOR_OLIVE;
        } else {
            m_info->text() = "Could not retrieve address from core wallet, is it running and configured?";
            m_info->textColor() = OS_COLOR_ORANGE;
        }
    }

public:
    void onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) override {
        switch( commandId ) {
            case GUI_MESSAGEID_ADD:
                onNewAddress();
                break;

            default:
                GuiControl::onCommand( source ,commandId ,param ,params ,extra );
                break;
        }
    }

protected:
    String m_ticker; //! current (last build list) ticker

    GuiImageBox m_coinbox;
    GuiComboBox m_addressbox;
    GuiLabel *m_info;

protected:
    ListOf<String> m_addresses;

    CWalletServiceRef m_wallet;
};

class UiPoolList : public GuiList {
public:
    UiPoolList() {
        itemSize() = UIWALL_THUMBSIZE;

        // mining mode for pools
        // mmUnknown = 0 ,SoloLocal ,SoloRemote ,PoolSolo ,PoolShared
    }

    void buildList( const char *ticker ) {
        auto &poolList = getPoolListInstance();

        ListOf<String> pools;

        poolList.listPools( pools ,ticker ); //+ mining mode

        removeAllControls();

        for( const auto &it : pools ) {
            const char *name = tocstr(it);

            auto *image = Assets::Image().get( name );
            auto &thumb = * new GuiImageBox( image );

            thumb.backgroundColor() = 0;
            thumb.text() = name;

            addControl( thumb );
        }

        Update( false ,refreshResized );
    }
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
///-- Components

#define SOLOMINER_COIN_PUID    0x093be7e167beca8fa

struct Coin {
    String ticker;
};

DECLARE_STRUCT(Coin,SOLOMINER_COIN_PUID);
REGISTER_STRUCTNAME( Coin );

struct GuiCoinBox : GuiComboBox {
    GuiCoinBox() {
        setPropertiesWithString("menu={}");

        menu().addItem( 0 ,"MAXE" ,NullPtr );
        menu().addItem( 1 ,"RTC" ,NullPtr );
        menu().addItem( 1 ,"RTM" ,NullPtr );
    }

    DECLARE_GUICONTROL(GuiComboBox,GuiCoinBox,SOLOMINER_COIN_PUID);
};

REGISTER_CLASS(GuiCoinBox);
REGISTER_EDITBOX(Coin,GuiCoinBox);

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
///-- Wizard Pages

struct WizSelectCoin : UiCoinList {
    UiConnectionWizard &m_wizard;

    enum WhichCoin {
        miningCoin ,tradeCoin
    } m_whichCoin;

    WizSelectCoin( UiConnectionWizard &wizard ,WhichCoin whichCoin ) : m_wizard(wizard) ,m_whichCoin(whichCoin)
    {
        placement() = placeDiamond;
        direction() = (Direction) (directionBottom | directionRight);
        origin() = center;

        buildList();
    }

    void onItemSelect( GuiControl &item ,int index ,bool selected ) override {
        auto *box = item.As_<GuiImageBox>();
        assert( box );

        const char *ticker = tocstr( box ? box->text() : "" );

        switch( m_whichCoin ) {
            case miningCoin:
                m_wizard.info().mineCoin.coin = ticker;
                m_wizard.info().mineCoin.wallet = "core"; //! LATER wallet select
                break;
            case tradeCoin:
                if( m_wizard.info().tradeCoin.coin != m_wizard.info().mineCoin.coin )
                    makeTrade( m_wizard.info() ,ticker );
                else
                    noTrade( m_wizard.info() );

                break;
        }

        m_wizard.NextPage();
    }

    void makeTrade( ConnectionInfo &info ,const char *ticker ) {
        m_wizard.info().tradeCoin.coin = ticker;
        m_wizard.info().tradeCoin.wallet = "core"; //! LATER wallet select
        m_wizard.info().market = "xeggex";
        m_wizard.info().trading.percent = 100.f;
        m_wizard.info().trading.withdraw = true;
    }

    void noTrade( ConnectionInfo &info ) {
        m_wizard.info().tradeCoin.coin = "";
        m_wizard.info().tradeCoin.wallet = "";
        m_wizard.info().market = "";
        m_wizard.info().trading.percent = 100.f;
        m_wizard.info().trading.withdraw = false;
    }

    bool filter( const char *ticker ) override {
        switch( m_whichCoin ) {
            case miningCoin:
                return filterWallet(ticker);
            case tradeCoin:
                return true; //TODO has market par with mining coin

            default:
                return true;
        }
    }
};

struct WizSelectAddress : UiAddressList ,CGuiTabControl {
    UiConnectionWizard &m_wizard;

    WizSelectAddress( UiConnectionWizard &wizard ) : UiAddressList(wizard.root()) ,m_wizard(wizard)
    {}

    void onTabEnter( int fromTabIndex ) override {
        buildList( tocstr( m_wizard.info().mineCoin.coin ) );
    }

    void onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) override {
        switch( commandId ) {
            case GUI_MESSAGEID_OK:
                m_wizard.info().mineCoin.address = m_addressbox.text();
                m_wizard.NextPage();
                break;

            default:
                UiAddressList::onCommand( source ,commandId ,param ,params ,extra );
                break;
        }
    }
};

struct WizSelectPool : UiPoolList ,CGuiTabControl {
    UiConnectionWizard &m_wizard;
    Params m_vars;

    WizSelectPool( UiConnectionWizard &wizard ) : m_wizard(wizard)
    {
        placement() = placeDiamond;
        direction() = (Direction) (directionBottom | directionRight);
        origin() = center;

        auto &global = getGlobalConfig();

        m_vars = global.params;
    }

    void onTabEnter( int fromTabIndex ) override {
        auto &info = m_wizard.info();

        buildList( tocstr( info.mineCoin.coin ) );

        m_vars["coin"] = info.mineCoin.coin;
        m_vars["address"] = info.mineCoin.address;
    }

    String resolveField( const char *s ) {
        String r;

        replaceTextVariables( s ,m_vars ,r );

        return r;
    }

    void onItemSelect( GuiControl &item ,int index ,bool selected ) override {
        auto *box = item.As_<GuiImageBox>();
        assert( box );

        const char *name = tocstr( box ? box->text() : "" );

        m_wizard.info().pool = name;

        ///-- import pool infos
        auto &poolList = getPoolListInstance();

        CPoolRef pool;

        ListOf<PoolConnectionInfo> infos;

        const char *ticker = tocstr( m_wizard.info().mineCoin.coin );

        if( name && *name && poolList.findPoolByName( name ,pool ) && pool->findPoolConnections( infos ,ticker ) && !infos.empty() ) {
            auto &poolinfo = infos[0];

            fromString( m_wizard.info().connection ,resolveField( tocstr(poolinfo.host) ) );
            m_wizard.info().credential.user = resolveField( tocstr(poolinfo.user) );
            m_wizard.info().credential.password = resolveField( tocstr(poolinfo.password) );
            m_wizard.info().options = poolinfo.options;
        }

        m_wizard.NextPage();
    }
};

struct WizSelectTrade : GuiGroup ,CGuiTabControl {
    UiConnectionWizard &m_wizard;

    WizSelectTrade( UiConnectionWizard &wizard ) : m_address(wizard.root()) ,m_wizard(wizard)
    {
        setRoot( wizard.root() );

        m_sheet.headings() = {
            { "tradePercent" ,"Trade Percent" }
            ,{ "tradeWithdraw" ,"Withdraw Trade" }
        };

        m_sheet.Bind( &wizard );
        m_sheet.setPropertiesWithString( "coords={25%,5%,75%,18%}" );

        addControl( "sheet" ,m_sheet );

        m_address.setPropertiesWithString( "coords={0%,20%,100%,100%}");
        addControl( "address" ,m_address );

        setPropertiesWithString( "align=fill;" );

        //+ address visible only if withdraw ? or address required ?

        m_address.getControl("ok")->As_<GuiButton>()->Subscribe(*this);
    }

    void onTabEnter( int fromTabIndex ) override {
        m_wizard.updateData();

        m_address.buildList( tocstr( m_wizard.info().tradeCoin.coin ) );
    }

    void onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) override {
        switch( commandId ) {
            case GUI_MESSAGEID_OK:
                m_wizard.info().tradeCoin.address = m_address.getAddress();
                m_wizard.NextPage();
                break;

            default:
                m_address.onCommand( source ,commandId ,param ,params ,extra );
                break;
        }
    }

    GuiSheet m_sheet;
    UiAddressList m_address;
};

struct WizSettings : GuiGroup ,CGuiTabControl {
    UiConnectionWizard &m_wizard;

    WizSettings( UiConnectionWizard &wizard ) : m_wizard(wizard)
    {
        setRoot( wizard.root() );

        m_sheet.headings() = {
            { "mineCoin" ,"Mining Coin" }
            ,{ "mineAddress" ,"Mining Address" }
            ,{ "Pool" ,"Pool" }
            ,{ "Host" ,"Host" }
            ,{ "User" ,"User" }
            ,{ "Password" ,"Password" }
            ,{ "tradeCoin" ,"Trade Coin" }
            ,{ "tradePercent" ,"Trade Percent" }
            ,{ "tradeWithdraw" ,"Withdraw Trade" }
            ,{ "tradeAddress" ,"Withdraw Address" }
            ,{ "*" }
            ,{ "Market" ,"" } //! not shown
        };

        m_sheet.Bind( &wizard );
        m_sheet.setPropertiesWithString( "coords={25%,5%,75%,82%}" );

        addControl( "sheet" ,m_sheet );

        setPropertiesWithString(
            "align=fill; controls={"
                "ok:GuiButton = { commandId=2; bind=dialog; coords={45%,88%,55%,94%} text=Confirm; }"
            "}"
        );
    }

    void onTabEnter( int fromTabIndex ) override {
        m_wizard.updateData();
    }

    GuiSheet m_sheet;
};

//////////////////////////////////////////////////////////////////////////////
///-- wizard

UiConnectionWizard::UiConnectionWizard( GuiControlWindow &parent ) :
    CDataConnectionInfo2(m_info) ,UiWizardDialog(parent)
{
    setRoot(parent);

    addPage( * new WizSelectCoin( *this ,WizSelectCoin::miningCoin ) ,{"What do you want to mine ?"} );
    addPage( * new WizSelectAddress( *this ) ,{"Your mining address"} );
    addPage( * new WizSelectPool( *this ) ,{"Where do you want to mine ?"} );
    addPage( * new WizSelectCoin( *this ,WizSelectCoin::tradeCoin ) ,{"What reward do you want to get (exchange) ?"} );
    addPage( * new WizSelectTrade( *this ) ,{"Configure trading"} );
    addPage( * new WizSettings( *this ) ,{"Verify connection settings"} );

    selectPage(0);
}

void UiConnectionWizard::onStepEnter( int step ,int fromStep ) {
}

void UiConnectionWizard::onConfirm()  {
    if( m_index < 0 )
        confirmAddConnection();
    else
        confirmEditConnection();

    UiWizardDialog::onConfirm();
}

void UiConnectionWizard::onCancel() {
    UiWizardDialog::onCancel();
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF