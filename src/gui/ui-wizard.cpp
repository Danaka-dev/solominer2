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

    addControl( "header:GuiGroup/title" ,m_title );
    addControl( "footer:GuiGroup" );
    addControl( "body" ,m_body );

    setPropertiesWithString(
        "controls = {"
            "header = { background=#101010; coords={0,0,100%,10%} align=top,vertical; controls={"
                "title={ align=center; coords={0,0,100%,25%} font=huge; text=title; textalign=center; }"
            "} }"
            "footer = { background=#101010; coords={0,0,100%,5%} align=bottom,vertical; controls={"
                "first:GuiLink = { bind=dialog; commandId=24; align=left,horizontal; coords={0,5%,5%,95%} text=|<; textalign=center; font=large; }"
                "prev:GuiLink = { bind=dialog; commandId=22; align=left,horizontal; coords={0,5%,5%,95%} text=<<; textalign=center; font=large; }"
                "last:GuiLink = { bind=dialog; commandId=25; align=right,horizontal; coords={0,5%,5%,95%} text=>|; textalign=center; font=large; }"
                "next:GuiLink = { bind=dialog; commandId=23; align=right,horizontal; coords={0,5%,5%,95%} text=>>; textalign=center; font=large; }"
            "} }"
            "body = { background=#0; coords={0,0,100%,100%} align=fill; }"
        "}"
    );
}

//--
void UiWizardDialog::addPage( GuiControl &page ,const PageInfo &info ) {
    m_body.addControl( page );
    m_pages.emplace_back( info );
}

bool UiWizardDialog::selectPage( int index ) {
    if( index < 0 || index >= getPageCount() ) {
        return false;
    }

    int icurrent = getCurrentPage();

    if( !onStepLeave( icurrent ,index ) ) return false;

    m_body.selectTab( index );

    m_title.text() = m_pages[ index ].title;

    onStepEnter( index ,icurrent );

    Refresh();

    return true;
}

//--
void UiWizardDialog::onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) {
    switch( commandId ) {
        case GUI_MESSAGEID_FIRST:
            FirstPage();
            return;
        case GUI_MESSAGEID_PREV:
            PreviousPage();
            return;
        case GUI_MESSAGEID_NEXT:
            NextPage();
            return;
        case GUI_MESSAGEID_LAST:
            LastPage();
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

#define UIWALL_THUMBSIZE {120,120}

//////////////////////////////////////////////////////////////////////////////
//! UiCoinList

class UiCoinList : public GuiList {
public:
    UiCoinList() {
        itemSize() = UIWALL_THUMBSIZE;

        getMarket( "xeggex" ,m_market );
    }

    void buildList( const char *opt=NullPtr ) {
        removeAllControls();

        const auto &coins = CCoinStore::getInstance().getList();

        for( const auto &it : coins ) if( !it.isNull() ) {
            const char *ticker = tocstr( it->getTicker() );

            if( !filter(ticker) ) continue;

            auto *image = Assets::Image().get( ticker );
            auto &thumb = * new GuiImageBox( image );

            thumb.backgroundColor() = 0;
            thumb.text() = ticker;

            addControl( thumb );
        }

        if( opt ) {
            auto *image = Assets::Image().get( "nocoin" );
            auto &thumb = * new GuiImageBox( image );

            thumb.backgroundColor() = 0;
            thumb.text() = opt;

            addControl( thumb );
        }
    }

    //! @note filter out coin without wallet
    bool filterWallet( const char *ticker ) {
        String name = ticker;

        name += "-core";
        tolower(name);

        CWalletServiceRef pwallet;

        return getWallet( tocstr(name) ,pwallet ) && !pwallet.isNull();
    }

    //! @note filter out coin without a trade pair
    bool filterTrade( const char *ticker ,const char *coin ) {
        if( m_market.isNull() )
            return true; //! no filtering

        if( ticker && *ticker && coin && *coin ) {} else
            return true; //! no filtering

        if( stricmp(ticker,coin) == 0 )
            return false; //! don't include reference coin

        MarketPair pair;

        if( m_market->getMarketPair( coin ,ticker ,pair ) != IOK )
            return false;

        return pair.hasMarket;
    }

    //! @note override this to direct to proper filter
    virtual bool filter( const char *ticker ) {
        return true;
    }

protected:
    CMarketServiceRef m_market;
};

//////////////////////////////////////////////////////////////////////////////
//! UiAddressList

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

//////////////////////////////////////////////////////////////////////////////
//! UiPoolList

void PrettyMiningMode( MiningMode mode ,String &s ) {
    switch( mode ) {
        case SoloLocal: s = "Solo"; return;
        case SoloRemote: s = "Remote"; return;
        case PoolSolo: s = "Pool"; return;
        case PoolShared: s = "Shared"; return;
        default: s = ""; return;
    }
}

class UiPoolList : public GuiList {
public:
    struct PoolThumb : GuiGroup {
        PoolThumb( UiPoolList &list ,const ListOf<PoolConnectionInfo> &a_infos ,GuiImage *image ,const char *text ) :
            infos(a_infos)
        {
            thumb = new GuiImageBox(image,text);

            addControl("thumb",thumb);

            setPropertiesWithString( "background=0;"
                "/thumb={ align=top,vertical; coords={0,0,100%,80%} background=0; }"
                "/label:GuiLabel={ align=top,vertical; coords={0,80%,100%,100%} text=; textalign=center; }"
            );

            if( infos.size() != 1 ) return; //! only display options for single connection

            auto *label = getControlAs_<GuiLabel>("label");
            auto &info = infos[0];

            StringList opt;
            String s;

            if( !list.hasMode() && info.mode != mmUnknown ) {
                PrettyMiningMode( info.mode ,s );
                if( !s.empty() ) opt.emplace_back(s);
            }
            if( !list.hasRegion() && !info.region.empty() && info.region != "*" ) {
                opt.emplace_back(info.region);
            }
            if( !list.hasSsl() && info.ssl ) {
                opt.emplace_back( "ssl" );
            }

            toString( opt ,s );
            Format( label->text() ,"(%s)" ,64 ,(const char*) tocstr(s) );
        }

        GuiImageBox *thumb;

        ListOf<PoolConnectionInfo> infos;
    };

public:
    UiPoolList() {
        itemSize() = UIWALL_THUMBSIZE;
    }

    String &Mode() { return m_mode; }
    String &Region() { return m_region; }
    String &Ssl() { return m_ssl; }

    static bool hasFilter( const String &s ) { return ! (s.empty() || s == "*" ); }

    bool hasMode() const { return hasFilter(m_mode); }
    bool hasRegion() const { return hasFilter(m_region); }
    bool hasSsl() const { return hasFilter(m_ssl); }

//--
    void buildList( const char *name ,const char *ticker ) {
        auto &poolList = getPoolListInstance();

        ListOf<String> pools;

        MiningMode mode = mmUnknown;

        if( hasFilter(Mode() ) )
            enumFromString( mode ,Mode() );

        poolList.listPools( pools ,ticker ,mode );

        //-- build list
        removeAllControls();

        for( const auto &it : pools ) {
            const char *s = tocstr(it);

            if( name && *name && stricmp(name,s)!=0 ) continue;

            buildPoolList( s ,ticker ,name && *name );
        }

        Update( false ,refreshResized );
    }

    //! @return (int) the number of connections for this pool and filters
    int buildPoolList( const char *name ,const char *ticker ,bool digest ) {
        auto &list = getPoolListInstance();

        CPoolRef pool;

        if( !list.findPoolByName( name ,pool ) )
            return false;

        ListOf<PoolConnectionInfo> infos;

        //-- filter
        MiningMode mode = mmUnknown; bool ssl = true;

        if( hasMode() ) enumFromString( mode ,Mode() );
        const char *region = hasRegion() ? tocstr(Region()) : NullPtr;
        if( hasSsl() ) fromString( ssl ,Ssl() );

        if( !pool->findPoolConnections( infos ,ticker ,mode ,region ,hasSsl() ? &ssl : NullPtr ) )
            return false;

        //-- add control
        auto *image = Assets::Image().get( name );

        if( digest ) { //! one thumb for each info
            for( auto &info : infos ) {
                ListOf<PoolConnectionInfo> oneInfo;
                oneInfo.emplace_back(info);
                addControl( * new PoolThumb( *this ,oneInfo ,image ,name ) );
            }
        } else { //! one thumb regroupiing all
            addControl( * new PoolThumb( *this ,infos ,image ,name ) );
        }

        return (int) infos.size();
    }

protected:
//-- filters
    String m_mode;
    String m_region;
    String m_ssl;
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
        //TODO

        setPropertiesWithString("menu={}");

        menu().addItem( 0 ,"MAXE" ,NullPtr );
        menu().addItem( 1 ,"RTC" ,NullPtr );
        menu().addItem( 2 ,"RTM" ,NullPtr );
    }

    DECLARE_GUICONTROL(GuiComboBox,GuiCoinBox,SOLOMINER_COIN_PUID);
};

REGISTER_CLASS(GuiCoinBox);
REGISTER_EDITBOX(Coin,GuiCoinBox);

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
///-- Wizard Pages

//////////////////////////////////////////////////////////////////////////////
//! Select coin

struct WizSelectCoin : UiCoinList ,CGuiTabControl {
    UiConnectionWizard &m_wizard;

    enum WhichCoin {
        miningCoin ,tradeCoin
    } m_whichCoin;

    WizSelectCoin( UiConnectionWizard &wizard ,WhichCoin whichCoin ) : m_wizard(wizard) ,m_whichCoin(whichCoin)
    {
        placement() = placeDiamond;
        direction() = (Direction) (directionBottom | directionRight);
        origin() = center;

        if( m_whichCoin == miningCoin ) {
            buildList();
        }
    }

    bool hasTrade( const char *trade ,const char *mine ) {
        return trade && *trade && stricmp(trade,"No Trade")!=0 && stricmp(mine,trade)!=0;
    }

    void onTabEnter( int fromTabIndex ) override {
        if( m_whichCoin == tradeCoin ) {
            buildList("No Trade");
            Update(false,refreshResized);
        }
    }

    void onItemSelect( GuiControl &item ,int index ,bool selected ) override {
        auto *box = item.As_<GuiImageBox>();
        assert( box ); if( !box ) return;

        const char *ticker = tocstr( box ? box->text() : "" );

        switch( m_whichCoin ) {
            case miningCoin:
                m_wizard.info().mineCoin.coin = ticker;
                m_wizard.info().mineCoin.wallet = "core"; //! LATER wallet select
                break;
            case tradeCoin:
                if( hasTrade( ticker ,tocstr(m_wizard.info().mineCoin.coin) ) ) {
                    makeTrade( m_wizard.info() ,ticker );
                }
                else {
                    noTrade( m_wizard.info() );
                    m_wizard.NextPage(); //! @note skip next page (trading)
                }
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
        m_wizard.info().trading.percent = 0.f;
        m_wizard.info().trading.withdraw = false;
    }

    bool filter( const char *ticker ) override {
        switch( m_whichCoin ) {
            case miningCoin:
                return filterWallet(ticker);
            case tradeCoin:
                return filterTrade(ticker,tocstr(m_wizard.info().mineCoin.coin) );

            default:
                return true;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////
//! Select address

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

//////////////////////////////////////////////////////////////////////////////
//! Select pool

struct WizSelectPool : GuiGroup ,CGuiTabControl {
    UiConnectionWizard &m_wizard;

    WizSelectPool( UiConnectionWizard &wizard ) : m_wizard(wizard)
    {
        setRoot(wizard.root());
        root().addBinding("page",this);

        auto &global = getGlobalConfig();

        m_vars = global.params;

        //-- controls
        m_pools.placement() = UiPoolList::placeDiamond;
        m_pools.direction() = (Direction) (directionBottom | directionRight);
        m_pools.origin() = center;

        addControl( "pools" ,m_pools );

        setPropertiesWithString( "controls={"
                "pools = { bind=page; }"
                "modeLabel:GuiLabel = { align=none; coords={78%,2%,85%,7%} text=Mode; textalign=center; }"
                "mode:GuiComboBox = { bind=page; align=none; coords={85%,2%,98%,7%} listonly=true; text=*; menu={ items=*; } }"
                "regionLabel:GuiLabel = { align=none; coords={78%,9%,85%,14%} text=Region; textalign=center; }"
                "region:GuiComboBox = { bind=page; align=none; coords={85%,9%,98%,14%} listonly=true; text=*; menu={ items=*; } }"
                "sslLabel:GuiLabel = { align=none; coords={78%,16%,85%,21%} text=SSL; textalign=center; }"
                "ssl:GuiComboBox = { bind=page; align=none; coords={85%,16%,98%,21%} listonly=true; text=*; menu={ items=*,true,false; } }"
                "back:GuiButton = { commandId=22; bind=page; align=none; coords={45%,92%,55%,97%} text=<<; }"
            "}"
        );

        makeComboList();
    }

    void makeComboList() {
        Params params;

    //-- modes
        ListOf<String> modes;

        listEnum<MiningMode>( modes );
        ReplaceEntry( modes ,"none" ,"*" );

        toString( modes ,params["a"] );

        setPropertiesWithString( "/mode = { listonly=true; text=*; menu={ items=${a}; } }" ,params );

    //-- regions
        StringList regions;

        getPoolListInstance().getPoolRegionList( regions );

        toString( regions ,params["a"] );

        setPropertiesWithString( "/region = { listonly=true; text=*; menu={ items=${a}; } }" ,params );
    }

    String resolveField( const char *s ) const {
        String r;

        replaceTextVariables( s ,m_vars ,r );

        return r;
    }

    void buildRootList() {
        auto &info = m_wizard.info();

        m_pool = "";
        m_pools.buildList( NullPtr ,tocstr(info.mineCoin.coin) );

        getControlAs_<GuiButton>("back")->visible() = false;
    }

    void BuildSubList( const char *poolName ) {
        auto &info = m_wizard.info();

        m_pool = poolName;
        m_pools.buildList( poolName ,tocstr(info.mineCoin.coin) );

        getControlAs_<GuiButton>("back")->visible() = true;
    }

    //--
    void onTabEnter( int fromTabIndex ) override {
        auto &info = m_wizard.info();

        m_pools.Mode() = getControlAs_<GuiComboBox>("mode")->text();
        m_pools.Region() = getControlAs_<GuiComboBox>("region")->text();
        m_pools.Ssl() = getControlAs_<GuiComboBox>("ssl")->text();

        buildRootList();

        m_vars["coin"] = info.mineCoin.coin;
        m_vars["address"] = info.mineCoin.address;
    }

    void onItemSelect( int index ,bool selected ) {
        auto *box = m_pools.getControlAs_<UiPoolList::PoolThumb>( index );
        assert( box ); if( !box ) return;

        const char *poolName = tocstr( box->thumb->text() );

        if( box->infos.size() != 1 ) { //! sub list
            BuildSubList( poolName ); return;
        }

        ///-- info from pool, resolve
        auto &info = box->infos[0];

        m_wizard.info().pool = poolName;
        fromString( m_wizard.info().connection ,resolveField( tocstr(info.host) ) );
        m_wizard.info().credential.user = resolveField( tocstr(info.user) );
        m_wizard.info().credential.password = resolveField( tocstr(info.password) );
        m_wizard.info().options = info.options;

        m_wizard.NextPage();
    }

    void onComboSelect() {
        auto &info = m_wizard.info();
        m_pools.buildList( m_pool.empty() ? NullPtr : tocstr(m_pool) ,tocstr( info.mineCoin.coin ) );
        Refresh();
    }
    void onModeSelect( GuiComboBox &mode ) {
        m_pools.Mode() = mode.getText();
        onComboSelect();
    }

    void onRegionSelect( GuiComboBox &region ) {
        m_pools.Region() = region.getText();
        onComboSelect();
    }

    void onSslSelect( GuiComboBox &ssl ) {
        m_pools.Ssl() = ssl.getText();
        onComboSelect();
    }

    void onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) override {
        if( commandId == GUI_MESSAGEID_PREV ) {
            buildRootList();
            return;
        }

        if( source ) {
            auto *mode = getControlAs_<GuiComboBox>("mode");
            auto *region = getControlAs_<GuiComboBox>("region");
            auto *ssl = getControlAs_<GuiComboBox>("ssl");

            if( mode && *source == mode->menu() ) {
                onModeSelect(*mode); return;
            } else if( region && *source == region->menu() ) {
                onRegionSelect(*region); return;
            } else if( ssl && *source == ssl->menu() ) {
                onSslSelect( *ssl ); return;
            }
        }

        GuiGroup::onCommand(source,commandId,param,params,extra);
    }

    void onNotify( IObject *source ,messageid_t notifyId ,long param ,Params *params ,void *extra ) override {
        if( m_pools == *source && notifyId == GUI_MESSAGEID_SELECT ) {
            onItemSelect( (int) abs(param) ,param < 0 );
            return;
        }

        CGuiTabControl::onNotify( source ,notifyId ,param ,params ,extra );
    }

//-- members
    UiPoolList m_pools;
    Params m_vars;
    String m_pool; //! selected pool (with multi step select)
};

//////////////////////////////////////////////////////////////////////////////
//! Select trade

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

//////////////////////////////////////////////////////////////////////////////
//! Review settings

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
//////////////////////////////////////////////////////////////////////////////
///-- wizard

UiConnectionWizard::UiConnectionWizard( GuiControlWindow &parent ) :
    CDataConnectionInfo2(m_info) ,UiWizardDialog(parent) ,m_settings(NullPtr)
{
    setRoot(parent);

    WizSettings *wizSettings;

    addPage( * new WizSelectCoin( *this ,WizSelectCoin::miningCoin ) ,{"What do you want to mine ?"} );
    addPage( * new WizSelectAddress( *this ) ,{"Your mining address"} );
    addPage( * new WizSelectPool( *this ) ,{"Where do you want to mine ?"} );
    addPage( * new WizSelectCoin( *this ,WizSelectCoin::tradeCoin ) ,{"What reward do you want to get (exchange) ?"} );
    addPage( * new WizSelectTrade( *this ) ,{"Configure trading"} );
    addPage( * (wizSettings = new WizSettings( *this )) ,{"Verify connection settings"} );

    m_settings = &wizSettings->m_sheet;

    selectPage(0);
}

//--
void UiConnectionWizard::showAddConnection( GuiControlWindow &parent ) {
    m_index = -1; Zero(m_info);
    selectPage(0);

    parent.ShowModal( *this );
}

void UiConnectionWizard::showEditConnection( GuiControlWindow &parent ,int index ,const ConnectionInfo &info ) {
    m_index = index; m_info = info;
    selectPage(0);

    parent.ShowModal( *this );
}

void UiConnectionWizard::confirmAddConnection() {
    Params settings;

    toManifest( m_info ,settings );

    root().onCommand( this ,GUI_MESSAGEID_OK ,-1 ,&settings ,(void*) "connection" );
}

void UiConnectionWizard::confirmEditConnection() {
    Params settings;

    toManifest( m_info ,settings );

    root().onCommand( this ,GUI_MESSAGEID_OK ,m_index ,&settings ,(void*) "connection" );
}

void UiConnectionWizard::updateData() {
    Params data;

    readData( data );

    CDataConnectionInfo::adviseDataChanged( data );
}

//--
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