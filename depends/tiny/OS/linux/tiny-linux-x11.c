/****************************************************** //
//               tiny-for-c v3 library                  //
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

#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-reserved-identifier"

//////////////////////////////////////////////////////////////////////////
//! DEVNOTE

// #include <X11/Xcursor/Xcursor.h>
// Cursor cursor = XcursorLibraryLoadCursor( xdisplay ,"sb_v_double_arrow" );

//////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <assert.h>

#include <unistd.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <fcntl.h>
#include <sys/stat.h>

#include <semaphore.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#define __USE_GNU
#include <pthread.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xdefs.h>
#include <X11/keysymdef.h>

#include <png.h>

//////////////////////////////////////////////////////////////////////////
#include "../../tiny-os.h"

#include "tiny-linux-x11.h"
#include "tiny-linux-defs.h"

#define SIZEOFARRAY(__p_)		(sizeof(__p_) / sizeof(__p_[0]))
#define LASTOFARRAY(__p_)		(__p_[SIZEOFARRAY(__p_)-1])


//////////////////////////////////////////////////////////////////////////
extern struct OsGuiSystemTable *_guiSystemX11;

extern OsError LinuxAddWindowToThread( Window window ,struct GuiWindowHandle *handle );

struct OsHandle;

#define X11_HANDLETYPE_WINDOW	1
#define X11_HANDLETYPE_FONT		2
#define X11_HANDLETYPE_IMAGE	3

#define OS_LINUX_DEFAULTFONTNAME    "10x20"

//////////////////////////////////////////////////////////////////////////
static Display *_linux_xdisplay = NULL;

static pthread_mutex_t _gui_mutex = PTHREAD_MUTEX_INITIALIZER;

Display *_linux_getdisplay( void ) {
    //! Initiate connection to the X server with XOpenDisplay

    Status status;

    //TODO use XLockDisplay + XUnlockDisplay for concurrent thread to use X server

    pthread_mutex_lock( &_gui_mutex );
    {
        if( _linux_xdisplay == NULL ) {
            status = XInitThreads();

            assert( status != 0 );

            _linux_xdisplay = XOpenDisplay(NULL);
        }
    }
    pthread_mutex_unlock( &_gui_mutex );

    return _linux_xdisplay;
}

Display *_linux_peekdisplay( void ) {
    //! Use peekDisplay where it's garanteed that display has already been initalized

    return _linux_xdisplay;
}

void _x11_CloseDisplay( void ) {
    pthread_mutex_lock( &_gui_mutex );
    {
        if( _linux_xdisplay == NULL ) {
            XCloseDisplay( _linux_xdisplay );

            _linux_xdisplay = NULL;
        }
    }
    pthread_mutex_unlock( &_gui_mutex );
}

//////////////////////////////////////////////////////////////////////////
static struct GuiWindowHandle *NewGuiWindowHandle( void ) {
    const size_t size = sizeof(struct GuiWindowHandle);

    struct GuiWindowHandle *p = (struct GuiWindowHandle*) malloc(size);

    if( p == NULL ) return NULL;

    memset( p ,0 ,size );

    p->_magic = GUIWINDOWHANDLE_MAGIC;

    return p;
}

static struct GuiWindowHandle *CastGuiWindowHandle( OsHandle handle ) {
    struct GuiWindowHandle *p = (struct GuiWindowHandle *) handle;

    if( p == NULL ) return NULL;

    if( p->_magic != GUIWINDOWHANDLE_MAGIC ) return NULL;

    return p;
}

//////////////////////////////////////////////////////////////////////////
void mapX11KeyState( struct OsKeyState *keyState ,unsigned int xstate ) {
    memset( keyState ,0 ,sizeof(struct OsKeyState) ); //! clean & tidy

    keyState->ctrl = ( (xstate & ControlMask) ? 1 : 0 );
    keyState->shift = ( (xstate & ShiftMask) ? 1 : 0 );
    keyState->alt = ( (xstate & Mod1Mask) ? 1 : 0 );
}

OsKeyCode mapX11KeyCode( unsigned int x11code ) {
    switch( x11code ) {
        case 22: return OS_KEYCODE_BACKSPACE; //? XK_BackSpace
        case 104: return OS_KEYCODE_RETURN;
        case 110: return OS_KEYCODE_HOME;
        case 111: return OS_KEYCODE_UP;
        case 113: return OS_KEYCODE_LEFT; // XK_Left
        case 114: return OS_KEYCODE_RIGHT; // XK_Linefeed
        case 115: return OS_KEYCODE_END;
        case 116: return OS_KEYCODE_DOWN;
        case 119: return OS_KEYCODE_DELETE;

    //-- reflect
        case OS_KEYCODE_BACKSPACE: return 22;
        case OS_KEYCODE_RETURN: return 104;
        case OS_KEYCODE_HOME: return 110;
        case OS_KEYCODE_UP: return 111;
        case OS_KEYCODE_LEFT: return 113;
        case OS_KEYCODE_RIGHT: return 114;
        case OS_KEYCODE_END: return 115;
        case OS_KEYCODE_DOWN: return 116;
        case OS_KEYCODE_DELETE: return 119;

        default:
            return x11code;
    }
}

OsError _linux_guiwindowproc( struct GuiWindowHandle *p ,XEvent *event ) {
	struct OsEventMessage eventMessage;
    struct GuiContextHandle context;
    struct OsPoint points[OS_MAX_MOUSEPOS];

    int width ,height;

    char keybuf[32];
    KeySym keysym;

    memset( &context ,0 ,sizeof(struct GuiContextHandle) );
    
    context._hwindow = p;
    context._xdisplay = _linux_getdisplay(); //TODO peekdisplay ?
	context._guiSystem = p->_guiSystem;

    /* switch( event->type ) {
        case MotionNotify:
        case GenericEvent:
        case EnterNotify:
        case LeaveNotify:
            break;
        default:
            printf( "XEVENT : %d\n" ,event->type );
            break;
    } */

    switch( event->type )
    {
    case SelectionNotify:
        break;

    case CreateNotify:
		eventMessage.eventType = osExecuteEvent;
		eventMessage.executeMessage.executeAction = osExecuteStart;

		return p->_function( &eventMessage ,p->_userData );

    case ConfigureNotify:
        p->_width = event->xconfigure.width;
        p->_height = event->xconfigure.height;
        ++ p->_resized;
        break;

    case ResizeRequest:
        p->_width = event->xresizerequest.width;
        p->_height = event->xresizerequest.height;
        ++ p->_resized;
        break;

    // case NoExpose:
    case Expose:
        width = event->xexpose.width ? event->xexpose.width : p->_width;
        height = event->xexpose.height ? event->xexpose.height : p->_height;

        /* if( event->xexpose.x == 0 && event->xexpose.y == 0 ) {
            if( p->_width != event->xexpose.width || p->_height != event->xexpose.height ) {
                p->_width = event->xexpose.width;
                p->_height = event->xexpose.height;
                ++ p->_resized;
            }
        } */

        context._xdrawable = p->_xpixmap;
        context._xgc = p->_xgc;
        
		context._region.left = event->xexpose.x;
		context._region.top = event->xexpose.y;
		context._region.right = event->xexpose.x + width; // event->xexpose.width;
		context._region.bottom = event->xexpose.y + height; // event->xexpose.height;

        context._offsetx = context._offsety = 0;
        context._scalex = context._scaley = 1.0;
        
        eventMessage.eventType = osRenderEvent;
        eventMessage.renderMessage.context = (OsGuiContext) &context;
		eventMessage.renderMessage.updateRect = &context._region;
        eventMessage.renderMessage.resized = p->_resized;

        p->_function( &eventMessage ,p->_userData );
        p->_resized = 0;

		p->_guiSystem->_WindowSwapBuffers( (OsHandle*) p );
        break;
        
    case ButtonPress:
		eventMessage.eventType = osMouseEvent;
        eventMessage.mouseMessage.mouseAction = osMouseButtonDown;
        p->_button = event->xbutton.button;

        switch( event->xbutton.button ) {
            default:
            case 1: eventMessage.mouseMessage.mouseButton = osLeftMouseButton; break;
            case 2: eventMessage.mouseMessage.mouseButton = osMiddleMouseButton; break;
            case 3: eventMessage.mouseMessage.mouseButton = osRightMouseButton; break;
            case 4:
                eventMessage.mouseMessage.mouseButton = osNoMouseButton;
                eventMessage.mouseMessage.mouseAction = osMouseWheelUp;
                break;
            case 5:
                eventMessage.mouseMessage.mouseButton = osNoMouseButton;
                eventMessage.mouseMessage.mouseAction = osMouseWheelDown;
                break;
        };

		points[0].x = (int) event->xbutton.x;
		points[0].y = (int) event->xbutton.y;

		eventMessage.mouseMessage.points = 1;
		eventMessage.mouseMessage.pos = points;

        mapX11KeyState( &(eventMessage.mouseMessage.keyState) ,event->xbutton.state );

		p->_function( &eventMessage ,p->_userData );        
		break;
    
    case ButtonRelease:  
      	eventMessage.eventType = osMouseEvent;
		eventMessage.mouseMessage.mouseAction = osMouseButtonUp;
        p->_button = osNoMouseButton;

        switch( event->xbutton.button ) {
            default:
            case 1: eventMessage.mouseMessage.mouseButton = osLeftMouseButton; break;
            case 2: eventMessage.mouseMessage.mouseButton = osMiddleMouseButton; break;
            case 3: eventMessage.mouseMessage.mouseButton = osRightMouseButton; break;
            case 4: case 5:
                //! only map ButtonPress => MouseWheel, ignored here
                return ENOERROR;
        };

        points[0].x = (int) event->xbutton.x;
		points[0].y = (int) event->xbutton.y;

		eventMessage.mouseMessage.points = 1;
		eventMessage.mouseMessage.pos = points;

        mapX11KeyState( &(eventMessage.mouseMessage.keyState) ,event->xbutton.state );

        p->_function( &eventMessage ,p->_userData );
		break;
        
    case MotionNotify:
		eventMessage.eventType = osMouseEvent;
		eventMessage.mouseMessage.mouseAction = osMouseMove;
		eventMessage.mouseMessage.mouseButton = p->_button;

		points[0].x = (int) event->xmotion.x;
		points[0].y = (int) event->xmotion.y;

		eventMessage.mouseMessage.points = 1;
		eventMessage.mouseMessage.pos = points;

		p->_function( &eventMessage ,p->_userData );                
		break;
        
    case KeyPress:
        eventMessage.eventType = osKeyboardEvent;
        eventMessage.keyboardMessage.keyAction = osKeyDown;
        eventMessage.keyboardMessage.keyCode = mapX11KeyCode( event->xkey.keycode );
        // eventMessage.keyboardMessage.c = (int) XLookupKeysym( & event->xkey ,0 ); //? index = 0

        XLookupString( &event->xkey ,keybuf ,32 ,&keysym ,NULL );
        eventMessage.keyboardMessage.c = keybuf[0]; //? or keysym

        mapX11KeyState( &(eventMessage.keyboardMessage.keyState) ,event->xkey.state );

        p->_function( &eventMessage ,p->_userData );

        //!TODO @note no osKeyChar event ... normalize with windows to only have press and release key event

        break;

    case KeyRelease:
        eventMessage.eventType = osKeyboardEvent;
        eventMessage.keyboardMessage.keyAction = osKeyUp;
        eventMessage.keyboardMessage.keyCode = event->xkey.keycode;
        eventMessage.keyboardMessage.c = (int) XLookupKeysym( & event->xkey ,0 );;

        // XkbKeycodeToKeysym( context._xdisplay, event->xkey.keycode ,0 ,event->xkey.state & ShiftMask ? 1 : 0 );
        // mykey = XKeycodeToKeysym( display, event.xkey.keycode, 1 );

        mapX11KeyState( &(eventMessage.keyboardMessage.keyState) ,event->xkey.state );

        p->_function( &eventMessage ,p->_userData );
        break;

    case GenericEvent:
        if( event->xclient.message_type == XTIME_EVENT ) {
            eventMessage.eventType = osTimerEvent;
            eventMessage.timerMessage.now = (OsEventTime) event->xclient.data.l[0];
            eventMessage.timerMessage.last = (OsEventTime) event->xclient.data.l[1];

            p->_function( &eventMessage ,p->_userData );
        }
        break;

    default:
        // printf( "XEVENT (unhandled) : %d\n" ,event->type );
        break;
    }

    return ENOERROR;
}

OsError _X11WindowCreate( OsHandle *handle ,const char_t *name ,const struct OsGuiWindowProperties *properties ,OsEventFunction eventFunction ,void *userData ) {
	struct GuiWindowHandle *p = NULL;
   
    int width = 1024 ,height = 768 ,depth;
    
    XSetWindowAttributes xattributes;
    
    Window xwindow;

    Display *xdisplay = _linux_getdisplay();

    Screen *xscreen = XDefaultScreenOfDisplay(xdisplay);

    Visual *xvisual = DefaultVisual( xdisplay ,0 );
    
    depth = DefaultDepth( xdisplay ,0 );
    
    // xattributes.background_pixel = XBlackPixel( xdisplay ,0 );
    //! NB can't use this with off display render+copy .. some sync issue prob

    xattributes.event_mask =
            ExposureMask | StructureNotifyMask
            | SubstructureNotifyMask | VisibilityChangeMask // | PropertyChangeMask
            | EnterWindowMask | LeaveWindowMask
            | ButtonPress | ButtonReleaseMask | ButtonMotionMask
            | KeyPressMask | KeyReleaseMask // | KeymapStateMask
            | PointerMotionMask // | ColormapChangeMask
            // | OwnerGrabButtonMask // mask avoid automatic grab
    ;

    if( properties != NULL )     {
        width = properties->defaultWidth;
        height = properties->defaultHeight;
    }
    
    xwindow = XCreateWindow(
        xdisplay ,XRootWindow(xdisplay,0) 
        ,0 ,0 ,width ,height ,5 ,depth 
        ,InputOutput ,xvisual
        ,/* CWBackPixel| */ CWEventMask ,&xattributes
    );
    
    XStoreName( xdisplay ,xwindow ,name );
    
    /* XSelectInput( xdisplay ,xwindow ,ExposureMask | KeyPressMask | ButtonPress |
                          StructureNotifyMask | ButtonReleaseMask |
                          KeyReleaseMask | EnterWindowMask | LeaveWindowMask |
                          PointerMotionMask | Button1MotionMask | VisibilityChangeMask |
                          ColormapChangeMask );
    */

    p = NewGuiWindowHandle();
    
    p->_guiSystem = _guiSystemX11;
    p->_xwindow = xwindow;
    p->_width = width;
    p->_height = height;
    p->_depth = depth;
    p->_function = eventFunction;
    p->_userData = userData;

//-- set min / max
    int maxWidth = (xscreen ? xscreen->width : width);
    int maxHeight = (xscreen ? xscreen->height : height);

    {
        //TODO min should be application defined, max also but with limit to display resolution
        XSizeHints *sizeHints = XAllocSizeHints();

        sizeHints->flags = PMinSize | PMaxSize;
        sizeHints->min_width = 200;
        sizeHints->min_height = 100;
        sizeHints->max_width = maxWidth;
        sizeHints->max_height = maxHeight;
        XSetWMNormalHints( xdisplay ,xwindow ,sizeHints );

        XFree(sizeHints);
    }

    LinuxAddWindowToThread( xwindow ,p );
    XMapWindow( xdisplay ,xwindow );

//-- setup drawing context
    p->_xpixmap = XCreatePixmap( xdisplay ,xwindow ,maxWidth ,maxHeight ,depth );
    p->_xgc = XCreateGC( xdisplay ,p->_xpixmap ,0 ,0 );
    p->_xcolormap = DefaultColormap( xdisplay ,0 );
    p->_xfont = XLoadQueryFont( xdisplay ,OS_LINUX_DEFAULTFONTNAME );
    
    XSetFont( xdisplay ,p->_xgc ,p->_xfont->fid );

    *handle = (OsHandle) p;

//-- notify app of creation
    struct OsEventMessage eventMessage;
    
    eventMessage.eventType = osExecuteEvent;
	eventMessage.executeMessage.executeAction = osExecuteStart;

	p->_function( &eventMessage ,p->_userData );

	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
//! Window common

OsError _linux_x11window_destroy( OsHandle *handle ) {
    struct GuiWindowHandle *p = CastGuiWindowHandle( handle );

    Display *xdisplay = _linux_peekdisplay();

    if( p == NULL || xdisplay == NULL ) return EBADMSG;

    XDestroyWindow( xdisplay ,p->_xwindow );
}

//-- state
void _X11GetClientArea( OsHandle handle ,struct OsPoint *size ) {
	struct GuiWindowHandle *p = CastGuiWindowHandle( handle );

    Display *xdisplay = _linux_peekdisplay();

    XWindowAttributes xattributes;
    
	if( p == NULL || size == NULL || xdisplay == NULL ) return;

    int xstatus = XGetWindowAttributes( xdisplay ,p->_xwindow ,&xattributes );

    if( xstatus == 0 ) return;

    size->x = xattributes.width;
    size->y = xattributes.height;

        // border_width ?
}

//-- visibility
void _X11WindowRefresh( OsHandle handle ,const struct OsRect *updateRect,int update ) {
	struct GuiWindowHandle *p = CastGuiWindowHandle( handle );

    Display *xdisplay = _linux_peekdisplay();
    
    XEvent xevent;
    
	if( p == NULL || xdisplay == NULL ) return;

    if( (update & OS_REFRESH_RESIZED) != 0 )
        p->_resized++;

    xevent.type = Expose;
    xevent.xexpose.window = p->_xwindow;

    xevent.xexpose.count = 0;
    xevent.xexpose.x = (updateRect) ? updateRect->left : 0;
    xevent.xexpose.y = (updateRect) ? updateRect->top : 0;
    xevent.xexpose.width = (updateRect) ? updateRect->right - updateRect->left : p->_width;
    xevent.xexpose.height = (updateRect) ? updateRect->bottom  - updateRect->top  : p->_height;

    XSendEvent( xdisplay ,p->_xwindow ,False ,ExposureMask ,&xevent );
    XFlush( xdisplay );
}

OsError _X11SwapBuffers( OsHandle *handle ) {
	struct GuiWindowHandle *p = CastGuiWindowHandle( handle );

	Display *xdisplay = _linux_getdisplay();

	XCopyArea( xdisplay ,p->_xpixmap ,p->_xwindow ,p->_xgc ,0 ,0 ,p->_width ,p->_height ,0 ,0 );
	XFlush( xdisplay );

	return ENOERROR;
}

void _X11WindowShow( OsHandle handle ,int visible ) {
	struct GuiWindowHandle *p = CastGuiWindowHandle( handle );

	if( p == NULL ) return;

//	ShowWindow( p->_handle ,(visible != 0) ? SW_HIDE : SW_SHOW ); //? SW_RESTORE
}

///-- cursor
#include <X11/Xcursor/Xcursor.h>

//! @note see https://tronche.com/gui/x/xlib/appendix/b/

unsigned int _x11_mapCursorId( int cursorId ) {
    switch( cursorId ) {
        case OS_CURSOR_NOCURSOR:
            return 0;

        default:
        case OS_CURSOR_ARROW:
            return 2;
        case OS_CURSOR_WAIT:
            return 150;
        case OS_CURSOR_CROSS:
            return 0;
        case OS_CURSOR_BEAM:
            return 86;
        case OS_CURSOR_CANNOT:
            return 0;
        case OS_CURSOR_UPARROW:
            return 6;
        case OS_CURSOR_SIZEALL:
            return 52;
        case OS_CURSOR_SIZETOPLEFT:
            return 134;
        case OS_CURSOR_SIZETOPRIGHT:
            return 136;
        case OS_CURSOR_SIZEWIDTH:
            return 108;
        case OS_CURSOR_SIZEHEIGHT:
            return 116;
    }
}

static Cursor x11_cursors[10] = {
    0 ,0 ,0 ,0 ,0
    ,0 ,0 ,0 ,0 ,0
};

void _X11MouseSetCursor( OsHandle handle ,int cursorId ) {
    struct GuiWindowHandle *p = CastGuiWindowHandle( handle );

    Display *xdisplay = _linux_peekdisplay();

    if( p == NULL || xdisplay == NULL ) return;

    if( cursorId == 0 ) {
        XUndefineCursor( xdisplay ,p->_xwindow );
        return;
    }

    cursorId = MIN( cursorId ,9 );
    if( x11_cursors[cursorId] == 0 ) {
        x11_cursors[cursorId] = XCreateFontCursor( xdisplay ,_x11_mapCursorId(cursorId) );
    }

    XDefineCursor( xdisplay ,p->_xwindow ,x11_cursors[cursorId] );

    XFlush(xdisplay);
}

void _X11MouseCapture( OsHandle handle ) {
    struct GuiWindowHandle *p = CastGuiWindowHandle( handle );

    Display *xdisplay = _linux_peekdisplay();

    if( p == NULL || xdisplay == NULL ) return;

    long eventMask =
        ButtonPressMask |
        ButtonReleaseMask |
        PointerMotionMask |
        FocusChangeMask |
        EnterWindowMask |
        LeaveWindowMask
    ;

    XGrabPointer( xdisplay ,p->_xwindow ,True ,eventMask
        ,GrabModeAsync ,GrabModeAsync
        ,p->_xwindow ,None /*cursor*/ ,CurrentTime
    );
}

void _X11MouseRelease( OsHandle handle ) {
    struct GuiWindowHandle *p = CastGuiWindowHandle( handle );

    Display *xdisplay = _linux_peekdisplay();

    if( p == NULL || xdisplay == NULL ) return;

    XUngrabPointer( xdisplay ,CurrentTime );
    XFlush( xdisplay );
}

//////////////////////////////////////////////////////////////////////////
void _X11SetForeColor( OsGuiContext context ,OsColorRef c ) {
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	struct OsColor *C = (struct OsColor*) &c;

    assert( osContext != NULL && osContext->_xdisplay != NULL && osContext->_xgc != 0 );
    
    osContext->_xforecolor.red = C->r * 256;
    osContext->_xforecolor.green = C->g * 256;
    osContext->_xforecolor.blue = C->b * 256;
    osContext->_xforecolor.flags = DoRed|DoGreen|DoBlue;
    
    XAllocColor( osContext->_xdisplay ,osContext->_hwindow->_xcolormap ,&osContext->_xforecolor );
    
    XSetForeground( osContext->_xdisplay ,osContext->_xgc ,osContext->_xforecolor.pixel );
}

void _X11SetFillColor( OsGuiContext context ,OsColorRef c ) {
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	struct OsColor *C = (struct OsColor*) &c;

    assert( osContext != NULL && osContext->_xdisplay );
    
    osContext->_xfillcolor.red = C->r * 256;
    osContext->_xfillcolor.green = C->g * 256;
    osContext->_xfillcolor.blue = C->b * 256;
    osContext->_xfillcolor.flags = DoRed|DoGreen|DoBlue;
    
    XAllocColor( osContext->_xdisplay ,osContext->_hwindow->_xcolormap ,&osContext->_xfillcolor );
    
    osContext->_xnofill = (C->a == 0) ? True : False;
}

///-- text color
void _X11SetTextColor( OsGuiContext context ,OsColorRef c ) {
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	struct OsColor *C = (struct OsColor*) &c;

    assert( osContext != NULL && osContext->_xdisplay != NULL );
    
    osContext->_xtextcolor.red = C->r * 256;
    osContext->_xtextcolor.green = C->g * 256;
    osContext->_xtextcolor.blue = C->b * 256;
    osContext->_xtextcolor.flags = DoRed|DoGreen|DoBlue;
    
    XAllocColor( osContext->_xdisplay ,osContext->_hwindow->_xcolormap ,&osContext->_xtextcolor );
}

void _X11SetBackColor( OsGuiContext context ,OsColorRef c ) {
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	struct OsColor *C = (struct OsColor*) &c;

    assert( osContext != NULL && osContext->_xdisplay != NULL && osContext->_xgc != 0 );
    
    osContext->_xbackcolor.red = C->r * 256;
    osContext->_xbackcolor.green = C->g * 256;
    osContext->_xbackcolor.blue = C->b * 256;
    osContext->_xbackcolor.flags = DoRed|DoGreen|DoBlue;
    
    XAllocColor( osContext->_xdisplay ,osContext->_hwindow->_xcolormap ,&osContext->_xbackcolor );
    
    osContext->_xnoback = (C->a == 0) ? True : False;
    
    XSetBackground( osContext->_xdisplay ,osContext->_xgc ,osContext->_xbackcolor.pixel );
}

void _X11SetColor( OsGuiContext context ,int selectColor ,OsColorRef color ) {
	switch( selectColor )
	{
	case OS_SELECT_FORECOLOR: _X11SetForeColor( context ,color ); break;
	case OS_SELECT_FILLCOLOR: _X11SetFillColor( context ,color ); break;
	case OS_SELECT_TEXTCOLOR: _X11SetTextColor( context ,color ); break;
	case OS_SELECT_BACKCOLOR: _X11SetBackColor( context ,color ); break;
	default: break;
	}
}

//////////////////////////////////////////////////////////////////////////
#define _TX(_x_)	(osContext->_offsetx+(int)((_x_)*osContext->_scalex))
#define _TY(_y_)	(osContext->_offsety+(int)((_y_)*osContext->_scaley))

void _X11RegionSetArea( OsGuiContext context ,int left ,int top ,int right ,int bottom ,int useOffset )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;
    	
    assert( osContext != NULL && osContext->_xdisplay != NULL && osContext->_xgc != 0 );
        
    XRectangle rect;

    rect.x = (short) left;
    rect.y = (short) top;
    rect.width = (short) (right - left +1); //! NB edge inclusive
    rect.height = (short) (bottom - top +1);

    int status = XSetClipRectangles( osContext->_xdisplay ,osContext->_xgc ,0 ,0 ,&rect ,1 ,Unsorted );

    if( status == 0 ) return; //! failed

    osContext->_region.left = left;
    osContext->_region.top = top;
    osContext->_region.right = right;
    osContext->_region.bottom = bottom;

    if( useOffset ) {
        osContext->_offsetx = left;
        osContext->_offsety = top;
    }
}

void _X11RegionGetArea( OsGuiContext context ,struct OsRect *area )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	assert( osContext != NULL && osContext->_xdisplay != NULL && osContext->_xgc != 0 );

	if( area != NULL )
	{
		area->left = osContext->_region.left;
		area->top = osContext->_region.top;
		area->right = osContext->_region.right;
		area->bottom = osContext->_region.bottom;
	}
}

void _X11RegionSetOffset( OsGuiContext context ,int x ,int y ) {
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	assert( osContext != NULL && osContext->_xdisplay != NULL && osContext->_xgc != 0 );

	osContext->_offsetx = x;
	osContext->_offsety = y;
}

void _X11RegionSetScale( OsGuiContext context ,float x ,float y ) {
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	assert( osContext != NULL && osContext->_xdisplay != NULL && osContext->_xgc != 0 );

	osContext->_scalex = x;
	osContext->_scaley = y;
}

void _X11RegionGetSize( OsGuiContext context ,int *width ,int *height ) {
	_ASSERT( 0 );
}

void _X11PointToCoords( OsGuiContext context ,int surfaceForP ,const struct OsPoint *p ,int surfaceForCoords ,struct OsPoint *coords ) {
    struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

    if( !p || !coords ) return;

    //TODO what is surfaceForP ??

    coords[0].x = _TX(p[0].x);
    coords[0].y = _TY(p[0].y);

	_ASSERT( 0 );
}

//////////////////////////////////////////////////////////////////////////
#define GUIFONTHANDLE_MAGIC	0x06F5CFC35 // 0x04D7EFC35

struct GuiFontHandle
{
	uint32_t _magic;

	struct OsGuiSystemTable *_guiSystem;

    XFontStruct *_xfont;
};

static struct GuiFontHandle *NewGuiFontHandle( void )
{
	const size_t size = sizeof(struct GuiFontHandle);

	struct GuiFontHandle *p = (struct GuiFontHandle*) malloc(size);

	if( p == NULL ) return NULL;

	memset( p ,0 ,size );

	p->_magic = GUIFONTHANDLE_MAGIC;

	return p;
}

static struct GuiFontHandle *CastGuiFontHandle( OsHandle handle )
{
	struct GuiFontHandle *p = (struct GuiFontHandle *) handle;

	if( p == NULL ) return NULL;

	if( p->_magic != GUIFONTHANDLE_MAGIC ) return NULL;

	return p;
}

//--
XFontStruct *XLoadQueryScalableFont( Display *dpy ,int screen ,char *name ,int size )
{
    int i,j, field;
    char newname[500];    /* big enough for a long font name */
    int res_x, res_y;     /* resolution values for this screen */

    /* catch obvious errors */
    if ((name == NULL) || (name[0] != '-')) return NULL;

    /* calculate our screen resolution in dots per inch. 25.4mm = 1 inch */
    res_x = DisplayWidth(dpy, screen)/(DisplayWidthMM(dpy, screen)/25.4);
    res_y = DisplayHeight(dpy, screen)/(DisplayHeightMM(dpy, screen)/25.4);

    /* copy the font name, changing the scalable fields as we do so */
    for(i = j = field = 0; name[i] != '\0' && field <= 14; i++) {
        newname[j++] = name[i];
        if (name[i] == '-') {
            field++;
            switch(field) {
            case 7:  /* pixel size */
            case 12: /* average width */
                /* change from "-0-" to "-*-" */
                newname[j] = '*';
                j++;
                if (name[i+1] != '\0') i++;
                break;
            case 8:  /* point size */
                /* change from "-0-" to "-<size>-" */
                sprintf(&newname[j], "%d", size);
                while (newname[j] != '\0') j++;
                if (name[i+1] != '\0') i++;
                break;
            case 9:  /* x-resolution */
            case 10: /* y-resolution */
                /* change from an unspecified resolution to res_x or res_y */
                sprintf(&newname[j], "%d", (field == 9) ? res_x : res_y);
                while(newname[j] != '\0') j++;
                while((name[i+1] != '-') && (name[i+1] != '\0')) i++;
                break;
            }
        }
    }
    newname[j] = '\0';

    /* if there aren't 14 hyphens, it isn't a well formed name */
    if (field != 14) return NULL;

    return XLoadQueryFont(dpy, newname);
}

// static int _win_fontpitch_map[] = { DEFAULT_PITCH ,FIXED_PITCH ,VARIABLE_PITCH };

// static char *_linux_fontweight_map[] = { "*" ,"thin" ,"ultralight" ,"light" ,"normal" ,"medium" ,"semibold" ,"bold" ,"ultrabold" ,"heavy" };
static char *_linux_fontweight_map[] = { "*" ,"thin" ,"ultralight" ,"medium" ,"medium" ,"medium" ,"semibold" ,"bold" ,"ultrabold" ,"heavy" }; // temp
// FW_DONTCARE ,FW_THIN ,FW_ULTRALIGHT ,FW_LIGHT ,FW_NORMAL ,FW_MEDIUM ,FW_SEMIBOLD ,FW_BOLD ,FW_ULTRABOLD ,FW_HEAVY };

// #define HASFONTSTYLE(__p_,__style_)		(((__p_&__style_)!=0)?TRUE:FALSE)

OsError _X11FontCreate( OsHandle *handle ,const char_t *faceName ,int pointSize ,int weight ,int style ,int family )
{
	struct GuiFontHandle *p = NewGuiFontHandle();
    
    Display *xdisplay = _linux_getdisplay();

/*
    int fontCount;
    char **font_list = XListFonts( xdisplay ,"-*-*-*" ,256 ,&fontCount );

    XFontStruct *font = XLoadQueryFont( xdisplay ,"*" );

    // char *fontFaceName = stricmp( faceName ,"default" )
  */

    char fontName[256] = "10x20";
    
    char *fontWeight = _linux_fontweight_map[ min(weight,SIZEOFARRAY(_linux_fontweight_map)) ];
    
    // sprintf( fontName ,"*%s-%s-o-normal--%d*" ,faceName ,fontWeight ,pointSize );
    // sprintf( fontName ,"*%s-%s-r-normal--*-%d*-iso8859-1" ,"fixed" ,fontWeight ,pointSize*10 );
    // sprintf( fontName ,"*%s-%s-r-normal--*-%d*-iso8859-1" ,"fixed" ,fontWeight ,pointSize*10 );
    // sprintf( fontName ,"-*-%s-%s-r-normal--0-0-*-*-*-0-iso8859-1" ,"fixed" ,fontWeight );

    sprintf( fontName ,"-*-%s*-%s-r-normal--0-0-*-*-*-0-*-*" ,(faceName && faceName[0] ? faceName : "clean") ,fontWeight );
    
    // int count = 0;
    
    // char **list = XListFonts( xdisplay ,"*courier*-r-normal--0-0-*-*-*-0-iso8859-*" ,256 ,&count );
    
    p->_guiSystem = _guiSystemX11;
    
    p->_xfont = XLoadQueryScalableFont( xdisplay ,0 ,fontName ,pointSize*7 );

//  XLoadQueryFont( xdisplay ,fontName );
    
    // if( )
    assert( p->_xfont != NULL );
    
/*
	HFONT hfont = NULL;

	int pitchAndFamilly = _win_fontpitch_map[family%SIZEOFARRAY(_win_fontpitch_map)];

	int fontWeight = _win_fontweight_map[ min(weight,LASTOFARRAY(_win_fontweight_map)) ];

	if( p == NULL ) return ENOMEM;

	hfont = CreateFont( pointSize,0,0,0 ,fontWeight 
		,HASFONTSTYLE(style,OS_FONTSTYLE_ITALIC) ,HASFONTSTYLE(style,OS_FONTSTYLE_UNDERLINE) ,HASFONTSTYLE(style,OS_FONTSTYLE_STRIKEOUT)
		,DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY
		,pitchAndFamilly ,faceName
		);

	if( hfont == NULL )
	{
		free( p );

		return GetLastOsError();
	}

	p->_handle = hfont; */

    *handle = (OsHandle) p;
    
	return ENOERROR;
}

void _X11FontCalcSize( OsHandle handle ,const char_t *text ,struct OsPoint *size )
{
    struct GuiFontHandle *p = CastGuiFontHandle( handle );

    if( p == NULL || size == NULL || p->_xfont == NULL ) return;

    XCharStruct toverall;

    int tdirection ,tascent ,tdescent;

    XTextExtents( p->_xfont ,text ,strlen(text) ,&tdirection ,&tascent ,&tdescent ,&toverall );

    size->x = toverall.width;
    size->y = toverall.ascent + toverall.descent;
}

void _X11SetFont( OsGuiContext context ,OsHandle fontHandle )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	struct GuiFontHandle *p = CastGuiFontHandle( fontHandle );

	if( p == NULL || p->_xfont == NULL ) return;

    XSetFont( osContext->_xdisplay ,osContext->_xgc ,p->_xfont->fid );
    
    osContext->_currentFont = p;
}

static struct GuiImageHandle *NewGuiImageHandle( void )
{
	const size_t size = sizeof(struct GuiImageHandle);

	struct GuiImageHandle *p = (struct GuiImageHandle*) malloc(size);

	if( p == NULL ) return NULL;

	memset( p ,0 ,size );

	p->_magic = GUIIMAGEHANDLE_MAGIC;
	
	p->_guiSystem = _guiSystemX11;

	return p;
}

static struct GuiImageHandle *CastGuiImageHandle( OsHandle handle )
{
	struct GuiImageHandle *p = (struct GuiImageHandle *) handle;

	if( p == NULL ) return NULL;

	if( p->_magic != GUIIMAGEHANDLE_MAGIC ) return NULL;
	
	p->_guiSystem = _guiSystemX11;

	return p;
}

//////////////////////////////////////////////////////////////////////////
OsError _X11ImageCreate( OsHandle *handle ,int width ,int height )
{
	struct GuiImageHandle *p = NULL;

    Display *xdisplay = _linux_getdisplay();
    
    // XImage *ximage;
    
    // void *data;
    
    if( handle == NULL ) return EINVAL;
    
    if( *handle != OS_INVALID_HANDLE ) return EEXIST;
    
    p = NewGuiImageHandle();
    
    if( p == NULL ) return ENOMEM;
	
    int rootDepth = DefaultDepth( xdisplay ,DefaultScreen(xdisplay) );
    
	p->_xpixmap = XCreatePixmap( xdisplay ,DefaultRootWindow(xdisplay) ,width ,height ,rootDepth );
    
    // data = malloc( width*3*height );
    // ximage = XCreateImage( xdisplay ,CopyFromParent ,24 ,ZPixmap ,0 ,data ,width ,height ,32 ,0 );
    
    if( p->_xpixmap == 0 )
    {
         //? get status 
         free( p );
         
         return EFAILED;
    }
    
    p->_width = width; p->_height = height;
    
    *handle = (OsHandle) p;
    
	return ENOERROR;
}

void _X11ImageGetInfo( OsHandle handle ,struct OsGuiImageInfo *info )
{
	struct GuiImageHandle *p = CastGuiImageHandle( handle );

//	_ASSERT( p != NULL && info != NULL );

	if( p == NULL || info == NULL ) return;

	info->width = (int) p->_width;
	info->height = (int) p->_height;
}

OsError _X11ImageWrite( OsHandle handle ,const uint8_t *pixelValues ,size_t size )
{
	struct GuiImageHandle *p = CastGuiImageHandle( handle );

    Display *xdisplay = _linux_getdisplay();

	int rootDepth = DefaultDepth( xdisplay ,DefaultScreen(xdisplay) );
	
    XImage ximage; // ,*pxi;
    
    int ierror = 0;
    
	if( p == NULL || pixelValues == NULL ) return EINVAL;
    
    memset( &ximage ,0 ,sizeof(ximage) );
    
    ximage.width = p->_width;
    ximage.height = p->_height;
    ximage.format = ZPixmap;
    ximage.data = (char*) pixelValues;
    ximage.byte_order = LSBFirst;
    ximage.bitmap_bit_order = LSBFirst;
    ximage.bitmap_pad = 32;
    ximage.depth = rootDepth;
    ximage.bytes_per_line = p->_width * 3;
    ximage.bits_per_pixel = 24;
    ximage.red_mask = 0x0ff0000;
    ximage.blue_mask = 0x0ff00;
    ximage.green_mask = 0x0ff;
    
    ierror = XInitImage( &ximage );
    
    if( ierror == 0 )
         return EFAILED;
    
    // DefaultVisual(xdiplay,0) 
    // pxi = XCreateImage( xdisplay ,CopyFromParent ,24 ,ZPixmap ,0 ,pixelValues ,p->_width ,p->_height ,24 ,0 );
    // pxi = XCreateImage( xdisplay ,CopyFromParent ,24 ,ZPixmap ,0 ,data ,width ,height ,24 ,0 );
    
    GC gc = XCreateGC( xdisplay ,p->_xpixmap ,0 ,NULL );
    
    ierror = XPutImage( xdisplay ,p->_xpixmap ,gc ,&ximage ,0,0 ,0,0 ,p->_width,p->_height );
    // ierror = XPutImage( xdisplay ,p->_xpixmap ,gc ,pxi ,0,0 ,0,0 ,p->_width,p->_height );
    
    XFreeGC( xdisplay ,gc );
    
    // XDestroyImage( pxi );
    
    if( ierror == BadAlloc )
        return ENOMEM;
        
    return ENOERROR;
}

void _X11SetAlphaBlend( OsGuiContext context ,float alpha )
{
/*	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	_ASSERT( osContext != NULL && osContext->_hdc != NULL );

	osContext->_blendfunction.BlendOp = AC_SRC_OVER;
	osContext->_blendfunction.BlendFlags = 0;
	osContext->_blendfunction.AlphaFormat = 0;
	osContext->_blendfunction.SourceConstantAlpha = (alpha < 1.0) ? ((alpha > 0) ? (BYTE) (255*alpha) : 0) : 255;*/
}

//-- render

// XImage *g_ximage = NULL;

void _X11DrawImage( OsGuiContext context ,int left ,int top ,int right ,int bottom ,OsHandle handle ,int imageLeft ,int imageTop ,int imageRight ,int imageBottom )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	struct GuiImageHandle *p = CastGuiImageHandle( handle );
 
	int destWidth = right - left;
	int destHeight = bottom - top;
	int srcWidth = imageRight - imageLeft;
	int srcHeight = imageBottom - imageTop;

	assert( osContext != NULL );

	if( p == NULL ) return;

    Display *xdisplay = osContext->_xdisplay;

	if( right < 0 ) destWidth = p->_width; else if( left > right ) left = right ,destWidth = -destWidth;
	if( bottom < 0 ) destHeight = p->_height; else if( top > bottom ) top = bottom ,destHeight = -destHeight;
	if( imageRight < 0 ) srcWidth = p->_width; else if( imageLeft > imageRight ) imageLeft = imageRight ,srcWidth = -srcWidth;
	if( imageBottom < 0 ) srcHeight = p->_height; else if( imageTop > imageBottom ) imageTop = imageBottom ,srcHeight = -srcHeight;

    /* if( imageTop > 0 ) {
        imageTop++;
    } */

    srcWidth = MIN( srcWidth ,p->_width - imageLeft );
    srcHeight = MIN( srcHeight ,p->_height - imageTop );

	if( 0 ) // osContext->_blendfunction.SourceConstantAlpha != 255 )
		{} // AlphaBlend( osContext->_hdc ,left ,top ,destWidth ,destHeight ,p->_hdc ,imageLeft ,imageTop ,srcWidth ,srcHeight ,osContext->_blendfunction );

	else if( destWidth != srcWidth || destHeight != srcHeight ) {
        //TODO blitMode = stretch/clip

        XCopyArea( xdisplay ,p->_xpixmap ,osContext->_xdrawable ,osContext->_xgc ,imageLeft ,imageTop ,srcWidth ,srcHeight ,_TX(left) ,_TY(top) );
    }

	else {
        XCopyArea( xdisplay ,p->_xpixmap ,osContext->_xdrawable ,osContext->_xgc ,imageLeft ,imageTop ,srcWidth ,srcHeight ,_TX(left) ,_TY(top) );
    }
}

//-- shape
void _X11DrawRectangle( OsGuiContext context ,int left ,int top ,int right ,int bottom )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

    int width = (right - left) * osContext->_scalex;
    int height = (bottom - top) * osContext->_scaley;
    
    if( width < 1 || height < 1 ) return;
    
    assert( osContext != NULL && osContext->_hwindow != NULL && osContext->_hwindow->_xgc != 0 );
       
    //! filling 
    if( !osContext->_xnofill && width > 2 && height > 2 )
    {
        XSetForeground( osContext->_xdisplay ,osContext->_xgc ,osContext->_xfillcolor.pixel );
    
        XFillRectangle( osContext->_xdisplay ,osContext->_xdrawable ,osContext->_xgc
            ,_TX(left+1) ,_TY(top+1) ,(width-1) ,(height-1)
        );

        XSetForeground( osContext->_xdisplay ,osContext->_xgc ,osContext->_xforecolor.pixel );
    } 
    
    //! border 
    XDrawRectangle( osContext->_xdisplay ,osContext->_xdrawable ,osContext->_xgc
        ,_TX(left) ,_TY(top) ,width ,height
    );
}

void _X11DrawEllipse( OsGuiContext context ,int left ,int top ,int right ,int bottom ) {
    struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

    int width = (right - left) * osContext->_scalex;
    int height = (bottom - top) * osContext->_scaley;

    if( width < 1 || height < 1 ) return;

    assert( osContext != NULL && osContext->_hwindow != NULL && osContext->_hwindow->_xgc != 0 );

    //! filling
    if( !osContext->_xnofill && width > 2 && height > 2 )
    {
        XSetForeground( osContext->_xdisplay ,osContext->_xgc ,osContext->_xfillcolor.pixel );

        XFillArc( osContext->_xdisplay ,osContext->_xdrawable ,osContext->_xgc
            ,_TX(left+1) ,_TY(top+1) ,width-2 ,height-2
            ,0 ,360*64
        );

        XSetForeground( osContext->_xdisplay ,osContext->_xgc ,osContext->_xforecolor.pixel );
    }

    XDrawArc( osContext->_xdisplay ,osContext->_xdrawable ,osContext->_xgc
        ,_TX(left) ,_TY(top) ,width ,height
        ,0 ,360*64
    );
}

void _X11DrawLine( OsGuiContext context ,int left ,int top ,int right ,int bottom )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

    assert( osContext != NULL && osContext->_hwindow != NULL && osContext->_hwindow->_xgc != 0 );
    
    XDrawLine( osContext->_xdisplay ,osContext->_xdrawable ,osContext->_xgc
        ,_TX(left) ,_TY(top) ,_TX(right) ,_TY(bottom)
    );
}

void _X11DrawText( OsGuiContext context ,const char_t *text ,int left ,int top ,int right ,int bottom ,int align ,struct OsRect *rect )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

    assert( osContext != NULL && osContext->_hwindow != NULL && osContext->_hwindow->_xgc != 0 );
    
    XSetForeground( osContext->_xdisplay ,osContext->_xgc ,osContext->_xtextcolor.pixel );
    XSetBackground( osContext->_xdisplay ,osContext->_xgc ,osContext->_xbackcolor.pixel );
    
    XCharStruct toverall;
    
    int tdirection ,tascent ,tdescent;
    
    XFontStruct *xfont = (osContext->_currentFont != NULL) ? osContext->_currentFont->_xfont : osContext->_hwindow->_xfont;

    XTextExtents( xfont ,text ,strlen(text) ,&tdirection ,&tascent ,&tdescent ,&toverall );

    int x = left ,y = top;
    
    if( align & OS_ALIGN_CENTERH ) x = left + (right - left - toverall.width) / 2;
    else if( align & OS_ALIGN_RIGHT ) x = right - toverall.width;
    
    if( align & OS_ALIGN_CENTERV ) y = top + (bottom - top - toverall.ascent) / 2;
    else if( align & OS_ALIGN_BOTTOM ) y = bottom - toverall.ascent - toverall.descent;
    
    if( osContext->_xnoback )
    {
        XDrawString( osContext->_xdisplay ,osContext->_xdrawable ,osContext->_xgc
            ,_TX(x) ,_TY(y+toverall.ascent) ,text ,strlen(text) );
    }
    else
    {
        XDrawImageString( osContext->_xdisplay ,osContext->_xdrawable ,osContext->_xgc
            ,_TX(x) ,_TY(y+toverall.ascent) ,text ,strlen(text) );
    }
    
    XSetForeground( osContext->_xdisplay ,osContext->_xgc ,osContext->_xfillcolor.pixel );

    if( rect ) {
        rect->left = left;
        rect->top = top;
        rect->right = rect->left + toverall.width;
        rect->bottom = rect->top + toverall.ascent + toverall.descent;
    }
}

#define POLY_MAX_POINTS 256

struct XPointStub { short x,y; }; //! not defined for some reason

void _X11DrawPolygon( OsGuiContext context ,int npoints ,const struct OsPoint *points )
{
    struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

    assert( osContext != NULL && osContext->_hwindow != NULL && osContext->_hwindow->_xgc != 0 );

    struct XPointStub xpoints[POLY_MAX_POINTS];

    int n = MIN( npoints ,POLY_MAX_POINTS );

    for( int i=0; i<n; ++i ) {
        xpoints[i].x = _TX(points[i].x);
        xpoints[i].y = _TX(points[i].y);
    }

    if( osContext->_xnofill ) {
        XDrawLines( osContext->_xdisplay ,osContext->_xdrawable ,osContext->_xgc ,(XPoint*) xpoints ,n ,CoordModeOrigin );
    } else {
        XFillPolygon( osContext->_xdisplay ,osContext->_xdrawable ,osContext->_xgc ,(XPoint*) xpoints ,n ,Nonconvex ,CoordModeOrigin );
    }
}

//-- resource
int _X11ResourceGetType( OsHandle handle )
{
	struct GuiImageHandle *p = CastGuiImageHandle( handle );

	if( p != NULL ) return OS_GUIRESOURCETYPE_IMAGE;

//	_ASSERT( FALSE );

	return 0;
}

OsError _X11ResourceLoadFromMemory( OsHandle *handle ,int resourceTypeHint ,uint8_t *memory ,const void *resourceInfo )
{
	assert( 0 );
    
	return ENOSYS;
}

/*OsError _win_loadimage( OsHandle *handle ,HINSTANCE hinstance ,const char_t *name ,UINT fLoad )
{
	struct GuiImageHandle *p = NewGuiImageHandle();

	if( p == NULL ) return ENOMEM;

	p->_hdc = CreateCompatibleDC(NULL);

	if( p->_hdc == NULL )
	{
		free( p );

		return ENOMEM;
	}

	p->_handle = LoadImage( hinstance ,name ,IMAGE_BITMAP ,0,0 ,fLoad );

	if( p->_handle == NULL )
	{
		free( p );

		return ENOENT;
	}

	GetObject( p->_handle ,sizeof(p->_bm) ,&p->_bm );

	SelectObject( p->_hdc ,p->_handle );

	*handle = (OsHandle) p;

	return ENOERROR;
}*/

/* OsError imlib2_loadImage( Display *xdisplay ,const char *filename ) {
    Imlib_Image img = imlib_load_image(filename);

    if( !img ) {
        return EFAILED;
    }

    int width, height;

    imlib_context_set_image(img);
    width = imlib_image_get_width();
    height = imlib_image_get_height();

    Screen *xscreen = DefaultScreenOfDisplay(xdisplay);
    Window root = DefaultRootWindow(xdisplay);

    Pixmap pix = XCreatePixmap( xdisplay ,root ,width ,height ,DefaultDepthOfScreen(xscreen) );

    imlib_context_set_display(xdisplay);
    imlib_context_set_visual(DefaultVisualOfScreen(xscreen));
    imlib_context_set_colormap(DefaultColormapOfScreen(xscreen));
    imlib_context_set_drawable(pix);

    imlib_render_image_on_drawable(0, 0);

    / * XSetWindowBackgroundPixmap(dpy, root, pix);
    XClearWindow(dpy, root);

    while (XPending(dpy)) {
        XEvent ev;
        XNextEvent(dpy, &ev);
    }
     * /

    // XFreePixmap( xdisplay ,pix );
    imlib_free_image();

    return ENOERROR;
} */


OsError read_png( char *filename ) {
    // png_infop info_ptr;  // <-- Global info_ptr (good)
    png_bytepp row_pointers;

    FILE *fp = fopen( filename ,"rb" );

    if( fp == NULL )
        return EFAILED;

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    png_infop info_ptr = png_create_info_struct(png_ptr);  // <-- creating a new, local info_ptr

    png_init_io(png_ptr, fp);

    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    row_pointers = png_get_rows(png_ptr, info_ptr);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL); // <-- destroying the info_ptr (as well).

    fclose(fp);
}

static void TeardownPng( png_structp png ,png_infop info ) {

    if( png ) {
        png_infop *realInfo = (info? &info: NULL);

        png_destroy_read_struct (&png, realInfo, NULL);
    }
}

void LoadPng( FILE *file ,unsigned char **data ,char **clipData ,unsigned int *width ,unsigned int *height ,unsigned int *rowbytes ) {
    size_t size = 0,  clipSize = 0;

    png_structp png = NULL;
    png_infop info = NULL;

    unsigned char **rowPointers = NULL;

    int depth = 0,
            colortype = 0,
            interlace = 0,
            compression = 0,
            filter = 0
        ;

    unsigned clipRowbytes = 0;

    png = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    info = png_create_info_struct (png);

    png_init_io (png, file);

    png_read_info (png, info);

    png_get_IHDR (png, info, (png_uint_32*) width, (png_uint_32*) height, &depth, &colortype, &interlace, &compression, &filter);

    *rowbytes = png_get_rowbytes (png, info);

    if (colortype == PNG_COLOR_TYPE_RGB) {

        // X hates 24bit images - pad to RGBA
        png_set_filler (png, 0xff, PNG_FILLER_AFTER);

        *rowbytes = (*rowbytes * 4) / 3;

    }

    png_set_bgr (png);

    *width = png_get_image_width (png, info);

    *height = png_get_image_height (png, info);

    size = *height * *rowbytes;

    clipRowbytes = *rowbytes/32;

    if (*rowbytes % 32)

        ++clipRowbytes;

    clipSize = clipRowbytes * *height;

    // This gets freed by XDestroyImage

    *data = (unsigned char*) malloc( sizeof (png_byte) * size );

    rowPointers = (unsigned char**) malloc( *height * sizeof(unsigned char*) );

    png_bytep cursor = *data;

    int i=0,x=0,y=0;

    for( i=0; i<*height; ++i, cursor += *rowbytes ) {
        rowPointers[i] = cursor;
    }

    png_read_image( png ,rowPointers );

    *clipData = (char*) calloc ( clipSize ,sizeof(unsigned char) );

    if( colortype == PNG_COLOR_TYPE_RGB ) {
        memset( *clipData ,0xff ,clipSize );
    } else {
        // Set up bitmask for clipping fully transparent areas

        for( y=0; y<*height; ++y, cursor+=*rowbytes ) {
            for( x=0; x<*rowbytes; x+=4 ) {

                // Set bit in mask when alpha channel is nonzero

                if( rowPointers[y][x+3] )
                    (*clipData)[(y*clipRowbytes) + (x/32)] |= (1 << ((x/4)%8));
            }
        }
    }

    if( png ) {
        png_infop *realInfo = (info ? &info : NULL);
        png_destroy_read_struct( &png ,realInfo ,NULL );
    }

    // TeardownPng (png, info);

    // free( clipData );
    free( rowPointers );
}

/* OsError _linux_loadimage( OsHandle *handle ,HINSTANCE hinstance ,const char_t *name ,UINT fLoad )
{


    return ENOERROR;
} */

// XImage *g_ximage = NULL;

TINYFUN OsError _X11ResourceLoadFromFile( OsHandle *handle ,int resourceTypeHint ,const char_t *filename )
{
	struct GuiImageHandle *p = NULL;

	// OsError error = ENOERROR;
    
    Display *xdisplay = _linux_getdisplay();

    Visual *xvisual = DefaultVisual( xdisplay ,0 );

    // int xhot=0 ,yhot=0;
    
    int status;

	if( handle == NULL || filename == NULL ) return EINVAL;

	if( *handle != OS_INVALID_HANDLE ) return EEXIST;

	if( resourceTypeHint == OS_GUIRESOURCETYPE_ANY ) return ENOSYS;
    
	if( resourceTypeHint == OS_GUIRESOURCETYPE_IMAGE )
    {
        p = NewGuiImageHandle();

        FILE *file = fopen(filename ,"r" );

        if( !file )
            return EFAILED;

        unsigned width = 0, height = 0;
        unsigned char *data = NULL;
        char *clip = NULL;
        unsigned rowbytes = 0;
        unsigned long *image, *mask;

        LoadPng( file ,&data ,&clip ,&width ,&height ,&rowbytes );

        if (!data)
            return EFAILED;

        int rootDepth = DefaultDepth( xdisplay ,DefaultScreen(xdisplay) );

        XImage *ximage = XCreateImage( xdisplay ,DefaultVisual(xdisplay,DefaultScreen(xdisplay)) ,rootDepth ,ZPixmap ,0 ,(char*) data ,width ,height ,8 ,rowbytes );

        // g_ximage = ximage;

        if( !ximage )
            return EFAILED;

        status = 0;

        p->_width = width;
        p->_height = height;
        p->_xpixmap = XCreatePixmap( xdisplay ,DefaultRootWindow(xdisplay) ,p->_width ,p->_height ,rootDepth );

        GC gc = XCreateGC( xdisplay ,p->_xpixmap ,0 ,0 );
        // GC gc = XCreateGC( xdisplay ,DefaultScreen(xdisplay) ,0 ,0 );
        {
            XPutImage( xdisplay ,p->_xpixmap ,gc ,ximage ,0,0 ,0 ,0 ,p->_width,p->_height );
        }
        XFreeGC( xdisplay ,gc );

        XDestroyImage(ximage);

        // XFreePixmap(xdisplay,ximage);

        // free (data);
        // free(clip)


//--

        // status = XReadBitmapFile( xdisplay ,DefaultRootWindow(xdisplay) ,filename ,&(p->_width) ,&(p->_height) ,&(p->_xpixmap) ,NULL ,NULL ); // &xhot ,&yhot );

        // RootWindow(xdisplay);

        // ScreenOfDisplay(dpy,scr)->root)
        // DefaultRootWindow(xdisplay)
        //XRootWindow(xdisplay,0)

        // Screen *xscreen = XDefaultScreenOfDisplay(xdisplay);
        // XWindowAttributes attr;
        // XGetWindowAttributes( xdisplay ,xwindow ,&attr );

        // XCreatePixmapFromBitmapData
        // status = XReadBitmapFile( xdisplay ,DefaultRootWindow(xdisplay) ,filename ,&(p->_width) ,&(p->_height) ,&(xbmp) ,NULL ,NULL ); // &xhot ,&yhot );
/*
        Window xwindow = DefaultRootWindow(xdisplay);
        int rootDepth = DefaultDepth( xdisplay ,DefaultScreen(xdisplay) );

        Pixmap xbmp;

        status = XReadBitmapFile( xdisplay ,xwindow ,filename ,&(p->_width) ,&(p->_height) ,&(p->_xpixmap) ,NULL ,NULL ); // &xhot ,&yhot );

        //! NB convert to root depth, how can we check if xbmp is already good depth ?

        p->_xpixmap = XCreatePixmap( xdisplay ,DefaultRootWindow(xdisplay) ,p->_width ,p->_height ,rootDepth );
*/

        /* GC gc = XCreateGC( xdisplay ,p->_xpixmap ,0 ,0 );
        {
            XPutImage( xdisplay ,p->_xpixmap ,gc ,&xbmp ,0,0 ,p->_width,p->_height ,0 ,0 );
        }
        XFreeGC( xdisplay ,gc );

        XFreePixmap(xdisplay,xbmp); */

        /* int n;
        int *depths = XListDepths( xdisplay ,0 ,&n );

        int dps[7];

        memcpy( &dps ,depths ,7 * sizeof(int) ); */

        // OsError result = imlib2_loadImage( xdisplay ,filename );

        if( status == 0 )
        {           
            *handle = (OsHandle) p;
            
            return ENOERROR;
        }
        
        if( status == BadAlloc ) return ENOMEM;
        
        return EFAILED;
    }

	return ENOSYS;
}

TINYFUN OsError _X11ResourceLoadFromApp( OsHandle *handle ,int resourceTypeHint ,int resourceId ,const char_t *application )
{
	/*struct GuiImageHandle *p = NULL;

	OsError error = ENOERROR;*/

	if( handle == NULL ) return EINVAL;

	if( *handle != OS_INVALID_HANDLE ) return EEXIST;
/*
	if( resourceTypeHint == OS_GUIRESOURCETYPE_ANY ) return ENOSYS;

	if( resourceTypeHint == OS_GUIRESOURCETYPE_IMAGE )
		return _win_loadimage( handle ,GetModuleHandle(application) ,MAKEINTRESOURCE(resourceId) ,LR_DEFAULTCOLOR );

	_ASSERT( FALSE );*/

	return ENOSYS;
}

//-- clipboard
static const int XA_STRING = 31;
static Atom UTF8;

char *X11PasteType( struct GuiWindowHandle *handle ,Atom atom ) {
    XEvent event;
    int format ,lapse = 10;
    unsigned long N, size;
    char * data, * s = 0;
    Bool result;

    Display *xdisplay = _linux_getdisplay();

    Atom target;

    Atom CLIPBOARD = XInternAtom( xdisplay, "CLIPBOARD", 0), XSEL_DATA = XInternAtom(xdisplay, "XSEL_DATA", 0);
    XConvertSelection(xdisplay, CLIPBOARD, atom, XSEL_DATA, handle->_xwindow, CurrentTime);
    XSync( xdisplay ,0 );

    result = XCheckTypedEvent( xdisplay ,SelectionNotify ,&event );

    while( result == False && lapse > 0 ) {
        OsSleep(1); --lapse;
        result = XCheckTypedEvent( xdisplay ,SelectionNotify ,&event );
    }

    if( result == False ) return NULL;

    switch( event.type ) {
        case SelectionNotify:
            if(event.xselection.selection != CLIPBOARD) break;
            if(event.xselection.property)
            {
                XGetWindowProperty(event.xselection.display, event.xselection.requestor, event.xselection.property, 0L,(~0L), 0, AnyPropertyType, &target, &format, &size, &N,(unsigned char**)&data);
                if(target == UTF8 || target == XA_STRING)
                {
                    s = strndup(data, size);
                    XFree(data);
                }
                XDeleteProperty(event.xselection.display, event.xselection.requestor, event.xselection.property);
            }
    }

    return s;
}

TINYFUN OsError _X11ClipboardGetData( OsHandle handle ,char dataType[32] ,void **data ,int *length ) {
    struct GuiWindowHandle *p = CastGuiWindowHandle( handle );

    char *cstr = NULL;

    if( p == NULL ) return EINVAL;

    Display *xdisplay = _linux_getdisplay();

    UTF8 = XInternAtom( xdisplay ,"UTF8_STRING" ,True );

    *data = NULL;

    if( UTF8 != None ) {
        cstr = X11PasteType( p ,UTF8 );
    }

    if( cstr == NULL ) {
        cstr = X11PasteType( p ,XA_STRING );
    }

    *data = cstr;

    return cstr ? ENOERROR : ENODATA;
}

TINYFUN OsError _X11ClipboardSetData( OsHandle handle ,char dataType[32] ,void *data ,int length ) {
    return ENOEXEC;
}

//////////////////////////////////////////////////////////////////////////
struct X11AnyHandle 
{
	uint32_t _magic;

	struct OsGuiSystemTable *_guiSystem;

	uint32_t _handleType;
};

OsError X11CloseHandle( OsHandle *handle )
{
	return ENOSYS;
}

TINYFUN enum OsHandleType _X11HandleGetType( OsHandle handle )
{
	struct X11AnyHandle *p = (struct X11AnyHandle *) handle;

	if( p == NULL ) return osNotAnHandle;

	switch( p->_handleType )
	{
	case X11_HANDLETYPE_WINDOW: return osGuiWindowHandle;
	case X11_HANDLETYPE_FONT: return osGuiFontHandle;
	case X11_HANDLETYPE_IMAGE: return osGuiImageHandle;
	default:
		return osNotAnHandle;
	}
}

TINYFUN OsError _X11HandleDestroy( OsHandle *handle )
{
	struct X11AnyHandle *p = (struct X11AnyHandle *) handle;

	if( p == NULL ) return EINVAL;

	//TODO check magic

	switch( p->_handleType )
	{
	case X11_HANDLETYPE_WINDOW:
        _linux_x11window_destroy( handle );
        break;

	case X11_HANDLETYPE_FONT:
	case X11_HANDLETYPE_IMAGE: 
    //TODO
		return ENOSYS;

	default:
		return EINVAL;
	}
}

//////////////////////////////////////////////////////////////////////////
/**
 * @brief 
 * @param handle
 */
struct OsGuiSystemTable _systemX11 =
{
	_X11WindowCreate 
	,_X11WindowRefresh
	,_X11SwapBuffers
	,_X11WindowShow
	,_X11MouseSetCursor
	,_X11MouseCapture
	,_X11MouseRelease
	,_X11GetClientArea
	,_X11SetColor
	,_X11RegionSetArea
	,_X11RegionGetArea
	,_X11RegionSetOffset
	,_X11RegionSetScale
	,_X11RegionGetSize
	,_X11PointToCoords
	,_X11FontCreate
	,_X11FontCalcSize
	,_X11SetFont
	,_X11ImageCreate
	,_X11ImageGetInfo
	,_X11ImageWrite
	,_X11SetAlphaBlend
	,_X11DrawImage
	,_X11DrawRectangle
	,_X11DrawEllipse
	,_X11DrawLine
	,_X11DrawText
	,_X11DrawPolygon
	,_X11ResourceGetType
	,_X11ResourceLoadFromMemory
	,_X11ResourceLoadFromFile
	,_X11ResourceLoadFromApp
    ,_X11ClipboardGetData
    ,_X11ClipboardSetData
	,_X11HandleGetType
	,_X11HandleDestroy
};

//////////////////////////////////////////////////////////////////////////
struct OsGuiSystemTable *_guiSystemX11 = &_systemX11;

//////////////////////////////////////////////////////////////////////////
//
#pragma clang diagnostic pop