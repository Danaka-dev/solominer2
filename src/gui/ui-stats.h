#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_UI_STATS_H
#define SOLOMINER_UI_STATS_H

//////////////////////////////////////////////////////////////////////////////
#include <common/stats.h>

#include "ui.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
#define UISTATS_UUID        0x0d9ac96e634bc5600

//////////////////////////////////////////////////////////////////////////////
class UiStats : GUICONTROL_PARENT {
public:
    UiStats( int count=32 ) : m_stats( count ,Stats::TimeUnit::Minute )
        ,m_barImage(NULL)
    {}

    DECLARE_GUICONTROL(GuiControl,UiStats,UISTATS_UUID);

    Stats &stats() {
        return m_stats;
    }

    void setBarImage( GuiImage *image ) {
        m_barImage = image;
    }

public:
    virtual void onDraw( const OsRect &updateArea );

protected:
    Stats m_stats;

    GuiImage *m_barImage;
};

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_UI_STATS_H