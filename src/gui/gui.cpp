// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include <solominer.h>

#include "ui.h"
#include "ui-theme.h"
#include "ui-assets.h"
#include "ui-dashboard.h"

#include "gui.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
bool uiInitialize( CConnectionList &connections ) {
    static CDashboardWindow *mainWindow = nullptr;

    if( mainWindow != nullptr )
        return true;

//-- images
    gui::Assets::Image().addFromManifest( "header = { file=assets/header.png; }" );
    gui::Assets::Image().addFromManifest(
        "icons = { "
            "file=assets/icons/solo-icons.png;"
            // "thumbmap=regular,{8,8,72,72},8,10,25,46;"
            "thumbmap=regular,{16,16,64,64},8,10,41,62;"
        " }"
        "minis = { "
            "file=assets/icons/solo-icons32.png;"
            // "thumbmap=regular,{8,8,72,72},8,10,25,46;"
            "thumbmap=regular,{8,8,32,32},8,10,20,31;"
        " }"
    );

    //-- coins //TODO list them all and add
    gui::Assets::Image().addFromManifest(
        "nocoin = { file=assets/coins/nocoin.png; }"
        "BTC = { file=assets/coins/btc.png; }"
        "BTRM = { file=assets/coins/btrm.png; }"
        "MAXE = { file=assets/coins/maxe.png; }"
        "RTC = { file=assets/coins/rtc.png; }"
        "RTM = { file=assets/coins/rtm.png; }"
        "USDT = { file=assets/coins/usdt.png; }"
    );

    //-- pools
    gui::Assets::Image().addFromManifest(
        "FLOCKPOOL = { file=assets/pools/flockpool.png; }"
        "RPLANT = { file=assets/pools/rplant.png; }"
        "SOLO = { file=assets/pools/solo.png; }"
    );

//-- fonts
    gui::Assets::Font().addFromManifest(
        "default = ;"
        "small = { size=12; }"
        "medium = { size=24; }"
        "large = { size=32; }"
        "huge = { size=51; }"
        "italic = { style=italic; }"
    );

//--
    loadUiAssets(); //TODO remove this, use tiny assets instead

//-- window
    mainWindow = new CDashboardWindow( connections );

    if( mainWindow == nullptr ) {
        throw std::runtime_error("[uiInitialize]  Memory error while allocating main window");
    }

    mainWindow->EnableEditor();

    if( mainWindow->Create() != ENOERROR )
        return false;

//-- login
    if( getGlobalLogin().empty() )
        return true;

    mainWindow->showLoginDialog();

    while( OsSystemDoEvents() == ENOERROR ) {
        OsSleep(10);

        if( globalIsLogged() ) break;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF