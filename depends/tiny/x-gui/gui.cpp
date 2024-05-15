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
//! DEVNOTE check timers in tiny

/* struct ITimerListener {
    virtual void onTimer( uint32_t id ,OsEventTime now ,OsEventTime last ,void *userdata ) = 0;
};

struct TimerInfo {
    ITimerListener *listener;
    OsEventTime delay;
    OsEventTime last;
    void *userdata;
}; */

//////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

using namespace TINY_NAMESPACE_GUI;

//////////////////////////////////////////////////////////////////////////
//! Coords

template <>
GuiCoord &fromString( GuiCoord &p ,const String &s ,size_t &size ) {
    fromString( p.value ,s ,size );

    if( size == 0 ) return p;

    const char *str = s.c_str() + size;

    if( SkipSymbol( str ,'%' ) ) {
        ++size; p.unit = GuiCoord::unitPercent;
    } else {
        p.unit = GuiCoord::unitPixel;
    }

    return p;
}

template <>
String &toString( const GuiCoord &p ,String &s ) {
    if( p.unit == GuiCoord::Unit::unitPixel ) {
        toString( (int) p.value ,s );
    } else {
        Format( s ,"%d%%" ,32 ,(int) p.value );
    }
    return s;
}

///--
template <>
GuiCoords &fromString( GuiCoords &p ,const String &s ,size_t &size ) {
    StringList slist;

    fromString( slist ,s ,size ); if( size == 0 ) return p;

    if( slist.size() > 0 ) fromString( p.left ,slist[0] ); else return p;
    if( slist.size() > 1 ) fromString( p.top ,slist[1] ); else return p;
    if( slist.size() > 2 ) fromString( p.right ,slist[2] ); else return p;
    if( slist.size() > 3 ) fromString( p.bottom ,slist[3] ); else return p;

    return p;
}

template <>
String &toString( const GuiCoords &p ,String &s ) {
    String si;

    s = '{';
    toString( p.left ,si ); s += si; s += ',';
    toString( p.top ,si ); s += si; s += ',';
    toString( p.right ,si ); s += si; s += ',';
    toString( p.bottom ,si ); s += si;
    s += '}';

    return s;
}

//////////////////////////////////////////////////////////////////////////
//! Align

    //TODO merge TextAlign and GuiAlign ...

template <>
const char *Enum_<GuiAlign>::names[] = {
    "none"
    ,"left" ,"right" ,"centerh"
    ,"top" ,"bottom" ,"centerv"
    ,"horizontal" ,"vertical"
    ,"center"
};

template <>
const GuiAlign Enum_<GuiAlign>::values[] = {
    noAlign
    ,alignLeft ,alignRight ,alignCenterH
    ,alignTop ,alignBottom ,alignCenterV
    ,alignAnchorH ,alignAnchorV
    ,alignCenter
};

//TODO remove function below, use Enum_ pattern
template <>
GuiAlign &fromString( GuiAlign &p ,const String &s ,size_t &size ) {
    StringList slist;

    fromString( slist ,s ,size ); if( size == 0 ) return p;

    p = noAlign;

    for( const auto &it : slist ) {
        if( iMatch( it ,"left" ) ) p = (GuiAlign) (p | alignLeft);
        if( iMatch( it ,"right" ) ) p = (GuiAlign) (p | alignRight);
        if( iMatch( it ,"centerh" ) ) p = (GuiAlign) (p | alignCenterH);

        if( iMatch( it ,"top" ) ) p = (GuiAlign) (p | alignTop);
        if( iMatch( it ,"bottom" ) ) p = (GuiAlign) (p | alignBottom);
        if( iMatch( it ,"centerv" ) ) p = (GuiAlign) (p | alignCenterV);

        if( iMatch( it ,"fill" ) ) p = (GuiAlign) (p | alignFill);
        if( iMatch( it ,"center" ) ) p = (GuiAlign) (p | alignCenter);

        if( iMatch( it ,"horizontal" ) ) p = (GuiAlign) (p | alignAnchorH);
        if( iMatch( it ,"vertical" ) ) p = (GuiAlign) (p | alignAnchorV);
    }

    return p;
}

//////////////////////////////////////////////////////////////////////////////
//! ColorPair

//-- string
template <>
ColorPair &fromString( ColorPair &p ,const String &s ,size_t &size ) {
    StringList slist;

    fromString( slist ,s ,size ); if( slist.size() != 2 ) return p;

    ColorRef color;

    fromString( color ,slist[0] ); p.foreColor = color;
    fromString( color ,slist[1] ); p.fillColor = color;

    return p;
}

//-- manifest
template <>
ColorPair &fromManifest( ColorPair &p ,const Params &manifest ) {
    ColorRef color;

    fromString( color ,getMember( manifest ,"forecolor" ) ); p.foreColor = color;
    fromString( color ,getMember( manifest ,"fillcolor" ) ); p.fillColor = color;

    return p;
}

//-- helpers
void OsGuiSetDrawColors( IGuiDisplay &display ,const ColorPair &colors ) {
    display.SetColor( OS_SELECT_FORECOLOR ,colors.foreColor );
    display.SetColor( OS_SELECT_FILLCOLOR ,colors.fillColor );
}

void OsGuiSetTextColors( IGuiDisplay &display ,const ColorPair &colors ) {
    display.SetColor( OS_SELECT_TEXTCOLOR ,colors.foreColor );
    display.SetColor( OS_SELECT_BACKCOLOR ,colors.fillColor );
}

//////////////////////////////////////////////////////////////////////////
//! ColorQuad

ColorQuad &setDrawColors( ColorQuad &q ,const ColorPair &p ) {
    q.foreColor = p.foreColor;
    q.fillColor = p.fillColor;
    return q;
}

ColorPair &getDrawColors( const ColorQuad &q ,ColorPair &p ) {
    p.foreColor = q.foreColor;
    p.fillColor = q.fillColor;
    return p;
}

ColorPair getDrawColors( const ColorQuad &q ) {
    ColorPair p = { q.foreColor ,q.fillColor };
    return p;
}

ColorQuad &setTextColors( ColorQuad &q ,const ColorPair &p ) {
    q.textColor = p.foreColor;
    q.backColor = p.fillColor;
    return q;
}

ColorPair &getTextColors( const ColorQuad &q ,ColorPair &p ) {
    p.foreColor = q.textColor;
    p.fillColor = q.backColor;
    return p;
}

ColorPair getTextColors( const ColorQuad &q ) {
    ColorPair p = { q.textColor ,q.backColor };
    return p;
}

//-- string
template <>
ColorQuad &fromString( ColorQuad &p ,const String &s ,size_t &size ) {
    StringList slist;

    fromString( slist ,s ,size ); if( slist.size() != 4 ) return p;

    ColorRef color;

    fromString( color ,slist[0] ); p.foreColor = color;
    fromString( color ,slist[1] ); p.fillColor = color;
    fromString( color ,slist[2] ); p.textColor = color;
    fromString( color ,slist[3] ); p.backColor = color;

    return p;
}

//-- manifest
template <>
ColorQuad &fromManifest( ColorQuad &p ,const Params &manifest ) {
    ColorRef color;

    fromString( color ,getMember( manifest ,"forecolor" ) ); p.foreColor = color;
    fromString( color ,getMember( manifest ,"fillcolor" ) ); p.fillColor = color;
    fromString( color ,getMember( manifest ,"textcolor" ) ); p.textColor = color;
    fromString( color ,getMember( manifest ,"backcolor" ) ); p.backColor = color;

    return p;
}

//-- helpers
void OsGuiSetColors( IGuiDisplay &display ,const ColorQuad &colors ,bool setTextColors ) {
    display.SetColor( OS_SELECT_FORECOLOR ,colors.foreColor );
    display.SetColor( OS_SELECT_FILLCOLOR ,colors.fillColor );

    if( !setTextColors ) return;

    display.SetColor( OS_SELECT_TEXTCOLOR ,colors.textColor );
    display.SetColor( OS_SELECT_BACKCOLOR ,colors.backColor );
}

//////////////////////////////////////////////////////////////////////////
//! Highlights

template <>
const char *Enum_<Highlight>::names[] = {
    "normal"
    ,"hoover" ,"pushed" ,"notpushed"
    ,"dragged" ,"disabled"
};

template <>
const Highlight Enum_<Highlight>::values[] = {
    highlightNormal
    ,highlightHoover ,highlightPushed ,highlightNotPushed
    ,highlightDragged ,highlightDisabled
};

//////////////////////////////////////////////////////////////////////////
//! Fonts

static GuiFont g_fontDefault( "clean" ,24 ,OS_FONTWEIGHT_NORMAL ,OS_FONTSTYLE_ITALIC ,OS_FONTPITCH_ANY );

const GuiFont &getDefaultFont() {
    return g_fontDefault;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! Controls

void Emplace( GuiAlign align ,int &a ,int &b ,int &A ,int &B ,bool anchor ) {
    int ab = b - a ,AB2 = (A + B)/2; //! @note AB2 = A + (B-A)/2

    if( (align & (alignLeft | alignRight)) == (alignLeft | alignRight) ) //! both
        a = A ,b = B;
    else if( align & alignLeft )
        a = A ,b = A + ab ,A = anchor ? b : A;
    else if( align & alignRight )
        a = B - ab ,b = B ,B = anchor ? a : B;
    else if( align & alignCenterH )
        a = AB2 - ab/2 ,b = AB2 + ab/2;
    //! else no op
}

OsRect Emplace( GuiAlign align ,const GuiCoords &coords ,const OsRect &clientArea ,OsRect &area ) {
    OsRect r = coords.getOsRect(clientArea);

    auto halign = (GuiAlign) (align & 7);
    auto valign = (GuiAlign) ((align >> 3) & 7);

    if( halign ) Emplace( halign ,r.left ,r.right ,area.left ,area.right ,(align & alignAnchorH) != 0 );
    if( valign ) Emplace( valign ,r.top ,r.bottom ,area.top ,area.bottom ,(align & alignAnchorV) != 0 );

    return r;
}

//////////////////////////////////////////////////////////////////////////
//! GuiControl

static bool g_controlRegister = registerClassName_<GuiControl>();

//--
GuiControl::GuiControl( GuiControlWindow *root ) :
    m_root(root)
{
    VisualTheme &theme = root ? root->getTheme() : theTheme();

    m_colors = theme.getColors( MyUUID ,"normal" );
}

bool GuiControl::shouldDraw( const OsRect &updateArea ) const {
    return m_visible && m_root && (area() & updateArea);
}

void GuiControl::getCenteredArea( const OsPoint &dims ,Rect &r ) const {
    r = area();

    int w = r.getWidth();
    int h = r.getHeight();

    int w2 = (w - dims.x)/2;
    int h2 = (h - dims.y)/2;

    r.left += w2; r.right = r.left + dims.x;
    r.top += h2; r.bottom = r.top + dims.y;
}

///-- properties
void GuiControl::getProperties( Params &properties ) const {
    toString( m_id ,properties["id"] );
    toString( m_coords ,properties["coords"] );

    toString( ColorRef(m_colors.textColor) ,properties["textcolor:ColorRef"] );
}

void GuiControl::setProperties( const Params &properties ) {
    fromString( m_id ,getMember( properties ,"id" ) );
    fromString( m_coords ,getMember( properties ,"coords") );

    fromString( m_align ,getMember( properties ,"align") );
    const char *anchor = getMember( properties ,"anchor" );
    if( anchor && *anchor ) {
        GuiAlign anchor;

        fromString( anchor ,getMember( properties ,"anchor") );

        m_align = (GuiAlign) (m_align | anchor);
    }

    fromString( m_visible ,getMember( properties ,"visible") );
    fromString( m_enabled ,getMember( properties ,"enabled") );

//-- colors
    const char *prop ,*back;
    ColorRef color;

    prop = getMember( properties ,"textcolor"); if( prop && *prop ) {
        fromString( color ,prop ); m_colors.textColor = color;
    }

    prop = getMember( properties ,"bordercolor"); if( prop && *prop ) { fromString( color ,prop ); m_colors.foreColor = color; }
    back = getMember( properties ,"background"); if( back && *back ) { fromString( color ,back ); m_colors.fillColor = color; }
    prop = getMember( properties ,"backcolor"); if( (prop && *prop) || (back && *back) ) { fromString( color ,prop ? prop : back ); m_colors.backColor = color; }
}

void GuiControl::setPropertiesWithString( const char *properties ) {
    Params params;

    fromString( params ,properties );

    setProperties( params );
}

bool GuiControl::loadProperties( const char *filename ,const char *path ) {
    _TODO; return false;
}

bool GuiControl::saveProperties( const char *filename ,const char *path ) {
    _TODO; return false;
}

///-- IGuiContext (replication, not inherited)
void GuiControl::SetForeColor( OsColorRef c ) {
    if( !isOrphan() ) root().SetForeColor(c);
}

void GuiControl::SetFillColor( OsColorRef c ) {
    if( !isOrphan() ) root().SetFillColor(c);
}

void GuiControl::SetTextColor( OsColorRef c ) {
    if( !isOrphan() ) root().SetTextColor(c);
}

void GuiControl::SetBackColor( OsColorRef c ) {
    if( !isOrphan() ) root().SetBackColor(c);
}

void GuiControl::SetDrawColors( const ColorPair &p ) {
    if( isOrphan() ) return;

    root().SetForeColor( p.foreColor );
    root().SetFillColor( p.fillColor );
}

void GuiControl::SetTextColors( const ColorPair &p ) {
    if( isOrphan() ) return;

    root().SetTextColor( p.foreColor );
    root().SetBackColor( p.fillColor );
}

void GuiControl::SetColors( const ColorQuad &q ) {
    if( isOrphan() ) return;

    root().SetForeColor( q.foreColor );
    root().SetFillColor( q.fillColor );
    root().SetTextColor( q.textColor );
    root().SetBackColor( q.backColor );
}

void GuiControl::SetFont( const GuiFont &font ) {
    if( !isOrphan() ) root().SetFont( font );
}

void GuiControl::RegionSetArea( OsRect &r ) { // },bool useOffset ) {
    if( !isOrphan() ) root().RegionSetArea( Rect(r) + area().getTopLeft() ,true );
}

/* void GuiControl::RegionGetArea( OsRect &r ) {
    if( !isOrphan() ) root().RegionGetArea( r );
} */

void GuiControl::RegionSetOffset( int x ,int y ) {
    if( !isOrphan() ) root().RegionSetOffset( x ,y );
}

void GuiControl::RegionSetScale( float x ,float y ) {
    if( !isOrphan() ) root().RegionSetScale( x ,y );
}

///-- IGuiSurface (replication, not inherited)
void GuiControl::DrawRectangle( const OsRect &r ) {
    if( !isOrphan() ) root().DrawRectangle( Rect(r) + area().getTopLeft() );
}

void GuiControl::DrawEllipse( const OsRect &r ) {
    if( !isOrphan() ) root().DrawEllipse( Rect(r) + area().getTopLeft() );
}

void GuiControl::DrawLine( const OsRect &r ) {
    if( !isOrphan() ) root().DrawLine( Rect(r) + area().getTopLeft() );
}

void GuiControl::DrawTextAlign( const char_t *text ,const OsRect &r ,TextAlign align ,OsRect *rect ) {
    if( isOrphan() ) return;

    root().DrawTextAlign( text ,Rect(r) + area().getTopLeft() ,align ,rect );

    if( rect ) {
        (* (Rect*) rect) -= area().getTopLeft();
    }
}

void GuiControl::DrawPolygon( int npoints ,const OsPoint *points ) {
    if( isOrphan() ) return;

    OsPoint *p = new OsPoint[npoints];

    for( int i=0; i<npoints; ++i ) {
        p[i] = area().getTopLeft() + points[i];
    }

    root().DrawPolygon( npoints ,p );

    delete[] p;
}

void GuiControl::DrawImage( const GuiImage &image ,const OsRect &r ,const OsRect &imgRect ) {
    if( !isOrphan() ) root().DrawImage( image ,Rect(r) + area().getTopLeft() ,imgRect );
}

///-- IDisplay (replication, not inherited)
void GuiControl::SetCursor( int cursorId ) {
    if( !isOrphan() ) root().SetCursor( cursorId );
}

void GuiControl::Refresh( bool noArea ) {
    if( !isOrphan() ) root().Refresh( noArea ? nullptr : &area() );
}

void GuiControl::Update( bool noArea ) {
    if( !isOrphan() ) root().Update( noArea ? nullptr : &area() );
}

///-- IGuiControlEvents
void GuiControl::onLayout( const OsRect &clientArea ,OsRect &area ) {
    m_area = Emplace( m_align ,m_coords ,clientArea ,area );
}

void GuiControl::onDraw( const OsRect &updateArea ) {
    if( !shouldDraw(updateArea) ) return;

    ColorQuad colors = this->colors();

    if( !enabled() ) {
        colors = root().getTheme().getColors( MyUUID ,"disabled" );
    }

    SetColors( colors );

    if( OS_HASCOLOR(colors.foreColor) || OS_HASCOLOR(colors.fillColor) ) {
        root().DrawRectangle( m_area );
    }
}

void GuiControl::onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) {
    OsPoint p = (pos && points) ? pos[0] : OsPoint({0,0});

    switch( mouseAction  ) {
        case osMouseMove: {
            root().HitTrackMouseEnter( *this ,p ,mouseButton ,keyState );
            // HitTrackMouseEnter( p ,mouseButton ,keyState );
            onMouseMove( p ,mouseButton ,keyState );

            break;
        }

        case osMouseClick:
            onClick( p ,mouseButton ,keyState );
            break;

        case osMouseDoubleClick:
            onDoubleClick( p ,mouseButton ,keyState );
            break;

        case osMouseButtonDown:
            onMouseDown( p ,mouseButton ,keyState );
            break;

        case osMouseButtonUp:
            onMouseUp( p ,mouseButton ,keyState );
            break;

        //! @note not mapping MouseWheel in control, get from onMouse if required

        default:
            break;
    }
}

void GuiControl::onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) {
    switch( keyAction ) {
        case osKeyDown: onKeyDown( keyState ,keyCode ,c ); return;
        case osKeyChar: onKeyChar( keyState ,keyCode ,c ); return;
        case osKeyUp: onKeyUp( keyState ,keyCode ,c ); return;
        default:
            break;
    }
}

void GuiControl::onTimer( OsTimerAction timeAction ,OsEventTime now ,OsEventTime last ) {
    //...
}

//////////////////////////////////////////////////////////////////////////////
//! ICommand interface

void GuiCommandOnClick::getProperties( Params &properties ) const {
    toString( m_commandId ,properties["commandId"] );
}

void GuiCommandOnClick::setProperties( const Params &properties ) {
    fromString( m_commandId ,getMember(properties,"commandId" ) );
}

//////////////////////////////////////////////////////////////////////////////
//! GuiSet

void GuiSet::getProperties( Params &properties ) const {
    //TODO
}

void GuiSet::setProperties( const Params &properties ) {
    GuiControl::setProperties( properties );

    const char *controlset = getMember( properties ,"controls" ,"" );

    if( controlset && *controlset ) {} else return;

    ParamList controls; //! @note using ParamList here instread of Params to preserve declaration order
    fromString( controls ,controlset ); //! each entry is a name:type with a Param list of Properties

    NameType itemDecl;
    Params itemProps;

    for( const auto &it : controls ) {
        itemProps.clear();

        fromString( itemDecl ,it.key );
        fromString( itemProps ,it.value );

        GuiControl *control = ICreateGuiControl( itemDecl.type.c_str() );

        if( !control ) continue; //TODO log this

        control->setProperties( itemProps );

        addControl( itemDecl.name.c_str() ,*control );
    }
}

//--
bool GuiSet::setControl( int i ,GuiControl &control ) {
    if( !hasControl(i) ) { assert(false); return false; }

    m_controls.at(i) = &control;

    return true;
}

int GuiSet::addControl( GuiControl &control ) {
    size_t n = m_controls.size();

    if( control.id() == CONTROLID_NONE ) {
        control.setControlId(n+1);
    }

    control.setRoot( root() );

    m_controls.emplace_back( &control );

    return (int) n;
}

int GuiSet::addControl( const char *name ,GuiControl &control ) {
    int id = addControl( control );

    m_names[name] = id;

    return id;
}

bool GuiSet::removeControl( GuiControl &control ) {
    int i = findControlIndex( control );

    if( i < 0 ) return false;

    removeControl(i);
    return true;
}

bool GuiSet::removeControl( const char *name ) {
    auto *control = findControlByName( name );

    if( !control ) return false;

    return removeControl( *control );
}

void GuiSet::removeControl( int i ) {
    if( !hasControl(i) ) { assert(false); return; }

    auto it = m_controls.begin() + (int) i;

    m_controls.erase(it);

//-- reindex
    m_names.eachItem( [i]( const String &name ,int &id ) -> bool {
        if( id > i ) --id; return true;
    });
}

void GuiSet::removeControl() { //! topmost
    size_t n = getControlCount();

    if( n > 0 ) removeControl( (int) (n-1) );
}

void GuiSet::removeAllControls() {
    m_controls.clear();
    m_names.Clear();
}

//--
int GuiSet::findControlIndex( GuiControl &control ) {
    int i = 0;

    for( auto it = m_controls.begin(); it != m_controls.end(); ++it ,++i  ) {
        if( *it == control ) return i;
    }

    return -1;
}

GuiControl *GuiSet::findControlById( controlid_t id ) {
    for( auto &it : m_controls ) {
        if( it->id() == id ) return it.ptr();
    }

    return NullPtr;
}

GuiControl *GuiSet::findControlByName( const char *name ) {
    auto *it = m_names.findItem( name );

    if( it && hasControl(*it) )
        return getControl(*it);

    return NullPtr;
}

const char *GuiSet::digControlName( int i ) {
    auto *name = m_names.digItem( i );

    return name ? name->c_str() : NullPtr;
}

//--
OsError GuiSet::onDropAccept( const OsPoint &p ,IObject *source ,DragOperation operation ,IObject *object ,bool preview ) {
    GuiControl *control = object ? object->As_<GuiControl>() : NullPtr;

    if( !control ) return ENOEXEC;
    if( preview ) return IOK;

    //TODO placement & preview given position
    addControl( *control );

    return IOK;
}

void GuiSet::onLayout( const OsRect &clientArea ,OsRect &placeArea ) {
    GuiControl::onLayout( clientArea ,placeArea );

    for( auto &it : controls() ) if( it ) {
        it->setRoot( root() ); //! making sure child root is set (@note pre constructed control might have missing root)
    }
}

void GuiSet::onTimer( OsTimerAction timeAction ,OsEventTime now ,OsEventTime last ) {
    GuiControl::onTimer( timeAction ,now ,last );

    for( auto &it : controls() ) if( it ) {
        it->onTimer( timeAction ,now ,last );
    }
}

//////////////////////////////////////////////////////////////////////////////
//! GuiLayer

REGISTER_CLASS(GuiLayer)

//--
void GuiLayer::onLayout( const OsRect &clientArea ,OsRect &placeArea ) {
    GuiSet::onLayout( clientArea ,placeArea );

    OsRect groupArea = this->area();
    OsRect r = this->area();

    for( auto &it : controls() ) if( it ) {
        it->onLayout( groupArea ,r );
    }
}

void GuiLayer::onDraw( const OsRect &updateArea ) {
    GuiSet::onDraw( updateArea );

    if( !shouldDraw(updateArea) ) return;

    for( auto &it : controls() ) if( it && it->shouldDraw(updateArea) ) {
        it->onDraw( updateArea );
    }
}

void GuiLayer::onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) {
    GuiSet::onMouse( mouseAction ,keyState ,mouseButton ,points ,pos );

    // OsPoint p0 = pos[0]; //! TODO all points

    for( auto it = controls().rbegin(); it != controls().rend(); ++it ) if( *it ) {
        GuiControl &control = (*it).get();

        if( control.enabled() ) { // && control.visible() && TestHit( p0 ,control.area() ) ) {
            control.onMouse( mouseAction ,keyState ,mouseButton ,points ,pos );
            break;
        }
    }
}

void GuiLayer::onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) {
    GuiSet::onKey( keyAction ,keyState ,keyCode ,c );

    for( auto it = controls().rbegin(); it != controls().rend(); ++it ) if( *it ) {
        GuiControl &control = (*it).get();

        if( control.enabled() ) {
            control.onKey( keyAction ,keyState ,keyCode ,c );
            break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//! GuiTab

REGISTER_CLASS(GuiTab)

//--
void GuiTab::onLayout( const OsRect &clientArea ,OsRect &placeArea ) {
    GuiSet::onLayout( clientArea ,placeArea );

    OsRect groupArea = this->area();

    for( auto &it : m_controls ) {
        OsRect r = this->area();

        it->onLayout( groupArea ,r );
    }
}

void GuiTab::onDraw( const OsRect &updateArea ) {
    GuiSet::onDraw( updateArea );

    if( !shouldDraw(updateArea) ) return;

    GuiControl *p = getCurrentTab();

    if( p && p->shouldDraw(updateArea) )
        p->onDraw( updateArea );
}

void GuiTab::onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) {
    GuiSet::onMouse( mouseAction ,keyState ,mouseButton ,points ,pos );

    GuiControl *p = getCurrentTab();

    if( p ) p->onMouse( mouseAction ,keyState ,mouseButton ,points ,pos );
}

void GuiTab::onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) {
    GuiSet::onKey( keyAction ,keyState ,keyCode ,c );

    GuiControl *p = getCurrentTab();

    if( p ) p->onKey( keyAction ,keyState ,keyCode ,c );
}

//////////////////////////////////////////////////////////////////////////////
GuiScroll::GuiScroll( GuiControl &group ) : m_control(group) {
    coords() = { 0 ,0 ,10 ,100.f };
    align() = (GuiAlign) (GuiAlign::alignRight | GuiAlign::alignAnchorH);
}

void GuiScroll::onLayout( const OsRect &clientArea ,OsRect &placeArea ) {
    GuiControl::onLayout( clientArea ,placeArea );

    int a = m_control.size().y;
    int b = m_control.area().getHeight();
    // int scrollSize = MAX( a-b ,0 );

    visible() = (b && MAX(a-b,0) > 0);
}

void GuiScroll::onDraw( const OsRect &updateArea ) {
    GuiControl::onDraw(updateArea);

    if( !visible() ) return;

    float a = (float) m_control.size().y;
    float b = (float) m_control.area().getHeight();
    if( (int) a <= 0 || (int) b <= 0 ) return;

    // int scrollSize = MAX( a-b ,0 );

    float factor = b / a;

    m_thumb = area();

    m_thumb.top += (int) ( (float) m_offset * factor);
    m_thumb.bottom =  (int) ( (float) m_thumb.top + b * factor);

    root().SetFillColor( OS_COLOR_DARKBLUE );
    root().DrawRectangle( m_thumb );
}

void GuiScroll::onMouseDown( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) {
    GuiControl::onMouseDown( p ,mouseButton ,keyState );

    if( m_thumb & p ) {
        m_grabPoint = p;
        m_grab = true;

        auto *parent = root().getInterface_<GuiControlWindow>();
        SAFECALL(parent)->MouseCapture( *this );
    }
}

void GuiScroll::onMouseUp( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) {
    GuiControl::onMouseUp( p ,mouseButton ,keyState );

    if( m_grab ) {
        auto *parent = root().getInterface_<GuiControlWindow>();
        SAFECALL(parent)->MouseRelease();
    }

    m_grab = false;
}

void GuiScroll::onMouseMove( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) {
    GuiControl::onMouseMove( p ,mouseButton ,keyState );

    if( m_grab ) {
        int y = p.y - m_grabPoint.y;

        int a = m_control.size().y;
        int b = m_control.area().getHeight();
        if( a <= 0 || b <= 0 ) return;

        m_offset = CLAMP( y ,0 ,a-b );

        root().Refresh();
    }
}

//////////////////////////////////////////////////////////////////////////////
//! GuiGroup

REGISTER_CLASS(GuiGroup)

//--
void GuiGroup::setFocus( GuiControl *focus ) {
    if( focus == m_focus.ptr() ) return;

    if( m_focus )
        m_focus->onLostFocus();

    m_focus = focus;

    if( m_focus )
        m_focus->onGotFocus();
}

void GuiGroup::onGotFocus() {
}

void GuiGroup::onLostFocus() {
    if( m_focus ) {
        m_focus->onLostFocus();
        m_focus = NullPtr;
    }
}

//--
void GuiGroup::onLayout( const OsRect &clientArea ,OsRect &placeArea ) {
    GuiSet::onLayout( clientArea ,placeArea );

    Rect groupArea = this->area();
    Rect r = this->area();
    Rect ri = this->area();

    for( auto &it : m_controls ) if( it ) {
            it->onLayout( groupArea ,r );

        ri |= it->area();
    }

    size() = Point( ri.getWidth() ,ri.getHeight() );

    if( m_scrollV ) {
        m_scrollV->setRoot( root() );
        m_scrollV->onLayout( groupArea ,r );
    }
}

void GuiGroup::onDraw( const OsRect &updateArea ) {
    GuiSet::onDraw( updateArea );

    if( !shouldDraw(updateArea) ) return;

    Rect previousArea; Point previousOffset;

    if( !isOrphan() ) { //! SetDrawAreaSpecs
        root().RegionGetOffset( previousOffset );
        root().RegionGetArea( previousArea );

        m_offset = previousOffset;
        m_offset.y -= m_scrollV ? m_scrollV->offset() : 0;

        Rect view = area();

        view += previousOffset;
        view &= previousArea;

        root().RegionSetOffset( m_offset.x ,m_offset.y );
        root().RegionSetArea( view );
    }

    for( auto &it : m_controls ) if( it && it->shouldDraw(updateArea) ) {
        it->onDraw( updateArea );
    }

    if( !isOrphan() ) { //! RestoreDrawAreaSpecs
        root().RegionSetOffset( previousOffset );
        root().RegionSetArea( previousArea );
    }

    SAFECALL(m_scrollV)->onDraw( updateArea );
}

void GuiGroup::onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) {
    GuiSet::onMouse( mouseAction ,keyState ,mouseButton ,points ,pos );

    SAFECALL(m_scrollV)->onMouse( mouseAction ,keyState ,mouseButton ,points ,pos );

    OsPoint p0 = pos[0]; //! TODO all points

    if( !isOrphan() && m_scrollV && m_scrollV->visible() ) {
        for( int i=0; i<points; ++i ) {
            p0.x = pos[i].x - m_offset.x;
            p0.y = pos[i].y - m_offset.y;
        }
    }

    for( auto it = controls().rbegin(); it != controls().rend(); ++it ) if( *it ) {
        GuiControl &control = (*it).get();

        if( control.enabled() && control.visible() && TestHit( p0 ,control.area() ) ) {
            (*it)->onMouse( mouseAction ,keyState ,mouseButton ,points ,&p0 );

            if( mouseAction == osMouseButtonDown ) {
                setFocus( &control );
            }

            break;
        }
    }
}

void GuiGroup::onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) {
    GuiSet::onKey( keyAction ,keyState ,keyCode ,c );

    SAFECALL(m_scrollV)->onKey( keyAction ,keyState ,keyCode ,c );

    if( m_focus && m_focus->enabled() ) {
        m_focus->onKey( keyAction ,keyState ,keyCode ,c );
    }
}

//////////////////////////////////////////////////////////////////////////
//! GuiTabBar

REGISTER_CLASS(GuiTabBar)

#define COMMANDID_TABSELECT     5000

void GuiTabBar::Bind( GuiTab &tabs ) {
    removeAllControls();

    const char *buttonProps = "align=left,horizontal; coords={0,0,10%,100%} "; //! can be set, eg left side tabs etc...

    int n = (int) tabs.getControlCount();

    for( int i=0; i<n; ++i ) {
        GuiButton &p = * new GuiButton();

        const char *name = tabs.digControlName( i );

        p.setPropertiesWithString( buttonProps );

        p.commandId() = COMMANDID_TABSELECT + i;
        p.text() = name;
        p.Subscribe(*this);

        addControl( p );
    }

    m_tabs = tabs;
}

void GuiTabBar::onCommand( GuiControl &source ,uint32_t commandId ,long param ,Params *params ,void *extra ) {
    if( m_tabs.isNull() ) return;

    GuiTab &tabs = m_tabs.get();

    if( commandId >= COMMANDID_TABSELECT && commandId < COMMANDID_TABSELECT+tabs.getControlCount() ) {
        tabs.selectTab( commandId-COMMANDID_TABSELECT );

        Refresh();
    }
}

//////////////////////////////////////////////////////////////////////////////
//! GuiMenu

REGISTER_CLASS(GuiMenu)

GuiMenu::GuiMenu() : m_hoover(-1)
{}

//-- properties
void GuiMenu::getProperties( Params &properties ) const {
    GuiControl::getProperties(properties);
}

void GuiMenu::setProperties( const Params &properties ) {
    GuiControl::setProperties( properties );

    StringList items;

    fromString( items ,getMember( properties ,"items" ) );
    //TODO parse shortcuts using GuiShortcut string api

    int i = 0; for( auto &it : items ) {

        addItem( i ,it.c_str() ,NullPtr );

        ++i;
    }
}

//--
void GuiMenu::addItem( int id ,const char *text ,GuiShortcut *shortcut ) {
    Item item;

    //TODO menu item fromString

    item.commandId = GUI_COMMANDID_MENU+id;
    item.text = text;
    if( shortcut ) {
        item.shortcut = *shortcut;
    }

    m_root.emplace_back( item );
}

bool GuiMenu::makeCommandParam( params_t &params ) {
    if( m_hoover == -1 ) return false;

    Item &item = m_root[m_hoover];

    m_commandId = item.commandId;
    m_commandParams.params["text"] = item.text;

    return GuiCommandOnClick::makeCommandParam( params );
}

//--
void GuiMenu::onLayout( const OsRect &clientArea ,OsRect &placeArea ) {
    GuiControl::onLayout( clientArea ,placeArea );

    if( isOrphan() ) return;

    Rect r = area().getDims();

    const int itemHeight = 32;
    const int sepHeight = 8;

    for( auto &it : m_root ) {
        if( it.text != "-" ) {
            r.bottom = r.top + itemHeight;
        } else {
            r.bottom = r.top + sepHeight;
        }

        it.area = r;
        r.top = r.bottom;
    }

    size().x = area().getWidth();
    size().y = r.bottom;

//-- snap area to computed size (no scroll for menus)
    area().right = area().left + MAX( area().getWidth() ,size().x );
    area().bottom = area().top + MAX( area().getHeight() ,size().y );
}

void GuiMenu::onDraw( const OsRect &updateArea ) {
    GuiControl::onDraw( updateArea );

    Rect r = area() ,ri;

    SetForeColor( OS_COLOR_BLACK );
    SetFillColor( OS_COLOR_BLACK );
    SetTextColor( OS_COLOR_LIGHTGRAY );
    SetBackColor( OS_COLOR_NONE );
    DrawRectangle( r.getDims() );

    int i=0; for( auto &it : m_root ) {
        if( i == m_hoover ) {
            SetForeColor( OS_COLOR_BURGUNDY );
            SetFillColor( OS_COLOR_BURGUNDY );
        } else {
            SetForeColor( OS_COLOR_BLACK );
            SetFillColor( OS_COLOR_BLACK );
        }

        DrawRectangle( it.area );

        ri = it.area;

        if( it.text != "-" ) {
            ri.top += 1; ri.bottom -= 1;
            DrawTextAlign( it.text.c_str() ,it.area ,textalignCenterLeft );
        } else {
            ri.top = ri.bottom = ri.getCenterV();
            ri.left += 2; ri.right -= 4;

            SetForeColor( OS_COLOR_DARKGRAY );
            DrawLine( ri );
        }

        ++i;
    }
}

void GuiMenu::onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) {

//-- find hoover
    Point p = pos[0]; p -= area().getTopLeft();
    int hoover = m_hoover;
    m_hoover = -1;

    int i = 0; for( auto &it : m_root ) {
        if( it.text != "-" && it.area & p ) {
            m_hoover = i; break;
        }

        ++i;
    }

    if( hoover != m_hoover ) {
        Refresh();
    }

    GuiControl::onMouse( mouseAction ,keyState ,mouseButton ,points ,pos );
}

void GuiMenu::onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) {
    // navigate
}

//////////////////////////////////////////////////////////////////////////
//! GuiPopup

GuiPopup::GuiPopup( GuiControl *control )
{
    borderColor() = backgroundColor() = OS_COLOR_NONE;

    setControl( control );
}

void GuiPopup::setControl( GuiControl *control ) {
    removeAllControls();

    m_control = control;

    if( m_control ) {
        addControl( m_control.get() );
    }
}

void GuiPopup::Open() {
}

void GuiPopup::Close() {
    Post( GUI_MESSAGEID_CLOSED ,(int) this->id() );
}

void GuiPopup::onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) {
    if( mouseAction == osMouseButtonDown && (m_control->area() & pos[0]) == false ) {
        Close(); //! auto closes when clicking out of popup area
        return;
    }

    GuiGroup::onMouse( mouseAction ,keyState ,mouseButton ,points ,pos );
}

void GuiPopup::onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) {
    GuiGroup::onKey( keyAction ,keyState ,keyCode ,c );

    if( keyAction == osKeyDown && * (int*) &keyState == 0 && c == OS_KEYCODE_ESCAPE ) {
        Close();
    }
}

//////////////////////////////////////////////////////////////////////////
//! GuiTitlebar

GuiTitlebar::GuiTitlebar() {
    const char *properties =
        "align=top,vertical; coords={0,0,100%,34} controls={"
        "help:GuiButton = { commandId=31; align=left,horizontal; coords={0,0,34,34} text=?; }"
        "close:GuiButton = { commandId=19; align=right,horizontal; coords={0,0,34,34} text=X; }"
        "title:GuiLabel = { align=center; coords={0,0,80%,100%} textalign=center; text=titlebar; font=large; }"
        "}"
    ;

    setPropertiesWithString( properties );
}

void GuiTitlebar::Bind( IGuiCommandEvent &owner ) {
    getControl(0)->As_<GuiButton>()->Subscribe( owner );
    getControl(1)->As_<GuiButton>()->Subscribe( owner );
}

//////////////////////////////////////////////////////////////////////////
//! GuiDialog

GuiDialog::GuiDialog( const char *title ) {
    colors().fillColor = OS_COLOR_DARKGRAY;

    coords() = { 0 ,0 ,90.f ,90.f };
    align() = alignCenter;

    if( title ) {
        m_title.getControl("title")->As_<GuiLabel>()->text() = title;

        addControl( m_title );
    }
}

void GuiDialog::Open() {
    m_title.Bind( *this );
}

void GuiDialog::Close() {
    Post( GUI_MESSAGEID_CLOSED ,(int) this->id() );
}

void GuiDialog::onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) {
    GuiGroup::onKey( keyAction ,keyState ,keyCode ,c );

    if( keyAction == osKeyDown && * (int*) &keyState == 0 && c == OS_KEYCODE_ESCAPE ) {
        Close();
    }
}

void GuiDialog::onCommand( GuiControl &source ,uint32_t commandId ,long param ,Params *params ,void *extra ) {
    switch( commandId ) {
        case GUI_COMMANDID_CLOSE:
            Close(); break;
        case GUI_COMMANDID_HELP:
            Help(); break;
    }
}

//////////////////////////////////////////////////////////////////////////
//! GuiMessageBox

GuiMessageBox::GuiMessageBox( IGuiCommandEvent &listener ,const char *title ,const char *text ,const Params &options ) {
    setPropertiesWithString(
        "coords={25%,30%,75%,70%} "
        "controls = {"
            "title:GuiLabel = { coords={0,0,100%,34} align=top,centerh,vertical; textalign=center; background=#4040A0; }"
            "text:GuiLabel = { coords={0,0,100%,90%} align=center; textalign=center; }"
            "footer:GuiGroup = { coords={0,0,100%,10%} align=bottom,centerh,vertical; }"
        "}"
    );

    if( title )
        getControlAs_<GuiLabel>("title")->text() = title;

    if( text )
        getControlAs_<GuiLabel>("text")->text() = text;

    GuiGroup *footer = getControlAs_<GuiGroup>("footer");

    if( options.empty() ) {
        footer->setPropertiesWithString(
            "controls = {"
                "ok:GuiButton = { commandId=2; coords={0,0,18%,100%} align=right,centerv,horizontal; text=Ok; }"
            "}"
        );
    } else {
        for( auto &it : options ) {
            GuiButton *cmd = new GuiButton();

            cmd->setPropertiesWithString( "coords={0,0,18%,100%} align=right,centerv,horizontal;" );
            cmd->text() = it.first;
            fromString( cmd->commandId() ,it.second );

            cmd->Subscribe(*this);
            footer->addControl( *cmd );
        }
    }

    GuiCommandPublisher::Subscribe(listener);
}

void GuiMessageBox::Open() {
}

void GuiMessageBox::Close() {
    Post( GUI_MESSAGEID_CLOSED ,(int) this->id() );
}

void GuiMessageBox::onCommand( GuiControl &source ,uint32_t commandId ,long param ,Params *params ,void *extra ) {
    PostCommand( commandId ,param ,params ,extra );

    Close();
}

/* void GuiMessageBox::onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) {
    GuiGroup::onKey( keyAction ,keyState ,keyCode ,c );

    if( keyAction == osKeyDown && * (int*) &keyState == 0 && c == OS_KEYCODE_ESCAPE ) {
        Close();
    }
} */

//////////////////////////////////////////////////////////////////////////
//! GuiControlWindow

static bool g_controlWindowRegister = registerClassName_<GuiControlWindow>();

//--
GuiControlWindow::GuiControlWindow( const char_t *name ,const char_t *title ,int width ,int height ,int style ,int flags ,OsColorRef backgroundColor ) :
    GuiWindow( name ,title ,width ,height ,style ,flags ,backgroundColor )
    ,m_mouseCapture(NullPtr) ,m_mouseMoveDetect(false) ,m_mouseDragDetect(false)
    ,m_theme(NullPtr)
{
    m_background.coords() = { 0 ,0 ,100.f ,100.f };
    m_foreground.coords() = { 0 ,0 ,100.f ,100.f };
    m_layers.coords() = { 0 ,0 ,100.f ,100.f };

    m_layers.setRoot( *this );
    m_layers.addControl(m_background);
    m_layers.addControl(m_foreground);

    m_colors = theTheme().getColors( MyUUID ,"normal" );

    m_theme = &theTheme();
}

//-- dialog
void GuiControlWindow::ShowModal( GuiDialog &dialog ) {
    static uint32_t g_modalId = 0x0ff04a001;

    if( dialog.id() == CONTROLID_NONE ) {
        dialog.setControlId(g_modalId++);
    }

    dialog.setRoot( *this );
    dialog.Subscribe( *this );

    m_layers.addControl( dialog );
    dialog.Open(); //! IE event

    Update( nullptr ,refreshResized );
}

void GuiControlWindow::ShowPopup( GuiPopup &popup ) {
    // control.setRoot( *this );

    // GuiPopup *popup = new GuiPopup( control );

    popup.setRoot(*this);
    popup.Subscribe(*this);

    m_layers.addControl( popup );
    popup.Open();

    Update( nullptr ,refreshResized );
}

void GuiControlWindow::ShowMessageBox( GuiMessageBox &messageBox ) {
    messageBox.setRoot(*this);
    messageBox.GuiPublisher::Subscribe(*this);

    m_layers.addControl( messageBox );
    messageBox.Open();

    Update( nullptr ,refreshResized );
}

//-- mouse capture
void GuiControlWindow::MouseCapture( GuiControl &control ) {
    if( m_mouseCapture ) { assert(false); return; } //! should not happen

    m_mouseCapture = &control;

    GuiWindow::MouseCapture();
}

void GuiControlWindow::MouseRelease() {
    m_mouseCapture = NullPtr;

    GuiWindow::MouseRelease();
}

//-- drag & drop
CriticalSection GuiControlWindow::m_csDrag;
GuiControlRef GuiControlWindow::m_dragSource;
IObjectRef GuiControlWindow::m_dragObject;
DragOperation GuiControlWindow::m_dragOperation;

void GuiControlWindow::StartDragDrop( const OsPoint &p ,OsKeyState keyState ) { // ,GuiControl &source ) {
    CriticalSection::Guard guard(m_csDrag);

    if( m_dragOperation != dragOpNone ) return; //! drag already in progress

    DragOperation op = keyState.ctrl ? dragOpCopy : dragOpMove; //! default behavior, onGrab may modify

    m_dragObject = getMouseTopHit();
    m_dragSource = NullPtr;

    for( auto it = m_hitTracker.rbegin(); it!=m_hitTracker.rend(); ++it ) if( !it->isNull()  ) {
        if( (*it)->onGrab( p ,keyState ,op ,m_dragObject ) == IOK ) {
            m_dragSource = *it;
            break;
        }
    }

    if( !m_dragSource ) {
        m_dragOperation = dragOpNone;
        m_dragObject.Release(); //! no drag, making sure
        return;
    }

    m_dragOperation = op;

    if( m_dragOperation == dragOpLocal ) {
        MouseCapture( m_dragSource.get() );
    }
}

void GuiControlWindow::UpdateDragDrop( const OsPoint &p ,OsKeyState keyState ,GuiControl *target ) {
    CriticalSection::Guard guard(m_csDrag);

    OsError result = ENOERROR;

    if( target && !isDragOpLocal(m_dragOperation) ) {
        result = target->onDropAccept( p ,m_dragSource.As_<IObject>() ,m_dragOperation ,m_dragObject.ptr() ,true );
    }

    m_dragSource->onDrag( p ,m_dragOperation ,m_dragObject.ptr() ,target ,result );
}

void GuiControlWindow::EndDragDrop( const OsPoint &p ,OsKeyState keyState ,GuiControl *target ,bool cancel ) {
    CriticalSection::Guard guard(m_csDrag);

    OsError result = cancel ? ENODATA : ENOERROR;

    if( result == ENOERROR && target && !isDragOpLocal(m_dragOperation) ) {
        result = target->onDropAccept( p ,m_dragSource.As_<IObject>() ,m_dragOperation ,m_dragObject.ptr() ,false );
    }

    m_dragSource->onDrop( p ,m_dragOperation ,m_dragObject.ptr() ,target ,result );

    if( m_dragOperation == dragOpLocal ) {
        MouseRelease();
    }

    m_dragSource.Release();
    m_dragOperation = dragOpNone;
}

void GuiControlWindow::CancelDragDrop() {
    EndDragDrop( Point() ,_noKeyState ,NullPtr ,true );
}

//-- editor
void GuiControlWindow::EnableEditor( bool enabled ) {
    m_enableEditor = enabled;
}

void GuiControlWindow::StartEditor() {
    if( !m_enableEditor ) return;

    CEditorProperties::getInstance().Create();
    CEditorControls::getInstance().Create();

    m_editor = GuiControlRef( new CDesigner() );
    m_editor->setRoot( *this );
}

void GuiControlWindow::StopEditor() {
    CEditorProperties::getInstance().Destroy();

    m_editor.Release();
}

///--
void GuiControlWindow::onLayout( const OsRect &clientArea ,OsRect &placeArea ) {
    GuiWindow::onLayout( clientArea ,placeArea );

    m_layers.onLayout( clientArea ,placeArea );
}

void GuiControlWindow::onDraw( const struct OsRect &updateRect ) {
    SetForeColor( m_colors.foreColor );
    SetBackColor( m_colors.fillColor );

    // OsRect r = updateRect; r.right ++; r.bottom ++;

    DrawRectangle( updateRect );

//-- theme default colors
    OsGuiSetColors( *this ,m_colors );
    GuiWindow::SetFont( getDefaultFont() );

//--
    GuiWindow::onDraw( updateRect );

    m_layers.onDraw( updateRect );

    if( m_editor ) {
        m_editor->onDraw( updateRect );
    }
}

void GuiControlWindow::onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) {
    GuiWindow::onMouse( mouseAction ,keyState ,mouseButton ,points ,pos );

//-- drag & drop
    if( isDragDropInProgress() ) {
        if( mouseAction == osMouseMove ) {
            UpdateDragDrop( m_mouseClickPoint ,keyState ,getMouseTopHit() );
        } else if( mouseAction == osMouseButtonUp ) {
            EndDragDrop( pos[0] ,keyState ,getMouseTopHit() );

            // m_controlAtClick.Release(); // funct this
            m_mouseMoveDetect = false;
            m_mouseDragDetect = false;
        }
    }

//-- mouse capture
    if( m_mouseCapture ) {
        if( !isDragDropInProgress() )
            m_mouseCapture->onMouse( mouseAction ,keyState ,mouseButton ,points ,pos );

        return;
    }

//--
    //! @note if drag & drop in progress mouse was captured, so no event here

    // printf( "mouse %d (%d,%d : %d)\n" ,(int) mouseAction ,(int) pos[0].x ,(int) pos[0].y ,(int) mouseButton );

    if( mouseAction == osMouseMove && points && pos ) {
        HitTrackMouseLeave( pos[0] ,mouseButton ,keyState );
    }

//-- mouse event mapping
    if( points && pos && mouseButton && !isDragDropInProgress() ) {
        int motion = (Point(m_mouseClickPoint) - pos[0]).Abs().Sum();

        switch( mouseAction ) {
            case osMouseButtonDown: {
                // m_controlAtClick = m_editor.ptr() ? m_editor.ptr() : getMouseTopHit();
                m_mouseClickPoint = pos[0];
                m_mouseMoveDetect = false;
                m_mouseDragDetect = false;
                break;
            }

            case osMouseMove: {
                bool moveDetect = (motion > GUI_MOUSE_MOTION_DETECT);
                bool dragDetect = (motion > GUI_MOUSE_DRAG_DETECT);

                if( dragDetect && !m_mouseDragDetect && mouseButton == osLeftMouseButton ) { // && m_controlAtClick ) {
                    StartDragDrop( m_mouseClickPoint ,keyState ); // ,* m_controlAtClick.ptr() );
                }

                m_mouseMoveDetect |= (motion > GUI_MOUSE_MOTION_DETECT);
                m_mouseDragDetect |= (motion > GUI_MOUSE_DRAG_DETECT);
                break;
            }

            case osMouseButtonUp: {
                if( !m_mouseMoveDetect ) {
                    this->onMouse( osMouseClick ,keyState ,mouseButton ,points ,pos );
                }

                // m_controlAtClick.Release();
                m_mouseMoveDetect = false;
                m_mouseDragDetect = false;
                break;
            }

            default:
                break;
        }
    }

//-- editor
    if( m_editor ) {
        m_editor->onMouse( mouseAction ,keyState ,mouseButton ,points ,pos );

        if( mouseAction != osMouseMove ) return; //! @note need to let mouse move flow to update hit tracker
    }

//-- forward
    m_layers.onMouse( mouseAction ,keyState ,mouseButton ,points ,pos );
}

void GuiControlWindow::onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) {
    GuiWindow::onKey( keyAction ,keyState ,keyCode ,c );

    if( keyAction == osKeyDown && keyCode == OS_KEYCODE_F(2) ) {
        if( !m_editor )
            StartEditor();
        else
            StopEditor();
    }

    m_layers.onKey( keyAction ,keyState ,keyCode ,c );
}

void GuiControlWindow::onTimer( OsTimerAction timeAction ,OsEventTime now ,OsEventTime last ) {
    GuiWindow::onTimer( timeAction ,now ,last );

    m_layers.onTimer( timeAction ,now ,last );
}

void GuiControlWindow::onPost( IObject *source ,uint64_t msg ,long param ,Params *params ,void *extra ) {
    if( msg == GUI_MESSAGEID_CLOSED ) {
        GuiControl *p = source ? source->getInterface_<GuiControl>() : NullPtr;

        assert( p ); // && p->id() == topmost().id() );

        if( p ) m_layers.removeControl( *p );

        Update( nullptr ,refreshResized );
    }
}

///-- mouse tracking
GuiControl *GuiControlWindow::getMouseTopHit() {
    auto &hits = m_hitTracker;

    return !hits.empty() ? hits.rbegin()->ptr() : NullPtr;
}

void GuiControlWindow::HitTrackMouseEnter( GuiControl &control ,const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) {
    if( TestHit( p ,control.area() ) ) {
        //! @note perf iterate from back, use simple pointer
        for( auto it = m_hitTracker.rbegin(); it!=m_hitTracker.rend(); ++it ) if( it->ptr() == &control ) return;

        m_hitTracker.emplace_back( control );

        control.onMouseEnter( p ,mouseButton ,keyState );
    }
}

void GuiControlWindow::HitTrackMouseLeave( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) {
    for( auto it = m_hitTracker.begin(); it != m_hitTracker.end(); ) {
        if( !TestHit( p ,(*it)->area())) {
            (*it)->onMouseLeave( p ,mouseButton ,keyState );

            m_hitTracker.erase( it );
        } else
            ++it;
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! Assets

    //...

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! Theme

//TODO move with Highlights above
const char *highlightNames[highlightCount] = {
    "normal" ,"hoover" ,"pushed" ,"notpushed" ,"dragged" ,"disabled"
};

static String g_themeTiny =
"colors = { white=#C4C4C4; white1=#FAFAFA; gray0=#303030; gray=#808080; blue=#404080; blue2=#A0A0F0; blue1=#101080; blue0=#202040; green=#408040; black=#101010; black1=#161616; red=#804040; } "
    "fonts = { default=; }"
    "GuiControl = { normal={$black,$black,$white,0} disabled={$black,black,$gray,0} }"
    "GuiLink = { hoover={$black ,$black ,$blue2 ,$black} }"
    "GuiControlWindow = { normal={$black,$black,$white,$black} }"
    "GuiCheck = { outter={$white,$black,0,0} tick={$green,$black,0,0} cross={$red,$black,0,0} option={$white,$black,0,0} }"
    "GuiButton = {"
        "normal={$blue0,$blue,$white,$blue}"
        "hoover={$blue,$blue,$white1,$blue}"
        "pushed={$green,$green,$white1,$green}"
        "disabled={$gray0,$gray0,$gray,$gray0}"
    "}"
    "GuiTextBox = {"
        "normal={$gray,$black,$white,none} disabled={$gray,$black,$gray,none}"
    "}"
    "GuiGrid = {"
        "normal={none,$black,$white,none}"
        "title={$white,$blue1,$white1,none}"
        "col1={$gray,$black1,$white,none}"
        "col2={$gray,$black,$white,none}"
        "row1={$gray0,none,$white,none}"
        "row2={$gray0,none,$white,none}"
    "}"
;

static String g_themeEditor =
    " colors = { white=#C4C4C4; white1=#FAFAFA; gray=#808080; blue=#404080; blue1=#101080; blue0=#202040; green=#408040; black=#101010; red=#804040; } "
    " fonts = { default=; }"
    " GuiControl = { normal={$white ,$white ,$black ,#00000000 } }"
;

//--
namespace VisualThemeManifest {

    enum declKeyword {
        keywordNone=0 ,keywordColors ,keywordImages ,keywordFonts
    };

    enum declType {
        typeNone=0 ,typeControl ,typeProperties
    };

    enum propType {
        propNone=0 ,propColor ,propSize ,propProperty
    };
}

//--
template <> const char *Enum_<VisualThemeManifest::declKeyword>::names[] = {
    "", "colors" ,"images" ,"fonts" ,NullPtr
};

template <>
const VisualThemeManifest::declKeyword Enum_<VisualThemeManifest::declKeyword>::values[] = {
    VisualThemeManifest::keywordNone ,VisualThemeManifest::keywordColors ,VisualThemeManifest::keywordImages ,VisualThemeManifest::keywordFonts
};

//--
template <> const char *Enum_<VisualThemeManifest::declType>::names[] = {
    "" ,"control" ,"properties" ,NullPtr
};

template <>
const VisualThemeManifest::declType Enum_<VisualThemeManifest::declType>::values[] = {
    VisualThemeManifest::typeNone ,VisualThemeManifest::typeControl ,VisualThemeManifest::typeProperties
};

//--
template <> const char *Enum_<VisualThemeManifest::propType>::names[] = {
    "" ,"color" ,"size" ,"property" ,NullPtr
};

template <>
const VisualThemeManifest::propType Enum_<VisualThemeManifest::propType>::values[] = {
    VisualThemeManifest::propNone ,VisualThemeManifest::propColor ,VisualThemeManifest::propSize ,VisualThemeManifest::propProperty
};

//--
ThemeStore::ThemeStore() :
    m_default("tiny") ,m_current(&m_default)
{
//-- tiny (default)
    Params properties;

    fromString( properties ,g_themeTiny );
    fromManifest( m_default ,properties );

    m_themes.addItem( "tiny" ,&m_default );

//-- editor
    VisualTheme &editor = * new VisualTheme("tiny");

    properties.clear();

    fromString( properties ,g_themeEditor );
    fromManifest( editor ,properties );

    m_themes.addItem( "editor" ,&editor );
}

VisualTheme::Controls &VisualTheme::Controls::fromManifest( const Params &manifest ) {
    using namespace VisualThemeManifest;

    for( auto &it : manifest ) {
        NameType decl;

        fromString( decl ,it.first );

        propType type;

        getByName( decl.type.c_str() ,type );

        index_t index = theme.index( decl.name.c_str() );

        if( !type ) {
            //TODO auto detect

            type = propColor;
        }

        switch( type ) {
            default:
            case propColor: {
                ColorQuad quad;
                fromString( quad ,it.second );
                colors.addItem( index ,quad );
                break;
            }

            case propSize: {
                Point size;
                fromString( size ,it.second );
                sizes.addItem( index ,size );
                break;
            }

            case propProperty: {
                Params params;
                fromString( params ,it.second );
                properties.addItem( index ,params );
                break;
            }
        }
    }

    return *this;
};

VisualTheme &VisualTheme::fromManifest( const Params &manifest ) {
    using namespace VisualThemeManifest;

///-- colors
    Params defs;

    fromString( defs ,getMember( manifest ,"colors" ) );

    for( auto &it : defs ) {
        const char *name = it.first.c_str();

        auto i = index( name );

        ColorRef color;

        fromString( color ,it.second );

        m_palette.addItem( i ,color );
    }

    addMembers( m_vars ,defs ); //! colors are automatically added to vars
        //TODO function for this ?

///-- images
    defs.clear();

    fromString( defs ,getMember( manifest ,"images" ) );

    for( auto &it : defs ) {
        const char *name = it.first.c_str();

        GuiImageRef *image = Assets::Image().findItem( name );

        if( image && !image->isNull() ) {
            auto i = index( name );

            m_images[i] = image;
        }
    }

///-- fonts
    defs.clear();

    fromString( defs ,getMember( manifest ,"fonts" ) );

    for( auto &it : defs ) {
        const char *name = it.first.c_str();

        GuiFontRef *font = Assets::Font().findItem( name );

        if( font && !font->isNull() ) {
            auto i = index( name );

            m_fonts[i] = font;
        }
    }

///-- per control / properties
    UUID classId = 0;

    for( auto &it : manifest ) if( !getByName<declKeyword>( it.first.c_str() ) ) {
        NameType decl;

        fromString( decl ,it.first );

        declType type;

        getByName( decl.type.c_str() ,type );

        String value;

        replaceTextVariables( it.second.c_str() ,m_vars ,value );

        if( !type || type == typeControl ) {
            type = getClassIdFromName( it.first.c_str() ,classId ) ? typeControl : typeProperties;
        }

        if( type == typeControl ) {
            //! control

            if( classId == 0 ) { //! not found
                //TODO log error
                continue;
            }

            Params properties;

            fromString( properties ,value);

            if( classId == GuiControl::classId() ) { //! default
                ::TINY_NAMESPACE_NAME::fromString( m_defaultColors ,getMember( properties ,"normal" ) );
                continue;
            }

            auto *p = new Controls(*this);

            m_controls.addItem( classId ,p );

            p->fromManifest( properties );

        } else {
            //! properties
            auto i = index( it.first.c_str() );

            m_properties.addItem( i ,value );
        }
    }

    return *this;
}

void VisualTheme::makeColorSet( const UUID &id ,ColorSet &colorSet ) {
    for( int i=0; i<highlightCount; ++i ) {
        colorSet.getColors(i) = this->getColors( id ,highlightNames[i] );
    }
}

bool ThemeStore::addThemeFromManifest( const Params &params ) {

    for( auto &it : params ) {
        const char *name = it.first.c_str();

        auto *p = new VisualTheme( name );

        Params properties;

        fromString( properties ,it.second );

        fromManifest( *p ,properties );

        m_themes.addItem( p->name() ,p );
    }

    return true;
}

bool ThemeStore::addThemeFromManifest( const char *properties ) {
    Params params;

    fromString( params ,properties );

    return addThemeFromManifest( params );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! Editor

void CDesigner::onClick( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) {
    GuiControl::onClick( p ,mouseButton ,keyState );

    GuiControl *control = root().getMouseTopHit();

    // if( !control ) return;

    CEditorProperties::getInstance().attachControl( control );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////
#endif //TINY_NO_XGUI