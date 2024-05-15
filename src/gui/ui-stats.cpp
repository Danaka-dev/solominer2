// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include "ui.h"
#include "ui-stats.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
void UiStats::onDraw( const OsRect &updateArea ) {
    GuiControl::onDraw(updateArea);

    int n = m_stats.getStatCount();
    int t = m_stats.getCurrentTime();

    Rect bar = area();

    int w = bar.getWidth() / n;
    int h = bar.getHeight();

    bar.left = bar.right - w;

    double mini ,maxi;

    m_stats.findBounds( mini ,maxi );

    mini = 0; maxi = (maxi > 1) ? maxi : 1;

    for( int i=0; i<n; ++i ) {
        double v = m_stats.getStat(t - i).sum; // getAvg();

        bar.top = bar.bottom - h * v / maxi;

        if( m_barImage ) {
            root().DrawImage( *m_barImage
                    ,bar.left ,bar.top ,bar.right ,bar.bottom
                    ,0 ,0 ,bar.getWidth() ,bar.getHeight()
            );
        }

        bar.right = bar.left; bar.left -= w;
    }
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF