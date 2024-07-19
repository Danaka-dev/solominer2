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

#ifndef TINY_GUI_EDIT_H
#define TINY_GUI_EDIT_H

//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
//! Definitions

#define TINY_GUIDATAEDIT_PUID   0x050832d6537d2b96f
#define TINY_DATALABEL_PUID     0x0efd8060142f9eacf
#define TINY_GUITEXTBOX_PUID    0x07d5b0de21313e320
#define TINY_GUICOMBOBOX_PUID   0x000640c9cf93771b3
#define TINY_GUICOLORBOX_PUID   0x0b0f71ead71b710a2
#define TINY_GUIBOOLBOX_PUID    0x0eaa2350b651020d0
#define TINY_GUIALIGNBOX_PUID   0x005f3d0f01712b1b7

//--
class GuiDataEdit;
class GuiDataLabel;
class GuiTextBox;
class GuiComboBox;
class GuiColorBox;
class GuiBoolBox;
class GuiAlignBox;

//////////////////////////////////////////////////////////////////////////////
//! GuiDataEdit

class GuiDataEdit : public virtual IDataEvents ,GUICONTROL_PARENT {
    //! Base class for edit control bound to one value from a data source

public:
    DECLARE_OBJECT_STD(GuiControl,GuiDataEdit,TINY_GUIDATAEDIT_PUID);
    DECLARE_GUIPROPERTIES;

public: //-- factory
    static bool RegisterEditor( const PUID &editableId ,const PUID &editorId );
    static bool findEditor( const PUID &editableId ,PUID &editorId );

public: //-- binding
    void Bind( const char *field ,IDataSource &source );

    virtual void setValue( const char *value ) = 0;
    virtual void getValue( String &value ) = 0;

protected: ///-- IDataEvents
    IAPI_IMPL onDataCommit( IDataSource &source ,Params &data ) IOVERRIDE;
    IAPI_IMPL onDataChanged( IDataSource &source ,const Params &data ) IOVERRIDE;

protected:
    IDataSourceRef m_dataSource; //! data source which bound to
    String m_dataField; //! data field name bound to
};

typedef RefOf<GuiDataEdit> GuiEditBoxRef;

GuiDataEdit *ICreateGuiEdit( const PUID &editableId );

#define REGISTER_EDITBOX(__editable,__editor) \
    static bool g_##__editor##_registered = GuiDataEdit::RegisterEditor( classId_<__editable>(),__editor::classId() );

//////////////////////////////////////////////////////////////////////////////
//! GuiDataEdit_

template <typename T>
class GuiDataEdit_ : public GuiDataEdit {
public:
    virtual T &getFieldValue() = 0;

public:
    API_IMPL(void) setValue( const char *v ) IOVERRIDE {
        fromString( getFieldValue() ,v );
    }

    API_IMPL(void) getValue( String &v ) IOVERRIDE {
        toString( getFieldValue() ,v );
    }

protected: //-- data interface
    void readData() {
        if( !m_dataSource || m_dataField.empty() ) return;

        Params data;

        data[ m_dataField ] = "";

        if( m_dataSource->readData( data ) != IOK ) return;

        setValue( tocstr( data[ m_dataField ] ) );
    }

    void onDataEdit( bool softEdit=false ) {
        if( !m_dataSource || m_dataField.empty() ) return;

        Params data;

        // if( !softEdit ) //! @note soft edit means we want to advise datasource edit is in progress, but don't want to transact data yet
        getValue( data[ m_dataField ] );

        m_dataSource->onDataEdit( data );
    };
};

//////////////////////////////////////////////////////////////////////////////
//! GuiLabelData

    //! @brief a label control with data binding

class GuiDataLabel : public GuiLabel ,public GuiDataEdit_<String> {
public:
    DECLARE_GUICONTROL(GuiDataEdit,GuiDataLabel,TINY_DATALABEL_PUID);
    DECLARE_GUIPROPERTIES;

    API_IMPL(String) &getFieldValue() IOVERRIDE { return GuiLabel::m_text; }
};

//////////////////////////////////////////////////////////////////////////////
//! GuiTextBox

    //TODO GuiPublisher -> Notify

class GuiTextBox : public GuiWithText ,public GuiDataEdit_<String> {
public:
    GuiTextBox();

    DECLARE_GUICONTROL(GuiDataEdit,GuiTextBox,TINY_GUITEXTBOX_PUID);
    DECLARE_GUIPROPERTIES;

    API_IMPL(String) &getFieldValue() IOVERRIDE { return m_text; }

public:
    void onGotFocus() override {
        showCursorNow(true);
    }

    void onLostFocus() override {
        m_offset = m_cursor = 0; showCursorNow(false);
    }

    void onMouseDown( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) override;

    void onDraw( const OsRect &updateArea ) override;
    //TODO on mouse down set cursor position
    void onKeyDown( OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) override;
    void onTimer( OsTimerAction timeAction ,OsEventTime now ,OsEventTime last ) override;

protected:
    int updateOffset( int delta );
    void showCursorNow( bool show=true );
    void drawCursor( int x );

protected:
    bool m_showCursor;
    bool m_blinkCursor;
    OsEventTime m_blinkLast;

    Rect m_extends; //! text extends & margins
    Rect m_margins;

    int m_offset; //! text position & cursor
    int m_cursor;
};

//////////////////////////////////////////////////////////////////////////////
//! GuiComboBox

class GuiComboBox : public GuiTextBox {
public:
    GuiComboBox();

    DECLARE_GUICONTROL(GuiDataEdit,GuiComboBox,TINY_GUICOMBOBOX_PUID);
    DECLARE_GUIPROPERTIES;

    void setListonly( bool listonly=true );

    GuiPopup &popup() { return m_popup; }
    GuiMenu &menu() { return m_menu; }

    //! @note popup command subscriber
    void Subscribe( IGuiMessage &listener );

public:
    void makeMenu( const ListOf<String> &items );

public:
    // API_IMPL(void) onClick( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) IOVERRIDE;
        //! @note probably not a good idea to handle event in onMouse (no move/drag detect) //TODO find better

    API_IMPL(void) onKeyDown( OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) IOVERRIDE;

public:
    void onGotFocus() override {
        if( !m_listonly ) GuiTextBox::onGotFocus();
    }

    void onLostFocus() override {
        if( !m_listonly ) GuiTextBox::onLostFocus();
    }

    API_IMPL(void) onLayout( const OsRect &clientArea ,OsRect &placeArea ) IOVERRIDE;
    API_IMPL(void) onDraw( const OsRect &uptadeArea ) IOVERRIDE;
    // mouse + key
    void onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) override;

    API_IMPL(void) onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) IOVERRIDE;

protected:
    GuiPopup m_popup;
    GuiPublisher *m_events;

    Rect m_caretArea;
    OsPoint m_caretPoints[3];

    GuiMenu m_menu;
    bool m_listonly;
};

//////////////////////////////////////////////////////////////////////////////
//! GuiColorBox

class GuiColorBox : public GuiComboBox {
public:
    GuiColorBox() {
        setPropertiesWithString("menu={items=#FF0000,#00FF00,#0000FF;}");
        //TODO list of colors from theme
    }

    DECLARE_GUICONTROL(GuiComboBox,GuiColorBox,TINY_GUICOLORBOX_PUID);

public:
    API_IMPL(void) onDraw( const OsRect &updateArea ) IOVERRIDE;
};

//////////////////////////////////////////////////////////////////////////////
//! GuiBoolBox

class GuiBoolBox : public GuiComboBox {
public:
    GuiBoolBox() {
        setPropertiesWithString("menu={items=true,false;}");
        setListonly();
    }

    DECLARE_GUICONTROL(GuiComboBox,GuiBoolBox,TINY_GUIBOOLBOX_PUID);
};

//////////////////////////////////////////////////////////////////////////////
//! GuiAlignBox

class GuiAlignBox : public GuiComboBox {
public:
    GuiAlignBox() {
        setPropertiesWithString("menu={items=none,left,right,centerh,top,bottom,centerv,center;}");
    }

    DECLARE_GUICONTROL(GuiComboBox,GuiAlignBox,TINY_GUIALIGNBOX_PUID);
};

//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
#endif //TINY_GUI_EDIT_H