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

#ifndef TINY_GUI_GUI_H
#define TINY_GUI_GUI_H

//////////////////////////////////////////////////////////////////////////////
//! @file "x-gui/gui.h"
//! @brief tiny-for-c++ GUI extension
//! @author the NEXTWave developers
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
#ifndef TINY_NAMESPACE_GUI_NAME
 #define TINY_NAMESPACE_GUI_NAME gui
#endif

#define TINY_NAMESPACE_GUI \
    namespace TINY_NAMESPACE_GUI_NAME

//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

    TINY_NAMESPACE_GUI {}

    using TINY_NAMESPACE_GUI;
    //! TODO (maybe later) namespace gui + rename all subclass

//////////////////////////////////////////////////////////////////////////////
//! Interface

#define TINY_IGUICONTROLEVENTS_PUID   0x0c69e82d1b186ca9d
#define TINY_IGUIMESSAGEEVENTS_PUID   0x07f584175924526c3
#define TINY_IGUIPROPERTIES_PUID      0x08fe381ed09c02626

struct IGuiControlEvents;
struct IGuiMessageEvents;
struct IGuiProperties;

//////////////////////////////////////////////////////////////////////////////
//! Struct

#define TINY_GUIALIGN_PUID          0x084f607186f639ada

//////////////////////////////////////////////////////////////////////////////
//! Class

#define TINY_GUICONTROL_PUID          0x001d19d4782da5133
#define TINY_GUISET_PUID              0x09eaa23e28319942a
#define TINY_GUILAYER_PUID            0x02a2268cb68b22621
#define TINY_GUITAB_PUID              0x0ce669856891313b6
#define TINY_GUIGROUP_PUID            0x03738884453df6435
#define TINY_GUITABBAR_PUID           0x0f6524432b75a5ed1
#define TINY_GUIMENU_PUID             0x05a27c3b2b902d95f
#define TINY_GUICONTROLWINDOW_PUID    0x0911441da7a69fe53

class GuiControl;
class GuiSet;
class GuiLayer;
class GuiTab;
class GuiGroup;
class GuiTabBar;
class GuiMenu;
class GuiControlWindow;

namespace gui {
    struct VisualTheme;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Definitions

//////////////////////////////////////////////////////////////////////////////
//! Coords

struct GuiCoord {
    enum Unit {
        unitPixel ,unitPercent
    };

    float value;
    Unit unit;

//--
    GuiCoord( int v=0 ) : value( (float) v ) ,unit(unitPixel) {}
    GuiCoord( float v ) : value( v ) ,unit(unitPercent) {}

//--
    template <typename T>
    int get( T ref ) const {
        return (int) ( (unit==unitPercent) ? ((value / 100.f) * (float) ref) : value );
    }
};

DEFINE_STRING_API(GuiCoord);

//--
struct GuiCoords {
    GuiCoord left ,top ,right ,bottom;

    GuiCoords &setOsRect( const OsRect &area ) {
        left = area.left; top = area.top; right = area.right; bottom = area.bottom;
        return *this;
    }

    OsRect getOsRect( const OsRect &area ) const {
        int w = area.right - area.left;
        int h = area.bottom - area.top;
        int x = area.left;
        int y = area.top;

        return OsRect( {
            left.get(w)+x ,top.get(h)+y ,right.get(w)+x ,bottom.get(h)+y
        } );
    }
};

DEFINE_STRING_API(GuiCoords);

//////////////////////////////////////////////////////////////////////////////
//! Orientation

enum Orientation {
    orientNone=0
    ,orientHorizontal=1
    ,orientVertical=2 ,orientBoth2=(orientHorizontal|orientVertical)
    ,orientDepth=4 ,orientBoth3=(orientBoth2|orientDepth)
    ,orientTime=8 ,orientBoth4=(orientBoth3|orientTime)
        //! etc... @note all values reserved
};

//TODO to/from String

//////////////////////////////////////////////////////////////////////////////
//! Direction

enum Direction {
    directionNone=0
    ,directionLeft=1 ,directionRight=2 ,directionHorizontal=(directionLeft|directionRight)
    ,directionBottom=4 ,directionTop=8 ,directionVertical=(directionTop|directionBottom)
    ,directionAll2=(directionHorizontal|directionVertical)
    ,directionBack=16 ,directionFront=32 ,directionDepth=(directionBack|directionFront)
    ,directionAll3=(directionAll2|directionDepth)
    ,directionBefore=64 ,directionAfter=128 ,directionTime=(directionBefore|directionAfter)
        //! etc... @note all values reserved
};

inline bool goesLeft( Direction d ) { return (d & directionLeft) != 0; }
inline bool goesRight( Direction d ) { return (d & directionRight) != 0; }
inline bool goesTop( Direction d ) { return (d & directionTop) != 0; }
inline bool goesBottom( Direction d ) { return (d & directionBottom) != 0; }

inline Direction getDimension( int i ,Direction d ) { return (Direction) ((d >> (2*i)) & 3); }
inline int getDimSign( int i ,Direction d ) { Direction d0 = getDimension(i,d); return (d0 & 1) != 0 ? -1 : (d0 &2) ? 1 : 0; }

//--

//TODO to/from String

//////////////////////////////////////////////////////////////////////////////
//! Align

enum GuiAlign {
    noAlign=0
    ,alignLeft=1 ,alignRight=2 ,alignCenterH=4
    ,alignTop=8 ,alignBottom=16 ,alignCenterV=32
    ,alignAnchorH=64 ,alignAnchorV=128

    ,alignFillH = (alignLeft | alignRight)
    ,alignFillV = (alignTop | alignBottom)
    ,alignFill = (alignFillH | alignFillV)
    ,alignCenter = (alignCenterH | alignCenterV)
    ,alignAnchor = (alignAnchorH | alignAnchorV)
};

DEFINE_STRING_API(GuiAlign);
DECLARE_STRUCT(GuiAlign,TINY_GUIALIGN_PUID);

///--
void Emplace( GuiAlign align ,int &a ,int &b ,int &A ,int &B );

OsRect Emplace( GuiAlign align ,const GuiCoords &coords ,const OsRect &clientArea ,OsRect &area );

///--
typedef Params GuiProperties;

//////////////////////////////////////////////////////////////////////////////
//! ColorPair

struct ColorPair {
    OsColorRef foreColor;
    OsColorRef fillColor;
};

//-- string
template <>
ColorPair &fromString( ColorPair &p ,const String &s ,size_t &size );

//TODO to String

//-- manifest
template <>
ColorPair &fromManifest( ColorPair &p ,const Params &manifest );

//TODO to Manifest

//-- helpers
void OsGuiSetDrawColors( IGuiDisplay &display ,const ColorPair &colors );
void OsGuiSetTextColors( IGuiDisplay &display ,const ColorPair &colors );

//////////////////////////////////////////////////////////////////////////////
//! ColorQuad

struct ColorQuad {
    OsColorRef foreColor; //! foreground color
    OsColorRef fillColor; //! background color
    OsColorRef textColor; //! text color
    OsColorRef backColor; //! text back color
};

ColorQuad &setDrawColors( ColorQuad &q ,const ColorPair &p );
ColorPair &getDrawColors( const ColorQuad &q ,ColorPair &p );
ColorPair getDrawColors( const ColorQuad &q );

ColorQuad &setTextColors( ColorQuad &q ,const ColorPair &p );
ColorPair &getTextColors( const ColorQuad &q ,ColorPair &p );
ColorPair getTextColors( const ColorQuad &q );

//-- manifest
template <>
ColorQuad &fromManifest( ColorQuad &p ,const Params &manifest );

//TODO  toString below

//-- operators
inline bool operator ==( const ColorQuad &a ,const ColorQuad &b ) {
    return
        a.foreColor == b.foreColor && a.fillColor == b.fillColor
        && a.textColor == b.textColor && a.backColor == b.backColor
    ;
}

inline bool operator !=( const ColorQuad &a ,const ColorQuad &b ) {
    return !operator ==( a ,b );
}

//-- helpers
void OsGuiSetColors( IGuiDisplay &display ,const ColorQuad &colors ,bool setTextColors=true );

//////////////////////////////////////////////////////////////////////////////
//! Highlight

#define TINY_GUI_HIGHLIGHTS     ((int) highlightCount)

enum Highlight { //TODO state ?
    highlightNormal=0 ,highlightHoover=1 ,highlightPushed=2 ,highlightNotPushed=3
    ,highlightDragged=4 ,highlightDisabled=5
    ,highlightMax=5 //! Max Value
    ,highlightCount=(highlightMax+1) //! Count of values
};

///--
class ColorSet {
public:
    const ColorQuad &getColors( int index=highlightNormal ) const {
        return m_colors[ CLAMP(index,highlightNormal,highlightMax) ];
    }

    ColorQuad &getColors( int index=highlightNormal ) {
        return m_colors[ CLAMP(index,highlightNormal,highlightMax) ];
    }

    bool hasColors( int index=highlightNormal ) {
        return index == 0 || getColors(0) != getColors(index);
    }

protected:
    ColorQuad m_colors[TINY_GUI_HIGHLIGHTS]; //! the table of colors for each highlight state
};

template <>
ColorSet &fromManifest( ColorSet &p ,const Params &manifest );

//TODO toString

//-- helpers
inline void OsGuiSetColors( IGuiDisplay &display ,const ColorSet &colorset ,int index ,bool setTextColors=true ) {
    OsGuiSetColors( display ,colorset.getColors(index) ,setTextColors );
}

//////////////////////////////////////////////////////////////////////////////
//! Font

const GuiFont &getDefaultFont();

//////////////////////////////////////////////////////////////////////////////
//! Shortcuts

struct GuiShortcut {
    OsKeyState keyState;
    OsKeyCode keyCode;
    char_t c;
};

DEFINE_STRING_API(GuiShortcut);

bool MatchKey( GuiShortcut &shortcut ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c );

//////////////////////////////////////////////////////////////////////////////
//! Drag

enum DragOperation {
    dragOpNone=0
    ,dragOpMove=1 //! std move operation
    ,dragOpCopy=2 //! std copy operation
    ,dragOpLocal=16 //! local (to the source control) grab operation
        //! will lookup targets or trigger onDropAccept
        //! this is a flag, may be use in conjunction with std or user defined operation
    ,dragOpUser=32 //! user defined value including and above
};

inline bool isDragOpLocal( DragOperation op ) { return (op & dragOpLocal) != 0; }
inline DragOperation setDragOpUser( DragOperation op ,int i ) { return (DragOperation) (op + (i << 5)); }
inline int getDragOpUser( DragOperation op ) { return (op >> 5); }

//////////////////////////////////////////////////////////////////////////////
//! Messaging

///-- message class
#define TINY_MESSAGE_COMMAND        0x01
#define TINY_MESSAGE_NOTIFY         0x02
#define TINY_MESSAGE_DATA           0x03
#define TINY_MESSAGE_ASSETS         0x04
#define TINY_MESSAGE_THEME          0x05

//--
#define GUI_COMMAND(__id)           TINY_MESSAGE(TINY_MESSAGE_COMMAND,__id)
#define GUI_ISCOMMAND(__msg)        TINY_ISMESSAGE(TINY_MESSAGE_COMMAND,__msg)
#define GUI_GETCOMMAND(__msg)       TINY_GETMESSAGE(TINY_MESSAGE_COMMAND,__msg)

#define GUI_NOTIFY(__id)            TINY_MESSAGE(TINY_MESSAGE_NOTIFY,__id)
#define GUI_ISNOTIFY(__msg)         TINY_ISMESSAGE(TINY_MESSAGE_NOTIFY,__msg)
#define GUI_GETNOTIFY(__msg)        TINY_GETMESSAGE(TINY_MESSAGE_NOTIFY,__msg)

#define GUI_DATAMEDSSAGE(__id)      TINY_MESSAGE(TINY_MESSAGE_DATA,__id)
#define GUI_ISDATAMESSAGE(__msg)    TINY_ISMESSAGE(TINY_MESSAGE_DATA,__msg)
#define GUI_GETDATAMESSAGE(__msg)   TINY_GETMESSAGE(TINY_MESSAGE_DATA,__msg)

///-- message id
#define GUI_MESSAGEID_NONE          0       //! no/invalid message id
#define GUI_MESSAGEID_ACTION        1       //! generic message, call to action
#define GUI_MESSAGEID_OK            2
#define GUI_MESSAGEID_CANCEL        3
#define GUI_MESSAGEID_OPEN          16
#define GUI_MESSAGEID_UPDATE        17
#define GUI_MESSAGEID_REFRESH       18
#define GUI_MESSAGEID_CLOSE         19
#define GUI_MESSAGEID_START         20
#define GUI_MESSAGEID_STOP          21
#define GUI_MESSAGEID_PREV          22
#define GUI_MESSAGEID_NEXT          23
#define GUI_MESSAGEID_FIRST         24
#define GUI_MESSAGEID_LAST          25
#define GUI_MESSAGEID_MOVE          27
#define GUI_MESSAGEID_ADD           28
#define GUI_MESSAGEID_EDIT          29
#define GUI_MESSAGEID_DELETE        30
#define GUI_MESSAGEID_HELP          31
#define GUI_MESSAGEID_ENTER         32
#define GUI_MESSAGEID_LEAVE         33

#define GUI_COMMANDID_MENU          5001 //! @note base command id for menu command (default, if none provided)
#define GUI_COMMANDID_MENUMAX       5499 //! @note maximum command id for default menu command


/*
#define GUI_MESSAGEID_ACTION        1   //! generic message, call to action // ought to be dispatched to 'onCommand'
#define GUI_MESSAGEID_ASSETS        2   //! assets has changed, object should update accordingly
#define GUI_MESSAGEID_THEME         3   //! theme changed
#define GUI_MESSAGEID_PROPS         4   //! properties changed
#define GUI_MESSAGEID_OPENED        8   //! child element opened
#define GUI_MESSAGEID_CLOSED        9   //! child element closed
*/

//--
class GuiPublisher : public CPublisher_<IGuiMessage> ,IOBJECT_PARENT {
public:
    void Post( message_t msg ,long param ,Params *params=NullPtr ,void *extra=NullPtr ) {
        for( auto &it : m_subscribers ) {
            it->onPost( this ,msg ,param ,params ,extra );
        }
    }

    void PostCommand( messageid_t commandId ,long param ,Params *params=NullPtr ,void *extra=NullPtr ) {
        Post( GUI_COMMAND(commandId) ,param ,params ,extra );
    }

    void PostNotify( messageid_t commandId ,long param ,Params *params=NullPtr ,void *extra=NullPtr ) {
        Post( GUI_NOTIFY(commandId) ,param ,params ,extra );
    }
};

//////////////////////////////////////////////////////////////////////////////
//! Interface

struct IGuiControlEvents : IOBJECT_PARENT {
    DECLARE_CLASSID(TINY_IGUICONTROLEVENTS_PUID);

    ///-- mouse events
    virtual void onClick( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) = 0;
    virtual void onDoubleClick( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) = 0;
    virtual void onMouseDown( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) = 0;
    virtual void onMouseUp( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) = 0;

    virtual void onMouseMove( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) = 0;
    virtual void onMouseEnter( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) = 0;
    virtual void onMouseLeave( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) = 0;

    ///-- drag & drop
    virtual OsError onGrab( const OsPoint &p ,OsKeyState keyState ,DragOperation &operation ,IObjectRef &object ) = 0;
    virtual void onDrag( const OsPoint &p ,DragOperation operation ,IObject *object ,IObject *target ,OsError result ) = 0;
    virtual void onDrop( const OsPoint &p ,DragOperation operation ,IObject *object ,IObject *target ,OsError result ) = 0;
    virtual OsError onDropAccept( const OsPoint &p ,IObject *source ,DragOperation operation ,IObject *object ,bool preview ) = 0;

    ///-- focus events
    virtual void onGotFocus() = 0;
    virtual void onLostFocus() = 0;

    ///-- key events
    virtual void onKeyChar( OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) = 0;
    virtual void onKeyDown( OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) = 0;
    virtual void onKeyUp( OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) = 0;
};

//! @note separable interface

struct IGuiMessageEvents : IOBJECT_PARENT {
    DECLARE_CLASSID(TINY_IGUIMESSAGEEVENTS_PUID);

    virtual void onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) = 0;
    virtual void onNotify( IObject *source ,messageid_t notifyId ,long param ,Params *params ,void *extra ) = 0;
};

struct IGuiProperties : IOBJECT_PARENT {
    DECLARE_CLASSID(TINY_IGUIPROPERTIES_PUID);

    virtual void getProperties( Params &params ) const = 0;
    virtual void setProperties( const Params &properties ) = 0;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Controls

#define GUICONTROL_PARENT   public virtual GuiControl

#define CONTROLID_NONE      0
#define CONTROLID_AUTOID    1001 //! first id from auto id

typedef uint32_t controlid_t;

class GuiControl : public IGuiEvents ,public IGuiControlEvents ,public IGuiMessageEvents ,public IGuiProperties ,COBJECT_PARENT {
public: ///-- instance
    GuiControl( GuiControlWindow *root=NullPtr );
        //TODO all controls to have this signature

    DECLARE_OBJECT(GuiControl,TINY_GUICONTROL_PUID);

public: ///-- getter/setter
    void setControlId( controlid_t id ) { m_id = id; } //TODO make sure id is unique ?
    NoDiscard controlid_t id() const { return m_id; } //TODO rename to controlId() ?

    GuiCoords &coords() { return m_coords; }
    GuiAlign &align() { return m_align; }
    ColorQuad &colors() { return m_colors; }

    OsColorRef &borderColor() { return colors().foreColor; }
    OsColorRef &backgroundColor() { return colors().fillColor; }
    OsColorRef &textColor() { return colors().textColor; }

    //TODO these to bitfield
    bool &visible() { return m_visible; }
    bool &enabled() { return m_enabled; }
    //TODO + editable, grabable

    const Point &size() const { return m_size; }
    Point &size() { return m_size; }

    const Rect &area() const{ return m_area; }
    Rect &area() { return m_area; }

    const GuiControlWindow &root() const { return *m_root; }
    GuiControlWindow &root() { return *m_root; }
    void setRoot( GuiControlWindow &root ) { m_root = &root; }
    bool isOrphan() const { return !m_root; }

    bool shouldDraw( const OsRect &updateArea ) const;
    void getCenteredArea( const OsPoint &dims ,Rect &r ) const;

public: ///-- IProperties
    API_IMPL(void) getProperties( Params &properties ) const IOVERRIDE;
    API_IMPL(void) setProperties( const Params &properties ) IOVERRIDE;

    void setPropertiesWithString( const char *properties );
    void setPropertiesWithString( const char *properties ,const Params &vars );

    bool loadProperties( const char *filename ,const char *path=NullPtr );
    bool saveProperties( const char *filename ,const char *path=NullPtr ); //? path ?

public: ///-- IGuiContext (replication, not inherited)
    void SetForeColor( OsColorRef c );
    void SetFillColor( OsColorRef c );
    void SetTextColor( OsColorRef c );
    void SetBackColor( OsColorRef c );
    void SetDrawColors( const ColorPair &p );
    void SetTextColors( const ColorPair &p );
    void SetColors( const ColorQuad &q );

    void SetFont( const GuiFont &font );
    void RegionSetArea( OsRect &r );
    // void RegionGetArea( OsRect &r ); //! @note coords in this api are transformed, ? how proper transform area here ?
    void RegionSetOffset( int x ,int y );
    void RegionSetScale( float x ,float y );

public: ///-- IGuiSurface (replication, not inherited)
    void DrawRectangle( const OsRect &r );
    void DrawEllipse( const OsRect &r );
    void DrawLine( const OsRect &r );
    void DrawTextAlign( const char_t *text ,const OsRect &r ,TextAlign align=textalignNormal ,OsRect *rect=NULL );
    void DrawPolygon( int npoints ,const OsPoint *points );
    void DrawImage( const GuiImage &image ,const OsRect &r ,const OsRect &imgRect );
        //! NB use OS_IMAGE_USEDIM as right/bottom value to use source image size

public: ///-- IDisplay (replication, not inherited)
    void SetCursor( int cursorId );

    void Refresh( bool noArea=false ,RefreshFlags flags=refreshNormal );
    void Update( bool noArea=false ,RefreshFlags flags=refreshNormal );

public: ///-- IGuiControlEvents

//-- mouse events
    API_IMPL(void) onClick( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) IOVERRIDE {}
    API_IMPL(void) onDoubleClick( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) IOVERRIDE {}
    API_IMPL(void) onMouseDown( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) IOVERRIDE {}
    API_IMPL(void) onMouseUp( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) IOVERRIDE {}

    API_IMPL(void) onMouseMove( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) IOVERRIDE {}
    API_IMPL(void) onMouseEnter( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) IOVERRIDE {}
    API_IMPL(void) onMouseLeave( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) IOVERRIDE {}

    API_IMPL(OsError) onGrab( const OsPoint &p ,OsKeyState keyState ,DragOperation &operation ,IObjectRef &object ) IOVERRIDE { return ENOEXEC; }
    API_IMPL(void) onDrag( const OsPoint &p ,DragOperation operation ,IObject *object ,IObject *target ,OsError result ) IOVERRIDE {}
    API_IMPL(void) onDrop( const OsPoint &p ,DragOperation operation ,IObject *object ,IObject *target ,OsError result ) IOVERRIDE {}
    API_IMPL(OsError) onDropAccept( const OsPoint &p ,IObject *source ,DragOperation operation ,IObject *object ,bool preview ) IOVERRIDE { return ENOEXEC; }

//-- focus events
    API_IMPL(void) onGotFocus() IOVERRIDE {}
    API_IMPL(void) onLostFocus() IOVERRIDE {}

//-- key events
    API_IMPL(void) onKeyChar( OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) IOVERRIDE {}
    API_IMPL(void) onKeyDown( OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) IOVERRIDE {}
    API_IMPL(void) onKeyUp( OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) IOVERRIDE {}

//-- messaging
    API_IMPL(void) onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) IOVERRIDE {}
    API_IMPL(void) onNotify( IObject *source ,messageid_t notifyId ,long param ,Params *params ,void *extra ) IOVERRIDE {}

public: ///-- IGuiEvents
    API_IMPL(void) onLayout( const OsRect &clientArea ,OsRect &placeArea ) IOVERRIDE;
    API_IMPL(void) onDraw( const OsRect &uptadeArea ) IOVERRIDE;
    API_IMPL(void) onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) IOVERRIDE;
    API_IMPL(void) onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) IOVERRIDE;
    API_IMPL(void) onTimer( OsTimerAction timeAction ,OsEventTime now ,OsEventTime last ) IOVERRIDE;
    API_IMPL(void) onPost( IObject *source ,uint64_t msg ,long param ,Params *params ,void *extra ) IOVERRIDE;

protected:
    uint32_t m_id = CONTROLID_NONE;
    GuiControlWindow *m_root; //! root/parent window (ptr, not ref, by design)

    GuiCoords m_coords = { 0,0,100.f,100.f };
    GuiAlign m_align = GuiAlign::noAlign;
    ColorQuad m_colors;

    bool m_visible = true;
    bool m_enabled = true;

    Point m_size; //! natural dimension of the control
    Rect m_area; //! area of the control as calculated by layout for the display
};

typedef RefOf<GuiControl> GuiControlRef;

//-- Declaration
#define DECLARE_GUICONTROL(__super,__class,__uid) \
    DECLARE_OBJECT_STD(__super,__class,__uid); \
    DECLARE_FACTORY_STD(GuiControl,__class);

#define DECLARE_GUIPROPERTIES \
    API_IMPL(void) getProperties( Params &properties ) const IOVERRIDE; \
    API_IMPL(void) setProperties( const Params &properties ) IOVERRIDE;

//-- String
    //TODO from/to String // serialize

//-- Manifest
template <>
inline GuiControl &fromManifest( GuiControl &p ,const Params &manifest ) {
    p.setProperties( manifest ); return p;
}

template <>
inline Params &toManifest( const GuiControl &p ,Params &manifest ) {
    p.getProperties( manifest ); return manifest;
}

//-- Factory
inline GuiControl *ICreateGuiControl( const char *name ) {
    return ICreateObject_<GuiControl>( name );
}

//////////////////////////////////////////////////////////////////////////////
//! Command control helper

class GuiCommandOnClick : public GuiPublisher ,GUICONTROL_PARENT {
public:
    GuiCommandOnClick() : m_commandId(0) ,m_commandParam(0) {}

    DECLARE_GUIPROPERTIES;

    messageid_t &commandId() { return m_commandId; }

    messageid_t getCommandId() const { return m_commandId; }
    void setCommandId( messageid_t id ) { m_commandId = id; }

public:
    API_IMPL(void) onClick( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) IOVERRIDE {
        PostCommand();
    }

    virtual void PostCommand() {
        GuiPublisher::PostCommand( m_commandId ,m_commandParam );
    }

protected:
    messageid_t m_commandId;
    long m_commandParam;
};

//////////////////////////////////////////////////////////////////////////////
//! PropertiesDataSource

    //! @brief Properties as a data source

class PropertiesDataSource : public CDataSource {
public:
    void BindProperties( IGuiProperties *properties ) {
        m_properties = properties;
    }

public: ///-- IDataSource

//-- control
    IAPI_IMPL Commit() IOVERRIDE;
    IAPI_IMPL Discard() IOVERRIDE;

//-- client
    IAPI_IMPL readHeader( Params &data ,bool requireValues=false ) IOVERRIDE;
    IAPI_IMPL readData( Params &data ) IOVERRIDE;
    IAPI_IMPL onDataEdit( Params &data ) IOVERRIDE;

protected:
    RefOf<IGuiProperties> m_properties;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! composites

//////////////////////////////////////////////////////////////////////////////
//! GuiSet

    //! @brief a variable size set of controls, foundation for the different geometries of group

class GuiSet : GUICONTROL_PARENT {
public:
    typedef ListOf<GuiControlRef> controlset_t;

protected:
    controlset_t m_controls;

    Map_<String,int> m_names;

public:
    DECLARE_OBJECT_STD(GuiControl,GuiSet,TINY_GUISET_PUID);
    DECLARE_GUIPROPERTIES;

    //! @note prefer using interface below instead of directly use the control set
    const controlset_t &controls() const { return m_controls; }
    controlset_t &controls() { return m_controls; }

    int index( int i ) const {
        return CLAMP( i ,0 ,(int) m_controls.size()-1 );
    }

public:
    size_t getControlCount() const { return m_controls.size(); }

    bool hasControl() const {
        return !m_controls.empty();
    }

    bool hasControl( int i ) const {
        return ( i >= 0 && i < (int) m_controls.size() );
    }

    GuiControl *getControl() { //! topmost
        int n = (int) getControlCount(); return n > 0 ? getControl(n-1) : nullptr;
    }

    const GuiControl *getControl() const { //! topmost
        int n = (int) getControlCount(); return n > 0 ? getControl(n-1) : nullptr;
    }

    GuiControl *getControl( int i ) { //! by index (not by controlId)
        return hasControl(i) ? m_controls.at( (size_t) index(i) ).ptr() : NullPtr;
    }

    const GuiControl *getControl( int i ) const { //! by index (not by controlId)
        return hasControl(i) ? m_controls.at( (size_t) index(i) ).ptr() : NullPtr;
    }

    GuiControl *getControl( const char *name ) {
        return findControlByName(name);
    }

    template <typename T>
    T *getControlAs_( const char *name ) {
        GuiControl *control = getControl( name );

        return control ? control->As_<T>() : NullPtr;
    }

    bool setControl( int i ,GuiControl &control );

    int addControl( GuiControl &control );
    int addControl( const char *name ,GuiControl &control );

    bool removeControl( GuiControl &control );
    bool removeControl( const char *name );
    void removeControl( int i );
    void removeControl(); //! topmost

    void removeAllControls();

//--
    int findControlIndex( GuiControl &control );
    GuiControl *findControlById( controlid_t id );
    GuiControl *findControlByName( const char *name );

    template <typename T>
    T *findControlByIdAs_( controlid_t id ) {
        GuiControl *control = findControlById(id);

        return control ? control->As_<T>() : NullPtr;
    }

    const char *digControlName( int i );

    const char *digControlName( GuiControl &control ) {
        return digControlName( findControlIndex(control) );
    }

public:
    OsError onDropAccept( const OsPoint &p ,IObject *source ,DragOperation operation ,IObject *object ,bool preview ) override;

public:
    void onLayout( const OsRect &clientArea ,OsRect &placeArea ) override;

    //! @note timer event are propagated independently of composition or state
    void onTimer( OsTimerAction timeAction ,OsEventTime now ,OsEventTime last ) override;
};

//////////////////////////////////////////////////////////////////////////////
//! GuiArray_

    //! @brief fixed size array of control behaving as a non final group (e.g. for small composite control...)

template <int TCount>
class GuiArray_ : GUICONTROL_PARENT {
public:
    static size_t getControlCount() { return (size_t) TCount; }

    GuiArray_() {
        memset( (byte*) m_controls ,0 ,sizeof(GuiControl*) * TCount );
    }

    // DECLARE_GUIPROPERTIES; //! not declared by design, its a template

public:
    API_IMPL(void) onLayout( const OsRect &clientArea ,OsRect &placeArea ) IOVERRIDE {
        GuiControl::onLayout( clientArea ,placeArea );

        Rect r = area(); //! sub area

        for( int i=0; i<TCount; ++i ) if( m_controls[i] ) {
            m_controls[i]->setRoot( root() );
            m_controls[i]->onLayout( area() ,r );
        }
    }

    API_IMPL(void) onDraw( const OsRect &updateArea ) IOVERRIDE {
        GuiControl::onDraw( updateArea );

        if( !visible() ) return;

        for( int i=0; i<TCount; ++i ) {
            SAFECALL(m_controls[i])->onDraw( updateArea );
        }
    }

    API_IMPL(void) onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) IOVERRIDE {
        GuiControl::onMouse( mouseAction ,keyState ,mouseButton ,points ,pos );

        for( int i=TCount-1; i>=0; --i ) {
            GuiControl *control = m_controls[i];

            if( control && control->visible() && (control->area() & pos[0]) ) {} else continue;

            if( control->enabled() )
                control->onMouse( mouseAction ,keyState ,mouseButton ,points ,pos );

            break;
        }
    }

protected:
    GuiControl *m_controls[TCount];
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Constructs

//////////////////////////////////////////////////////////////////////////////
class GuiLayer : public GuiSet {
public:
    GuiLayer();

    DECLARE_GUICONTROL(GuiSet,GuiLayer,TINY_GUILAYER_PUID);

public:
    API_IMPL(void) onLayout( const OsRect &clientArea ,OsRect &placeArea ) IOVERRIDE;
    API_IMPL(void) onDraw( const OsRect &updateArea ) IOVERRIDE;
    API_IMPL(void) onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) IOVERRIDE;
    API_IMPL(void) onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) IOVERRIDE;
};

//////////////////////////////////////////////////////////////////////////////
class GuiTab : public GuiSet {
public:
    GuiTab();

    DECLARE_GUICONTROL(GuiSet,GuiTab,TINY_GUITAB_PUID);

    int getTabCount() const {
        return (int) getControlCount();
    }

    int getCurrentTabIndex() const {
        return m_tab;
    }

    GuiControl *getCurrentTab() {
        return getControl(m_tab);
    }

    void selectTab( int at );

    void prevTab() {
        selectTab(getCurrentTabIndex()-1);
    }

    void nextTab() {
        selectTab(getCurrentTabIndex()+1);
    }

public:
    API_IMPL(void) onLayout( const OsRect &clientArea ,OsRect &placeArea ) IOVERRIDE;
    API_IMPL(void) onDraw( const OsRect &updateArea ) IOVERRIDE;
    API_IMPL(void) onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) IOVERRIDE;
    API_IMPL(void) onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) IOVERRIDE;

protected:
    int m_tab; //! currently selected tab
};

class CGuiTabControl : GUICONTROL_PARENT {
public: //! tab events
    API_DECL(void) onTabEnter( int fromTabIndex ) {}
    API_DECL(void) onTabLeave( int toTabIndex ) {}

public: //! tab notify dispatch
    API_IMPL(void) onNotify( IObject *source ,messageid_t notifyId ,long param ,Params *params ,void *extra ) IOVERRIDE;
};

//////////////////////////////////////////////////////////////////////////////
class GuiScroll : GUICONTROL_PARENT {
public:
    GuiScroll( GuiControl &group );

    NoDiscard int offset() const { return m_offset; }

public:
    API_IMPL(void) onMouseDown( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) IOVERRIDE;
    API_IMPL(void) onMouseUp( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) IOVERRIDE;
    API_IMPL(void) onMouseMove( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) IOVERRIDE;

    API_IMPL(void) onLayout( const OsRect &clientArea ,OsRect &placeArea ) IOVERRIDE;
    API_IMPL(void) onDraw( const OsRect &updateArea ) IOVERRIDE;

protected:
    GuiControl &m_control;
    Rect m_thumb;

    bool m_grab = false;
    Point m_grabPoint;

    int m_offset = 0;
};

//////////////////////////////////////////////////////////////////////////////
class GuiGroup : public GuiSet {
    //! variable size control set behaving as a group
    //! @note group clip events (drawing...) to their area

public:
    GuiGroup( bool autoscroll=false ) : m_scrollV(NullPtr) {

        m_colors.fillColor = m_colors.foreColor = OS_COLOR_TRANSPARENT;
        if( autoscroll ) {
            m_scrollV = new GuiScroll( *this );
        }
    }

    DECLARE_GUICONTROL(GuiSet,GuiGroup,TINY_GUIGROUP_PUID);

    GuiControl *getFocus() { return m_focus.ptr(); }

    void setFocus( GuiControl *focus );

    Point &offset() { return m_offset; }

public:
    API_IMPL(void) onGotFocus() IOVERRIDE;
    API_IMPL(void) onLostFocus() IOVERRIDE;

protected:
    API_IMPL(void) onLayout( const OsRect &clientArea ,OsRect &placeArea ) IOVERRIDE;
    API_IMPL(void) onDraw( const OsRect &updateArea ) IOVERRIDE;
    API_IMPL(void) onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) IOVERRIDE;
    API_IMPL(void) onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) IOVERRIDE;

protected:
    GuiControlRef m_focus;

    Point m_offset;
    GuiScroll *m_scrollV;
};

//////////////////////////////////////////////////////////////////////////////
//! TabBar

class GuiTabBar : public GuiGroup {
public:
    GuiTabBar() DEFAULT

    DECLARE_GUICONTROL(GuiGroup,GuiTabBar,TINY_GUITABBAR_PUID);

    void Bind( GuiTab &tabs );

protected:
    API_IMPL(void) onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) IOVERRIDE;

    RefOf<GuiTab> m_tabs;
};

//////////////////////////////////////////////////////////////////////////////
//! GuiMenu

class GuiMenu : public GuiCommandOnClick ,GUICONTROL_PARENT {
public:
    GuiMenu();

    DECLARE_GUICONTROL(GuiControl,GuiMenu,TINY_GUIMENU_PUID);
    DECLARE_GUIPROPERTIES;

    void clear() {
        m_root.clear();
    }

    void addItem( int id ,const char *text ,GuiShortcut *shortcut );
    const char *getItemText( int id );

    // virtual bool makeCommandParam( params_t &params );

public:
    API_IMPL(void) onLayout( const OsRect &clientArea ,OsRect &placeArea ) IOVERRIDE;
    API_IMPL(void) onDraw( const OsRect &updateArea ) IOVERRIDE;
    API_IMPL(void) onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) IOVERRIDE;
    API_IMPL(void) onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) IOVERRIDE;

    API_IMPL(void) PostCommand() IOVERRIDE;

protected:
    struct Item {
        uint32_t commandId;
        // thumbnail
        String text;
        GuiShortcut shortcut;

        Rect area; //! placement

        ListOf<RefOf<GuiMenu> > submenu;
    };

    Direction m_direction;
    ListOf<Item> m_root;

    int m_hoover;
};

//////////////////////////////////////////////////////////////////////////////
//! GuiPopup

class GuiPopup : public GuiPublisher ,public GuiGroup { //TODO GuiArray_<1>
    //! @note base class to derive popup from, show with GuiControlWindow::showPopup

public:
    GuiPopup( GuiControl *control=NullPtr );

    void setControl( GuiControl *control );
    GuiControl *getControl() { return m_control.ptr(); }
    bool hasControl() { return !m_control.isNull(); }

    virtual void Open();
    virtual void Close();

public:
    API_IMPL(void) onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) IOVERRIDE;
    API_IMPL(void) onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) IOVERRIDE;

protected:
    GuiControlRef m_control; //! @note single control embedded with the popup layer
};

//////////////////////////////////////////////////////////////////////////////
//! TitleBar

class GuiTitleBar : public GuiGroup {
public:
    GuiTitleBar();

    void Bind( IGuiMessage &owner );
};

//////////////////////////////////////////////////////////////////////////////
//! GuiDialog

class GuiDialog : public GuiPublisher ,public GuiGroup {
    //! @note base class to derive dialog from, show with GuiControlWindow::showModal

public:
    GuiDialog( const char *title=NullPtr );

    virtual void Open();
    virtual void Help() {} //! std help button
    virtual void Close();

public:
    API_IMPL(void) onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) IOVERRIDE;

    API_IMPL(void) onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) IOVERRIDE;

protected:
    GuiTitleBar m_title;
};

//////////////////////////////////////////////////////////////////////////////
//! Common

    //TODO + support for common dialog

//////////////////////////////////////////////////////////////////////////////
//! GuiMessageBox

class GuiMessageBox : public GuiPublisher ,public GuiGroup {
public:
    GuiMessageBox( IGuiMessage &listener ,const char *title ,const char *text ,const Params &options );

    virtual void Open();
    virtual void Close();

public:
    API_IMPL(void) onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) IOVERRIDE;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! ControlWindow

#define GUI_MOUSE_MOTION_DETECT     2
#define GUI_MOUSE_DRAG_DETECT       5

class GuiControlWindow : public IGuiMessageEvents ,public GuiWindow {
public:
    GuiControlWindow( const char_t *name ,const char_t *title ,int width ,int height ,int style= OS_WINDOWSTYLE_SIZEABLE,int flags=OS_WINDOWFLAG_NORMAL ,OsColorRef backgroundColor=OS_COLOR_BLACK );

    DECLARE_OBJECT(GuiControlWindow,TINY_GUICONTROLWINDOW_PUID);

    //-- properties
    ColorQuad &colors() 
    { 
        return m_colors; 
    }

    //-- layers
    GuiGroup &background() { return m_background; }
    GuiGroup &foreground() { return m_foreground; }
    GuiControl &topmost() { return * m_layers.getControl(); }

    //-- mouse tracking
    ListOf<GuiControlRef> &getHitTrackMouse() { return m_hitTracker; }

    GuiControl *getMouseTopHit();

    //! call from control on mouse move event
    void HitTrackMouseEnter( GuiControl &control ,const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState );

    //-- drag & drop
    static bool isDragDropInProgress() { return m_dragOperation != dragOpNone; }
    // const Point &mouseClickPoint() const { return m_mouseClickPoint; }

    //-- theme
    gui::VisualTheme &getTheme() { return *m_theme; }
    gui::VisualTheme &setTheme( gui::VisualTheme &theme ) { m_theme = &theme; return theme; }

public: ///-- GuiControlWindow
    virtual void ShowDialog( uint32_t id ) {}

    void ShowModal( GuiDialog &dialog );
    void ShowPopup( GuiPopup &popup );

    void ShowMessageBox( GuiMessageBox &messagebox );

    void MouseCapture( GuiControl &control );
    void MouseRelease();

    void StartDragDrop( const OsPoint &p ,OsKeyState keyState ); // ,GuiControl &source );
    void UpdateDragDrop( const OsPoint &p ,OsKeyState keyState ,GuiControl *target );
    void EndDragDrop( const OsPoint &p ,OsKeyState keyState ,GuiControl *target ,bool cancel=false );
    void CancelDragDrop();

    void addBinding( const char *name ,IObject *binding );
    IObject *getBinding( const char *name );
    const char *digBinding( IObject *binding ) const;

    template <class T>
    T *getBindingAs_( const char *name ) {
        IObject *p = getBinding(name); return p ? p->As_<T>() : NullPtr;
    }

    void EnableEditor( bool enabled=true );
    void StartEditor();
    void StopEditor();

public: ///-- IGuiEvents
    API_IMPL(void) onLayout( const OsRect &clientArea ,OsRect &placeArea ) IOVERRIDE;
    API_IMPL(void) onDraw( const OsRect &updateArea ) IOVERRIDE;
    API_IMPL(void) onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) IOVERRIDE;
    API_IMPL(void) onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) IOVERRIDE;
    API_IMPL(void) onTimer( OsTimerAction timeAction ,OsEventTime now ,OsEventTime last ) IOVERRIDE;
    API_IMPL(void) onPost( IObject *source ,message_t msg ,long param ,Params *params ,void *extra ) IOVERRIDE;

public: ///-- IGuiMessageEvents
    API_IMPL(void) onCommand( IObject *source ,messageid_t commandId ,long param ,Params *params ,void *extra ) IOVERRIDE;
    API_IMPL(void) onNotify( IObject *source ,messageid_t notifyId ,long param ,Params *params ,void *extra ) IOVERRIDE;

protected:
    ColorQuad m_colors;

    GuiGroup m_background;
    GuiGroup m_foreground;
    GuiLayer m_layers; //! @note background -> foreground -> stack of modal dialogs...

    Point m_mouseClickPoint;

    bool m_mouseMoveDetect;
    bool m_mouseDragDetect;

    GuiControl *m_mouseCapture; //TODO RefOf

    static CriticalSection m_csDrag; //! @note drag drop is a global (cross window...) operation, must be thread safe
    static GuiControlRef m_dragSource;
    static IObjectRef m_dragObject;
    static DragOperation m_dragOperation;

    gui::VisualTheme *m_theme;
    GuiControlRef m_editor; //TODO designer
    bool m_enableEditor;

private:
    ListOf<GuiControlRef> m_hitTracker;

    //! call from parent window mouse move event function
    void HitTrackMouseLeave( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState );

    //! object bindings available in this windows context (root will be set)
    MapOf<String,IObjectRef> m_bindings;
};

//////////////////////////////////////////////////////////////////////////
//! WindowList

    //TODO

//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
#endif //TINY_GUI_GUI_H