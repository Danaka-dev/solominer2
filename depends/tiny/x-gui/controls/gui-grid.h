#pragma once

/****************************************************** //
//              tiny-for-c++ v3 library                 //
//              -----------------------                 //
//   Copyright (c) 2016-2024 | NEXTWave Technologies    //
//      <http://www.nextwave-techs.com/>                //
// ******************************************************/

//! Check if your project qualifies for a free license
//!   at http://nextwave-techs.com/license

//! THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//! IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//! FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//!        AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//! LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//! OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//! SOFTWARE.

#ifndef TINY_GUI_GRID_H
#define TINY_GUI_GRID_H

//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
//! Definitions

#define TINY_GUIGRID_UUID   0x031ecaf6659b9050a

//////////////////////////////////////////////////////////////////////////////
//! GuiGrid

class GuiGrid : public GuiWithFont ,GUICONTROL_PARENT {
public:
    struct Cell {
        String text;
        GuiEditBoxRef edit;
        bool editable;
    };

    struct Row {
        Cell &col( int i ) { return m_cells[i]; }

        ListOf<Cell> m_cells;
    };

    struct Col {
        GuiCoord coord; //! configured coords
        int width; //! placed size

        String title;
        GuiEditBoxRef edit;
        bool editable;
    };

    struct Config {
        bool showTitle;
        bool showColLines;
        bool showRowLines;

        TextAlign titleAlign;
        TextAlign cellAlign;
    };

    struct Colors {
        ColorQuad normal;
        ColorQuad title;
        ColorQuad col1 ,col2; //! alternate coloring for cols / rows
        ColorQuad row1 ,row2;
    };

public:
    GuiGrid();

    DECLARE_GUICONTROL(GuiControl,GuiGrid,TINY_GUIGRID_UUID);
    DECLARE_GUIPROPERTIES;

    size_t rowCount() { return m_rows.size(); }
    size_t colCount() { return m_cols.size(); }

    Col &col( int i ) { return m_cols[i]; }
    Row &row( int i ) { return m_rows[i]; }

public:
    Config &config() { return m_config; }

    void setTitleHeight( const GuiCoord &height ) {
        m_titleCoord = height;
    }

    void setRowHeight( const GuiCoord &height ) {
        m_rowCoord = height;
    }

    void setCellMargin( int margin ) {
        m_cellMargin = margin;
    }

    void Reset( bool withRefresh=true );

public:
    void addCol( const GuiCoord &width ,const char *title=NullPtr );
    int addRow();

    void setCellText( int y ,int x ,const char *text );
    void setCellEdit( int y ,int x ,GuiEditBox *editor ,bool editable=false );
    void setCellType( int y ,int x ,const UUID &editableId ,bool editable=false );
    void setCellType( int y ,int x ,const char *type ,bool editable=false );

    // getCellArea() ...

    void Clear( bool withRefresh=true );

protected:
    API_IMPL(void) onLayout( const OsRect &clientArea ,OsRect &placeArea ) IOVERRIDE;
    API_IMPL(void) onDraw( const OsRect &updateArea ) IOVERRIDE;
    API_IMPL(void) onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) IOVERRIDE;
    API_IMPL(void) onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) IOVERRIDE;
    API_IMPL(void) onTimer( OsTimerAction timeAction ,OsEventTime now ,OsEventTime last ) IOVERRIDE;

protected:

//--
    GuiEditBox *getEditBox( const UUID &editableId );
    GuiEditBox *getEditBox( const char *editable );

    // void startEdit( int y ,int x );

//--
    ColorQuad &getColColors( int i ) {
        return (i & 1) == 0 ? m_colors.col1 : m_colors.col2;
    }

    ColorQuad &getRowColors( int i ) {
        return (i & 1) != 0 ? m_colors.row1 : m_colors.row2;
    }

    void drawColLines( bool forceDraw=false );
    void drawRowLine( int i ,const Rect &r ,bool forceDraw=false );

protected:

//-- configured coordinates
    GuiCoord m_titleCoord;
    GuiCoord m_rowCoord;

//-- placed values
    int m_titleHeight;
    int m_rowHeight;

//-- appearance
    int m_cellMargin;
    Colors m_colors;

//-- content
    Config m_config;

    ListOf<Col> m_cols;
    ListOf<Row> m_rows;

//-- edit
    // MapOf<UUID,GuiEditBoxRef> m_edits;
        //! @note editors my be set manually per col/cell, or using NameType in title

    GuiEditBoxRef m_editFocus;
};

//--
template <>
GuiGrid::Config &Init( GuiGrid::Config &p );

//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
#endif //TINY_GUI_GRID_H