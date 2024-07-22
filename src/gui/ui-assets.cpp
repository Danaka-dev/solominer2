// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include <solominer.h>
#include <coins/cores.h>
#include <pools/pools.h>

#include "ui.h"
#include "ui-assets.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//! Fonts

GuiFont g_fontSmall( UI_FONTNAME ,25 ,OS_FONTWEIGHT_NORMAL ,OS_FONTSTYLE_ITALIC ,OS_FONTPITCH_ANY );
GuiFont g_fontMedium( UI_FONTNAME ,38 ,OS_FONTWEIGHT_NORMAL ,OS_FONTSTYLE_ITALIC ,OS_FONTPITCH_ANY );
GuiFont g_fontLarge( UI_FONTNAME ,51 ,OS_FONTWEIGHT_NORMAL ,OS_FONTSTYLE_ITALIC ,OS_FONTPITCH_ANY );

GuiFont *g_assetFont[] = {
    &g_fontSmall ,&g_fontMedium ,&g_fontLarge
};

GuiFont &getFontSmall() { return g_fontSmall; }
GuiFont &getFontMedium() { return g_fontMedium; }
GuiFont &getFontLarge() { return g_fontLarge; }

GuiFont &getFont( AssetFont fontId ) {
    return * g_assetFont[ CLAMP(fontId,AssetFont::fontFirst,AssetFont::fontLast) ];
}

//////////////////////////////////////////////////////////////////////////////
//! Assets

///--
typedef MapOf<String,GuiImage> AssetStore;

AssetStore g_uiImages;
AssetStore g_uiIconImages;
AssetStore g_uiCoinImages;
AssetStore g_uiPoolImages;

///--
AssetStore &getAssetStore( AssetCategory category ) {
    switch( category ) {
        case assetImage: return g_uiImages;
        case assetIconImage: return g_uiIconImages;
        case assetCoinImage: return g_uiCoinImages;
        case assetPoolImage: return g_uiPoolImages;
    }

    assert(false);

    return g_uiImages; //! default
}

//--
void loadUiAssetFile( GuiImage &image ,const char *filename ) {
    std::string name = filename; tolower(name);

    image.LoadFromFile( name.c_str() );
}

bool loadUiAssets() {

///-- IMAGEs
    {
        GuiImage &headerImage = g_uiImages[ UIIMAGE_HEADER ];
        headerImage.LoadFromFile( UIASSETS_FOLDER "header.png" );

        GuiImage &barGreen = g_uiImages[ UIIMAGE_BAR_GREEN ];
        loadUiAssetFile( barGreen ,UIASSETS_FOLDER "bar-green.png" );

        GuiImage &exchange = g_uiImages[ UIIMAGE_EXCHANGE ];
        loadUiAssetFile( exchange ,UIASSETS_FOLDER "exchange.png" );
    }

///-- ICONs
    {
        GuiImage &soloIcons = g_uiIconImages[ UIICONS_MAIN ];
        loadUiAssetFile( soloIcons ,UIASSETS_FOLDER UIASSETS_FOLDER_ICONS "solo-icons.png" );

        ListOf<String> icons = { "rtc-core" };

        for( const auto &it : icons ) {
            String name = it; toupper(name);

            GuiImage &image = g_uiIconImages[ name.c_str() ];

            String path;
            Format( path ,"%s%s.png" ,1024 ,UIASSETS_FOLDER UIASSETS_FOLDER_ICONS ,it.c_str() );
            tolower( path );

            loadUiAssetFile( image ,path.c_str() );
        }
    }

///-- COINs
    CCoinStore &coinStore = CCoinStore::getInstance();

    auto coins = coinStore.getList();

    for( const auto &it : coins ) if( !it.isNull() ) {
        const auto &coin = **it;

        String name = coin.getTicker(); toupper( name );

        GuiImage &image = g_uiCoinImages[ name ];

        String path;
        Format( path ,"%s%s.png" ,1024 ,UIASSETS_FOLDER UIASSETS_FOLDER_COINS ,coin.getTicker().c_str() );
        tolower( path );

        image.LoadFromFile( path.c_str() );
    }

///-- POOLs
    CPoolList &poolList = getPoolListInstance();

    ListOf<String> pools;

    poolList.listPools( pools );

    for( const auto &pool : pools ) {
        String name = pool; toupper( name );

        GuiImage &image = g_uiPoolImages[ name ];

        String path;
        Format( path ,"%s%s.png" ,1024 ,UIASSETS_FOLDER UIASSETS_FOLDER_POOLS ,pool.c_str() );
        tolower( path );

        image.LoadFromFile( path.c_str() );
    }

///--
    return true;
}

///--
GuiImage &getAssetImage( const char *name ,AssetCategory category ) {
    static GuiImage noImage;

    AssetStore &store = getAssetStore(category);

    std::string s_name = name; toupper( s_name );

    auto it = store.find( s_name );

    if( it == store.end() )
        return noImage;

    return it->second;
}

GuiImage &getAssetIconImage( const char *name ) {
    return getAssetImage( name ,assetIconImage );
}

GuiImage &getAssetCoinImage( const char *name ) {
    return getAssetImage( name ,assetCoinImage );
}

GuiImage &getAssetPoolImage( const char *name ) {
    return getAssetImage( name ,assetPoolImage );
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//!EOF