#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_UI_THEME_H
#define SOLOMINER_UI_THEME_H

//////////////////////////////////////////////////////////////////////////////
#include "ui.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
/*
enum VisualTheme {
    themeCosmogony=0 ,themeDark=1 ,themeLight=2
    ,themeFirst=0 ,themeLast=2 ,themeDefault=0
};

//////////////////////////////////////////////////////////////////////////////
//! Cosmogony

#define THEME_COSMOS_BLACK
#define THEME_COSMOS_WHITE
#define THEME_COSMOS_BLUE

//////////////////////////////////////////////////////////////////////////////
struct UiTheme {
//-- default
    OsColorRef foreColor = OS_RGB(210,210,210);
    OsColorRef backColor = OS_RGB(8,8,8);

    OsColorRef fillColor = OS_COLOR_BLUE; // OS_RGB(8,8,8)
    OsColorRef textColor = OS_RGB(210,210,210); // OS_RGB(8,8,8)

};

///-- stock theme
const UiTheme &getStockTheme( VisualTheme themeId );

//+ save/load

///-- active theme
void setTheme( VisualTheme themeId );

void setTheme( const UiTheme &theme );

const UiTheme &getTheme();
*/

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_UI_THEME_H