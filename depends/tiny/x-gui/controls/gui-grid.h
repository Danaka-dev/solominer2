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

#define TINY_GUIGRID_PUID       0x031ecaf6659b9050a
#define TINY_GUINAVBAR_PUID     0x07de60d82627b3359
#define TINY_GUINAVGRID_PUID    0x0fec5716871a51908

//////////////////////////////////////////////////////////////////////////////
//! GuiGrid

class GuiGrid : public GuiWithFont ,public GuiPublisher ,public IDataEvents ,GUICONTROL_PARENT {
public:
    struct Cell {
        String text; //! @note if edit present text might not be updated
        GuiEditBoxRef edit;
        bool editable;
    };

    struct Row {
        Cell &col( int i ) { return m_cells[i]; }

        size_t index; //! @note used as seek id, if using datasource
        ListOf<Cell> m_cells;
    };

    struct Col {
        GuiCoord coord; //! configured coords
        int width; //! placed size

        String title;
        NameType field; //! data field
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

    DECLARE_GUICONTROL(GuiControl,GuiGrid,TINY_GUIGRID_PUID);
    DECLARE_GUIPROPERTIES;

    size_t rowCount() { return m_rows.size(); }
    size_t colCount() { return m_cols.size(); }

    Col &col( int i ) { return m_cols[(size_t) i]; }
    Row &row( int i ) { return m_rows[(size_t) i]; }

    size_t baseIndex() { return m_baseIndex; }

    bool &selectable() { return m_selectable; }

    IDataSource *getDataSource() {
        return m_datasource.ptr();
    }

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
    void setCols( int cols ,const ListOf<String> *titles=NullPtr ,const ListOf<String> *fields=NullPtr );
    void setRows( int rows ,size_t baseIndex=0 );

    void addCol( const GuiCoord &width ,const char *title=NullPtr ,const char *field=NullPtr ,bool editable=false );
    int addRow();

    void setCellText( int y ,int x ,const char *text );
    void setCellEdit( int y ,int x ,GuiDataEdit *editor ,bool editable=false );
    void setCellType( int y ,int x ,const PUID &editableId ,bool editable=false );
    void setCellType( int y ,int x ,const char *type ,bool editable=false );

    // getCellArea() ...

    void Clear( bool withRefresh=true );

public: ///-- Data
    //! @note Bind might be override to implement non standard binding between data source and grid layout
    API_DECL(void) Bind( IDataSource *datasource );
    API_DECL(void) UpdateData();

protected: ///-- IDataEvents
    IAPI_IMPL onDataCommit( IDataSource &source ,Params &data ) IOVERRIDE;
    IAPI_IMPL onDataChanged( IDataSource &source ,const Params &data ) IOVERRIDE;

public: ///-- IGuiControlEvents
    API_IMPL(void) onMouseLeave( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) IOVERRIDE;

protected: ///-- IGuiEvents
    API_IMPL(void) onLayout( const OsRect &clientArea ,OsRect &placeArea ) IOVERRIDE;
    API_IMPL(void) onDraw( const OsRect &updateArea ) IOVERRIDE;
    API_IMPL(void) onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) IOVERRIDE;
    API_IMPL(void) onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) IOVERRIDE;
    API_IMPL(void) onTimer( OsTimerAction timeAction ,OsEventTime now ,OsEventTime last ) IOVERRIDE;

protected:
    GuiDataEdit *makeEditBox( const char *editable );

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
    //! @note editors my be set manually per col/cell, or using NameType in title
    size_t m_baseIndex; //! @note base index (first row index)

    IDataSourceRef m_datasource;
    GuiEditBoxRef m_editFocus;

//-- select
    bool m_selectable;
    int m_hooverRow;

    ColorRef m_textHooverColor;
};

//--
template <>
GuiGrid::Config &Init( GuiGrid::Config &p );

//////////////////////////////////////////////////////////////////////////////
//! GuiNavBar

class GuiNavBar : public GuiGroup {
public:
    GuiNavBar();

    DECLARE_GUICONTROL(GuiGroup,GuiNavBar,TINY_GUINAVBAR_PUID);

    void Bind( IGuiMessage &listener );
};

//////////////////////////////////////////////////////////////////////////////
//! GuiNavGrid

class GuiNavGrid : public GuiGroup {
public:
    GuiNavGrid();

    DECLARE_GUICONTROL(GuiGroup,GuiNavGrid,TINY_GUINAVGRID_PUID);

    GuiGrid *Grid() { return m_grid.ptr(); }
    GuiNavBar *Nav() { return m_nav.ptr(); }

public:
    void updateInfo();
    void updatePage( int pageId );

protected:
    API_IMPL(void) onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) IOVERRIDE;

protected:
    RefOf<GuiGrid> m_grid;
    RefOf<GuiNavBar> m_nav;

    int m_pageId; //! active page in grid
    int m_dataCount; //! count of data
    int m_rowCount; //! grid row count
    int m_pageCount; //! page count from datasource
};

//////////////////////////////////////////////////////////////////////////////
class GuiSheet : public GuiGrid {
public:
    struct Heading {
        String field; //! @note "*" will also add all fields from datasource
        String label; //! @note "" empty label will ignore field

        GuiControlRef edit;
    };

public:
    GuiSheet() {
        addCol( 50.f ); // ,"property" );
        addCol( 50.f ); // ,"value" );

        setCellMargin(4);
        setTitleHeight( GuiCoord(36) );
        setRowHeight( GuiCoord(32) );

        config().showTitle = false;
    }

    ListOf<Heading> &headings() { return m_headings; }

    bool hasHeading( const char *field );

public: ///-- interface
    IAPI_DECL onBindField( NameType &decl ) {
        return IOK;
    }

public: ///-- GuiGrid
    API_IMPL(void) Bind( IDataSource *datasource ) IOVERRIDE;

protected:
    ListOf<Heading> m_headings;
};

//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
#endif //TINY_GUI_GRID_H