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

#ifndef TINY_GUI_H
#define TINY_GUI_H

//////////////////////////////////////////////////////////////////////////////
//! Helpers for C interface

inline void OsGuiSetForeColor( OsGuiContext context ,OsColorRef color ) { OsGuiSetColor( context ,OS_SELECT_FORECOLOR ,color ); }
inline void OsGuiSetFillColor( OsGuiContext context ,OsColorRef color ) { OsGuiSetColor( context ,OS_SELECT_FILLCOLOR ,color ); }
inline void OsGuiSetTextColor( OsGuiContext context ,OsColorRef color ) { OsGuiSetColor( context ,OS_SELECT_TEXTCOLOR ,color ); }
inline void OsGuiSetBackColor( OsGuiContext context ,OsColorRef color ) { OsGuiSetColor( context ,OS_SELECT_BACKCOLOR ,color ); }

inline void OsGuiDrawRectangle( OsGuiContext context ,const OsRect &r ) { OsGuiDrawRectangle( context ,r.left ,r.top ,r.right ,r.bottom ); }
inline void OsGuiDrawEllipse( OsGuiContext context ,const OsRect &r ) { OsGuiDrawEllipse( context ,r.left ,r.top ,r.right ,r.bottom ); }
inline void OsGuiDrawLine( OsGuiContext context ,const OsRect &r ) { OsGuiDrawLine( context ,r.left ,r.top ,r.right ,r.bottom ); }
inline void OsGuiDrawText( OsGuiContext context ,const char_t *text ,const OsRect &r ,int style ,OsRect *rout=NULL ) {
    OsGuiDrawText( context ,text ,r.left ,r.top ,r.right ,r.bottom ,style ,rout );
}

//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
//! Interface

#define TINY_IGUICONTEXT_PUID    0x014a48cfea18664fb
#define TINY_IGUISURFACE_PUID    0x0bbf3d3370c971986
#define TINY_IGUIDISPLAY_PUID    0x0786c5fd92a26e23a
#define TINY_IGUIMESSAGE_PUID    0x0b0f71ead71b710a2
#define TINY_IGUIEVENTS_PUID     0x0e90f416d700cf967

struct IGuiContext;
struct IGuiSurface;
struct IGuiDisplay;
struct IGuiMessage;
struct IGuiEvents;

//////////////////////////////////////////////////////////////////////////////
//! Class

#define TINY_GUIFONT_PUID      0x0c703ce2ee00596af
#define TINY_GUIIMAGE_PUID     0x05a6063982fa973d6
#define TINY_GUIWINDOW_PUID    0x0565a62f13093a7cb

class GuiFont;
class GuiImage;
class GuiWindow;

//////////////////////////////////////////////////////////////////////////
//! Enums

enum TextAlign {
    textalignNormal=OS_ALIGN_NORMAL //! top+left
    ,textalignLeft=OS_ALIGN_LEFT
    ,textalignCenterH=OS_ALIGN_CENTERH
    ,textalignRight=OS_ALIGN_RIGHT
    ,textalignTop=OS_ALIGN_TOP
    ,textalignCenterV=OS_ALIGN_CENTERV
    ,textalignBottom=OS_ALIGN_BOTTOM
    ,textalignCenter=OS_ALIGN_CENTER

    ,textalignTopLeft=(OS_ALIGN_TOP|OS_ALIGN_LEFT)
    ,textalignTopRight=(OS_ALIGN_TOP|OS_ALIGN_RIGHT)
    ,textalignBottomRight=(OS_ALIGN_BOTTOM|OS_ALIGN_RIGHT)
    ,textalignBottomLeft=(OS_ALIGN_BOTTOM|OS_ALIGN_LEFT)

    ,textalignTopCenter=(OS_ALIGN_TOP|OS_ALIGN_CENTERH)
    ,textalignBottomCenter=(OS_ALIGN_BOTTOM|OS_ALIGN_CENTERH)
    ,textalignCenterLeft=(OS_ALIGN_CENTERV|OS_ALIGN_LEFT)
    ,textalignCenterRight=(OS_ALIGN_CENTERV|OS_ALIGN_RIGHT)
};

enum RefreshFlags {
    refreshNormal=OS_REFRESH_NORMAL
    ,refreshResized=OS_REFRESH_RESIZED
    ,refreshContent=OS_REFRESH_CONTENT
    ,refreshBackground=OS_REFRESH_BACKGROUND
    ,refreshUpdate=OS_REFRESH_UPDATE
};

//////////////////////////////////////////////////////////////////////////
//! Drawing interface

//! context states for drawing to a surface
struct IGuiContext : IOBJECT_PARENT {
    DECLARE_CLASS(IGuiContext,TINY_IGUICONTEXT_PUID);

    virtual void SetColor( int selectColor ,OsColorRef c ) = 0;
    virtual void SetAlphaBlend( float alpha ) = 0;
    virtual void SetFont( const GuiFont &font ) = 0;
    virtual void RegionSetArea( int left ,int top ,int right ,int bottom ,bool useOffset=true ) = 0;
    virtual void RegionGetArea( OsRect &area ) = 0;
    virtual void RegionSetOffset( int x ,int y ) = 0;
    virtual void RegionSetScale( float x ,float y ) = 0;
    virtual void PointToCoords( int surfaceForP ,const OsPoint &p ,int surfaceForCoords ,OsPoint &coords ) = 0; //TODO review this function

    //TODO RegionGetOffset & RegionGetScale
};

//! drawing function to a surface
struct IGuiSurface : IOBJECT_PARENT {
    DECLARE_CLASS(IGuiSurface,TINY_IGUISURFACE_PUID);

    virtual void DrawRectangle( int left ,int top ,int right ,int bottom ) = 0;
    virtual void DrawEllipse( int left ,int top ,int right ,int bottom ) = 0;
    virtual void DrawLine( int left ,int top ,int right ,int bottom ) = 0;
    virtual void DrawTextAlign( const char_t *text ,int left ,int top ,int right ,int bottom ,TextAlign align=textalignNormal ,OsRect *rect=NULL ) = 0;
    virtual void DrawPolygon( int npoints ,const OsPoint *points ) = 0;
    virtual void DrawImage( const GuiImage &image
        ,int left ,int top ,int right=OS_IMAGE_USEDIM ,int bottom=OS_IMAGE_USEDIM
        ,int imageLeft=0 ,int imageTop=0 ,int imageRight=OS_IMAGE_USEDIM ,int imageBottom=OS_IMAGE_USEDIM
    ) = 0;

//-- helpers
    void DrawRectangle( const OsRect &r ) { DrawRectangle( r.left ,r.top ,r.right ,r.bottom ); }
    void DrawEllipse( const OsRect &r ) { DrawEllipse( r.left ,r.top ,r.right ,r.bottom ); }
    void DrawLine( const OsRect &r ) { DrawLine( r.left ,r.top ,r.right ,r.bottom ); }
    void DrawTextAlign( const char_t *text ,const OsRect &r ,TextAlign align=textalignNormal ,OsRect *rect=NULL ) { DrawTextAlign( text ,r.left ,r.top ,r.right ,r.bottom ,align ,rect ); }
    void DrawImage( const GuiImage &image ,const OsRect &r ,const OsRect &imgRect ) { DrawImage( image ,r.left ,r.top ,r.right ,r.bottom ,imgRect.left ,imgRect.top ,imgRect.right ,imgRect.bottom ); }
};

struct IGuiDisplay : IGuiSurface ,IGuiContext {
    DECLARE_CLASS(IGuiDisplay,TINY_IGUIDISPLAY_PUID);

    virtual void SetCursor( int cursorId ) = 0;
    //TODO // virtual void GetDimensions( OsPoint &size ,OsPoint &dpi ) = 0;

    virtual void Refresh( const OsRect *area=NULL ,RefreshFlags flags=refreshNormal ) = 0;
    virtual void Update( const OsRect *area=NULL ,RefreshFlags flags=refreshNormal ) = 0;
};

//////////////////////////////////////////////////////////////////////////////
//! Message interface

#define TINY_MESSAGE(__category,__id)       INT64_MAKE(__category,__id)
#define TINY_ISMESSAGE(__category,__msg)    (INT64_HIPART(__msg) == (__category))
#define TINY_GETMESSAGE(__category,__msg)   ( (TINY_ISMESSAGE(__category,__msg) ? INT64_LOPART(__msg) : 0) )

#define TINY_NOMESSAGE      0
#define TINY_MESSAGE_USER   0x08000

#define TINY_USERMESSSAGE(__id)     TINY_MESSAGE(TINY_MESSAGE_USER,__id)
#define TINY_ISUSERMESSAGE(__msg)   TINY_ISMESSAGE(TINY_MESSAGE_USER,__msg)
#define TINY_GETUSERMESSAGE(__msg)  TINY_GETMESSAGE(TINY_MESSAGE_USER,__msg)

typedef uint64_t message_t;

typedef uint32_t messageclass_t;
typedef uint32_t messageid_t;

struct IGuiMessage : IOBJECT_PARENT {
    DECLARE_CLASS(IGuiMessage,TINY_IGUIMESSAGE_PUID);

    virtual void onPost( IObject *source ,message_t msg ,long param ,Params *params ,void *extra ) = 0;
};

//////////////////////////////////////////////////////////////////////////////
//! Events interface

struct IGuiEvents : IGuiMessage ,IOBJECT_PARENT {
    DECLARE_CLASS(IGuiEvents,TINY_IGUIEVENTS_PUID);

    //! @return boolean value indicates if the event should continue to be propagated or not
    virtual void onLayout( const OsRect &clientArea ,OsRect &placeArea ) = 0;
    virtual void onDraw( const OsRect &uptadeArea ) = 0;
    virtual void onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) = 0;
    virtual void onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) = 0;
    virtual void onTimer( OsTimerAction timeAction ,OsEventTime now ,OsEventTime last ) = 0;
};

//////////////////////////////////////////////////////////////////////////////
//! Font

#define OS_GUIFONT_DEFAULTSIZE  18

enum FontWeight {
    fontWeightAny=OS_FONTWEIGHT_ANY
    ,fontWeightThin=OS_FONTWEIGHT_THIN
    ,fontWeightUltraLight=OS_FONTWEIGHT_ULTRALIGHT
    ,fontWeightLight=OS_FONTWEIGHT_LIGHT
    ,fontWeightNormal=OS_FONTWEIGHT_NORMAL
    ,fontWeightMedium=OS_FONTWEIGHT_MEDIUM
    ,fontWeightSemiBold=OS_FONTWEIGHT_SEMIBOLD
    ,fontWeightBold=OS_FONTWEIGHT_BOLD
    ,fontWeightUltraBold=OS_FONTWEIGHT_ULTRABOLD
    ,fontWeightHeavy=OS_FONTWEIGHT_HEAVY
};

enum FontStyle {
    fontStyleNormal=OS_FONTSTYLE_NORMAL
    ,fontStyleItalic=OS_FONTSTYLE_ITALIC
    ,fontStyleUnderline=OS_FONTSTYLE_UNDERLINE
    ,fontStyleStrikeout=OS_FONTSTYLE_STRIKEOUT
};

enum FontPitch {
    fontPitchAny=OS_FONTPITCH_ANY
    ,fontPitchFixed=OS_FONTPITCH_FIXED
    ,fontPitchVariable=OS_FONTPITCH_VARIABLE
};

///--
class GuiFont : COBJECT_PARENT {
public:
    friend class GuiWindow;

    GuiFont( int guiSystemId=OS_GUISYSTEMID_CURRENT ) : _hfont(OS_INVALID_HANDLE) ,_guiSystemId(guiSystemId)
        ,_facename(NULL) ,_pointSize(0) ,_weight(0) ,_style(0) ,_pitch(0)
    {}

    GuiFont( const char_t *faceName ,int pointSize ,int weight ,int style ,int pitch ) : _hfont(OS_INVALID_HANDLE) ,_guiSystemId(OS_GUISYSTEMID_CURRENT)
        ,_facename(NULL) ,_pointSize(0) ,_weight(0) ,_style(0) ,_pitch(0)
    {
        Create( faceName ,pointSize ,weight ,style ,pitch );
    }

    ~GuiFont() { Destroy(); }

    DECLARE_OBJECT_STD(CObject,GuiFont,TINY_GUIFONT_PUID)
    // DECLARE_FACTORY_IOBJECT(GuiFont) //TODO pattern not included yet here

public:
    OsError Create( const char_t *faceName ,int pointSize ,int weight ,int style ,int pitch ) {
        struct OsGuiSystemTable *guiSystem;

        OsError error = OsGuiGetSystemTable( _guiSystemId ,&guiSystem );

        if( OS_FAILED(error) ) return error;

        if( guiSystem == NULL ) return ENOSYS;

        Destroy();

        error = guiSystem->_FontCreate( &_hfont ,faceName ,pointSize ,weight ,style ,pitch );

        if( OS_SUCCEED(error) ) {
            SetFaceName(faceName);
            _pointSize = pointSize;
            _weight = weight;
            _style = style;
            _pitch = pitch;
        }

        return error;
    }

    void Destroy( void ) {
        if( _hfont != OS_INVALID_HANDLE )
            OsHandleDestroy( &_hfont );
    }

    OsHandle GetHandle( void ) const { return _hfont; }

    void CalcTextSize( const char_t *text ,struct OsPoint *size ) const { OsGuiFontCalcSize( _hfont ,text ,size ); }

public:
    OsError LoadFromMemory( uint8_t *memory ,const void *info=NULL ) { OsError error = OsGuiResourceLoadFromMemory( &_hfont ,OS_GUIRESOURCETYPE_FONT ,memory ,info ); assert( error != ENOERROR || OsGuiResourceGetType(_hfont) == OS_GUIRESOURCETYPE_FONT ); return error; }
    OsError LoadFromFile( const char_t *filename ) { OsError error = OsGuiResourceLoadFromFile( &_hfont ,OS_GUIRESOURCETYPE_FONT ,filename ); assert( error != ENOERROR || OsGuiResourceGetType(_hfont) == OS_GUIRESOURCETYPE_FONT ); return error; }
    OsError LoadFromApp( int resourceId ,const char_t *application=NULL ) { OsError error = OsGuiResourceLoadFromApp( &_hfont ,OS_GUIRESOURCETYPE_FONT ,resourceId ,application ); assert( error != ENOERROR || OsGuiResourceGetType(_hfont) == OS_GUIRESOURCETYPE_FONT ); return error; }

    static GuiFont &getDefault() {
        static GuiFont *g_default = new GuiFont( NULL ,18 ,OS_FONTWEIGHT_NORMAL ,OS_FONTSTYLE_NORMAL ,OS_FONTPITCH_ANY );
        return *g_default;
    }

protected:
    OsHandle _hfont;

    int _guiSystemId;

    void SetFaceName( const char_t *facename )     {
        if( _facename ) free( _facename );

        _facename = _tcsdup( facename ? facename : "default" );
    }

public:
    char_t *_facename;

    int _pointSize;
    int _weight;
    int _style;
    int _pitch;
};

typedef RefOf<GuiFont> GuiFontRef;

DEFINE_MANIFEST_API(GuiFont,Params);

//////////////////////////////////////////////////////////////////////////
//! ThumbMap

    //! @note ThumbMap serves to configure set of thumbnail from within an image

struct ThumbMap {
    ListOf<Rect> rects;
    //LATER can hold ref point, e.g. a baseline for font etc...
};

void addThumbmapRegular( ThumbMap &p ,const OsRect &canvas ,int nx ,int ny ,int stepx=0 ,int stepy=0 );
void addThumbmapFromPoints( ThumbMap &p ,const OsRect &canvas ,const ListOf<Point> &points );

DEFINE_STRING_API(ThumbMap);

//////////////////////////////////////////////////////////////////////////
#define GUIIMAGE_UPDATE		if( OS_SUCCEED(error) ) UpdateInfo()

class GuiImage : COBJECT_PARENT {
public:
    friend class GuiWindow;

    GuiImage( int guiSystemId=OS_GUISYSTEMID_CURRENT ) :
        _guiSystemId(guiSystemId) ,_himage(OS_INVALID_HANDLE)
    {
        _info.pixelFormat = OS_IMAGE_RGB24; _info.width = _info.height = 0;
    }

    ~GuiImage() { Destroy(); }

    DECLARE_OBJECT_STD(CObject,GuiImage,TINY_GUIIMAGE_PUID)

public:
    int GetGuiSystemId() const { return _guiSystemId; }

    OsHandle GetHandle() const { return _himage; }

    bool IsEmpty() const { return _himage == OS_INVALID_HANDLE; }

    int GetWidth( void ) const { return _info.width; }
    int GetHeight( void ) const { return _info.height; }

    ThumbMap &thumbmap() { return _thumbmap; }

    const Rect getThumbRect( int &i ) const {
        int n = (int) _thumbmap.rects.size(); if( n==0 ) Rect();

        return _thumbmap.rects[ i = CLAMP(i,0,n-1) ];
    }

public:
    OsError Create( int width ,int height ) {
        struct OsGuiSystemTable *guiSystem;

        OsError error = OsGuiGetSystemTable( _guiSystemId ,&guiSystem );

        if( OS_FAILED(error) ) return error;

        if( guiSystem == NULL ) return ENOSYS;

        Destroy();

        error = guiSystem->_ImageCreate( &_himage ,width ,height );

        GUIIMAGE_UPDATE;

        return error;
    }

    void Destroy( void ) {
        if( _himage != OS_INVALID_HANDLE ) OsHandleDestroy( &_himage );
    }

public:
    OsError Write( const uint8_t *pixelValues ,size_t size ) { return OsGuiImageWrite( _himage ,pixelValues ,size ); }

    void DrawThumbnail( OsGuiContext context ,int left ,int top ,int thumbnailId ) const {
        if( IsEmpty() || _thumbmap.rects.empty() ) return;

        auto &thumb = thumbrect(thumbnailId);

        int w = thumb.getWidth();
        int h = thumb.getHeight();

        OsGuiDrawImage( context ,left ,top ,left+w ,top+h ,_himage ,thumb.left ,thumb.top ,thumb.right ,thumb.bottom );
    }

    void DrawImage( OsGuiContext context ,int left ,int top ,int right ,int bottom
            ,int imageLeft ,int imageTop ,int imageRight=OS_IMAGE_USEDIM ,int imageBottom=OS_IMAGE_USEDIM
    ) const {
        if( IsEmpty() ) return;

        OsGuiDrawImage( context ,left ,top ,right,bottom ,_himage ,imageLeft ,imageTop ,imageRight ,imageBottom );
    }

public:
    OsError LoadFromMemory( uint8_t *memory ,const void *info=NULL ) {
        Destroy();

        OsError error = OsGuiResourceLoadFromMemory( &_himage ,OS_GUIRESOURCETYPE_IMAGE ,memory ,info );
        assert( error != ENOERROR || OsGuiResourceGetType(_himage) == OS_GUIRESOURCETYPE_IMAGE );

        GUIIMAGE_UPDATE;
        return error;
    }

    OsError LoadFromFile( const char_t *filename ) {
        Destroy();

        OsError error = OsGuiResourceLoadFromFile( &_himage ,OS_GUIRESOURCETYPE_IMAGE ,filename );
        assert( error != ENOERROR || OsGuiResourceGetType(_himage) == OS_GUIRESOURCETYPE_IMAGE );

        GUIIMAGE_UPDATE;
        return error;
    }

    OsError LoadFromApp( int resourceId ,const char_t *application=NULL ) {
        Destroy();

        OsError error = OsGuiResourceLoadFromApp( &_himage ,OS_GUIRESOURCETYPE_IMAGE ,resourceId ,application );
        assert( error != ENOERROR || OsGuiResourceGetType(_himage) == OS_GUIRESOURCETYPE_IMAGE );

        GUIIMAGE_UPDATE;
        return error;
    }

protected:
    void UpdateInfo( void ) { OsGuiImageGetInfo( _himage ,&_info ); }

    const Rect &thumbrect( int i ) const {
        int n = (int) _thumbmap.rects.size();
        assert(n>0);
        return _thumbmap.rects[ CLAMP(i,0,n-1) ];
    }

protected:
    int _guiSystemId; //! gui system associated with this image

    OsHandle _himage; //! image handle
    OsGuiImageInfo _info; //! image info

    ThumbMap _thumbmap;
};

typedef RefOf<GuiImage> GuiImageRef;

DEFINE_MANIFEST_API(GuiImage,Params);

//////////////////////////////////////////////////////////////////////////
struct GuiTimerInfo {
    IGuiEvents *listener;
    OsEventTime delay;
    OsEventTime last;
};

//////////////////////////////////////////////////////////////////////////
//! Window

extern OsError GuiWindowFunction( const struct OsEventMessage *msg ,void *userData );

class GuiWindow : public IGuiEvents ,public virtual IGuiDisplay ,COBJECT_PARENT {
public: ///-- instance
    friend OsError GuiWindowFunction( const struct OsEventMessage *msg ,void *userData );

    GuiWindow( const char_t *name ,const char_t *title ,int width ,int height ,int style= OS_WINDOWSTYLE_SIZEABLE,int flags=OS_WINDOWFLAG_NORMAL ,OsColorRef backgroundColor=OS_COLOR_BLACK ) :
        _hwindow(OS_INVALID_HANDLE)
    {
        _name = name; _title = title;

        _properties.title = _title.c_str();
        _properties.defaultWidth = width;
        _properties.defaultHeight = height;
        _properties.style = style;
        _properties.flags = flags;
        _properties.backgroundColor = backgroundColor;

        _lastLayoutTime = 0;
        _lastRenderTime = 0;
    }

    virtual ~GuiWindow() {
        Destroy();
    }

    //DECLARE_OBJECT_STD(CObject,GuiWindow,TINY_GUIWINDOW_PUID)
    DECLARE_OBJECT(GuiControlWindow,TINY_GUIWINDOW_PUID);

public:
    OsHandle GetHandle( void ) const { return _hwindow; }

    OsError Create( int guiSystem=OS_GUISYSTEMID_CURRENT ,bool visible=true );

    void Destroy( void ) { OsHandleDestroy( &_hwindow ); }

public: //-- function
    void GetClientSize( OsPoint &size ) { OsGuiGetClientArea( _hwindow ,&size ); }
    void GetClientRect( OsRect &rect ) { OsPoint size; OsGuiGetClientArea( _hwindow ,&size ); rect.left = rect.top = 0; rect.right = size.x; rect.bottom = size.y; }

    //-- visibility
    void Show( void ) { OsGuiWindowShow( _hwindow ,1 ); }
    void Hide( void ) { OsGuiWindowShow( _hwindow ,0 ); }

    //-- input
    void MouseCapture( void ) { OsGuiMouseCapture( _hwindow ); }
    void MouseRelease( void ) { OsGuiMouseRelease( _hwindow ); }

public: //-- GuiDisplay
    OsGuiContext GetContext( void ) const { return _context; }

//-- context
    virtual void SetColor( int selectColor ,OsColorRef c ) { OsGuiSetColor( _context ,selectColor ,c ); }
    virtual void SetFont( const GuiFont &font ) { if( &font != NULL ) OsGuiSetFont( _context ,font.GetHandle() ); }
    virtual void SetAlphaBlend( float alpha ) { OsGuiSetAlphaBlend( _context ,alpha ); }
    virtual void RegionSetArea( int left ,int top ,int right ,int bottom ,bool useOffset=true ) { OsGuiRegionSetArea( _context ,left ,top ,right ,bottom ,(useOffset ? 1 : 0) ); }
    virtual void RegionGetArea( OsRect &area ) { OsGuiRegionGetArea( _context ,&area ); }
    virtual void RegionSetOffset( int x ,int y ) { _offset.x=x; _offset.y=y; OsGuiRegionSetOffset( _context ,x ,y ); }
    virtual void RegionGetOffset( int &x ,int &y ) { x=_offset.x; y=_offset.y; } //!TODO in tiny-for-c
    virtual void RegionSetScale( float x ,float y ) { _scale.x=x; _scale.y=y; OsGuiRegionSetScale( _context ,x ,y ); }
    virtual void RegionGetScale( float &x ,float &y ) { x=_scale.x; y=_scale.y; }
    virtual void PointToCoords( int surfaceForP ,const OsPoint &p ,int surfaceForCoords ,OsPoint &coords ) {
        OsGuiPointToCoords( _context ,surfaceForP ,&p ,surfaceForCoords ,&coords );
    }

//-- surface
    virtual void DrawRectangle( int left ,int top ,int right ,int bottom ) { OsGuiDrawRectangle( _context ,left ,top ,right ,bottom ); }
    virtual void DrawEllipse( int left ,int top ,int right ,int bottom ) { OsGuiDrawEllipse( _context ,left ,top ,right ,bottom ); }
    virtual void DrawLine( int left ,int top ,int right ,int bottom ) { OsGuiDrawLine( _context ,left ,top ,right ,bottom ); }
    virtual void DrawPolygon( int npoints ,const OsPoint *points ) { OsGuiDrawPolygon( _context ,npoints ,points ); }
    virtual void DrawTextAlign( const char_t *text ,int left ,int top ,int right ,int bottom ,TextAlign align=textalignNormal ,OsRect *extends=NULL ) {
        if( _context != OS_INVALID_HANDLE ) OsGuiDrawText( _context ,text ,left ,top ,right ,bottom ,align ,extends );
    }

    virtual void DrawImage( const GuiImage &image
        ,int left ,int top ,int right=OS_IMAGE_USEDIM ,int bottom=OS_IMAGE_USEDIM
        ,int imageLeft=0 ,int imageTop=0 ,int imageRight=OS_IMAGE_USEDIM ,int imageBottom=OS_IMAGE_USEDIM
    ) {
        if( &image != NULL && image._himage != OS_INVALID_HANDLE )
            image.DrawImage( _context ,left ,top ,right ,bottom ,imageLeft ,imageTop ,imageRight ,imageBottom );
    }

//-- display
    virtual void SetCursor( int cursorId ) { OsGuiMouseSetCursor( _hwindow ,cursorId ); }

    virtual void Refresh( const OsRect *area=NULL ,RefreshFlags flags=refreshNormal ) {
        if( _hwindow ) OsGuiWindowRefresh( _hwindow ,area ,(int) flags );
    }

    virtual void Update( const OsRect *area=NULL ,RefreshFlags flags=refreshNormal ) {
        if( _hwindow ) OsGuiWindowRefresh( _hwindow ,area ,OS_REFRESH_UPDATE | (int) flags );
    }

public: //-- helpers
    void SetForeColor( OsColorRef c ) { OsGuiSetForeColor( _context ,c ); }
    void SetFillColor( OsColorRef c ) { OsGuiSetFillColor( _context ,c ); }
    void SetTextColor( OsColorRef c ) { OsGuiSetTextColor( _context ,c ); }
    void SetBackColor( OsColorRef c ) { OsGuiSetBackColor( _context ,c ); }

    void RegionSetArea( const OsRect &area ,bool useOffset=true ) { RegionSetArea( area.left ,area.top ,area.right ,area.bottom ,useOffset ); }
    void RegionSetOffset( OsPoint p ) { _offset = p; RegionSetOffset( p.x ,p.y ); }
    void RegionGetOffset( OsPoint &p ) { p = _offset; } //TODO in C interface
    // void RegionSetScale( Point_<float> p ) { _scale = p; RegionSetScale( p.x ,p.y ); } //TODO prob with Point_ template, fix first
    // void RegionGetScale( Point_<float> &p ) { p = _scale; } //TODO dito

    void DrawRectangle( const OsRect &r ) { DrawRectangle( r.left ,r.top ,r.right ,r.bottom ); }
    void DrawEllipse( const OsRect &r ) { DrawEllipse( r.left ,r.top ,r.right ,r.bottom ); }
    void DrawLine( const OsRect &r ) { DrawLine( r.left ,r.top ,r.right ,r.bottom ); }
    void DrawTextAlign( const char_t *text ,const OsRect &r ,TextAlign align=textalignNormal ,OsRect *extends=NULL ) { DrawTextAlign( text ,r.left ,r.top ,r.right ,r.bottom ,align ,extends ); }
    void DrawImage( const GuiImage &image ,const OsRect &r ,const OsRect &imgRect ) { DrawImage( image ,r.left ,r.top ,r.right ,r.bottom ,imgRect.left ,imgRect.top ,imgRect.right ,imgRect.bottom ); }

public: //-- timers
    //! @note several timers may have the same listener (different delay/data)...
    uint32_t addTimer( IGuiEvents &listener ,OsEventTime delay ,void *userdata=NullPtr );
    void setTimer( uint32_t id ,OsEventTime delay );
    void removeTimer( uint32_t id );
    void removeTimers( IGuiEvents &listener );

    OsEventTime getLayoutTime() { return _lastRenderTime; }
    OsEventTime getRenderTime() { return _lastRenderTime; }

public: ///-- Windows event
    virtual OsError onCreate( void ) { return ENOERROR; }
    virtual OsError onClose( void ) { return ENOERROR; }
    virtual void onDestroy( void ) {}

    virtual void onLayout( struct OsRect &clientRect ) {
        _lastLayoutTime = OsTimerConvertToMs( OsTimerGetTimeNow() );

        OsRect placearea = clientRect;

        onLayout( clientRect ,placearea );
    }

    virtual void onRender( const OsRect &updateRect ) {
        _lastRenderTime = OsTimerConvertToMs( OsTimerGetTimeNow() );

        onDraw( updateRect );
    }

public: ///-- IGuiEvents
    virtual void onLayout( const OsRect &clientArea ,OsRect &placeArea ) {}
    virtual void onDraw( const OsRect &uptadeArea ) {}
    virtual void onMouse( OsMouseAction mouseAction ,OsKeyState keyState ,OsMouseButton mouseButton ,int points ,const OsPoint *pos ) {}
    virtual void onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) {}
    virtual void onTimer( OsTimerAction timeAction ,OsEventTime now ,OsEventTime last );
    virtual void onPost( IObject *source ,uint64_t msg ,long param ,Params *params ,void *extra ) {};

protected: //-- members
    String _name ,_title;

    OsGuiWindowProperties _properties;

    OsHandle _hwindow;

    OsEventTime _lastLayoutTime;
    OsEventTime _lastRenderTime;

    std::map<uint32_t,GuiTimerInfo> m_timers;
    uint32_t m_timerIds = 0;

private:
    OsGuiContext _context;

    static Point_<int> _offset;
    static Point_<float> _scale;
};

//////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////
#endif //TINY_GUI_H