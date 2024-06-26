// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
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
    );

    //-- coins //TODO list them all and add
    gui::Assets::Image().addFromManifest(
        "MAXE = { file=assets/coins/maxe.png; }"
        "RTC = { file=assets/coins/rtc.png; }"
        "RTM = { file=assets/coins/rtm.png; }"
        "USDT = { file=assets/coins/usdt.png; }"
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

    // mainWindow->EnableEditor();

    return mainWindow->Create() == ENOERROR;
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF