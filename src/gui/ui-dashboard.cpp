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
            coinThumbnail.colors().fillColor = OS_COLOR_TRANSPARENT;
            addControl( coinThumbnail );

            if( m_connection.hasTrade() ) {
                tradeIcon.setImage( getAssetImage( "EXCHANGE" ) );
                tradeIcon.align() = (GuiAlign) (alignLeft | alignAnchor);
                tradeIcon.coords() = { 0 ,0 ,12 ,90.f  };
                tradeIcon.colors().fillColor = OS_COLOR_TRANSPARENT;
                addControl( tradeIcon );

                statLeftPos += 16;

                tradeThumbnail.setImage( getAssetCoinImage( tradeName ) );
                tradeThumbnail.text() = tradeName;
                tradeThumbnail.align() = (GuiAlign) (alignLeft | alignAnchor);
                tradeThumbnail.coords() =  { 0 ,0 ,(int) (UI_CONNECTION_HEIGHT*0.8f) ,90.f  };
                tradeThumbnail.colors().fillColor = OS_COLOR_TRANSPARENT;
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
            poolIcon.setText( "" ); // poolName );
            poolIcon.coords() = { poolLeftPos ,10.f ,actionLeftPos,70.f };
            poolIcon.colors().fillColor = OS_COLOR_TRANSPARENT;
            addControl( poolIcon );

            poolLabel.setPropertiesWithString( "text=disconnected; textalign=center; font=normal;" );
            poolLabel.colors().fillColor = OS_COLOR_TRANSPARENT;
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
            actionAuto.colors().fillColor = OS_COLOR_TRANSPARENT;
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
            balanceLabel.colors().fillColor = OS_COLOR_TRANSPARENT;
            balanceLabel.hooverColor() = OS_RGB(160,160,250);
            addControl( balanceLabel );

            ///-- earning
            defaultValueReference( m_earningReference );
            earningLabel.setCommandId( COMMAND_EARNINGS );
            earningLabel.textAlign() = (TextAlign) (textalignRight | textalignCenterV);
            earningLabel.setFont( getFontSmall() );
            earningLabel.coords() = { balanceLeftPos ,60.f ,97.f ,100.f };
            earningLabel.colors().fillColor = OS_COLOR_TRANSPARENT;
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
    ,m_uiLoginDialog(this)
    ,m_uiEarningsDialog( connections.earnings() )
    ,m_uiTradeDialog(*this)
    ,m_mainSettings(this)
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