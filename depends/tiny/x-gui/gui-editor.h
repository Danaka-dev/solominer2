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

#ifndef TINY_GUI_EDITOR_H
#define TINY_GUI_EDITOR_H

//////////////////////////////////////////////////////////////////////////////
#include "controls/gui-grid.h"

//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE_GUI {

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! CDesigner

    //! @brief control position & size editor

class CDesigner : GUICONTROL_PARENT {
public:
    int m_caretSize = 10;

    Rect m_carets[8];

    CDesigner() {
        ColorQuad color = { OS_COLOR_RED ,OS_NOCOLOR ,OS_COLOR_RED ,OS_NOCOLOR };

        colors() = color;
    }

public:
    void onClick( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) override;

public: /// drag & drop
    GuiControlRef m_drag;

    OsError onGrab( const OsPoint &p ,OsKeyState keyState ,DragOperation &operation ,IObjectRef &object ) override {
        GuiControl *control = root().getMouseTopHit();

        if( !control ) return ENOEXEC;

        m_drag = control;

        for( int i=0; i<8; ++i ) if( m_carets[i] & p ) {
            operation = setDragOpUser( dragOpLocal ,i );
            return ENOERROR;
        }

        return ENOEXEC;
    }

    void onDrag( const OsPoint &p ,DragOperation operation ,IObject *object ,IObject *target ,OsError result ) override {
        int i = getDragOpUser(operation);

        // m_drag->area().left = p.x; //TEST

        switch( i ) {
            case 0: //! topLeft
                break;

            case 7: //! left
                // sizeCoordsX( m_drag ,root().mouseClickPoint().x ,p.x );
                //? how to fully inverse emplace function ? ... update with a delta ?
                break;
        }

        root().Refresh();
        // root().Update( NullPtr ,OS_REFRESH_RESIZED );
    }

    void onDrop( const OsPoint &p ,DragOperation operation ,IObject *object ,IObject *target ,OsError result ) override {
        int i = getDragOpUser(operation);

        switch( i ) {
            case 0: //! topLeft
            break;
        }

        m_drag.Release();
    }

public:
    void drawCaret( int i ,const OsRect &r ) {
        m_carets[i] = r;

        root().SetForeColor( OS_COLOR_RED );
        root().SetFillColor( OS_COLOR_RED );

        root().DrawRectangle( r );
    }

    void drawSizer( int i ,int n ,GuiControl *control ) {
        if( !control ) return;

        Rect r = control->area();
        bool dragdrop = GuiControlWindow::isDragDropInProgress();

        //-- draw box
        if( i == n-1 ) {
            SetForeColor( dragdrop ? OS_COLOR_BLUE : OS_COLOR_RED );
        } else {
            byte color = (byte) (0x0FF * (float) i / (float) n);

            root().SetForeColor( OS_RGB(color,0,0) );
        }

        root().SetFillColor( OS_COLOR_TRANSPARENT );
        root().DrawRectangle( control->area() );

        //-- draw carets
        if( !dragdrop && i == n-1 ) {
            const int &cs = m_caretSize;

            //-- corners
            drawCaret( 0 ,Rect( r.getTopLeft() ,r.getTopLeft() + cs ) );
            drawCaret( 1 ,Rect( r.getTopRight() + Point(-cs,0) ,r.getTopRight() + Point(0,cs) ) );
            drawCaret( 2 ,Rect( r.getBottomRight() - cs ,r.getBottomRight() ) );
            drawCaret( 3 ,Rect( r.getBottomLeft() + Point(0,-cs) ,r.getBottomLeft() + Point(cs,0) ) );

            //-- sides
            drawCaret( 4 ,Rect( Point( r.getCenterH() ,r.top+cs/2 ) ).Inflate( cs/2 ) );
            drawCaret( 5 ,Rect( Point( r.right-cs/2 ,r.getCenterV() ) ).Inflate( cs/2 ) );
            drawCaret( 6 ,Rect( Point( r.getCenterH() ,r.bottom-cs/2 ) ).Inflate( cs/2 ) );
            drawCaret( 7 ,Rect( Point( r.left+cs/2 ,r.getCenterV() ) ).Inflate( cs/2 ) );
        }
    }

public:
    void onDraw( const OsRect &updateArea ) override {
        GuiControl::onDraw( updateArea );

        auto &controls = root().getHitTrackMouse();

        int n = (int) controls.size();

        int i=0; for( auto &control : controls ) {
            drawSizer( i ,n ,control.ptr() ); ++i;
        }
    }

    void onMouseMove( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) override {
        GuiControl::onMouseMove( p ,mouseButton ,keyState );

        auto &controls = root().getHitTrackMouse();

        if( controls.size() > 0 && *controls.rbegin() != m_control ) {
            m_control = controls.rbegin()->ptr();
            root().Refresh();
        }
    }

    GuiControl *m_control = NullPtr; //! last topmost control
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! CControlList

    //! @brief list of available controls (drag and drop)

class CControlList : public GuiList {
public:
    static Map_<String,String> &friendlyNames() { return m_friendlyNames; }

    void addFriendly( const char *name ,const char *friendly ) {
        friendlyNames().addItem( name ,name+3 );
    }

    const String *findFriendly( const char *name ) {
        if( name && strimatch(name,"Gui")==0 ) {
            addFriendly( name ,name+3 );
        }

        return friendlyNames().findItem( name );
    }

    const String *digFriendly( const char *name ) const {
        return friendlyNames().digItem( name );
    }

public:
    void Initialize() {
        placement() = placeZigzag;
        direction() = (Direction) (directionRight | directionBottom);
        origin() = topLeft;

        itemSize() = Point(105,105);

        makeList();
    }

    void makeList() {
        auto &factory = Factory_<GuiControl>::getInstance();

        ListOf<PUID> classes;

        factory.listClasses( classes );

        String name;

        for( auto &it : classes ) {
            if( !findClassNameById( it ,name ) ) continue;

            GuiImage *image = Assets::Image().get( tocstr(name) );
            const String *friendly = findFriendly( tocstr(name) );

            addControl( * new GuiImageBox( image ,tocstr( friendly ? *friendly : name ) ) );
        }
    }

    OsError onGrab( const OsPoint &p ,OsKeyState keyState ,DragOperation &operation ,IObjectRef &object ) override {
        GuiImageBox *image = object ? object->As_<GuiImageBox>() : NullPtr;
        //! @note object is guaranteed to be a child control

        if( !image ) return ENODATA;

        operation = dragOpCopy;

        const char *friendly = image->getText();

        const String *name = digFriendly( friendly );

        object = ICreateObject_<GuiControl>( name ? tocstr( *name ) : friendly );

        return object ? ENOERROR : EFAILED;
    }

    OsError onDropAccept( const OsPoint &p ,IObject *source ,DragOperation operation ,IObject *object ,bool preview ) override {
        return ENOEXEC;
    }

protected:
    static Map_<String,String> m_friendlyNames;
};

class CEditorControls : public GuiControlWindow ,public Singleton_<CEditorControls> {
public:
    CEditorControls() : GuiControlWindow( "tiny-editor-controls" ,"Controls" ,360 ,800 ,OS_WINDOWSTYLE_TOOLBOX )
    {
        Assets::Image().addFromManifest(
            "GuiButton = { file=assets/gui-button.png; }"
            "GuiImageBox = { file=assets/gui-imagebox.png; }"
            "GuiLabel = { file=assets/gui-label.png; }"
            "GuiTextBox = { file=assets/gui-textbox.png; }"
        );

        m_controlList.Initialize();
        foreground().addControl( m_controlList );
    }

    CControlList m_controlList;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! CEditProperties

    //! @brief control properties editor

    //TODO use GuiSheet
class CEditProperties : public GuiGrid {
public:
    void Initialize() {
        addCol( 50.f ,"property" );
        addCol( 50.f ,"value" );

        setCellMargin(4);
        setTitleHeight( GuiCoord(36) );
        setRowHeight( GuiCoord(32) );

        //! @note TODO would like to have a theme by window, not global
        // auto *theme = theThemeStore().getTheme("editor");
    }

    virtual void Bind( IDataSource *datasource ) {
        Clear(false);

        m_datasource = datasource;

        if( !datasource ) return;

        Params params;

        datasource->readHeader( params ,true );

        for( const auto &it : params ) {
            int irow = addRow();

            NameType decl;

            fromString( decl ,tocstr(it.first) );

            setCellText( irow ,0 ,tocstr(decl.name) );
            setCellText( irow ,1 ,tocstr(it.second) );

            if( !decl.type.empty() ) {
                setCellType( irow ,1 ,tocstr(decl.type) ,true );
            } else {
                setCellType( irow ,1 ,TINY_STRING_PUID ,true );
            }

            row(irow).col(1).edit->Bind( tocstr(decl.name) ,*datasource );
        }

        Update();
    }

protected:
    GuiControlRef m_control;
};

//////////////////////////////////////////////////////////////////////////////
class CEditorProperties : public GuiControlWindow ,public Singleton_<CEditorProperties> {
public:
    CEditorProperties();

    void makeControlStack( ListOf<GuiControlRef> &controlStack );
    void selectStackControl( int i );
    void attachControl( GuiControl *control ,ListOf<GuiControlRef> &controlStack );

public:
    API_IMPL(void) onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) IOVERRIDE;

protected:
    PropertiesDataSource m_propertiesData;
    ListOf<GuiControlRef> m_controlStack;

    GuiComboBox m_propertiesStack;
    CEditProperties m_propertiesList;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

    //! LATER EditorEvent

//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE_GUI

//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
#endif //TINY_GUI_EDITOR_H