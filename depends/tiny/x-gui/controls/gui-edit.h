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

#define TINY_GUIEDITBOX_UUID    0x050832d6537d2b96f

#define TINY_GUITEXTBOX_UUID    0x07d5b0de21313e320
#define TINY_GUICOMBOBOX_UUID   0x000640c9cf93771b3
#define TINY_GUICOLORBOX_UUID   0x0b0f71ead71b710a2

//////////////////////////////////////////////////////////////////////////////
//! GuiEditBox

class GuiEditBox : public virtual IDataEvent ,GUICONTROL_PARENT {
    //! Base class for edit control bound to one value from a data source

public:
    DECLARE_OBJECT_STD(GuiControl,GuiEditBox,TINY_GUIEDITBOX_UUID);
    DECLARE_GUIPROPERTIES;

public: //-- factory
    static bool RegisterEditor( const UUID &editableId ,const UUID &editorId );
    static bool findEditor( const UUID &editableId ,UUID &editorId );

    // virtual void getEditObjectId( UUID &id ) = 0;

public: //-- binding
    void Bind( const char *field ,IDataSource &source );

    virtual void setValue( const char *value ) = 0;
    virtual void getValue( String &value ) const = 0;

protected: //-- data interface
    IAPI_IMPL onDataCommit( IDataSource &source ,Params &data ) IOVERRIDE;
    IAPI_IMPL onDataChanged( IDataSource &source ,const Params &data ) IOVERRIDE;

    IDataSourceRef m_dataSource;
    String m_dataField; //! data name which edit is bound to
};

typedef RefOf<GuiEditBox> GuiEditBoxRef;

GuiEditBox *ICreateGuiEdit( const UUID &editableId );

#define REGISTER_EDITBOX(__editable,__editor) \
    static bool g_##__editor##_registered = GuiEditBox::RegisterEditor( classId_<__editable>(),__editor::classId() );

//////////////////////////////////////////////////////////////////////////////
//! GuiEditBox_

template <typename T>
class GuiEditBox_ : public GuiEditBox {
public:
    T &value() { return m_value; }

public:
    void setValue( const char *value ) override {
        fromString( m_value ,value );
    }

    void getValue( String &value ) const override {
        toString( m_value ,value );
    }

protected: //-- data interface
    void readData() {
        if( !m_dataSource || m_dataField.empty() ) return;

        Params data;

        data[ m_dataField ] = "";

        if( m_dataSource->readData( data ) != IOK) return;

        setValue( data[ m_dataField ].c_str() );
    }

    void onDataEdit( bool softEdit=false ) {
        if( !m_dataSource || m_dataField.empty() ) return;

        Params data;

        if( !softEdit ) //! @note soft edit means we want to advise datasource edit is in progress, but don't want to transact data yet
            getValue( data[ m_dataField ] );

        m_dataSource->onDataEdit( data );
    };

    T m_value;
};

//////////////////////////////////////////////////////////////////////////////
//! GuiTextBox

class GuiTextBox : public GuiWithText ,public GuiEditBox_<String> {
public:
    GuiTextBox();

    DECLARE_GUICONTROL(GuiEditBox,GuiTextBox,TINY_GUITEXTBOX_UUID);
    DECLARE_GUIPROPERTIES;

public:
    void setValue( const char *value ) override {
        GuiEditBox_<String>::setValue( value );

        m_text = m_value; //TODO can merge ?
    }

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

class GuiComboBox : public IGuiCommandEvent ,public GuiTextBox {
public:
    GuiComboBox();

    DECLARE_GUICONTROL(GuiEditBox,GuiComboBox,TINY_GUICOMBOBOX_UUID);
    DECLARE_GUIPROPERTIES;

    void setListonly( bool listonly=true );

    void setPopup( GuiControl &popup ) {}

    //! @note popup command subscriber
    void Subscribe( IGuiCommandEvent &listener );

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

    API_IMPL(void) onCommand( GuiControl &source ,uint32_t commandId ,long param ,Params *params ,void *extra ) IOVERRIDE;

protected:
    GuiPopup m_popup;
    GuiCommandPublisher *m_events;

    Rect m_caretArea;
    OsPoint m_caretPoints[3];

    bool m_listonly;
};

//////////////////////////////////////////////////////////////////////////////
//! GuiColorBox

class GuiColorBox : public GuiEditBox_<ColorRef> {
public:
    DECLARE_GUICONTROL(GuiEditBox,GuiColorBox,TINY_GUICOLORBOX_UUID);
    // DECLARE_GUIPROPERTIES;

    //TODO with a popup menu, list of colors from theme

public:
    API_IMPL(void) onDraw( const OsRect &updateArea ) IOVERRIDE;
};

//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
#endif //TINY_GUI_EDIT_H