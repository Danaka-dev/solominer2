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
//! @author the NExTwave developers
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
#ifndef TINY_NAMESPACE_GUI
 #define TINY_NAMESPACE_GUI gui
#endif

#define TINY_GUI_NAMESPACE TINY_NAMESPACE::TINY_NAMESPACE_GUI

//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

    //! TODO namespace gui + rename all subclass

//////////////////////////////////////////////////////////////////////////////
//! Interface

#define TINY_IGUICONTROLEVENTS_UUID   0x0c69e82d1b186ca9d
#define TINY_IGUIPROPERTIES_UUID      0x08fe381ed09c02626
#define TINY_IGUICOMMANDEVENT_UUID    0x07f584175924526c3

class IGuiControlEvents;
class IGuiProperties;
class IGuiCommandEvent;

//////////////////////////////////////////////////////////////////////////////
//! Class

#define TINY_GUICONTROL_UUID          0x001d19d4782da5133
#define TINY_GUILAYER_UUID            0x02a2268cb68b22621
#define TINY_GUITAB_UUID              0x0ce669856891313b6
#define TINY_GUIGROUP_UUID            0x03738884453df6435
#define TINY_GUITABBAR_UUID           0x0f6524432b75a5ed1
#define TINY_GUIMENU_UUID             0x05a27c3b2b902d95f
#define TINY_GUICONTROLWINDOW_UUID    0x0911441da7a69fe53

class GuiControl;
class GuiSet; //! @note base class only, no UUID
class GuiLayer;
class GuiTab;
class GuiGroup;
class GuiTabBar;
class GuiMenu;
class GuiControlWindow;

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
    GuiCoord( int v=0 ) : value( (int) v ) ,unit(unitPixel) {}
    GuiCoord( float v ) : value( v ) ,unit(unitPercent) {}

//--
    template <typename T>
    int get( T ref ) const {
        return (unit==unitPercent) ? ((value / 100) * ref) : value;
    }
};

template <>
GuiCoord &fromString( GuiCoord &p ,const String &s ,size_t &size );

template <>
String &toString( const GuiCoord &p ,String &s );

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

template <>
GuiCoords &fromString( GuiCoords &p ,const String &s ,size_t &size );

template <>
String &toString( const GuiCoords &p ,String &s );

//////////////////////////////////////////////////////////////////////////////
//! Direction

enum Orientation {
    orientNone=0
    ,orientHorizontal=1
    ,orientVertical=2 ,orientBoth2=(orientHorizontal|orientVertical)
    ,orientDepth=4 ,orientBoth3=(orientBoth2|orientDepth)
    ,orientTime=8 ,orientBoth4=(orientBoth3|orientTime)
        //! etc... @note all values reserved
};

//TODO to/from String

//TODO EnumFlags_

///--
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

//TODO separate align and Anchor (still allow merge for placement calc ?)

template <>
GuiAlign &fromString( GuiAlign &p ,const String &s ,size_t &size );
    //TODO remove, use enumFromString

//TODO toString

///--
// void calc_placement( GuiAlign align ,int &a ,int &b ,int &A ,int &B );

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
    OsColorRef foreColor;
    OsColorRef fillColor;
    OsColorRef textColor;
    OsColorRef backColor;
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
//! Interface

class IGuiControlEvents {
    DECLARE_CLASSID(TINY_IGUICONTROLEVENTS_UUID);

public: ///-- mouse events
    virtual void onClick( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) = 0;
    virtual void onDoubleClick( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) = 0;
    virtual void onMouseDown( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) = 0;
    virtual void onMouseUp( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) = 0;

    virtual void onMouseMove( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) = 0;
    virtual void onMouseEnter( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) = 0;
    virtual void onMouseLeave( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) = 0;

    //-- drag & drop
    virtual OsError onGrab( const OsPoint &p ,OsKeyState keyState ,DragOperation &operation ,IObjectRef &object ) = 0;
    virtual void onDrag( const OsPoint &p ,DragOperation operation ,IObject *object ,IObject *target ,OsError result ) = 0;
    virtual void onDrop( const OsPoint &p ,DragOperation operation ,IObject *object ,IObject *target ,OsError result ) = 0;
    virtual OsError onDropAccept( const OsPoint &p ,IObject *source ,DragOperation operation ,IObject *object ,bool preview ) = 0;

public: ///-- focus events
    virtual void onGotFocus() = 0;
    virtual void onLostFocus() = 0;

public: ///-- key events
    virtual void onKeyChar( OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) = 0;
    virtual void onKeyDown( OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) = 0;
    virtual void onKeyUp( OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) = 0;
};

struct IGuiProperties {
    DECLARE_CLASSID(TINY_IGUIPROPERTIES_UUID);

    virtual void getProperties( Params &params ) const = 0;
    virtual void setProperties( const Params &properties ) = 0;
};

//////////////////////////////////////////////////////////////////////////////
//! Messaging

#define TINY_MESSAGE_USER           0x08000
#define TINY_USERMSG(__id)          TINY_MESSAGE(TINY_MESSAGE_USER,__id)

#define TINY_MESSAGE_ACTION         0x01
#define TINY_MESSAGE_EVENT          0x02
#define TINY_MESSAGE_DATA           0x03
#define TINY_MESSAGE_ASSETS         0x04
#define TINY_MESSAGE_THEME          0x05
//? EDIT

//TODO fix messaging below, from here
#define GUI_MESSAGEID_NONE          0

#define GUI_MESSAGEID_ACTION        1   //! generic message, call to action // ought to be dispatched to 'onCommand'
#define GUI_MESSAGEID_ASSETS        2   //! assets has changed, object should update accordingly
#define GUI_MESSAGEID_THEME         3   //! theme changed
#define GUI_MESSAGEID_PROPS         4   //! properties changed
#define GUI_MESSAGEID_OPENED        8   //! child element opened
#define GUI_MESSAGEID_CLOSED        9   //! child element closed
//TODO fo here

//--
class GuiPublisher : public CPublisher_<IGuiEvents> ,IOBJECT_PARENT {
public:
    void Post( uint64_t msg ,long param ,Params *params=NullPtr ,void *extra=NullPtr ) {
        for( auto &it : m_subscribers ) {
            it->onPost( this ,msg ,param ,params ,extra );
        }
    }
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Controls

#define GUICONTROL_PARENT   public virtual GuiControl

#define CONTROLID_NONE      0

typedef uint32_t controlid_t;

class GuiControl : public IGuiEvents ,public IGuiControlEvents ,public IGuiProperties ,COBJECT_PARENT {
public: ///-- instance
    GuiControl( GuiControlWindow *root=NullPtr );

    DECLARE_OBJECT_STD(CObject,GuiControl,TINY_GUICONTROL_UUID);

public: ///-- getter/setter
    void setControlId( controlid_t id ) { m_id = id; }
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

    bool isOrphan() const { return !m_root; }
    GuiControlWindow &root() { return *m_root; }
    void setRoot( GuiControlWindow &root ) { m_root = &root; }

    bool shouldDraw( const OsRect &updateArea ) const;
    void getCenteredArea( const OsPoint &dims ,Rect &r ) const;

public: ///-- IProperties
    API_IMPL(void) getProperties( Params &properties ) const IOVERRIDE;
    API_IMPL(void) setProperties( const Params &properties ) IOVERRIDE;

    void setPropertiesWithString( const char *properties );

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
    void RegionSetArea( OsRect &r ); //? useOffset
    // void RegionGetArea( OsRect &r ); //? not here ? ... TOOD check if we should use a stack facilities in window to help with areas
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

    void Refresh( bool noArea=false );
    void Update( bool noArea=false );

public: ///-- IGuiControlEvents

//-- mouse events
    void onClick( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) override {}
    void onDoubleClick( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) override {}
    void onMouseDown( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) override {}
    void onMouseUp( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) override {}

    void onMouseMove( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) override {}
    void onMouseEnter( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) override {}
    void onMouseLeave( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) override {}

    OsError onGrab( const OsPoint &p ,OsKeyState keyState ,DragOperation &operation ,IObjectRef &object ) override { return ENOEXEC; }
    void onDrag( const OsPoint &p ,DragOperation operation ,IObject *object ,IObject *target ,OsError result ) override {}
    void onDrop( const OsPoint &p ,DragOperation operation ,IObject *object ,IObject *target ,OsError result ) override {}
    OsError onDropAccept( const OsPoint &p ,IObject *source ,DragOperation operation ,IObject *object ,bool preview ) override { return ENOEXEC; }

//-- focus events
    API_IMPL(void) onGotFocus() IOVERRIDE {}
    API_IMPL(void) onLostFocus() IOVERRIDE {}

//-- key events
    void onKeyChar( OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) override {}
    void onKeyDown( OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) override {}
    void onKeyUp( OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) override {}

public: ///-- IGuiEvents
    void onLayout( const OsRect &clientArea ,OsRect &placeArea ) override;
    void onDraw( const OsRect &uptadeArea ) override;
    void onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) override;
    void onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) override;
    void onTimer( OsTimerAction timeAction ,OsEventTime now ,OsEventTime last ) override;
    void onPost( IObject *source ,uint64_t msg ,long param ,Params *params ,void *extra ) override {}

protected:
    uint32_t m_id = CONTROLID_NONE;
    GuiControlWindow *m_root; //! root/parent window (ptr, not ref, by design)

    GuiCoords m_coords = { 0,0,100.f,100.f };
    GuiAlign m_align = GuiAlign::noAlign;
    ColorQuad m_colors;

    bool m_visible = true;
    bool m_enabled = true;

    Point m_size; //! natural dimension of the control
    Rect m_area; //! area of the control as layout in the display
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
//! ICommand interface

    //TODO review this with proper message struct

#define GUICOMMAND_ID           0x0b9faf354
#define GUICOMMAND_MASK         INT64_MAKE(GUICOMMAND_ID,0)

#define GUICOMMAND_IS(_x_)      (INT64_HIPART(_x_) == GUIMESSAGE_ID) // (((_x_ & 0x0fffffff00000000) ^ MSGID_GUICOMMAND_MASK) == 0)
#define GUICOMMAND_GET(_x_)     (GUICOMMAND_IS(_x_) ? INT64_LOPART(_x_) : 0)
#define GUICOMMAND(_x_)         INT64_MAKE(GUICOMMAND_ID,_x_)

#define GUI_COMMANDID_NONE          0
#define GUI_COMMANDID_ACTION        1
#define GUI_COMMANDID_OK            2
#define GUI_COMMANDID_CANCEL        3
#define GUI_COMMANDID_OPEN          16
#define GUI_COMMANDID_UPDATE        17
#define GUI_COMMANDID_REFRESH       18
#define GUI_COMMANDID_CLOSE         19
#define GUI_COMMANDID_START         20
#define GUI_COMMANDID_STOP          21
#define GUI_COMMANDID_PREV          22
#define GUI_COMMANDID_NEXT          23
#define GUI_COMMANDID_FIRST         24
#define GUI_COMMANDID_LAST          25
#define GUI_COMMANDID_MOVE          27
#define GUI_COMMANDID_ADD           28
#define GUI_COMMANDID_EDIT          29
#define GUI_COMMANDID_REMOVE        30
#define GUI_COMMANDID_HELP          31
//.. select ...

#define GUI_COMMANDID_MENU          5001 //! @note base command for menu if none are provided
#define GUI_COMMANDID_MENU_MAX      5099

///--
struct IGuiCommandEvent : IOBJECT_PARENT {
    DECLARE_CLASSID(TINY_IGUICOMMANDEVENT_UUID);

    struct IParams {
        long param;
        Params params;
        void *extra;
    };

    virtual void onCommand( GuiControl &source ,uint32_t commandId ,long param ,Params *params ,void *extra ) = 0;
};

class GuiCommandPublisher : public CPublisher_<IGuiCommandEvent> ,GUICONTROL_PARENT {
public:
    typedef IGuiCommandEvent::IParams params_t;

    void PostCommand( uint32_t commandId ,long param ,Params *params=NullPtr ,void *extra=NullPtr ) {
        for( auto &it : subscribers() ) {
            it->onCommand( *this ,commandId ,param ,params ,extra );
        }
    }

    void PostCommand( uint32_t commandId ,params_t &params ) {
        PostCommand( commandId ,params.param ,params.params.empty() ? NullPtr : &params.params ,params.extra );
    }
};

class GuiCommandOnClick : public GuiCommandPublisher ,GUICONTROL_PARENT {
public:
    GuiCommandOnClick() : m_commandId(0)
    {
        m_commandParams.param = 0;
        m_commandParams.extra = NullPtr;
    }

    DECLARE_GUIPROPERTIES;

    uint32_t &commandId() { return m_commandId; }
    params_t &commandParam() { return m_commandParams; }

    //--
    uint32_t getCommandId() const { return m_commandId; }
    void setCommandId( uint32_t id ) { m_commandId = id; }

    virtual bool makeCommandParam( params_t &params ) {
        params = m_commandParams;
        return true; //! @note return false to not send the command (e.g. invalid state)
    }

public:
    API_IMPL(void) onClick( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) IOVERRIDE {
        params_t params;

        if( !makeCommandParam( params ) ) return;

        GuiCommandPublisher::PostCommand( m_commandId ,params );
    }

protected:
    uint32_t m_commandId;
    params_t m_commandParams;
};

//////////////////////////////////////////////////////////////////////////////
//! IData interface

#define TINY_DATAMSG(__id)          TINY_MESSAGE(TINY_MESSAGE_DATA,__id)
#define TINY_ISDATAMSG(__id)        TINY_ISMESSAGE(TINY_MESSAGE_DATA,__id)

inline bool isDataMessage( msgid_t id ) {
    return TINY_ISDATAMSG(id);
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! composites

class GuiSet : GUICONTROL_PARENT {
    //! @brief GuiSet : a variable size set of controls, foundation for the different geometries of group

public:
    typedef ListOf<GuiControlRef> controlset_t;

protected:
    controlset_t m_controls;

    Map_<String,int> m_names;

public:
    DECLARE_GUIPROPERTIES;

    //! @note prefer using interface below instead of directly use the control set
    const controlset_t &controls() const { return m_controls; }
    controlset_t &controls() { return m_controls; }

    int index( int i ) const {
        return CLAMP( i ,0 ,m_controls.size()-1 );
    }

public:
    size_t getControlCount() const { return m_controls.size(); }

    bool hasControl() const {
        return !m_controls.empty();
    }

    bool hasControl( int i ) const {
        return ( i >= 0 && i < m_controls.size() );
    }

    GuiControl *getControl() { //! topmost
        int n = getControlCount(); return n > 0 ? getControl(n-1) : nullptr;
    }

    const GuiControl *getControl() const { //! topmost
        int n = getControlCount(); return n > 0 ? getControl(n-1) : nullptr;
    }

    GuiControl *getControl( int i ) { //! by index (not by controlId)
        return hasControl(i) ? m_controls.at(index(i)).ptr() : NullPtr;
    }

    const GuiControl *getControl( int i ) const { //! by index (not by controlId)
        return hasControl(i) ? m_controls.at(index(i)).ptr() : NullPtr;
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

    const char *digControlName( int i );

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

public:
    API_IMPL(void) onLayout( const OsRect &clientArea ,OsRect &placeArea ) IOVERRIDE {
        GuiControl::onLayout( clientArea ,placeArea );

        Rect r = area();

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
        GuiControl *control;

        for( int i=TCount; i>=0; --i ) {
            if( (control = m_controls[i]) && (control->area() & pos[0]) ) {} else continue;

            control->onMouse( mouseAction ,keyState ,mouseButton ,points ,pos );
        }

        GuiControl::onMouse( mouseAction ,keyState ,mouseButton ,points ,pos );
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
    DECLARE_GUICONTROL(GuiControl,GuiLayer,TINY_GUILAYER_UUID);

public:
    virtual void onLayout( const OsRect &clientArea ,OsRect &placeArea );
    virtual void onDraw( const OsRect &updateArea );
    virtual void onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos );
    virtual void onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c );
};

//////////////////////////////////////////////////////////////////////////////
class GuiTab : public GuiSet {
protected:
    int m_tab = 0; //! currently selected tab

public:
    DECLARE_GUICONTROL(GuiControl,GuiTab,TINY_GUITAB_UUID);

    int getTabCount() const { return getControlCount(); }

    void selectTab( int at ) { m_tab = MIN( at ,getTabCount()-1 ); }

    int getCurrentTabIndex() const { return m_tab; }

    GuiControl *getCurrentTab() {
        return getControl(m_tab);
    }

public:
    virtual void onLayout( const OsRect &clientArea ,OsRect &placeArea );
    virtual void onDraw( const OsRect &updateArea );
    virtual void onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos );
    virtual void onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c );
};

//////////////////////////////////////////////////////////////////////////////
class GuiScroll : GUICONTROL_PARENT { //! TODO listener for SCROLL event POST
public:
    GuiScroll( GuiControl &group );

    NoDiscard int offset() const { return m_offset; }

public:
    void onMouseDown( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) override;
    void onMouseUp( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) override;
    void onMouseMove( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) override;

    void onLayout( const OsRect &clientArea ,OsRect &placeArea ) override;
    void onDraw( const OsRect &updateArea ) override;

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
        if( autoscroll ) {
            m_scrollV = new GuiScroll( *this );
        }
    }

    DECLARE_GUICONTROL(GuiControl,GuiGroup,TINY_GUIGROUP_UUID);

    GuiControl *getFocus() { return m_focus.ptr(); }

    void setFocus( GuiControl *focus );

    Point &offset() { return m_offset; }

public:
    API_IMPL(void) onGotFocus() IOVERRIDE;
    API_IMPL(void) onLostFocus() IOVERRIDE;

protected:
    virtual void onLayout( const OsRect &clientArea ,OsRect &placeArea );
    virtual void onDraw( const OsRect &updateArea );
    virtual void onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos );
    virtual void onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c );

protected:
    GuiControlRef m_focus;

    Point m_offset;
    GuiScroll *m_scrollV;
};

//////////////////////////////////////////////////////////////////////////////
//! TabBar

class GuiTabBar : public IGuiCommandEvent ,public GuiGroup {
public:
    GuiTabBar() DEFAULT

    DECLARE_GUICONTROL(GuiGroup,GuiTabBar,TINY_GUITABBAR_UUID);

    void Bind( GuiTab &tabs );

protected:
    void onCommand( GuiControl &source ,uint32_t commandId ,long param ,Params *params ,void *extra ) override;

    RefOf<GuiTab> m_tabs;
};

//////////////////////////////////////////////////////////////////////////////
//! GuiMenu

class GuiMenu : public GuiCommandOnClick ,GUICONTROL_PARENT {
public:
    GuiMenu();

    DECLARE_GUICONTROL(GuiControl,GuiMenu,TINY_GUIMENU_UUID);
    DECLARE_GUIPROPERTIES;

    void addItem( int id ,const char *text ,GuiShortcut *shortcut );

    virtual bool makeCommandParam( params_t &params );

public:
    API_IMPL(void) onLayout( const OsRect &clientArea ,OsRect &placeArea ) IOVERRIDE;
    API_IMPL(void) onDraw( const OsRect &updateArea ) IOVERRIDE;
    API_IMPL(void) onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) IOVERRIDE;
    API_IMPL(void) onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) IOVERRIDE;

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

class GuiTitlebar : public GuiGroup {
public:
    GuiTitlebar();

    void Bind( IGuiCommandEvent &owner );
};

//////////////////////////////////////////////////////////////////////////////
//! GuiDialog

class GuiDialog : public GuiPublisher ,public IGuiCommandEvent ,public GuiGroup {
    //! @note base class to derive dialog from, show with GuiControlWindow::showModal

public:
    GuiDialog( const char *title=NullPtr );

    virtual void Open();
    virtual void Help() {} //! std help button
    virtual void Close();

public:
    void onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) override;

    void onCommand( GuiControl &source ,uint32_t commandId ,long param ,Params *params ,void *extra ) override;

protected:
    GuiTitlebar m_title;
};

//TODO + support for common dialog

//////////////////////////////////////////////////////////////////////////////
//! GuiMessageBox

class GuiMessageBox : public GuiPublisher ,public GuiCommandPublisher ,public IGuiCommandEvent ,public GuiGroup {
public:
    GuiMessageBox( IGuiCommandEvent &listener ,const char *title ,const char *text ,const Params &options );

    virtual void Open();
    virtual void Close();

public:
    void onCommand( GuiControl &source ,uint32_t commandId ,long param ,Params *params ,void *extra ) override;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! ControlWindow

#define GUI_MOUSE_MOTION_DETECT     2
#define GUI_MOUSE_DRAG_DETECT       5

namespace gui {
    struct VisualTheme;
}

class GuiControlWindow : public GuiWindow {
public:
    GuiControlWindow( const char_t *name ,const char_t *title ,int width ,int height ,int style=OS_WINDOWSTYLE_NORMAL ,int flags=OS_WINDOWFLAG_NORMAL ,OsColorRef backgroundColor=OS_COLOR_BLACK );

    DECLARE_OBJECT_STD(GuiWindow,GuiControlWindow,TINY_GUICONTROLWINDOW_UUID);

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
    bool isDragDropInProgress() const { return m_dragOperation!= dragOpNone; }
    // const Point &mouseClickPoint() const { return m_mouseClickPoint; }

    //-- theme
    gui::VisualTheme &getTheme() { return *m_theme; }
    gui::VisualTheme &setTheme( gui::VisualTheme &theme ) { m_theme = &theme; return theme; }

public: ///-- GuiControlWindow
    void ShowModal( GuiDialog &dialog );
    void ShowPopup( GuiPopup &popup );
    // void ClosePopup( GuiControl &control );

    void ShowMessageBox( GuiMessageBox &messagebox );

    void MouseCapture( GuiControl &control );
    void MouseRelease();

    void StartDragDrop( const OsPoint &p ,OsKeyState keyState ); // ,GuiControl &source );
    void UpdateDragDrop( const OsPoint &p ,OsKeyState keyState ,GuiControl *target );
    void EndDragDrop( const OsPoint &p ,OsKeyState keyState ,GuiControl *target ,bool cancel=false );
    void CancelDragDrop();

    void EnableEditor( bool enabled=true );
    void StartEditor();
    void StopEditor();

public: ///-- IGuiEvents
    virtual void onLayout( const OsRect &clientArea ,OsRect &placeArea );
    virtual void onDraw( const OsRect &uptadeArea );
    virtual void onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos );
    virtual void onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c );
    virtual void onTimer( OsTimerAction timeAction ,OsEventTime now ,OsEventTime last );
    virtual void onPost( IObject *source ,uint64_t msg ,long param ,Params *params ,void *extra );

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
    GuiControlRef m_editor;
    bool m_enableEditor;

private:
    ListOf<GuiControlRef> m_hitTracker;

    //! call from parent window mouse move event function
    void HitTrackMouseLeave( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState );
};

//////////////////////////////////////////////////////////////////////////
//! WindowList

    //TODO

//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
#endif //TINY_GUI_GUI_H