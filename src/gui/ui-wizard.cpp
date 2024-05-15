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

UiWizardDialog::UiWizardDialog() {
    colors().fillColor = OS_COLOR_BLACK;
    coords() = { 0 ,0 ,95.f ,95.f };

//-- header
    m_header.coords() = { 0 ,0 ,100.f ,5.f };
    m_header.align() = (GuiAlign) (GuiAlign::alignTop | GuiAlign::alignAnchorV);

    m_header.m_cancel.setImage( getAssetIconImage( UIICONS_MAIN ) );
    // m_header.m_cancel.origin() = { 5+75*7,5 };
    // m_header.m_cancel.size() = { 75 ,75 };
    m_header.m_cancel.coords() = { 0 ,0 ,75 ,100.f };
    m_header.m_cancel.align() = (GuiAlign) (GuiAlign::alignLeft | GuiAlign::alignAnchorH);
    m_header.m_cancel.setCommandId( GUI_COMMANDID_CANCEL );
    m_header.m_cancel.GuiCommandPublisher::Subscribe( *this );
    m_header.addControl(m_header.m_cancel);

    m_header.m_info.setImage( getAssetIconImage( UIICONS_MAIN ) );
    // m_header.m_info.origin() = { 5+75*4,111*4 };
    // m_header.m_info.size() = { 75 ,75 };
    m_header.m_info.coords() = { 0 ,0 ,75 ,100.f };
    m_header.m_info.align() = (GuiAlign) (GuiAlign::alignRight | GuiAlign::alignAnchorH);
    m_header.addControl(m_header.m_info);

    addControl(m_header);

//-- title
    m_title.text() = "title";
    m_title.setFont( getFontMedium() );
    m_title.coords() = { 0 ,0 ,100.f ,10.f };
    m_title.align() = (GuiAlign) (GuiAlign::alignTop | GuiAlign::alignAnchorV);
    m_title.textAlign() = (TextAlign) (textalignCenterH | textalignCenterV);

    addControl(m_title);

//-- footer
    m_footer.coords() = { 0 ,0 ,100.f ,5.f };
    m_footer.align() = (GuiAlign) (GuiAlign::alignBottom | GuiAlign::alignAnchorV);

    //--
    m_footer.m_prev.text() = "Prev";
    m_footer.m_prev.coords() = { 0 ,0 ,10.f ,100.f };
    m_footer.m_prev.align() = (GuiAlign) (GuiAlign::alignLeft | GuiAlign::alignAnchorH);
    m_footer.m_prev.setCommandId( GUI_COMMANDID_PREV );
    m_footer.m_prev.GuiCommandPublisher::Subscribe( *this );
    m_footer.addControl(m_footer.m_prev);

    m_footer.m_next.text() = "Next";
    m_footer.m_next.coords() = { 0 ,0 ,10.f ,100.f };
    m_footer.m_next.align() = (GuiAlign) (GuiAlign::alignRight | GuiAlign::alignAnchorH);
    m_footer.m_next.setCommandId( GUI_COMMANDID_NEXT );
    m_footer.m_next.GuiCommandPublisher::Subscribe( *this );
    m_footer.addControl(m_footer.m_next);

    addControl(m_footer);

//--
    m_steps.coords() = { 0 ,0 ,100.f ,70.f };
    m_steps.align() = GuiAlign::alignCenter; // (GuiAlign) (GuiAlign::alignTop | GuiAlign::alignAnchorV);
    addControl(m_steps);
}

IAPI_DEF UiWizardDialog::getInterface( UUID id ,void **ppv ) {
    if( !ppv || *ppv ) return false;

    return
        honorInterface_<UiWizardDialog>(this,id,ppv) ? IOK :
        GuiDialog::getInterface( id ,ppv )
    ;
}

//--
void UiWizardDialog::onCommand( GuiControl &source ,uint32_t commandId ,long param ,Params *params ,void *extra ) {
    switch( commandId ) {
        case GUI_COMMANDID_PREV:
            onPrevious();
            return;

        case GUI_COMMANDID_NEXT:
            if( m_steps.getCurrentTabIndex() == m_steps.getTabCount() - 1 ) {
                GuiCommandPublisher::PostCommand( GUI_COMMANDID_OK ,getCurrentStep());
                onConfirm();
            } else {
                onNext();
            }
            return;

        case GUI_COMMANDID_CANCEL:
            GuiCommandPublisher::PostCommand( GUI_COMMANDID_CANCEL ,getCurrentStep() );
            onCancel();
            return;

        default:
            assert(false);
            break;
    }

    // GuiCommandPublisher::PostCommand( source ,commandId ,param ,params ,extra );
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! UiConnectionWizard

#define UIWALL_THUMBSIZE {100,100}

class UiCoinWall : public GuiThumbwall {
public:
    UiCoinWall() : GuiThumbwall( UIWALL_THUMBSIZE ) {}

    void buildWall() {
        auto coins = CCoinStore::getInstance().getList();

        for( const auto &it : coins ) if( !it.isNull() ) {
                const char *ticker = it->getTicker().c_str();

                addThumb( &getAssetCoinImage(ticker) ,ticker );
            }
    }
};

class UiWalletWall : public GuiThumbwall {
public:
    struct WalletInfo {
        String name;
        GuiControlRef control;
        CWalletServiceRef wallet;
    };

    ListOf<WalletInfo> &wallets() { return m_infos; };

protected:
    ListOf<WalletInfo> m_infos;

public:
    UiWalletWall() : GuiThumbwall( UIWALL_THUMBSIZE ) {
        buildWall();
    }

    WalletInfo &getWalletInfo( int i ) {  return m_infos[i]; }

    void buildWall() {
        auto &store = getWalletStore();

        ListOf<String> wallets;
        store.listServiceSupport( wallets );

        WalletInfo info;

        for( auto &it : wallets ) {
            const char *name = it.c_str();

            getWallet( name ,info.wallet );
            if( info.wallet.isNull() ) continue; //! should not happen, tho some config/install/... mishaps may lead to it anyway, simply protect

            int thumbIndex = addThumb( &getAssetIconImage(name) ,name );

            info.name = name;
            info.control = getControl(thumbIndex);

            assert( !info.control.isNull() );

            m_infos.emplace_back(info);
        }
    }

    void filterWall( const char *coin ) {
        for( auto &info : m_infos ) {

            if( info.control.isNull() && info.wallet.isNull() ) {
                assert(false); continue;
            }

            info.control->visible() = (info.wallet->hasCoinSupport(coin) == IOK);
        }

        root().Update( NullPtr ,refreshResized );
    }
};

class UiAddressList : public GuiGroup {
protected:
    ListOf<String> m_addresses;

protected:
    // GuiList m_addressList;
    // GuiTextbox m_address;
    GuiButton m_newAddress;

    IWalletRef m_wallet;

public:
    UiAddressList() {}

    const char *getAddress() {
        return NullPtr;
    }

public:
    void Clear() {

    }

    void makeList( IWalletRef &wallet ) {
        m_addresses.clear();

        m_wallet = wallet;

        m_wallet->listAddresses( NullPtr ,m_addresses );


    }
};

//////////////////////////////////////////////////////////////////////////////
///-- steps

struct WizSelectCoin : UiCoinWall {
    UiConnectionWizard &m_wizard;

    enum WhichCoin {
        miningCoin ,tradeCoin
    } m_whichCoin;

    WizSelectCoin( UiConnectionWizard &wizard ,WhichCoin whichCoin ) : m_wizard(wizard) ,m_whichCoin(whichCoin)
    {
        buildWall();
    }

    void onItemSelected( GuiControl &item ,int index ,bool selected ) override {
        CCoinStore &coinlist = CCoinStore::getInstance();

        const char *ticker = coinlist.getList().at( index-1 )->getTicker().c_str();

        switch( m_whichCoin ) {
            case miningCoin:
                m_wizard.info().mineCoin.coin = ticker;
                break;
            case tradeCoin:
                m_wizard.info().tradeCoin.coin = ticker;
                break;
        }

        m_wizard.onNext();
    }
};

struct WizStepWallet : IGuiCommandEvent ,GuiGroup { //! IE mining wallet
    UiConnectionWizard &m_wizard;
    int m_stepIndex;

    UiWalletWall m_wallets;
    UiAddressList m_address;

    //! 1) select wallet
    //! 2) select/enter address
    WizStepWallet( UiConnectionWizard &wizard ,int index ) : m_wizard(wizard) ,m_stepIndex(index)
    {
        m_wallets.GuiCommandPublisher::Subscribe(*this);
        m_wallets.coords() = { 0 ,0 ,100.f ,50.f };
        m_wallets.align() = GuiAlign::alignTop;
        addControl(m_wallets);

        m_address.coords() = { 0 ,0 ,100.f ,50.f };
        m_address.align() = GuiAlign::alignBottom;
        addControl(m_address);

        m_wizard.GuiCommandPublisher::Subscribe(*this);
    }

    void onEnterStep() {
        m_wallets.filterWall( m_wizard.info().mineCoin.coin.c_str() );
    }

    void onWalletSelected( GuiControl &item ,int index ,bool selected ) {
        m_wizard.info().mineCoin.wallet = m_wallets.getWalletInfo(index).name;
        m_wizard.onNext();
    }

    void onAddressSelected( ) {

    }

    void onCommand( GuiControl &source ,uint32_t commandId ,long param ,Params *params ,void *extra ) override {
        if( source == m_wallets ) {
            onWalletSelected( source ,(int) source.id() ,true );
            return;
        }

        if( source == m_wizard ) {
            switch( commandId ) {
                case GUI_COMMANDID_OPEN :
                    if( param == m_stepIndex ) onEnterStep();
                default:
                    break;
            }
        }
    }
};

struct WizStepTrade { // MAYBE market and trade same ?
    UiConnectionWizard &m_wizard;

    //=> select wallet, or manual enter
    //=> or leave

    // market list, select

    //+ how much to trade / withdraw
};

struct WizStepPool {
    //! where to mine

    //=> pool list

    //=> core list
};

//+ advanced ?

//////////////////////////////////////////////////////////////////////////////
///-- wizard

UiConnectionWizard::UiConnectionWizard() {
    addStep( *new WizSelectCoin(*this,WizSelectCoin::miningCoin) ,{"What do you want to mine ?"} );
    addStep( *new WizStepWallet(*this,1) ,{"Where to store your well earned coins ?"} );
    addStep( *new WizSelectCoin(*this,WizSelectCoin::tradeCoin) ,{"What coin to do you want to get ?"} );
    addStep( *new GuiLabel("Your all set, leggo!") ,{""} );
    setStep(0);
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF