#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_UI_ASSETS_H
#define SOLOMINER_UI_ASSETS_H

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>

#include "ui.h"
#include "ui-assets.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//! Fonts

#define UI_FONTNAME       "clean"

//? to theme ?

enum AssetFont {
    fontSmall=0 ,fontMedium=1 ,fontLarge=2
    ,fontFirst=0 ,fontLast=2 ,fontDefault=1
};

GuiFont &getFontSmall();
GuiFont &getFontMedium();
GuiFont &getFontLarge();

GuiFont &getFont( AssetFont fontId=AssetFont::fontDefault );

//////////////////////////////////////////////////////////////////////////////
//! Images

//! @note index here to be uppercase only //TODO ??
#define UIIMAGE_HEADER          "HEADER"
#define UIIMAGE_BAR_GREEN       "BAR-GREEN"
#define UIIMAGE_EXCHANGE        "EXCHANGE"

#define UIICONS_MAIN            "MAIN"

//////////////////////////////////////////////////////////////////////////////
//! Assets

#define UIASSETS_FOLDER         "assets/"
#define UIASSETS_FOLDER_ICONS   "icons/"
#define UIASSETS_FOLDER_COINS   "coins/"
#define UIASSETS_FOLDER_POOLS   "pools/"

///--
enum AssetCategory {
    assetImage
    ,assetIconImage
    ,assetCoinImage
    ,assetPoolImage
};

//////////////////////////////////////////////////////////////////////////////
void loadUiAssetFile( GuiImage &image ,const char *filename );
bool loadUiAssets();

///--
GuiImage &getAssetImage( const char *name ,AssetCategory category=assetImage );
GuiImage &getAssetIconImage( const char *name );
GuiImage &getAssetCoinImage( const char *name );
GuiImage &getAssetPoolImage( const char *name );

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_UI_ASSETS_H