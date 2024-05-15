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
TINY_GUI_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
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
                //? how to fully inverse emplace function ? .. update with a delta ?
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

        //-- draw box
        if( i == n-1 ) {
            root().SetForeColor( OS_COLOR_RED );
        } else {
            byte color = (byte) (0x0FF * (float) i / (float) n);

            root().SetForeColor( OS_RGB(color,0,0) );
        }

        root().SetFillColor( OS_COLOR_TRANSPARENT );
        root().DrawRectangle( control->area() );

        //-- draw carets
        if( i == n-1 ) {
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
    void onDraw( const OsRect &uptadeArea ) override {
        GuiControl::onDraw( uptadeArea );

        auto &controls = root().getHitTrackMouse();

        int n = controls.size();

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
class CControlList : public GuiList {
public:
    void Initialize() {
        placement() = placeLine;
        direction() = directionBottom;
        origin() = topLeft;

        itemSize() = Point(75,75);

        makeList();
    }

    void makeList() {
        auto &factory = Factory_<GuiControl>::getInstance();

        ListOf<UUID> classes;

        factory.listClasses( classes );

        String name;

        for( auto &it : classes ) {
            if( !getClassNameFromId( it ,name ) ) continue;

            GuiImage *image = Assets::Image().get( name.c_str() );

            addControl( * new GuiImageBox( image ,name.c_str() ) );
        }
    }

    OsError onGrab( const OsPoint &p ,OsKeyState keyState ,DragOperation &operation ,IObjectRef &object ) override {
        GuiImageBox *image = object ? object->As_<GuiImageBox>() : NullPtr;
        //! @note object is guaranteed to be a child control

        if( !image ) return ENODATA;

        operation = dragOpCopy;

        const char *name = image->getText();

        object = ICreateObject_<GuiControl>( name );

        return object ? ENOERROR : EFAILED;
    }

protected:
    //+ Drag & Drag control from here to windows (may accept drop of control directly in GuiSet)
};

class CEditorControls : public GuiControlWindow ,public Singleton_<CEditorControls> {
public:
    CEditorControls() : GuiControlWindow( "tiny-editor-controls" ,"Controls" ,360 ,800 ,OS_WINDOWSTYLE_TOOLBOX )
    {
        gui::Assets::Image().addFromManifest(
            "GuiImageBox = { file=assets/icon_imagebox.png; }"
            "GuiLabel = { file=assets/icon_label.png; }"
        );

        m_controlList.Initialize();
        foreground().addControl( m_controlList );
    }

    CControlList m_controlList;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
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

    void makeList( GuiControl *control ) {
        Clear(false);

        m_control = control; if( !control ) return;

        Params params;

        control->getProperties( params );

        for( const auto &it : params ) {
            int row = addRow();

            NameType decl;

            fromString( decl ,it.first.c_str() );

            setCellText( row ,0 ,decl.name.c_str() );
            setCellText( row ,1 ,it.second.c_str() );

            if( !decl.type.empty() ) {
                setCellType( row ,1 ,decl.type.c_str() ,true );
            } else {
                setCellType( row ,1 ,TINY_STRING_UUID ,true );
            }
        }

        Refresh();
    }

protected:
    GuiControlRef m_control;
};

/* class CEditProperties : public GuiList { // GuiGrid ... => edit
public:
    // DECLARE_GUICONTROL(GuiImageBox,TINY_GUIIMAGEBOX_UUID);

    void Initialize() {
        placement() = placeLine;
        direction() = directionBottom;
        origin() = topLeft;

        itemSize() = Point(200,50);
    }

    GuiControlRef m_control;

    void makeList( GuiControl *control ) {
        m_control = control; if( !control ) return;

        removeAllControls();

        Params params;

        control->getProperties( params );

        for( const auto &it : params ) {
            String s;

            Format( s ,"%s = %s" ,256 ,it.first.c_str() ,it.second.c_str() );
            addControl( * new GuiLabel( s.c_str() ) );

            // addControl( * new GuiLabel( it.second.c_str() ) );
        }

        root().Update( NullPtr ,refreshResized );
    }
}; */

//////////////////////////////////////////////////////////////////////////////
class CEditorProperties : public GuiControlWindow ,public Singleton_<CEditorProperties> {
public:
    CEditorProperties() : GuiControlWindow( "tiny-editor-properties" ,"Properties" ,360 ,800 ,OS_WINDOWSTYLE_TOOLBOX )
    {
        m_propertiesList.Initialize();
        foreground().addControl( m_propertiesList );
    }

    void attachControl( GuiControl *control ) {
        m_propertiesList.makeList( control );
    }

    CEditProperties m_propertiesList;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

    //! LATER EditorEvent

//////////////////////////////////////////////////////////////////////////////
} //TINY_GUI_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
#endif //TINY_GUI_EDITOR_H