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

using TINY_NAMESPACE_GUI;

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
    if( !shouldDraw(updateArea) ) return;

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

    fromMember( text() ,properties ,"text" );

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

void GuiList::onItemSelect( GuiControl &item ,int index ,bool selected ) {

}

void GuiList::onLayout( const OsRect &clientArea ,OsRect &placeArea ) {
    GuiGroup::onLayout( clientArea ,placeArea );

    Rect group = area();

    int n = controls().size();

    int i=0; for( auto &it : controls() ) {
        Rect area ,r;

        m_placer.Emplace( i ,n ,group ,area );

        it->onLayout( area ,r );

        ++i;
    }
}

void GuiList::onClick( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) {
    GuiControl::onClick( p ,mouseButton ,keyState );

    int i = 0; for( auto &it : controls() ) {
        if( it->area() & p ) {
            onItemSelect( it.get() ,i ,true ); return;
        }

        ++i;
    }
}

//--
void GuiList::Placer::Emplace( int i ,int n ,const Rect &client ,Rect &area ) {
    switch( placement ) {
        default:
        case placeLine:
            EmplaceLine( i ,n ,client ,area ); break;
        case placeZigzag:
            EmplaceZigzag( i ,n ,client ,area ); break;
        case placeDiamond:
            EmplaceDiamond( i ,n ,client ,area ); break;
    }
}

void GuiList::Placer::EmplaceLine( int i ,int n ,const Rect &client ,Rect &area ) {
    Point p0 = getRectPoint( client ,origin );

    Point pos = Point(
        size.x * getDimSign(0,direction)
        ,size.y * getDimSign(1,direction) * -1 //! @note inverted
    );

    area = Rect( p0 ,p0+size ) + (pos * i);
}

void GuiList::Placer::EmplaceZigzag( int i ,int n ,const Rect &client ,Rect &area ) {
    Point p0 = getRectPoint( client ,origin );

    int nx = client.getWidth() / size.x;
    int ny = client.getHeight() / size.y;

    //! TODO flip direction priority ? (here x is first)
    if( nx == 0 ) return;
    int ix = i % nx;
    int iy = i / nx;

    Point pos = Point(
        ix * size.x * getDimSign(0,direction)
        ,iy * size.y * getDimSign(1,direction) * -1 //! @note inverted
    );

    area = Rect( p0 ,p0+size ) + pos;
}

void GuiList::Placer::EmplaceDiamond( int i ,int n ,const Rect &client ,Rect &area ) {
    if( n == 0 ) return;

    Point p0 = client.getCenter(); // getRectPoint( client ,origin );
    // DIRECTION Y (expand Y)

    int nx = client.getWidth() / size.x;
    int ny = client.getHeight() / size.y;

    int m = MIN( nx ,(int) ceil( sqrt( (double) n ) ) );

    int ix = i % m;
    int iy = i / m;

    Point pos = Point(
        ix*size.x - (m*size.x)/2
        ,iy*size.y - MIN( client.getHeight() ,m*size.y ) /2
    );

    area = Rect( p0 ,p0+size ) + pos;

     /* X X X
    X X X X
     X X X */
}

//////////////////////////////////////////////////////////////////////////////
//! GuiImageBox

REGISTER_CLASS(GuiImageBox)

//--
GuiImageBox::GuiImageBox( GuiImage *image ,const char *text ,IGuiMessage *listener ) :
    m_image(NullPtr) ,m_thumbId(-1) ,m_textPlacement(alignBottom)
{
    if( image ) setImage(*image);
    if( text && *text ) setText(text);
    if( listener ) GuiPublisher::Subscribe(*listener);

    m_textAlign = textalignTopCenter;
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
    GuiControl::onDraw(updateArea);

    if( !shouldDraw(updateArea) ) return;

    Rect &r = this->area();

    if( !m_text.empty() ) {
        SetForeColor( OS_COLOR_WHITE );
        GuiWithText::Draw( m_textArea );
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
//! GuiProgressBar

REGISTER_CLASS(GuiProgressBar)

template <>
const char *Enum_<GuiProgressBar::LabelStyle>::names[] = {
    "value" ,"valuemax" ,"percent"
};

template <>
const GuiProgressBar::LabelStyle Enum_<GuiProgressBar::LabelStyle>::values[] = {
    GuiProgressBar::labelValue
    ,GuiProgressBar::labelValueMax
    ,GuiProgressBar::labelPercent
};

//--
GuiProgressBar::GuiProgressBar( int low ,int high ,GuiFont *font ) :
    GuiWithText( "" ,textalignCenter ,font )
    ,m_low(low) ,m_high(high) ,m_value(0)
    ,m_labelStyle(labelValue) ,m_inset(0)
{
    const ColorQuad &colors = theTheme().getColors( MyPUID ,"progress" );

    m_barColor = colors.fillColor ? colors.fillColor : OS_COLOR_OLIVE;

    m_textAlign = textalignCenter;
}

//-- properties
void GuiProgressBar::getProperties( Params &properties ) const {
    GuiWithText::getProperties( properties );

    //TODO
}

void GuiProgressBar::setProperties( const Params &properties ) {
    GuiWithText::setProperties( properties );

    fromMember( m_low ,properties ,"low" );
    fromMember( m_high ,properties ,"high" );
    fromMember( m_value ,properties ,"value" );

    enumFromMember( m_labelStyle ,properties ,"labelstyle" );
    fromMember( m_inset ,properties ,"inset" );
    fromMember( m_barColor ,properties ,"barcolor" );

    updateLabelText();
}

//--
float GuiProgressBar::getProgressFactor() const {
    int q = (m_high - m_low);

    return q ? (float) (m_value - m_low) / (float) q : 0.f;
}

void GuiProgressBar::updateLabelText() {
    switch( m_labelStyle ) {
        case labelValue:
            toString( m_value ,m_text ); break;
        case labelValueMax:
            Format( m_text ,"%d/%d" ,32 ,(int) m_value ,(int) m_high ); break;
        case labelPercent:
            toString( (int) (getProgressFactor() * 100.f) ,m_text ); m_text += "%"; break;
    }
}

//--
void GuiProgressBar::onDraw( const OsRect &updateArea ) {
    if( !shouldDraw(updateArea) ) return;

    GuiControl::onDraw( updateArea );

    Rect dims = area().getDims();

    Rect r = dims;
    int w = r.getWidth();

    r.right = MIN( (int) (getProgressFactor() * w) ,w );
    r.Deflate( m_inset );

    SetFillColor( m_barColor );
    DrawRectangle( r );

    GuiWithText::onDraw(updateArea);
    GuiWithText::Draw( dims );
}

//////////////////////////////////////////////////////////////////////////
//! GuiEdit

GuiDataEdit *ICreateGuiEdit( const PUID &editableId ) {
    PUID editorId;

    if( !GuiDataEdit::findEditor( editableId ,editorId ) )
        return NullPtr;

    GuiControl *editor = Factory_<GuiControl>::getInstance().Create( editorId );

    return editor ? editor->As_<GuiDataEdit>() : NullPtr;
}

//-- static
static MapOf<PUID,PUID> g_editors; //! editable => editor

bool GuiDataEdit::RegisterEditor( const PUID &editableId ,const PUID &editorId ) {
    g_editors[editableId] = editorId; return true;
}

bool GuiDataEdit::findEditor( const PUID &editableId ,PUID &editorId ) {
    const auto &it = g_editors.find( editableId );

    if( it == g_editors.end() ) return false;

    editorId = it->second;

    return true;
}

//-- properties
void GuiDataEdit::getProperties( Params &properties ) const {
    GuiControl::getProperties( properties );

    properties["datafield"] = m_dataField; // .empty() ? m_dataField : "";

    const char *name = "";
    if( !m_dataSource.isNull() ) {
        name = root().digBinding( (IObject*) m_dataSource.ptr() );
    }

    properties["datasource"] = (name ? name : "");
}

void GuiDataEdit::setProperties( const Params &properties ) {
    GuiControl::setProperties( properties );

    fromMember( m_dataField ,properties ,"datafield" );

    const char *s;

    if( (s = getMember( properties ,"datasource" )) != NullPtr && *s ) {
        auto *datasource = root().getBindingAs_<IDataSource>( s );

        if( datasource )
            Bind( tocstr(m_dataField) ,*datasource );
    }
}

//--
void GuiDataEdit::Bind( const char *field ,IDataSource &source ) {
    if( m_dataSource ) {
        m_dataSource->Revoke( *this );
    }

    m_dataSource = source;
    m_dataField = field;

    if( m_dataSource ) {
        m_dataSource->Subscribe( *this );
    }
}

IAPI_DEF GuiDataEdit::onDataCommit( IDataSource &source ,Params &data ) {
    if( m_dataField.empty() ) return INODATA;

    String s; getValue( s );

    data[ m_dataField ] = s;

    return IOK;
}

IAPI_DEF GuiDataEdit::onDataChanged( IDataSource &source ,const Params &data ) {
    if( m_dataField.empty() ) return INODATA;

    NameType decl; String value;

    if( getDecl( data ,tocstr(m_dataField) ,decl ,value ) ) {
        setValue( tocstr(value) );
    }

    /* const String *field = peekMember( data ,tocstr(m_dataField) );

    if( field )
        setValue( field->c_str() ); */

    return IOK;
}

//////////////////////////////////////////////////////////////////////////////
//! GuiDataLabel

REGISTER_CLASS(GuiDataLabel)

//-- properties
void GuiDataLabel::getProperties( Params &properties ) const {
    GuiDataEdit_<String>::getProperties(properties);
    GuiLabel::getProperties(properties);
}

void GuiDataLabel::setProperties( const Params &properties ) {
    GuiLabel::setProperties( properties );
    GuiDataEdit_<String>::setProperties( properties );
}

//////////////////////////////////////////////////////////////////////////////
//! GuiTextBox

REGISTER_CLASS(GuiTextBox)
REGISTER_EDITBOX(String,GuiTextBox);

GuiTextBox::GuiTextBox() :
    m_offset(0) ,m_cursor(0) ,m_margins(4)
    ,m_showCursor(false) ,m_blinkCursor(true) ,m_blinkLast(0)
{
    m_colors = theTheme().getColors( MyPUID ,"normal" );
}

//-- properties
void GuiTextBox::getProperties( Params &properties ) const {
    GuiDataEdit_<String>::getProperties(properties);
    GuiWithText::getProperties(properties);
}

void GuiTextBox::setProperties( const Params &properties ) {
    GuiWithText::setProperties( properties );
    GuiDataEdit_<String>::setProperties( properties );

    // setValue( m_text.c_str() );
}

//--
void GuiTextBox::onMouseDown( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) {
    GuiControl::onMouseDown( p ,mouseButton ,keyState );

    Point pos = p; pos -= area().getTopLeft();
    OsPoint point;

    int w = pos.x - m_margins.left;
    int d = 0;

    m_cursor = 0;

    for( int i=0; i<m_text.size(); ++i ) {
        String before( m_text.c_str()+m_offset ,MAX(m_cursor-m_offset,0) );

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
    root().RegionSetArea( area() ,false );

//-- area
    GuiDataEdit_<String>::onDraw( updateArea );
    GuiWithText::onDraw( updateArea );

    Rect r = area().getDims().Deflate(m_margins);

    OsRect extends;
    String before( tocstr(m_text)+m_offset ,MAX(m_cursor-m_offset,0) ); //! MAX should not be needed, but protects from invalid params
    DrawTextAlign( tocstr(before) ,r ,textalignCenterLeft ,&extends );
    m_extends = extends;

    int cursorx = r.left = extends.right;
    DrawTextAlign( tocstr(m_text)+m_cursor ,r ,textalignCenterLeft ,&extends );
    m_extends |= extends;

    drawCursor( cursorx - (m_cursor ? -1 : 0) );

//--
    root().RegionSetArea( clip0 ,false );
}

void GuiTextBox::onKeyDown( OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) {
    GuiControl::onKeyDown( keyState ,keyCode ,c );

    if( keyState.ctrl && c == 22 ) { //TODO why 22  // && c == 'v' ) {
        char dataType[32] = "STRING";
        void *data;
        int length;
        OsHandle h = root().GetHandle();

        if( OsClipboardGetData( h ,dataType ,&data ,&length ) == ENOERROR && data ) {
            text() = (char*) data;
        }

        return;
    }

    switch( keyCode ) {
        case OS_KEYCODE_BACKSPACE:
            if( m_cursor ) {
                m_text.erase( --m_cursor ,1 );
                onDataEdit( true );
            }
            break;

        case OS_KEYCODE_DELETE:
            if( m_cursor < m_text.size() ) {
                m_text.erase( m_cursor ,1 );
                onDataEdit( true );
            }
            break;

        case OS_KEYCODE_HOME: {
            if( m_cursor == m_offset )
                m_cursor = m_offset = 0;
            else
                m_cursor = m_offset;
        } break;

        case OS_KEYCODE_END: if( m_cursor < m_text.size() ) {
            ++m_cursor;

            if( updateOffset(1) != 0 ) {
                int delta = m_text.size() - m_cursor;
                m_cursor = m_text.size();
                updateOffset(delta);
            } else {
                while( ++m_cursor < m_text.size() && updateOffset(1) == 0 ) {}
                --m_cursor; --m_offset;
            }
        } break;

        case OS_KEYCODE_LEFT:
            if( m_cursor == 0 ) break;

            if( keyState.ctrl ) { //! word
                const char *str = m_text.c_str();
                bool base = cisalphanum( str[m_cursor-1] );

                int &i = m_cursor; for( --i ; i>0; --i ) {
                    if( cisalphanum(str[i-1]) != base ) break;
                }

            } else { //! char
                 --m_cursor;
            }
            m_offset = MIN( m_offset ,m_cursor );
            break;

        case OS_KEYCODE_RIGHT: if( m_cursor < m_text.size() ) {
            if( keyState.ctrl ) { //! word
                const char *str = m_text.c_str();
                int &i = m_cursor; int j = i;
                bool base = cisalphanum( str[i] );

                for( ++i; i < m_text.size(); ++i ) {
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
                m_text.insert( m_cursor++ ,1 ,c );
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
        String before( m_text.c_str()+m_offset ,MAX(m_cursor-m_offset,0) );

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

        auto *menu = &m_menu; // new GuiMenu();

        menu->clear();

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

void GuiComboBox::Subscribe( IGuiMessage &listener ) {
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

void GuiComboBox::onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) {
    if( commandId >= GUI_COMMANDID_MENU && commandId <= GUI_COMMANDID_MENUMAX && params ) {
        const auto it = params->find( "text" );

        if( it == params->end() ) return;

        text() = it->second;
        onDataEdit();

        m_popup.Close();

        Refresh();
    }
}

//////////////////////////////////////////////////////////////////////////////
//! GuiColorBox

REGISTER_CLASS(GuiColorBox);
REGISTER_EDITBOX( ColorRef ,GuiColorBox );

void GuiColorBox::onDraw( const OsRect &updateArea ) {
    int h = area().getHeight();

    //-- text
    m_margins.left = h;
    GuiComboBox::onDraw( updateArea );

    //-- color
    Rect r = { 0 ,0 ,h ,h };

    r.Deflate( 4 );

    ColorRef color;

    fromString( color ,text() );

    root().SetForeColor( color );
    root().SetFillColor( color );

    DrawRectangle( r );
}

//////////////////////////////////////////////////////////////////////////
//! GuiBoolBox

REGISTER_CLASS(GuiBoolBox);
REGISTER_EDITBOX( bool ,GuiBoolBox );

//////////////////////////////////////////////////////////////////////////
//! GuiAlignBox

REGISTER_CLASS(GuiAlignBox);
REGISTER_EDITBOX( GuiAlign ,GuiAlignBox );

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
    m_colors.normal = theTheme().getColors( MyPUID ,"normal" );
    m_colors.title = theTheme().getColors( MyPUID ,"title" );
    m_colors.col1 = theTheme().getColors( MyPUID ,"col1" );
    m_colors.col2 = theTheme().getColors( MyPUID ,"col2" );
    m_colors.row1 = theTheme().getColors( MyPUID ,"row1" );
    m_colors.row2 = theTheme().getColors( MyPUID ,"row2" );
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

    ListOf<String> fields;
    fromString( fields ,getMember( properties ,"fields" ) );

    int n = MAX( coords.size() ,titles.size() );

    for( int i=0; i<n; ++i ) {
        GuiCoord coord = ( i < coords.size() ? coords[i] : 10.f );
        String title = ( i < titles.size() ? titles[i] : "" );
        String field = ( i < fields.size() ? fields[i] : "" );

        addCol( coord ,tocstr(title) ,tocstr(field) );
    }

    const char *pRows = getMember( properties ,"rows" ,NullPtr );
    if( pRows && *pRows ) {
        int nRows;

        fromString( nRows ,pRows );
        setRows( nRows );
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
void GuiGrid::setCols( int cols ,const ListOf<String> *titles ,const ListOf<String> *fields ) {
    int n = (int) colCount();

    GuiCoord c0;

    for( int x=0; x<cols; ++x ) {
        const char *title = titles ? tocstr( (*titles)[x] ) : "";
        const char *field = fields ? tocstr( (*fields)[x] ) : "";

        if( x >= n ) {
            addCol( c0 ,title ); ++n;
        } else {
            if( titles ) col(x).title = title;
        }

        fromString( col(x).field ,field );
    }
}

void GuiGrid::setRows( int rows ,size_t baseIndex ) {
    int n = (int) rowCount();
    int i = (int) baseIndex;

    for( int y=0; y<rows; ++y ) {
        if( y >= n ) {
            addRow(); ++n; //! expand
        }

        row(y).index = i++;
    }

    //-- trim
    if( rows > 0 ) while( n > rows ) {
        m_rows.pop_back(); --n;
    }
}

void GuiGrid::addCol( const GuiCoord &width ,const char *title ,const char *field ,bool editable ) {
    m_cols.emplace_back();

    auto &col = m_cols.back();

    col.coord = width;

    if( title ) {
        col.title = title ? title : "";
        fromString( col.field ,field ? field : "" );

        if( editable && !col.field.type.empty() ) { //! declaring editor using type
            PUID editableId;

            col.edit = makeEditBox( tocstr(col.field.type) );
        }
    }

    //-- add col to existing rows if any
    for( auto &it : m_rows ) {
        it.m_cells.emplace_back();
    }
}

int GuiGrid::addRow() {
    int i = (int) m_rows.size();

    m_rows.emplace_back();

    auto &row = m_rows.back();

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
        cell.edit->setValue( tocstr(cell.text) );
    }
}

void GuiGrid::setCellEdit( int y ,int x ,GuiDataEdit *editor ,bool editable ) {
    if( y < rowCount() && x < colCount() ) {} else return;

    auto &cell = row(y).col(x);

    cell.edit = editor;
    cell.editable = editable;

    if( cell.edit ) {
        cell.edit->setValue( tocstr(cell.text) );
    }
}

void GuiGrid::setCellType( int y ,int x ,const PUID &editableId ,bool editable ) {
    GuiDataEdit *p = ICreateGuiEdit( editableId );

    if( p ) {
        setCellEdit( y ,x ,p ,editable );
    }
}

void GuiGrid::setCellType( int y ,int x ,const char *type ,bool editable ) {
    PUID editableId;

    if( findClassIdByName( type ,editableId ) ) {
        setCellType( y ,x ,editableId ,editable );
    }
}

void GuiGrid::Clear( bool withRefresh ) {
    m_rows.clear();

    if( withRefresh ) Refresh();
}

//-- Data
void GuiGrid::Bind( IDataSource *datasource ) {
    if( m_datasource ) {
        m_datasource->Revoke( *this );
    }

    m_datasource = datasource;

    if( !datasource ) return;

    datasource->Subscribe( *this );

    //-- edit & bindings
    int nCols = (int) colCount();
    int nRows = (int) rowCount();

    String id;
    Params data;

    for( auto y=0; y<nRows; ++y ) {
        toString( row(y).index ,id );

        if( m_datasource->Seek( tocstr(id) ) != IOK ) continue;

        if( m_datasource->readHeader( data ) != IOK ) {
            data.clear();
        }

        for( int x=0; x<nCols; ++x ) {
            auto &field = col(x).field;

            setCellText( y ,x ,getMember( data ,tocstr(field.name) ,"" ) );

            if( !col(x).editable ) continue;

            if( !field.type.empty() ) {
                setCellType( y ,x ,tocstr(field.type) ,true );
            }

            if( !row(y).col(x).edit ) {
                setCellType( y ,x ,TINY_STRING_PUID ,true );
            }

            if( row(y).col(x).edit ) {
                row(y).col(x).edit->Bind( tocstr(field.name) ,*datasource );
            }
        }
    }
}

void GuiGrid::UpdateData() {
    int nCols = (int) colCount();
    int nRows = (int) rowCount();

    String id;
    Params data;

    //-- list fields
    for( int x=0; x<nCols; ++x ) {
        auto &field = col(x).field;

        if( !field.name.empty() )
            data[field.name] = "";
    }

    //-- read rows
    for( int y=0; y<nRows; ++y ) {
        toString( row(y).index ,id );

        bool hasData = ( m_datasource->Seek( tocstr(id) ) == IOK && m_datasource->readData( data ) == IOK);

        for( int x=0; x<nCols; ++x ) {
            auto &field = col(x).field;

            const char *value = hasData ? getMember( data ,tocstr(field.name) ,"" ) : "";

            setCellText( y ,x ,value );
        }
    }
}

//-- IDataEvents
IAPI_DEF GuiGrid::onDataCommit( IDataSource &source ,Params &data ) {
    return ENOEXEC; //TODO LATER
}

IAPI_DEF GuiGrid::onDataChanged( IDataSource &source ,const Params &data ) {
    return ENOEXEC; //TODO LATER
}

//-- IGuiEvents
void GuiGrid::onLayout( const OsRect &clientArea ,OsRect &placeArea )  {
    GuiControl::onLayout( clientArea ,placeArea );

    size() = 0; //! nb used for scroll etc...

    m_titleHeight = m_titleCoord.get( area().getHeight() );

    //-- calc row/col placement
    m_rowHeight = m_rowCoord.get( area().getHeight() );

    for( auto &col : m_cols ) {
        col.width = col.coord.get( area().getWidth() );

        size().x += col.width; //! sum of cell widths

        //TODO layout cell edit if any ...
    }

    //-- calc client area size (nb used for scroll)
    size().y = (int) (m_titleHeight + m_rowHeight * m_rows.size());

    //-- layout edit if any
    Rect rrow = { 0 ,0 ,size().x ,m_rowHeight };

    if( config().showTitle ) {
        rrow += Point( 0 ,m_rowHeight );
    }

    Rect rinner ,region;

    int j=0; for( auto &row : m_rows ) {
        rrow.bottom = rrow.top + m_rowHeight;

        Rect rcell = rrow;

        int i=0; for( auto &cell : row.m_cells ) {
            auto &col = m_cols[i];

            rcell.right = rcell.left + col.width;

            rinner = rcell; rinner.Deflate( m_cellMargin );

            if( cell.edit ) {
                Rect redit = rinner + area().getTopLeft();

                cell.edit->setRoot( root() );
                cell.edit->coords() = { 0 ,0 ,100.f ,100.f };
                cell.edit->onLayout( redit ,redit );
            }

            rcell.left = rcell.right;
            ++i;
        }

        rrow.top = rrow.bottom;
        ++j;
    }
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

    if( config().showTitle ) {
        SetColors( m_colors.title );
        DrawRectangle( rrow );

        rrow.bottom = rrow.top + m_rowHeight;

        Rect rcell = rrow;

        int i=0; for( auto &col : m_cols ) {
            rcell.right = rcell.left + col.width;

            if( !col.title.empty() ) {
                DrawTextAlign( tocstr(col.title) ,rcell ,config().titleAlign );
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

            if( cell.edit ) { //TODO only use on the fly placement if col edit ... not for cell edit
                Rect redit = rinner + area().getTopLeft();

                // cell.edit->setRoot( root() );
                // cell.edit->area() = redit;
                cell.edit->onDraw( redit );
            } else {
                DrawTextAlign( cell.text.c_str() ,rinner ,config().cellAlign );
            }

            rcell.left = rcell.right;
            ++i;
        }

        rrow.top = rrow.bottom;
        ++j;
    }

    root().RegionSetArea( region ,false );
}

void GuiGrid::onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) {
    GuiControl::onMouse( mouseAction ,keyState ,mouseButton ,points ,pos );

    if( mouseAction != osMouseButtonDown ) return;

    Point p = pos[0]; p -= area().getTopLeft();

//-- row
    int y = (p.y - (config().showTitle ? m_titleHeight : 0) ) / m_rowHeight;

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

    //TODO col edit if any

    if( cell.edit && cell.editable ) {

        // cell.edit->setValue( cell.text.c_str() ); //TODO should not be here, works with datasource bound control, what if manual feed cell content ? ..check

        SAFECALL(m_editFocus)->onLostFocus();

        m_editFocus = cell.edit;

        if( m_editFocus ) {
            m_editFocus->onGotFocus();
            m_editFocus->onMouse( mouseAction ,keyState ,mouseButton ,points ,pos );
        }
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
GuiDataEdit *GuiGrid::makeEditBox( const char *editable ) {
    PUID editableId;

    return findClassIdByName( editable ,editableId ) ? ICreateGuiEdit( editableId ) : NullPtr;
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
//! GuiNavBar

REGISTER_CLASS(GuiNavBar);

static const char *guiNavBar = {
    "controls = {"
        "first:GuiLink = { commandId=24; align=left,horizontal; font=large; text=|<; coords={0,0,6%,100%} }"
        "prev:GuiLink = { commandId=22; align=left,horizontal; font=large; text=<<; coords={0,0,6%,100%} }"
        "last:GuiLink = { commandId=25; align=right,horizontal; font=large; text=>|; textalign=right; coords={0,0,6%,100%} }"
        "next:GuiLink = { commandId=23; align=right,horizontal; font=large; text=>>; textalign=right; coords={0,0,6%,100%} }"
        "label:GuiLabel = { align=center; font=normal; text=; textalign=center; coords={0,0,60%,100%} }"
    "}"
};

GuiNavBar::GuiNavBar() {
    setPropertiesWithString(guiNavBar);
}

void GuiNavBar::Bind( IGuiMessage &listener ) {
    getControlAs_<GuiLink>("first")->Subscribe(listener);
    getControlAs_<GuiLink>("prev")->Subscribe(listener);
    getControlAs_<GuiLink>("next")->Subscribe(listener);
    getControlAs_<GuiLink>("last")->Subscribe(listener);
}

//////////////////////////////////////////////////////////////////////////
//! GuiNavGrid

REGISTER_CLASS(GuiNavGrid);

static const char *guiNavGrid = {
    "controls = {"
        "grid:GuiGrid = { align=top; anchor=vertical; coords={0,0,100%,90%} }"
        "nav:GuiNavBar = { align=bottom; anchor=vertical; coords={0,0,100%,10%} }"
    "}"
};

GuiNavGrid::GuiNavGrid() : m_pageId(0) ,m_pageCount(0) {
    setPropertiesWithString(guiNavGrid);

    m_grid = getControlAs_<GuiGrid>("grid");
    m_nav = getControlAs_<GuiGroup>("nav");

    if( !m_nav.isNull() )
        m_nav->Bind( *this );
}

//--
void GuiNavGrid::updateInfo() {
    if( m_grid.isNull() || !m_grid->getDataSource() ) return;

    m_dataCount = getInfoField_<int>( m_grid->getDataSource() ,"count" );
    m_rowCount = (int) m_grid->rowCount();

//-- info
    if( m_rowCount > 0 )
        m_pageCount = (int) ( m_dataCount / m_rowCount ) + 1;
    else
        m_pageCount = 0;
}

void GuiNavGrid::updatePage( int pageId ) {
    updateInfo();

    m_pageId = pageId;

//-- grid
    m_grid->setRows( m_rowCount ,pageId * m_rowCount );
    m_grid->UpdateData();

//-- nav
    String label = "Page ";

    Format( label ,"Page %d/%d" ,128 ,m_pageId+1 ,m_pageCount );
    m_nav->getControlAs_<GuiLabel>("label")->text() = label;
}

void GuiNavGrid::onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) {
    updateInfo();

    switch( commandId ) {
        case GUI_MESSAGEID_PREV:
            updatePage( MAX( m_pageId-1 ,0 ) ); break;
        case GUI_MESSAGEID_NEXT:
            updatePage( MIN( m_pageId+1 ,m_pageCount-1 ) ); break;
        case GUI_MESSAGEID_FIRST:
            updatePage( m_pageId = 0 ); break;
        case GUI_MESSAGEID_LAST:
            updatePage( m_pageId = m_pageCount-1 ); break;
        default:
            GuiGroup::onCommand( source ,commandId ,param ,params ,extra );
            return;
    }

    Refresh();
}

//////////////////////////////////////////////////////////////////////////
//! GuiSheet

bool GuiSheet::hasHeading( const char *field ) {
    for( auto &heading : m_headings ) {
        if( heading.field == field ) return true;
    }

    return false;
}

void GuiSheet::Bind( IDataSource *datasource ) {
    Clear(false);

    m_datasource = datasource;

    if( !datasource ) return;

    Params params;

    datasource->readHeader( params ,true );

    bool listAll = false;

    //-- process headings
    for( auto &heading : headings() ) {
        if( heading.field == "*" ) {
            listAll = true; continue;
        }

        if( heading.label.empty() ) continue;

        NameType decl;
        String value;

        if( !getDecl( params ,tocstr(heading.field) ,decl ,value ) )
            continue;

        if( IFAILED(onBindField( decl )) ) continue;

        int iRow = addRow();

        setCellText( iRow ,0 ,tocstr(heading.label) );
        setCellText( iRow ,1 ,tocstr(value) );

        if( !decl.type.empty() ) {
            setCellType( iRow ,1 ,tocstr(decl.type) ,true );
        }

        if( !row(iRow).col(1).edit ) {
            setCellType( iRow ,1 ,TINY_STRING_PUID ,true );
        }

        heading.edit = row(iRow).col(1).edit;

        if( heading.edit ) {
            row(iRow).col(1).edit->Bind( tocstr(decl.name) ,*datasource );
        }
    }

    //-- [option] complete with remaining fields from datasource
    if( !listAll ) return;

    for( const auto &it : params ) {
        NameType decl;

        fromString( decl ,tocstr(it.first) );

        if( hasHeading(tocstr(decl.name)) ) continue;

        if( IFAILED(onBindField( decl )) ) continue;

        int iRow = addRow();

        setCellText( iRow ,0 ,tocstr(it.first) );
        setCellText( iRow ,1 ,tocstr(it.second) );

        if( !decl.type.empty() ) {
            setCellType( iRow ,1 ,tocstr(decl.type) ,true );
        }

        if( !row(iRow).col(1).edit ) {
            setCellType( iRow ,1 ,TINY_STRING_PUID ,true );
        }

        if( row(iRow).col(1).edit ) {
            row(iRow).col(1).edit->Bind( tocstr(decl.name) ,*datasource );
        }
    }

    Update();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////
#endif //TINY_NO_XGUI