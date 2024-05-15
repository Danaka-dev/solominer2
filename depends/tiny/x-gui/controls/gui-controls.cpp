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

//////////////////////////////////////////////////////////////////////////
#include "tiny.h"

#ifndef TINY_NO_XGUI

//////////////////////////////////////////////////////////////////////////
#include "../gui.h"

#include "gui-controls.h"
#include "gui-edit.h"
#include "gui-grid.h"
#include "gui-graph.h"
#include "gui-plot.h"

//////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

using namespace TINY_NAMESPACE_GUI;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Controls

//////////////////////////////////////////////////////////////////////////////
//! GuiWithFont

GuiWithFont::GuiWithFont( GuiFont *font )
{
    if( !font ) {
        m_font = theTheme().getFont( "default" );
    } else {
        setFont( *font );
    }
}

//-- properties
void GuiWithFont::getProperties( Params &properties ) const {
    GuiControl::getProperties( properties );

    if( m_font.isNull() ) return;

    String name;

    if( m_font && FontStore::getInstance().digItem( m_font ,name ) ) {
        properties["font"] = name;
    }
}

void GuiWithFont::setProperties( const Params &properties ) {
    GuiControl::setProperties( properties );

    auto font = FontStore::getInstance().findItem( getMember( properties ,"font" ) );

    if( font ) {
        m_font = font->ptr();
    }
}

//--
void GuiWithFont::onDraw( const OsRect &updateArea ) {
    root().SetFont( m_font.isNull() ? GuiFont::getDefault() : m_font.get() );
}

//////////////////////////////////////////////////////////////////////////////
//! GuiWithText

//-- properties
void GuiWithText::getProperties( Params &properties ) const {
    GuiWithFont::getProperties( properties );

    enumToStringList( m_textAlign ,properties["textalign"] );
    properties["text"] = m_text;
}

void GuiWithText::setProperties( const Params &properties ) {
    GuiWithFont::setProperties( properties );

    enumFromStringList( m_textAlign ,getMember( properties ,"textalign" ) );
    m_text = getMember( properties ,"text" );
}

//--
void GuiWithText::Draw( const OsRect &r ) {
    DrawTextAlign( m_text.c_str() ,r ,m_textAlign );
}

//////////////////////////////////////////////////////////////////////////////
//! GuiMargin

REGISTER_CLASS(GuiMargin)

//-- properties
void GuiMargin::getProperties( Params &properties ) const {
    GuiControl::getProperties( properties );

    toString( m_margin ,properties["margin"] );
}

void GuiMargin::setProperties( const Params &properties ) {
    GuiControl::setProperties( properties );

    fromString( m_margin ,getMember( properties ,"margin" ) );
}

//////////////////////////////////////////////////////////////////////////////
//! GuiLabel

REGISTER_CLASS(GuiLabel)

//-- properties
void GuiLabel::getProperties( Params &properties ) const {
    GuiControl::getProperties( properties );
    GuiWithFont::getProperties( properties );

    enumToStringList( m_textAlign ,properties["textalign"] );
    properties["text"] = m_text;
}

void GuiLabel::setProperties( const Params &properties ) {
    GuiControl::setProperties( properties );
    GuiWithFont::setProperties( properties );

    enumFromStringList( m_textAlign ,getMember( properties ,"textalign" ) );
    m_text = getMember( properties ,"text" );
}

//--
void GuiLabel::onDraw( const OsRect &updateArea ) {
    GuiControl::onDraw(updateArea);

    if( m_text.empty() ) return;

    GuiWithFont::onDraw(updateArea);

    OsRect &r = this->area();

    root().DrawTextAlign( m_text.c_str() ,r.left ,r.top ,r.right ,r.bottom ,m_textAlign );
}

//////////////////////////////////////////////////////////////////////////////
//! GuiLink

REGISTER_CLASS(GuiLink)

//-- properties
void GuiLink::getProperties( Params &properties ) const {
    GuiLabel::getProperties( properties );
    GuiCommandOnClick::getProperties( properties );

    toString( m_textHooverColor ,properties["hoovercolor:ColorRef"] );
}

void GuiLink::setProperties( const Params &properties ) {
    GuiLabel::setProperties( properties );
    GuiCommandOnClick::setProperties( properties );

    fromString( m_textHooverColor ,getMember( properties ,"hoovercolor" ) );
}

//////////////////////////////////////////////////////////////////////////////
//! GuiButton

REGISTER_CLASS(GuiButton)

//-- properties
void GuiButton::getProperties( Params &properties ) const {
    GuiControl::getProperties( properties );
    GuiCommandOnClick::getProperties( properties );

    toString( text() ,properties["text"] );
}

void GuiButton::setProperties( const Params &properties ) {
    GuiControl::setProperties( properties );
    GuiCommandOnClick::setProperties( properties );

    fromString( text() ,getMember( properties ,"text" ) );

    // String s = getMember( properties ,"colors" );

    /* if( !s.empty() ) {
        Params colors;

        fromString( colors ,s );

        fromManifest( m_colorset ,colors );
    } */
}

void GuiButton::onLayout( const OsRect &clientArea ,OsRect &placeArea ) {
    GuiControl::onLayout( clientArea ,placeArea );

    OsRect r = area();

    m_label.setRoot( root() );
    m_label.onLayout( area() ,r );
}

void GuiButton::onDraw( const OsRect &updateArea ) {
    GuiControl::onDraw(updateArea);

    if( !shouldDraw(updateArea) ) return;

    m_label.onDraw( updateArea );
}

//////////////////////////////////////////////////////////////////////////////
//! GuiCheck

REGISTER_CLASS(GuiCheck)

//--
void GuiCheck::onClick( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) {
    GuiControl::onClick( p ,mouseButton ,keyState );

    m_checked ^= true;

    root().Refresh();
}

//--
void GuiCheck::onDraw( const OsRect &updateArea ) {
    GuiControl::onDraw(updateArea);

    if( !visible() ) return;

    Rect r = area().getCenter();

    //-- outter
    OsGuiSetColors( root() ,m_outterColor );

    r.Inflate( m_size );

    if( m_style == styleRound ) {
        root().DrawEllipse( r );
    } else {
        root().DrawRectangle( r );
    }

    //-- inner
    if( !checked() ) return;

    OsGuiSetColors( root() ,m_checkColor );

    r.Inflate( 1 );

    switch( m_style ) {
        default:
        case styleTick: drawTick(r); break;
        case styleCross: drawCross(r); break;
        case styleRound: drawRound(r); break;
    }
}

void GuiCheck::drawTick( const Rect &r ) {
    //TODO OsGuiDrawTick

    root().DrawLine( Rect( r.getTopLeft() ,r.getBottomMid() ) );
    root().DrawLine( Rect( r.getBottomMid() ,r.getTopRight() ) );
}

void GuiCheck::drawCross( const Rect &r ) {
    root().DrawLine( Rect( r.getTopLeft() ,r.getBottomRight() ) );
    root().DrawLine( Rect( r.getTopRight() ,r.getBottomLeft() ) );
}

void GuiCheck::drawRound( const Rect &r ) {
    Rect inner = r; inner.Deflate( (int) ( (float) r.getWidth() / 10 ) );

    root().DrawEllipse( inner );
}

//////////////////////////////////////////////////////////////////////////////
//! GuiCheckBox

REGISTER_CLASS(GuiCheckBox)

//////////////////////////////////////////////////////////////////////////////
//! GuiList

REGISTER_CLASS(GuiList)

void GuiList::Placer::EmplaceLine( int i ,int n ,const Rect &client ,Rect &area ) {
    Point p0 = getRectPoint( client ,origin );

    Point pos = Point(
            size.x * getDimSign(0,direction)
            ,size.y * -1 * getDimSign(1,direction)
    );

    area = Rect( p0 ,p0+size ) + (pos * i);
}

void GuiList::Placer::Emplace( int i ,int n ,const Rect &client ,Rect &area ) {
    switch( placement ) {
        default:
        case placeLine:
            EmplaceLine( i ,n ,client ,area ); break;
    }
}

//////////////////////////////////////////////////////////////////////////////
//! GuiImageBox

REGISTER_CLASS(GuiImageBox)

//--
GuiImageBox::GuiImageBox( GuiImage *image ,const char *text ,IGuiCommandEvent *listener ) :
    m_image(NullPtr) ,m_thumbId(-1) ,m_textPlacement(alignCenter)
{
    if( image ) setImage(*image);
    if( text && *text ) setText(text);
    if( listener ) GuiCommandPublisher::Subscribe(*listener);
}

///-- properties
void GuiImageBox::getProperties( Params &properties ) const {
    GuiWithText::getProperties( properties );

    //TODO
}

void GuiImageBox::setProperties( const Params &properties ) {
    GuiWithText::setProperties( properties );
    GuiCommandOnClick::setProperties( properties );

    String name = getMember( properties ,"image" );

    GuiImageRef p; p = Assets::Image().get( name.c_str() );
    if( p ) setImage( p.get() );

    fromString( m_thumbId ,getMember( properties ,"thumbid" ) );
    if( m_thumbId >= 0 ) setThumbnail( m_thumbId );

    if( hasMember(properties ,"textplacement") ) {
        enumFromString( m_textPlacement ,getMember( properties ,"textplacement" ) );
    }

    enumFromString( m_textPlacement ,getMember( properties ,"textplacement" ) );
}

///--
Rect &GuiImageBox::getImageArea( Rect &r ) const {
    getCenteredArea( m_thumbnail.getSize() ,r ); return r;
}

Rect &GuiImageBox::getTextArea( Rect &r ) const {
    return r = m_textArea;
}

void GuiImageBox::onLayout( const OsRect &clientArea ,OsRect &placeArea ) {
    GuiWithText::onLayout( clientArea ,placeArea );

//-- text area
    Rect &r = getImageArea( m_textArea );

    switch( m_textPlacement ) {
        default:
        case alignCenter:
            break;
        case alignLeft:
            r.right = r.left; r.left = area().left; break;
        case alignRight:
            r.left = r.right; r.right = area().right; break;
        case alignTop:
            r.bottom = r.top; r.top = area().top; break;
        case alignBottom:
            r.top = r.bottom; r.bottom = area().bottom; break;
    }

    r -= area().getTopLeft();
}

void GuiImageBox::onDraw( const OsRect &updateArea ) {
    GuiWithText::onDraw(updateArea);

    if( !shouldDraw(updateArea) ) return;

    Rect &r = this->area();

    if( !m_text.empty() ) {
        SetForeColor( OS_COLOR_WHITE );
        GuiWithText::Draw( m_textArea );
        // root().DrawTextAlign( m_text.c_str() ,r.left ,r.top ,r.right ,r.bottom , (TextAlign) (textalignCenterH | textalignCenterV) );
    }

    Rect ri; getImageArea(ri);

    if( m_thumbId >= 0 ) {
        m_image->DrawThumbnail( root().GetContext() ,ri.left ,ri.top ,m_thumbId );
        return;
    }

    if( !m_image.isNull() ) {
        root().DrawImage( m_image.get()
            ,ri.left ,ri.top ,ri.right ,ri.bottom
            ,m_thumbnail.left ,m_thumbnail.top ,m_thumbnail.right ,m_thumbnail.bottom
        );
    }
}

//////////////////////////////////////////////////////////////////////////////
//! GuiThumbnail

REGISTER_CLASS(GuiThumbnail)

//--
void GuiThumbnail::onLayout( const OsRect &clientArea ,OsRect &placeArea ) {
    GuiControl::onLayout( clientArea ,placeArea );

    //TODO use GuiArray_ instead
    m_image.setRoot( root() );
    m_label.setRoot( root() );

    OsRect r = this->area(); //! sub area
    {
        m_image.onLayout( this->area() ,r );

        Rect imageArea; m_image.getImageArea(imageArea);

        m_label.coords().top = imageArea.bottom - this->area().top;

        m_label.onLayout( this->area() ,r );
    }
}

void GuiThumbnail::onDraw( const OsRect &updateArea ) {
    GuiControl::onDraw(updateArea);

    m_image.onDraw(updateArea);
    m_label.onDraw(updateArea);
}

//////////////////////////////////////////////////////////////////////////
//! GuiThumbwall

IAPI_DEF GuiThumbwall::getInterface( UUID id ,void **ppv ) {
    if( !ppv || *ppv ) return IBADARGS;

    return
            honorInterface_<GuiThumbwall>(this,id,ppv) || honorInterface_<IGuiCommandEvent>(this,id,ppv) ? IOK
                                                                                                         : GuiControl::getInterface( id ,ppv )
            ;
}

void GuiThumbwall::getProperties( Params &properties ) const {
    GuiWithFont::getProperties( properties );
    GuiGroup::getProperties( properties );
}

void GuiThumbwall::setProperties( const Params &properties ) {
    GuiWithFont::setProperties( properties );
    GuiGroup::setProperties( properties );
}

//--
int GuiThumbwall::addThumb( GuiImage *image ,const char *label ) {
    assert(label!=NullPtr);

    GuiThumbnail *p = new GuiThumbnail(image,label,m_font.ptr());

    p->align() = GuiAlign::noAlign;
    p->GuiCommandPublisher::Subscribe( *this );

    return addControl( *p );
}

void GuiThumbwall::removeThumb( int index ) {
    removeControl( index );
}

void GuiThumbwall::onLayout( const OsRect &clientArea ,OsRect &placeArea ) {
    GuiControl::onLayout( clientArea ,placeArea );

    int n = getThumbCount(); if( n == 0 ) return;
    int w = area().getWidth();
    int h = area().getHeight();

    Point sz = m_thumbSize;
    int nx = (int) (w / sz.x);
    int ny = (int) (w / sz.y);

    int i=0; for( auto &thumb : controls() ) if( thumb->visible() ) {
        int y = (i / nx) * sz.y;
        int x = (i % nx) * sz.x;

        thumb->coords() = { x ,y ,x+m_thumbSize.x ,y+m_thumbSize.y };

        ++i;
    }

    OsRect r = area();

    GuiGroup::onLayout( clientArea ,r );
}

void GuiThumbwall::onDraw( const OsRect &updateArea ) {
    GuiGroup::onDraw( updateArea );
}

void GuiThumbwall::onCommand( GuiControl &source ,uint32_t commandId ,long param ,Params *params ,void *extra ) {
    onItemSelected( source ,(int) source.id() ,true );

//! forward to subscribers
    GuiCommandPublisher::PostCommand( commandId ,param ,params ,extra );
}

//////////////////////////////////////////////////////////////////////////
//! GuiEdit

GuiEditBox *ICreateGuiEdit( const UUID &editableId ) {
    UUID editorId;

    if( !GuiEditBox::findEditor( editableId ,editorId ) )
        return NullPtr;

    GuiControl *editor = Factory_<GuiControl>::getInstance().Create( editorId );

    return editor ? editor->As_<GuiEditBox>() : NullPtr;
}

//-- static
static MapOf<UUID,UUID> g_editors; //! editable => editor

bool GuiEditBox::RegisterEditor( const UUID &editableId ,const UUID &editorId ) {
    g_editors[editableId] = editorId; return true;
}

bool GuiEditBox::findEditor( const UUID &editableId ,UUID &editorId ) {
    const auto &it = g_editors.find( editableId );

    if( it == g_editors.end() ) return false;

    editorId = it->second;

    return true;
}

//-- properties
void GuiEditBox::getProperties( Params &properties ) const {
    GuiControl::getProperties( properties );

    if( !m_dataField.empty() ) properties["datafield"] = m_dataField;
}

void GuiEditBox::setProperties( const Params &properties ) {
    GuiControl::setProperties( properties );

    // getMember( properties ,"datasource" ); // @config/service => store ...
    m_dataField = getMember( properties ,"datafield" );
}

//--
void GuiEditBox::Bind( const char *field ,IDataSource &source ) {
    if( m_dataSource ) {
        m_dataSource->Revoke( *this );
    }

    m_dataSource = source;
    m_dataField = field;

    if( m_dataSource ) {
        m_dataSource->Subscribe( *this );
    }
}

/* void GuiEditBox::adviseEditStart() {
    if( m_dataField.empty() ) return;

    Params params; //! @note empty, simply advise source edit has started

    m_dataSource->onDataEdit( params );
} */

IAPI_DEF GuiEditBox::onDataCommit( IDataSource &source ,Params &data ) {
    if( m_dataField.empty() ) return INODATA;

    String s; getValue( s );

    data[ m_dataField ] = s;

    return IOK;
}

IAPI_DEF GuiEditBox::onDataChanged( IDataSource &source ,const Params &data ) {
    if( m_dataField.empty() ) return INODATA;

    const String *field = peekMember( data ,m_dataField.c_str() );

    if( field )
        setValue( field->c_str() );

    return IOK;
}

//////////////////////////////////////////////////////////////////////////////
//! GuiTextBox

REGISTER_CLASS(GuiTextBox)
REGISTER_EDITBOX(String,GuiTextBox);

GuiTextBox::GuiTextBox() :
    m_offset(0) ,m_cursor(0) ,m_margins(4)
    ,m_showCursor(false) ,m_blinkCursor(true) ,m_blinkLast(0)
{
    m_colors = theTheme().getColors( MyUUID ,"normal" );
}

//-- properties
void GuiTextBox::getProperties( Params &properties ) const {
    GuiEditBox_<String>::getProperties(properties);
    GuiWithText::getProperties(properties);
}

void GuiTextBox::setProperties( const Params &properties ) {
    GuiWithText::setProperties( properties );
    GuiEditBox_<String>::setProperties( properties );

    setValue( m_text.c_str() ); //TODO remove when merged value & text
}

//--

void GuiTextBox::onMouseDown( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) {
    GuiControl::onMouseDown( p ,mouseButton ,keyState );

    Point pos = p; pos -= area().getTopLeft();
    OsPoint point;

    int w = pos.x - m_margins.left;
    int d = 0;

    m_cursor = 0;

    for( int i=0; i<m_value.size(); ++i ) {
        String before( m_value.c_str()+m_offset ,MAX(m_cursor-m_offset,0) );

        OsGuiFontCalcSize( m_font->GetHandle() ,before.c_str() ,&point );

        if( point.x < w ) {
            ++m_cursor; d = (w - point.x);
        } else {
            if( m_cursor ) // && d <= point.x - w )
                --m_cursor;
            break;
        }
    }

    showCursorNow(true);
}

//--
void GuiTextBox::onDraw( const OsRect &updateArea )  {
    if( !shouldDraw(updateArea) ) return;

    Rect clip0;
    root().RegionGetArea( clip0 );
    root().RegionSetArea( area() );

    GuiEditBox_<String>::onDraw( updateArea );
    GuiWithText::onDraw( updateArea );

    Rect r = area().getDims().Deflate(m_margins);

    OsRect extends;
    String before( m_value.c_str()+m_offset ,MAX(m_cursor-m_offset,0) ); //! MAX should not be needed, but protects from invalid params
    DrawTextAlign( before.c_str() ,r ,textalignCenterLeft ,&extends );
    m_extends = extends;

    int cursorx = r.left = extends.right;
    DrawTextAlign( m_value.c_str()+m_cursor ,r ,textalignCenterLeft ,&extends );
    m_extends |= extends;

    drawCursor( cursorx - (m_cursor ? -1 : 0) );

    root().RegionSetArea( clip0 );
}

void GuiTextBox::onKeyDown( OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) {
    GuiControl::onKeyDown( keyState ,keyCode ,c );

    switch( keyCode ) {
        case OS_KEYCODE_BACKSPACE:
            if( m_cursor ) {
                m_value.erase( --m_cursor ,1 );
                onDataEdit( true );
            }
            break;

        case OS_KEYCODE_DELETE:
            if( m_cursor < m_value.size() ) {
                m_value.erase( m_cursor ,1 );
                onDataEdit( true );
            }
            break;

        case OS_KEYCODE_HOME: {
            if( m_cursor == m_offset )
                m_cursor = m_offset = 0;
            else
                m_cursor = m_offset;
        } break;

        case OS_KEYCODE_END: if( m_cursor < m_value.size() ) {
            ++m_cursor;

            if( updateOffset(1) != 0 ) {
                int delta = m_value.size() - m_cursor;
                m_cursor = m_value.size();
                updateOffset(delta);
            } else {
                while( ++m_cursor < m_value.size() && updateOffset(1) == 0 ) {}
                --m_cursor; --m_offset;
            }
        } break;

        case OS_KEYCODE_LEFT:
            if( m_cursor == 0 ) break;

            if( keyState.ctrl ) { //! word
                const char *str = m_value.c_str();
                bool base = cisalphanum( str[m_cursor-1] );

                int &i = m_cursor; for( --i ; i>0; --i ) {
                    if( cisalphanum(str[i-1]) != base ) break;
                }

            } else { //! char
                 --m_cursor;
            }
            m_offset = MIN( m_offset ,m_cursor );
            break;

        case OS_KEYCODE_RIGHT: if( m_cursor < m_value.size() ) {
            if( keyState.ctrl ) { //! word
                const char *str = m_value.c_str();
                int &i = m_cursor; int j = i;
                bool base = cisalphanum( str[i] );

                for( ++i; i < m_value.size(); ++i ) {
                    if( cisalphanum( str[i] ) != base ) break;
                }

                updateOffset( i-j );

            } else { //! char
                ++m_cursor; updateOffset( 1 );
            }
        } break;

        case OS_KEYCODE_TAB:
        case OS_KEYCODE_RETURN:
            onDataEdit(); //! ok, should get to next focus
            break;

        default:
            if( c >= 32 && c <= 127 ) {
                m_value.insert( m_cursor++ ,1 ,c );
                onDataEdit( true );

                updateOffset( 1 );
            }
            break;
    }

    showCursorNow( m_showCursor );
}

void GuiTextBox::onTimer( OsTimerAction timeAction ,OsEventTime now ,OsEventTime last ) {
    GuiControl::onTimer( timeAction ,now ,last );

    if( now - m_blinkLast > 500 ) {
        m_blinkCursor ^= true;
        m_blinkLast = now;
        Refresh();
    }
};

//--
int GuiTextBox::updateOffset( int delta ) {
    OsPoint point;

    int w = area().getWidth() - m_margins.getWidth();
    int d = 0;

    for( int i=0; i<delta; ++i ) {
        String before( m_value.c_str()+m_offset ,MAX(m_cursor-m_offset,0) );

        OsGuiFontCalcSize( m_font->GetHandle() ,before.c_str() ,&point );

        if( point.x >= w ) {
            ++m_offset; ++d;
        }
    }

    return d;
}

void GuiTextBox::showCursorNow( bool show ) {
    m_blinkLast = OsTimerConvertToMs( OsTimerGetTimeNow() );
    m_blinkCursor = true;
    m_showCursor = show;

    Refresh();
}

void GuiTextBox::drawCursor( int x ) {
    if( m_showCursor && m_blinkCursor ) {} else return;

    int ha = area().getHeight();
    int hx = m_extends.getHeight();
    int h2 = (ha - hx) / 2;

    Rect r = { x ,h2 ,x ,area().getHeight()-h2 };

    SetForeColor( OS_COLOR_WHITE );
    SetFillColor( OS_COLOR_WHITE );

    DrawLine( r );
}

//////////////////////////////////////////////////////////////////////////
//! GuiComboBox

REGISTER_CLASS(GuiComboBox);

GuiComboBox::GuiComboBox() : m_listonly(false) ,m_events(NullPtr) {
    m_margins.right = 28;
}

//-- properties
void GuiComboBox::getProperties( Params &properties ) const {
    GuiTextBox::getProperties(properties);
}

void GuiComboBox::setProperties( const Params &properties ) {
    GuiTextBox::setProperties( properties );

    fromString( m_listonly ,getMember(properties,"listonly","false") );

    if( hasMember( properties ,"menu" ) ) {
        //! menu

        auto *menu = new GuiMenu();

        const char *props = getMember( properties ,"menu" );

        Params params;

        fromString( params ,props );

        menu->setProperties( params );
        menu->Subscribe(*this);

        m_popup.setControl( menu );
        m_events = menu;

    } else if( hasMember( properties ,"list" ) ) {
        //! list

    }
}

//--
void GuiComboBox::setListonly( bool listonly ) {
    m_listonly = listonly;

    m_showCursor = !listonly;
}

void GuiComboBox::Subscribe( IGuiCommandEvent &listener ) {
    if( m_events ) m_events->Subscribe(listener);
}

//--
/* void GuiComboBox::onClick( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) {
    if( isOrphan() || !m_popup.hasControl() ) return;

    root().ShowPopup( m_popup );
} */

void GuiComboBox::onKeyDown( OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) {
    if( m_listonly ) return;

    GuiTextBox::onKeyDown( keyState ,keyCode ,c );
}

//--
void GuiComboBox::onLayout( const OsRect &clientArea ,OsRect &placeArea ) {
    GuiTextBox::onLayout( clientArea ,placeArea );

    Rect r = area();

    /* if( m_listonly ) {
        m_showCursor = false;
    } */

//-- caret
    Rect &rc = m_caretArea;

    rc = r;
    rc -= area().getTopLeft();

    int h = rc.bottom;
    rc.left = rc.right - 28;
    rc.top = rc.top + (h-28)/2;
    rc.bottom = rc.bottom - (h-28)/2;

    int m = 6 ,q = 4;
    m_caretPoints[0] = { rc.left+m ,rc.getCenter().y-q+2 };
    m_caretPoints[1] = { rc.getCenter().x ,rc.getCenter().y+q+2 };
    m_caretPoints[2] = { rc.right-m ,rc.getCenter().y-q+2 };

//-- popup
    if( !m_popup.hasControl() ) return;

    Rect clientPlace;

    r.top = r.bottom;
    r.bottom = r.top + area().getHeight();

    m_popup.getControl()->coords().setOsRect( r );
}

void GuiComboBox::onDraw( const OsRect &updateArea ) {
    GuiTextBox::onDraw( updateArea );

    if( !shouldDraw(updateArea) ) return;

//-- caret
    SetForeColor( OS_COLOR_DARKGRAY );
    SetFillColor( OS_COLOR_BLACK );
    DrawRectangle( m_caretArea );

    SetForeColor( OS_COLOR_GRAY );
    SetFillColor( OS_COLOR_NONE );
    DrawPolygon( 3 ,m_caretPoints );
}

void GuiComboBox::onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) {
    if( isOrphan() ) return;

    if( mouseAction == osMouseButtonDown && m_popup.hasControl() ) {
        OsPoint p = pos[0];

        bool hitpopup = false;

        if( m_listonly ) {
            hitpopup = (area() & p);
        } else {
            hitpopup = ((m_caretArea + area().getTopLeft()) & p);
        }

        if( hitpopup )
            root().ShowPopup( m_popup );
    }

    if( m_listonly ) return;

    GuiTextBox::onMouse( mouseAction ,keyState ,mouseButton ,points ,pos );
}

void GuiComboBox::onCommand( GuiControl &source ,uint32_t commandId ,long param ,Params *params ,void *extra ) {
    if( commandId >= GUI_COMMANDID_MENU && commandId <= GUI_COMMANDID_MENU_MAX && params ) {
        const auto it = params->find( "text" );

        if( it == params->end() ) return;

        m_value = it->second;

        m_popup.Close();

        Refresh();
    }
}

//////////////////////////////////////////////////////////////////////////////
//! GuiColorBox

REGISTER_CLASS(GuiColorBox);
REGISTER_EDITBOX( ColorRef ,GuiColorBox );

void GuiColorBox::onDraw( const OsRect &updateArea ) {
    GuiControl::onDraw( updateArea );

    int w = area().getWidth();
    int h = area().getHeight();

    //-- box
    Rect r = { Point() ,Point(h) };

    r.Deflate( 8 );

    root().SetFillColor( m_value );

    DrawRectangle( r );

    //-- text
    r.left = h;
    r.right = w - r.left;

    String s; //! should cache this

    getValue( s );

    DrawTextAlign( s.c_str() ,r ,textalignCenterLeft );
}

//////////////////////////////////////////////////////////////////////////
//! GuiGrid

template <>
GuiGrid::Config &Init( GuiGrid::Config &p ) {
    p.showTitle = true;
    p.showColLines = true;
    p.showRowLines = true;

    p.titleAlign = textalignCenter;
    p.cellAlign = textalignCenterLeft;

    return p;
}

REGISTER_CLASS(GuiGrid)

GuiGrid::GuiGrid() {
    Init( config() );

//-- dimension
    m_cellMargin = 4; //TODO from theme
    m_titleCoord = m_rowCoord = GuiCoord(10.f);

//-- theme & colors
    m_colors.normal = theTheme().getColors( MyUUID ,"normal" );
    m_colors.title = theTheme().getColors( MyUUID ,"title" );
    m_colors.col1 = theTheme().getColors( MyUUID ,"col1" );
    m_colors.col2 = theTheme().getColors( MyUUID ,"col2" );
    m_colors.row1 = theTheme().getColors( MyUUID ,"row1" );
    m_colors.row2 = theTheme().getColors( MyUUID ,"row2" );
}

//-- properties
void GuiGrid::getProperties( Params &properties ) const {
    GuiControl::getProperties( properties );

    //TODO
}

void GuiGrid::setProperties( const Params &properties ) {
    GuiControl::setProperties( properties );

    ListOf<GuiCoord> coords;
    fromString( coords ,getMember( properties ,"cols" ) );

    ListOf<String> titles;
    fromString( titles ,getMember( properties ,"titles" ) );

    int n = MAX( coords.size() ,titles.size() );

    for( int i=0; i<n; ++i ) {
        GuiCoord coord = ( i < coords.size() ? coords[i] : 10.f );
        String title = ( i < titles.size() ? titles[i] : "" );

        addCol( coord ,title.c_str() );
    }
}

//--
void GuiGrid::Reset( bool withRefresh ) {
    m_rows.clear();
    m_cols.clear();
    // m_edits.clear();

    if( withRefresh ) Refresh();
}

//--
void GuiGrid::addCol( const GuiCoord &width ,const char *title ) {
    auto &col = m_cols.emplace_back();

    col.coord = width;

    if( title ) {
        NameType decl;

        fromString( decl ,title );

        col.title = decl.name;

        if( !decl.type.empty() ) { //! declaring editor using type
            UUID editableId;

            col.edit = getEditBox( decl.type.c_str() );
        }

    }

    //-- add col to existing rows if any
    for( auto &it : m_rows ) {
        it.m_cells.emplace_back();
    }
}

int GuiGrid::addRow() {
    int i = (int) m_rows.size();

    auto &row = m_rows.emplace_back();

    for( auto &it : m_cols ) {
        row.m_cells.emplace_back();
    }

    return i;
}

void GuiGrid::setCellText( int y ,int x ,const char *text ) {
    if( y < rowCount() && x < colCount() ) {} else return;

    auto &cell = row(y).col(x);

    cell.text = text;

    if( cell.edit ) {
        cell.edit->setValue( cell.text.c_str() );
    }
}

void GuiGrid::setCellEdit( int y ,int x ,GuiEditBox *editor ,bool editable ) {
    if( y < rowCount() && x < colCount() ) {} else return;

    auto &cell = row(y).col(x);

    cell.edit = editor;
    cell.editable = editable;

    if( cell.edit ) {
        cell.edit->setValue( cell.text.c_str() );
    }
}

void GuiGrid::setCellType( int y ,int x ,const UUID &editableId ,bool editable ) {
    GuiEditBox *p = getEditBox( editableId );

    if( p ) {
        setCellEdit( y ,x ,p ,editable );
    }
}

void GuiGrid::setCellType( int y ,int x ,const char *type ,bool editable ) {
    UUID editableId;

    if( getClassIdFromName( type ,editableId ) ) {
        setCellType( y ,x ,editableId ,editable );
    }
}

void GuiGrid::Clear( bool withRefresh ) {
    m_rows.clear();

    if( withRefresh ) Refresh();
}

//--
void GuiGrid::onLayout( const OsRect &clientArea ,OsRect &placeArea )  {
    GuiControl::onLayout( clientArea ,placeArea );

    size() = 0; //! nb used for scroll etc...

    m_titleHeight = m_titleCoord.get( area().getHeight() );

    //-- calc row/col placement
    m_rowHeight = m_rowCoord.get( area().getHeight() );

    for( auto &col : m_cols ) {
        col.width = col.coord.get( area().getWidth() );

        size().x += col.width; //! sum of cell widths
    }

    //-- calc client area size (nb used for scroll)
    size().y = (int) (m_titleHeight + m_rowHeight * m_rows.size());
}

void GuiGrid::onDraw( const OsRect &updateArea )  {
    GuiControl::onDraw(updateArea);

    if( !shouldDraw(updateArea) ) return;

    //TODO HERE... check more shouldDraw

    //-- cols
    drawColLines();

    // m_offset.y -= m_scrollV ? m_scrollV->offset() : 0;
    Rect rrow = { 0 ,0 ,size().x ,m_rowHeight };

    //-- title
    SetColors( m_colors.title );
    DrawRectangle( rrow );

    if( config().showTitle ) {
        rrow.bottom = rrow.top + m_rowHeight;

        Rect rcell = rrow;

        int i=0; for( auto &col : m_cols ) {
            rcell.right = rcell.left + col.width;

            if( !col.title.empty() ) {
                DrawTextAlign( col.title.c_str() ,rcell ,config().titleAlign );
            }

            rcell.left = rcell.right;
            ++i;
        }

        rrow.top = rrow.bottom;
    }

    //-- rows
    Rect rinner ,region;

    root().RegionGetArea( region );

    int j=0; for( auto &row : m_rows ) {
        rrow.bottom = rrow.top + m_rowHeight;

        drawRowLine( j ,rrow );

        Rect rcell = rrow;

        int i=0; for( auto &cell : row.m_cells ) {
            auto &col = m_cols[i];

            rcell.right = rcell.left + col.width;

            rinner = rcell; rinner.Deflate( m_cellMargin );

            RegionSetArea( rinner );

            //TODO check for col edit
            if( cell.edit ) {
                cell.edit->setRoot( root() );
                cell.edit->area() = rinner;
                // cell.edit->setValue( cell.text.c_str() ); //TODO not here
                cell.edit->onDraw( rinner );

            } else {
                DrawTextAlign( cell.text.c_str() ,rinner ,config().cellAlign );
            }

            rcell.left = rcell.right;
            ++i;
        }

        rrow.top = rrow.bottom;
        ++j;
    }

    root().RegionSetArea( region );
}

void GuiGrid::onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) {
    GuiControl::onMouse( mouseAction ,keyState ,mouseButton ,points ,pos );

    if( mouseAction != osMouseButtonDown ) return;

    Point p = pos[0]; p -= area().getTopLeft();

//-- row
    int y = (p.y - m_titleHeight) / m_rowHeight;

    if( y < 0 || y >= m_rows.size() ) return;

    Rect r = { 0 ,m_titleHeight + y*m_rowHeight ,0 ,m_rowHeight };

    r.bottom += r.top;

//-- col
    int x=-1 ,width = 0;

    int i=0; for( auto &col : m_cols ) {
        width += col.width;

        if( p.x < width ) {
            x = i; r.right = width; r.left = r.right - width;
            break;
        }

        ++i;
    }

    if( x < 0 || x >= m_cols.size() ) return;

//-- start edit
    auto &cell = row(y).col(x);

    if( cell.edit && cell.editable ) {
        /* r.Deflate( m_cellMargin );

        cell.edit->setRoot( root() );
        cell.edit->area() = r;
        */

        // cell.edit->Bind( field ,*this );
        cell.edit->setValue( cell.text.c_str() );

        if( m_editFocus ) m_editFocus->onLostFocus();

        m_editFocus = cell.edit;

        if( m_editFocus ) m_editFocus->onGotFocus();
    }
}

void GuiGrid::onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) {
    GuiControl::onKey( keyAction ,keyState ,keyCode ,c );

    if( m_editFocus ) {
        m_editFocus->onKey( keyAction ,keyState ,keyCode ,c );
    }
}

void GuiGrid::onTimer( OsTimerAction timeAction ,OsEventTime now ,OsEventTime last ) {
    GuiControl::onTimer( timeAction ,now ,last );

    if( m_editFocus ) {
        m_editFocus->onTimer( timeAction ,now ,last );
    }
}

//--
GuiEditBox *GuiGrid::getEditBox( const UUID &editableId ) {
    GuiEditBox *p = ICreateGuiEdit( editableId );


    /*
    auto it = m_edits.find( editableId );

    if( it != m_edits.end() ) return it->second.ptr();

    auto &ref = m_edits[editableId];

    GuiEditBox *p = ICreateGuiEdit( editableId );

    ref = p;
*/

    return p;
}

GuiEditBox *GuiGrid::getEditBox( const char *editable ) {
    UUID editableId;

    return getClassIdFromName( editable ,editableId ) ? getEditBox(editableId) : NullPtr;
}

//--
void GuiGrid::drawColLines( bool forceDraw ) {
    if( forceDraw || config().showColLines ) {} else return;

    Rect r = { 0 ,0 ,0 ,area().getHeight() };

    int i = 0; for( auto &col : m_cols ) {
        r.right = r.left + col.width;

        SetColors( getColColors(i) );
        DrawRectangle( r );

        r.left = r.right;
        ++i;
    }
}

void GuiGrid::drawRowLine( int i ,const Rect &r ,bool forceDraw ) {
    if( forceDraw || config().showRowLines ) {} else return;

    SetColors( getRowColors(i) );
    DrawRectangle( r );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////
#endif //TINY_NO_XGUI