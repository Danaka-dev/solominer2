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

//////////////////////////////////////////////////////////////////////////
#define WIN32_LEAN_AND_MEAN 

#include <windows.h>
#include <windowsx.h>

//////////////////////////////////////////////////////////////////////////
#include <intrin.h>
#include <crtdbg.h>

//////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

//////////////////////////////////////////////////////////////////////////
#include "tiny-os.h"

#include "tiny-win-defs.h"

#define ARRAYSIZEOF(__p_)		(sizeof(__p_) / sizeof(__p_[0]))
#define ARRAYLASTOF(__p_)		(__p_[ARRAYSIZEOF(__p_)-1])

//////////////////////////////////////////////////////////////////////////
extern OsError GetLastOsError( void );

//////////////////////////////////////////////////////////////////////////
//! GDI system

extern struct OsGuiSystemTable *_guiSystemGDI;

#define GDI_HANDLETYPE_WINDOW	1
#define GDI_HANDLETYPE_FONT		2
#define GDI_HANDLETYPE_IMAGE	3

//////////////////////////////////////////////////////////////////////////
#define GUIWINDOWHANDLE_MAGIC	0x06F5CC4BD // 0x01ACD89F4

#define GUICONSOLE_BUFFERSIZE	1024

//-- window handle
struct GuiWindowHandle //GDI
{
	uint32_t _magic; //! GuiMagic tag

	struct OsGuiSystemTable *_guiSystem; //! must be second

	uint32_t _handleType;

	HANDLE _handle;

	int _resized;

	int _doubleBuffer;

	HBITMAP _hbitmap; //! double buffering

	struct OsPoint _bmpSize;

	HDC _hdcBitmap; 

	COLORREF _backgroundColor;

	BYTE _backgroundAlpha;

	//! function
	OsEventFunction _function;

	void *_userData;

	//? console type window
	// char_t *_consoleBuffer;
	// int _consoleTail;
};

static struct GuiWindowHandle *NewGuiWindowHandle( void )
{
	const size_t size = sizeof(struct GuiWindowHandle);

	struct GuiWindowHandle *p = (struct GuiWindowHandle*) malloc(size);

	if( p == NULL ) return NULL;

	memset( p ,0 ,size );

	p->_magic = GUISYSTEMHANDLE_MAGIC;

	p->_handleType = GDI_HANDLETYPE_WINDOW;

	p->_handle = INVALID_HANDLE_VALUE;

	return p;
}

static struct GuiWindowHandle *CastGuiWindowHandle( OsHandle handle )
{
	struct GuiWindowHandle *p = (struct GuiWindowHandle *) handle;

	if( p == NULL ) return NULL;

	if( p->_magic != GUISYSTEMHANDLE_MAGIC || p->_handleType != GDI_HANDLETYPE_WINDOW ) return NULL;

	return p;
}

//-- context handle
#define OS_GDIOBJ_OWNPEN		1
#define OS_GDIOBJ_OWNBRUSH		2

struct GuiContextHandle
{
	struct OsGuiSystemTable *_guiSystem;

	struct GuiWindowHandle *_hwindow;

	HDC _hdc;

	HRGN _hrgn;

	HPEN _hpen;

	HBRUSH _hbrush;

	int _ownedGdiObj; //! HPEN + HBRUSH

	BLENDFUNCTION _blendfunction;

	//! region
	struct OsRect _region;

	int _offsetx ,_offsety;

	float _scalex ,_scaley;
};

//! NB GuiContextHandle is not cast protected as it is intended to be used in fast GUI code, use with care

//-- win procedure
LRESULT CALLBACK _win_guiwindowproc( HWND hWnd ,UINT message ,WPARAM wParam ,LPARAM lParam )
{
	struct GuiWindowHandle *p = (struct GuiWindowHandle*) GetWindowLongPtr( hWnd ,GWLP_USERDATA );

	CREATESTRUCT *createStruct = NULL;

	struct OsEventMessage eventMessage;

	struct OsPoint points[OS_MAX_MOUSEPOS];

	struct OsRect updateRect;

	struct GuiContextHandle context;

	OsError error;

	PAINTSTRUCT ps; HDC hdc;
	
	RECT windowRect;

	int width ,height;

	WORD htcode;

	eventMessage.eventType = osNoEvent;

	switch (message)
	{
/* TODO when supporting menus
	int wmId, wmEvent;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(_hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
*/

		//-- creation
	case WM_CREATE:
		createStruct = (CREATESTRUCT*) lParam;

		p = (struct GuiWindowHandle *) createStruct->lpCreateParams;

		if( p == NULL ) return -1;

		eventMessage.eventType = osExecuteEvent;
		eventMessage.executeMessage.executeAction = osExecuteStart;

		error = p->_function( &eventMessage ,p->_userData );

		if( OS_FAILED(error) ) 
			return -1;

		SetCursor( LoadCursor( NULL ,MAKEINTRESOURCE(IDC_ARROW) ) );
		break;

	case WM_CLOSE:
		if( p == NULL ) return DefWindowProc( hWnd ,message ,wParam ,lParam );

		eventMessage.eventType = osExecuteEvent;
		eventMessage.executeMessage.executeAction = osExecuteStop;

		error = p->_function( &eventMessage ,p->_userData );
			
		if( OS_SUCCEED(error) ) 
			return DefWindowProc( hWnd ,message ,wParam ,lParam );

		break;

	case WM_DESTROY:
		if( p == NULL ) return DefWindowProc( hWnd ,message ,wParam ,lParam );

		eventMessage.eventType = osExecuteEvent;
		eventMessage.executeMessage.executeAction = osExecuteTerminate;

		p->_function( &eventMessage ,p->_userData );
		break;

	case WM_SETCURSOR:
		htcode = LOWORD(lParam);

		if( htcode != HTCLIENT ) return DefWindowProc( hWnd ,message ,wParam ,lParam );

		return TRUE;

	case WM_SIZE:
		if( p == NULL ) return DefWindowProc( hWnd ,message ,wParam ,lParam );

		++p->_resized;

		InvalidateRect(hWnd,NULL,FALSE); //? TODO test this 

		return 0;

		// -- render
	case WM_PAINT: 
		if( p == NULL ) return DefWindowProc( hWnd ,message ,wParam ,lParam );

		//> context default
		memset( &context ,0 ,sizeof(context) );
		context._guiSystem = _guiSystemGDI;
		context._blendfunction.SourceConstantAlpha = 255;

		GetClientRect( hWnd ,&windowRect );
		context._region.left = windowRect.left;
		context._region.top = windowRect.top;
		context._region.right = windowRect.right;
		context._region.bottom = windowRect.bottom;

		context._offsetx = context._offsety = 0;
		context._scalex = context._scaley = 1.0;
		
		hdc = BeginPaint( hWnd ,&ps );
		{
			eventMessage.eventType = osRenderEvent;

			eventMessage.renderMessage.updateRect = &updateRect;
			eventMessage.renderMessage.updateRect->left = ps.rcPaint.left;
			eventMessage.renderMessage.updateRect->top = ps.rcPaint.top;
			eventMessage.renderMessage.updateRect->right = ps.rcPaint.right;
			eventMessage.renderMessage.updateRect->bottom = ps.rcPaint.bottom;

			eventMessage.renderMessage.resized = (p->_resized > 0) ? 1 : 0;

			context._hwindow = p;

			//-- double buffer
			if( p->_doubleBuffer != 0 )
			{
				width = windowRect.right - windowRect.left;
				height = windowRect.bottom - windowRect.top;

				if( p->_hbitmap != NULL && (p->_bmpSize.x < width || p->_bmpSize.y < height) )
				{
					DeleteDC( p->_hdcBitmap ); p->_hdcBitmap = NULL;
					DeleteObject( p->_hbitmap ); p->_hbitmap = NULL;
				}

				if( p->_hbitmap == NULL )
				{
					p->_hdcBitmap = CreateCompatibleDC( hdc );
					p->_hbitmap = CreateCompatibleBitmap( hdc ,width ,height );

					p->_bmpSize.x = width; p->_bmpSize.y = height;

					SelectObject( p->_hdcBitmap ,p->_hbitmap );
				}

				context._hdc = p->_hdcBitmap;
			}
			else
			{
				context._hdc = hdc;
			}

			context._hpen = NULL;
			context._hbrush = NULL;

			eventMessage.renderMessage.context = (OsGuiContext) &context;

			p->_function( &eventMessage ,p->_userData );

			if( p->_doubleBuffer != 0 )
			{
				BitBlt( hdc ,0,0 ,width,height ,p->_hdcBitmap ,0,0, SRCCOPY );
			}
		}
		EndPaint( hWnd ,&ps );

		//! cleanup
		p->_resized = 0;

		if( context._hrgn != NULL )
			DeleteObject( context._hrgn );
		if( (context._ownedGdiObj & OS_GDIOBJ_OWNBRUSH) != 0 )
			DeleteObject( context._hbrush );
		if( (context._ownedGdiObj & OS_GDIOBJ_OWNPEN) != 0 )
			DeleteObject( context._hpen );

		context._ownedGdiObj = 0;

		break;

		//-- mouse
			//DoDragDrop
			//RegisterDragDrop / RevokeDragDrop
	case WM_MOUSEMOVE:
		eventMessage.eventType = osMouseEvent;
		eventMessage.mouseMessage.mouseAction = osMouseMove;
		eventMessage.mouseMessage.mouseButton = osNoMouseButton;
		break;

	case WM_LBUTTONDOWN:
		eventMessage.eventType = osMouseEvent;
		eventMessage.mouseMessage.mouseAction = osMouseButtonDown;
		eventMessage.mouseMessage.mouseButton = osLeftMouseButton;
		break;

	case WM_MBUTTONDOWN:
		eventMessage.eventType = osMouseEvent;
		eventMessage.mouseMessage.mouseAction = osMouseButtonDown;
		eventMessage.mouseMessage.mouseButton = osMiddleMouseButton;
		break;

	case WM_RBUTTONDOWN:
		eventMessage.eventType = osMouseEvent;
		eventMessage.mouseMessage.mouseAction = osMouseButtonDown;
		eventMessage.mouseMessage.mouseButton = osRightMouseButton;
		break;

	case WM_LBUTTONUP:
		eventMessage.eventType = osMouseEvent;
		eventMessage.mouseMessage.mouseAction = osMouseButtonUp;
		eventMessage.mouseMessage.mouseButton = osLeftMouseButton;
		break;

	case WM_MBUTTONUP:
		eventMessage.eventType = osMouseEvent;
		eventMessage.mouseMessage.mouseAction = osMouseButtonUp;
		eventMessage.mouseMessage.mouseButton = osMiddleMouseButton;
		break;

	case WM_RBUTTONUP:
		eventMessage.eventType = osMouseEvent;
		eventMessage.mouseMessage.mouseAction = osMouseButtonUp;
		eventMessage.mouseMessage.mouseButton = osRightMouseButton;
		break;

	case WM_LBUTTONDBLCLK:
		eventMessage.eventType = osMouseEvent;
		eventMessage.mouseMessage.mouseAction = osMouseDoubleClick;
		eventMessage.mouseMessage.mouseButton = osLeftMouseButton;
		break;

	case WM_MBUTTONDBLCLK:
		eventMessage.eventType = osMouseEvent;
		eventMessage.mouseMessage.mouseAction = osMouseDoubleClick;
		eventMessage.mouseMessage.mouseButton = osMiddleMouseButton;
		break;

	case WM_RBUTTONDBLCLK:
		eventMessage.eventType = osMouseEvent;
		eventMessage.mouseMessage.mouseAction = osMouseDoubleClick;
		eventMessage.mouseMessage.mouseButton = osRightMouseButton;
		break;

    case WM_MOUSEWHEEL:
    {
        float amount = 0.5f * (short) HIWORD (wParam);
        eventMessage.eventType = osMouseEvent;
        eventMessage.mouseMessage.mouseAction = osMouseWheel;
        eventMessage.mouseMessage.mouseButton = osNoMouseButton;
        if (amount < -1000.0f) amount = -1000.f;
        if (amount > 1000.0f) amount = 1000.f;
        eventMessage.mouseMessage.points = (int)amount;//GET_WHEEL_DELTA_WPARAM(wParam);
        break;
    }
		//-- key
	case WM_KEYDOWN:
		if( p == NULL ) return DefWindowProc( hWnd ,message ,wParam ,lParam );

		eventMessage.eventType = osKeyboardEvent;
		eventMessage.keyboardMessage.keyAction = osKeyDown;

		* (int*) &eventMessage.keyboardMessage.keyState = 0; //TODO
		eventMessage.keyboardMessage.keyCode = (OsKeyCode) wParam;
		eventMessage.keyboardMessage.c = (char_t) wParam; //! TODO (TCHAR) wParam;

		p->_function( &eventMessage ,p->_userData );
		break;

	case WM_KEYUP:
		if( p == NULL ) return DefWindowProc( hWnd ,message ,wParam ,lParam );

		eventMessage.eventType = osKeyboardEvent;
		eventMessage.keyboardMessage.keyAction = osKeyUp;

		* (int*) &eventMessage.keyboardMessage.keyState = 0; //TODO
		eventMessage.keyboardMessage.keyCode = (OsKeyCode) wParam;
		eventMessage.keyboardMessage.c = (char_t) wParam; //! TODO (TCHAR) wParam;

		p->_function( &eventMessage ,p->_userData );
		break;

	case WM_CHAR:
		if( p == NULL ) return DefWindowProc( hWnd ,message ,wParam ,lParam );

		eventMessage.eventType = osKeyboardEvent;
		eventMessage.keyboardMessage.keyAction = osKeyChar;

		* (int*) &eventMessage.keyboardMessage.keyState = 0; //TODO
		eventMessage.keyboardMessage.c = (char_t) wParam; //! TODO (TCHAR) wParam;

		p->_function( &eventMessage ,p->_userData );
		break;

		//TODO more key

	default:
		return DefWindowProc( hWnd ,message ,wParam ,lParam );
	}

	if( eventMessage.eventType == osMouseEvent )
	{
		if( p == NULL ) return DefWindowProc( hWnd ,message ,wParam ,lParam );

		eventMessage.keyboardMessage.keyState.alt = 0;
		eventMessage.keyboardMessage.keyState.ctrl = ((wParam & MK_CONTROL) != 0) ? 1 : 0;
		eventMessage.keyboardMessage.keyState.shift = ((wParam & MK_SHIFT) != 0) ? 1 : 0;

// 		eventMessage.keyboardMessage.keyState.leftButton = ((wParam & MK_LBUTTON) != 0) ? 1 : 0;
// 		eventMessage.keyboardMessage.keyState.midButton = ((wParam & MK_MBUTTON) != 0) ? 1 : 0;
// 		eventMessage.keyboardMessage.keyState.rightButton = ((wParam & MK_RBUTTON) != 0) ? 1 : 0;

		eventMessage.keyboardMessage.mouseButton = (enum OsMouseButton) (
			(((wParam & MK_LBUTTON) != 0) ? osLeftMouseButton : osNoMouseButton)
			| (((wParam & MK_MBUTTON) != 0) ? osMiddleMouseButton : osNoMouseButton)
			| (((wParam & MK_RBUTTON) != 0) ? osRightMouseButton : osNoMouseButton)
			);
// 		eventMessage.keyboardMessage.mouseButton |= ((wParam & MK_MBUTTON) != 0) ? osMiddleMouseButton : osNoMouseButton;
// 		eventMessage.keyboardMessage.mouseButton |= ((wParam & MK_RBUTTON) != 0) ? osRightMouseButton : osNoMouseButton;

		points[0].x = (int) GET_X_LPARAM(lParam);
		points[0].y = (int) GET_Y_LPARAM(lParam);

		eventMessage.mouseMessage.points = 1;
		eventMessage.mouseMessage.pos = points;

		p->_function( &eventMessage ,p->_userData );
	}

	return 0;
}

#define OS_GUIWINDOW_CLASSNAME	"TinyC::GuiWindow::"

ATOM RegisterGuiWindowClass( HINSTANCE hInstance ,const char *name ) //! keeping it per window as we will need hIcon etc
{
	// HBRUSH hbrush = CreateSolidBrush( (COLORREF) backgroundColor );

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= _win_guiwindowproc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon( NULL ,IDI_APPLICATION ); // LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ORIGIN0));
	wcex.hIconSm		= LoadIcon( NULL ,IDI_APPLICATION ); // LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL; //! using double buffer doesn't need erasing (done in bitmap) // CreateSolidBrush( RGB(255,0,0) ); // (COLORREF) backgroundColor ); // (HBRUSH) (COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL; // MAKEINTRESOURCE(IDC_ORIGIN0);
	wcex.lpszClassName	= name; // OS_GUIWINDOW_CLASSNAME;

	return RegisterClassEx(&wcex);
}

TINYFUN OsError _GdiWindowCreate( OsHandle *handle ,const char_t *name ,const struct OsGuiWindowProperties *properties ,OsEventFunction handler ,void *userData )
{
	//TODO:
	// find window from its name
	// if name is null a new name is assigned, garanteed to be unique

	struct GuiWindowHandle *p = NULL;

	HINSTANCE hInstance = GetModuleHandle(NULL);

	DWORD style ,exStyle;

	RECT clientRect = { 0 ,0 ,properties->defaultWidth ,properties->defaultHeight };

	HWND hWnd;

	if( handle == NULL ) return EINVAL;

	if( *handle != OS_INVALID_HANDLE ) return EEXIST;

	p = NewGuiWindowHandle();

	if( p == NULL ) return ENOMEM;

	p->_function = handler;

	p->_userData = userData;

	//-- create window
	RegisterGuiWindowClass( GetModuleHandle(NULL) ,name );

	style = WS_OVERLAPPEDWINDOW ^ ((!properties->style&OS_WINDOWSTYLE_SIZEABLE) ? WS_THICKFRAME|WS_MAXIMIZEBOX : 0);

	if( properties->style&OS_WINDOWSTYLE_TOOLBOX )
		exStyle = WS_EX_TOOLWINDOW;
	else
		exStyle = 0;

	hWnd = CreateWindowEx( exStyle ,name /*OS_GUIWINDOW_CLASSNAME*/ ,properties->title ,style
		,CW_USEDEFAULT,CW_USEDEFAULT ,CW_USEDEFAULT,CW_USEDEFAULT ,NULL ,NULL ,NULL ,(void*) p );

	if( !hWnd )
	{
		free( p );

		return GetLastOsError();
	}

	p->_guiSystem = _guiSystemGDI;

	p->_handleType = GDI_HANDLETYPE_WINDOW;

	p->_handle = hWnd;

	p->_resized = 1; //! we want a resize on first render event

	p->_backgroundColor = (COLORREF) (properties->backgroundColor & 0x0ffffff);

	p->_backgroundAlpha = ((struct OsColor*) &(properties->backgroundColor))->a;

	p->_doubleBuffer = (properties->flags & OS_WINDOWFLAG_DIRECTDRAW) ? 0 : 1;

	*handle = p;

	//-- set, show & update window
	SetWindowLongPtr( hWnd ,GWLP_USERDATA ,(LONG_PTR) p );

	AdjustWindowRect( &clientRect ,style^WS_OVERLAPPED ,FALSE );

	SetWindowPos( hWnd ,HWND_TOP ,0,0 ,clientRect.right-clientRect.left ,clientRect.bottom-clientRect.top ,SWP_NOMOVE|SWP_NOZORDER|SWP_NOREDRAW );
	
	ShowWindow( hWnd ,SW_SHOWNORMAL );

	UpdateWindow( hWnd );

	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
//! Window common

//-- visibility
/*
TINYFUN void _GdiWindowRefresh( OsHandle handle ,int update )
{
	struct GuiWindowHandle *p = CastGuiWindowHandle( handle );

	if( p == NULL ) return;

	if( update != 0 )
		InvalidateRect( p->_handle ,NULL ,FALSE );

	UpdateWindow( p->_handle );
}
*/

TINYFUN void _GdiWindowRefresh( OsHandle handle ,const struct OsRect *updateRect ,int flags  )
{
	struct GuiWindowHandle *p = CastGuiWindowHandle( handle );

	BOOL bErase = ((flags & OS_REFRESH_BACKGROUND) != 0) ? TRUE : FALSE;

	RECT rect;

	if( p == NULL ) return;

	if( flags & OS_REFRESH_RESIZED )
		++p->_resized;

	if( updateRect != NULL )
	{
		rect.left = updateRect->left; rect.top = updateRect->top;
		rect.right = updateRect->right; rect.bottom = updateRect->bottom;

		InvalidateRect( p->_handle ,&rect ,bErase );
	}
	else
		InvalidateRect( p->_handle ,NULL ,bErase );

	if( flags & OS_REFRESH_UPDATE )
		UpdateWindow( p->_handle );
}

TINYFUN void _GdiWindowShow( OsHandle handle ,int visible )
{
	struct GuiWindowHandle *p = CastGuiWindowHandle( handle );

	if( p == NULL ) return;

	ShowWindow( p->_handle ,(visible != 0) ? SW_SHOW : SW_HIDE ); //? SW_RESTORE
}

//-- input
TINYFUN void _GdiMouseSetCursor( OsHandle handle ,int cursorId )
{
/*
	HWND hWnd = GetForegroundWindow();
	DWORD foregroundThreadID = GetWindowThreadProcessId(hWnd, 0);
	DWORD currentThreadID = GetCurrentThreadId();

	AttachThreadInput(foregroundThreadID, currentThreadID, TRUE);
	SetCursor(hCursor);
	AttachThreadInput(foregroundThreadID, currentThreadID, FALSE); 
*/
	//TODO using proper window
	static BOOL _cursorShown = TRUE;

	HCURSOR hcursor = NULL;

	LPCTSTR res;

	if( cursorId == OS_CURSOR_NOCURSOR && _cursorShown == TRUE )
	{
		ShowCursor( _cursorShown = FALSE );

		return;
	}

	if( _cursorShown == FALSE )
		ShowCursor( _cursorShown = TRUE );

	switch( cursorId )
	{
		// case OS_CURSOR_NOCURSOR: 

	default:
	case OS_CURSOR_ARROW: res = MAKEINTRESOURCE(IDC_ARROW); break;
	case OS_CURSOR_WAIT: res = MAKEINTRESOURCE(IDC_WAIT); break;
	case OS_CURSOR_CROSS: res = MAKEINTRESOURCE(IDC_CROSS); break;
	case OS_CURSOR_BEAM: res = MAKEINTRESOURCE(IDC_IBEAM); break;
	case OS_CURSOR_CANNOT: res = MAKEINTRESOURCE(IDC_NO); break;
	case OS_CURSOR_UPARROW: res = MAKEINTRESOURCE(IDC_ARROW); break;
	case OS_CURSOR_SIZEALL: res = MAKEINTRESOURCE(IDC_SIZEALL); break;
	case OS_CURSOR_SIZETOPLEFT: res = MAKEINTRESOURCE(IDC_SIZENESW); break;
	case OS_CURSOR_SIZETOPRIGHT: res = MAKEINTRESOURCE(IDC_SIZENWSE); break;
	case OS_CURSOR_SIZEWIDTH: res = MAKEINTRESOURCE(IDC_SIZEWE); break;
	case OS_CURSOR_SIZEHEIGHT: res = MAKEINTRESOURCE(IDC_SIZENS); break;
	}

	hcursor = LoadCursor( NULL ,res );

	SetCursor( hcursor );
}

TINYFUN void _GdiMouseCapture( OsHandle handle )
{
	struct GuiWindowHandle *p = CastGuiWindowHandle( handle );

	if( p == NULL ) return;

	SetCapture( p->_handle ); // _TODO CHECK THIS
}

TINYFUN void _GdiMouseRelease( OsHandle handle )
{
	struct GuiWindowHandle *p = CastGuiWindowHandle( handle );

	if( p == NULL ) return;

	ReleaseCapture(); //  p->_handle );
}

//-- properties
TINYFUN void _GdiGetClientArea( OsHandle handle ,struct OsPoint *size )
{
	struct GuiWindowHandle *p = CastGuiWindowHandle( handle );

	RECT rect;

	if( p == NULL ) return;

	if( size == NULL ) return;

	GetClientRect( p->_handle ,&rect );

	size->x = rect.right - rect.left;

	size->y = rect.bottom - rect.top;
}

//////////////////////////////////////////////////////////////////////////
TINYFUN void _GdiSetForeColor( OsGuiContext context ,OsColorRef c )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	struct OsColor *C = (struct OsColor*) &c;

	_ASSERT( osContext != NULL && osContext->_hdc != NULL );

	if( (osContext->_ownedGdiObj & OS_GDIOBJ_OWNPEN) != 0 )
	{
		DeleteObject( osContext->_hpen );
		osContext->_ownedGdiObj ^= OS_GDIOBJ_OWNPEN;
	}

	if( C->a != 0 )
	{
		osContext->_hpen = CreatePen(  PS_SOLID ,1 ,(COLORREF) (c&0x0FFFFFF) );
		osContext->_ownedGdiObj |= OS_GDIOBJ_OWNPEN;
	}
	else 
		osContext->_hpen = GetStockPen( NULL_PEN );

	SelectObject( osContext->_hdc ,osContext->_hpen );
}

TINYFUN void _GdiSetFillColor( OsGuiContext context ,OsColorRef c )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	struct OsColor *C = (struct OsColor*) &c;

	_ASSERT( osContext != NULL && osContext->_hdc != NULL );

	if( (osContext->_ownedGdiObj & OS_GDIOBJ_OWNBRUSH) != 0 )
	{
		DeleteObject( osContext->_hbrush );
		osContext->_ownedGdiObj ^= OS_GDIOBJ_OWNBRUSH;
	}

	if( C->a != 0 )
	{
		osContext->_hbrush = CreateSolidBrush( (COLORREF) (c&0x0FFFFFF) );
		osContext->_ownedGdiObj |= OS_GDIOBJ_OWNBRUSH;
	}
	else 
		osContext->_hbrush = GetStockBrush( NULL_BRUSH );

	SelectObject( osContext->_hdc ,osContext->_hbrush );
}

//>text color
TINYFUN void _GdiSetTextColor( OsGuiContext context ,OsColorRef c )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	_ASSERT( osContext != NULL && osContext->_hdc != NULL );

	SetTextColor( osContext->_hdc ,(COLORREF) (c&0x0FFFFFF) );
}

TINYFUN void _GdiSetBackColor( OsGuiContext context ,OsColorRef c )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	struct OsColor *C = (struct OsColor*) &c;

	_ASSERT( osContext != NULL && osContext->_hdc != NULL );

	SetBkColor( osContext->_hdc ,(COLORREF) (c&0x0FFFFFF) );

	SetBkMode( osContext->_hdc ,(C->a != 0) ? OPAQUE : TRANSPARENT );
}

TINYFUN void _GdiSetColor( OsGuiContext context ,int selectColor ,OsColorRef color )
{
	switch( selectColor )
	{
	case OS_SELECT_FORECOLOR: _GdiSetForeColor( context ,color ); break;
	case OS_SELECT_FILLCOLOR: _GdiSetFillColor( context ,color ); break;
	case OS_SELECT_TEXTCOLOR: _GdiSetTextColor( context ,color ); break;
	case OS_SELECT_BACKCOLOR: _GdiSetBackColor( context ,color ); break;
	default: break;
	}
}

//////////////////////////////////////////////////////////////////////////
TINYFUN void _GdiRegionSetArea( OsGuiContext context ,int left ,int top ,int right ,int bottom ,int useOffset )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	BOOL setRectRgn;

	int selectClipRgn;

	_ASSERT( osContext != NULL && osContext->_hdc != NULL );

	osContext->_region.left = left;
	osContext->_region.top = top;
	osContext->_region.right = MAX(left,right);
	osContext->_region.bottom = MAX(top,bottom);

	if( osContext->_hrgn == NULL )
		osContext->_hrgn = CreateRectRgn( left ,top ,right ,bottom );

	if( osContext->_hrgn == NULL )
		return;

	setRectRgn = SetRectRgn( osContext->_hrgn ,left ,top ,right ,bottom ); _ASSERT( setRectRgn == TRUE );

	selectClipRgn = SelectClipRgn( osContext->_hdc ,osContext->_hrgn ); _ASSERT( selectClipRgn != ERROR );

	if( useOffset != 0 )
	{
		osContext->_offsetx = left;
		osContext->_offsety = top;
	}
}

TINYFUN void _GdiRegionGetArea( OsGuiContext context ,struct OsRect *area )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	_ASSERT( osContext != NULL && osContext->_hdc != NULL );

	if( area != NULL )
	{
		area->left = osContext->_region.left;
		area->top = osContext->_region.top;
		area->right = osContext->_region.right;
		area->bottom = osContext->_region.bottom;
	}
}

TINYFUN void _GdiRegionSetOffset( OsGuiContext context ,int x ,int y )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	_ASSERT( osContext != NULL && osContext->_hdc != NULL );

	osContext->_offsetx = x;
	osContext->_offsety = y;
}

TINYFUN void _GdiRegionSetScale( OsGuiContext context ,float x ,float y )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	_ASSERT( osContext != NULL && osContext->_hdc != NULL );

	osContext->_scalex = x;
	osContext->_scaley = y;
}

TINYFUN void _GdiRegionGetSize( OsGuiContext context ,int *width ,int *height )
{
	_ASSERT( FALSE );
}

TINYFUN void _GdiPointToCoords( OsGuiContext context ,int surfaceForP ,const struct OsPoint *p ,int surfaceForCoords ,struct OsPoint *coords )
{
	_ASSERT( FALSE );
}

//////////////////////////////////////////////////////////////////////////
#define GUIFONTHANDLE_MAGIC	0x06F5CC4BD // 0x04D7EFC35

struct GuiFontHandle
{
	uint32_t _magic;

	struct OsGuiSystemTable *_guiSystem;

	uint32_t _handleType;

	HANDLE _handle;
};

static struct GuiFontHandle *NewGuiFontHandle( void )
{
	const size_t size = sizeof(struct GuiFontHandle);

	struct GuiFontHandle *p = (struct GuiFontHandle*) malloc(size);

	if( p == NULL ) return NULL;

	memset( p ,0 ,size );

	p->_magic = GUIFONTHANDLE_MAGIC;

	p->_handleType = GDI_HANDLETYPE_FONT;

	p->_handle = INVALID_HANDLE_VALUE;

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
static int _win_fontpitch_map[] = { DEFAULT_PITCH ,FIXED_PITCH ,VARIABLE_PITCH };

static int _win_fontweight_map[] = { FW_DONTCARE ,FW_THIN ,FW_ULTRALIGHT ,FW_LIGHT ,FW_NORMAL ,FW_MEDIUM ,FW_SEMIBOLD ,FW_BOLD ,FW_ULTRABOLD ,FW_HEAVY };

#define HASFONTSTYLE(__p_,__style_)		(((__p_&__style_)!=0)?TRUE:FALSE)

TINYFUN OsError _GdiFontCreate( OsHandle *handle ,const char_t *faceName ,int pointSize ,int weight ,int style ,int family )
{
	struct GuiFontHandle *p = NewGuiFontHandle();

	HFONT hfont = NULL;

	int pitchAndFamilly = _win_fontpitch_map[family%ARRAYSIZEOF(_win_fontpitch_map)];

	int fontWeight = _win_fontweight_map[ min(weight,ARRAYLASTOF(_win_fontweight_map)) ];

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

	p->_guiSystem = _guiSystemGDI;

	p->_handle = hfont;

	*handle = (OsHandle) p;

	return ENOERROR;
}

TINYFUN void _GdiFontCalcSize( OsHandle handle ,const char_t *text ,struct OsPoint *size )
{
	struct GuiFontHandle *p = CastGuiFontHandle( handle );

	HDC hdc;

	SIZE osSize;

// 	RECT osRect;

	BOOL r;

	if( p == NULL || size == NULL ) return;
	
	// hdc = CreateCompatibleDC(NULL);
	hdc = GetDC(NULL);

	SelectObject( hdc ,p->_handle );

	r = GetTextExtentPoint32( hdc ,text ,(int) strlen(text) ,&osSize );

 	size->x = osSize.cx;
 	size->y = osSize.cy;

/*
	osRect.left = 0; osRect.top = 0; osRect.right = 0; osRect.bottom = 0;

	DrawText( hdc ,text ,-1 ,&osRect ,DT_SINGLELINE | DT_CALCRECT );

// 	if( size->x != osRect.right - osRect.left || size->y != osRect.bottom - osRect.top )
// 		return;

	size->x = osRect.right - osRect.left;
	size->y = osRect.bottom - osRect.top;
*/

	ReleaseDC( NULL ,hdc );
	// DeleteObject( hdc );
}

TINYFUN void _GdiSetFont( OsGuiContext context ,OsHandle fontHandle )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	struct GuiFontHandle *p = CastGuiFontHandle( fontHandle );

	if( p == NULL ) return;

	_ASSERT( osContext != NULL && osContext->_hdc != NULL );

	SelectObject( osContext->_hdc ,p->_handle );
}

//////////////////////////////////////////////////////////////////////////
//-- image 
#define GUIIMAGEHANDLE_MAGIC	0x06F5CC4BD // 0x079A3F45C

struct GuiImageHandle
{
	uint32_t _magic;

	struct OsGuiSystemTable *_guiSystem;

	uint32_t _handleType;

	HBITMAP _hbitmap;

	BITMAP _bm;

	HDC _hdc;
};

static struct GuiImageHandle *NewGuiImageHandle( void )
{
	const size_t size = sizeof(struct GuiImageHandle);

	struct GuiImageHandle *p = (struct GuiImageHandle*) malloc(size);

	if( p == NULL ) return NULL;

	memset( p ,0 ,size );

	p->_magic = GUIIMAGEHANDLE_MAGIC;

	p->_handleType = GDI_HANDLETYPE_IMAGE;

	return p;
}

static struct GuiImageHandle *CastGuiImageHandle( OsHandle handle )
{
	struct GuiImageHandle *p = (struct GuiImageHandle *) handle;

	if( p == NULL ) return NULL;

	if( p->_magic != GUIIMAGEHANDLE_MAGIC ) return NULL;

	return p;
}

static OsError DeleteGuiImageHandle( OsHandle *handle )
{
	struct GuiImageHandle *p = * (struct GuiImageHandle **) handle;

	if( *handle == OS_INVALID_HANDLE ) return ENOERROR;

	if( p == NULL ) return EINVAL;

	if( p->_hbitmap != NULL ) DeleteObject( p->_hbitmap );

	*handle = OS_INVALID_HANDLE;

	free( p );

	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
//> GUI image function
TINYFUN OsError _GdiImageCreate( OsHandle *handle ,int width ,int height )
{
	struct GuiImageHandle *p = NewGuiImageHandle();

	BITMAPINFO bminfo;
	
	void *bits = NULL;

	HDC hdcDisplay;

	if( p == NULL ) return ENOMEM;

	p->_hdc = CreateCompatibleDC(NULL);

	if( p->_hdc == NULL )
	{
		free( p );

		return ENOMEM;
	}

	bminfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bminfo.bmiHeader.biWidth = width;
	bminfo.bmiHeader.biHeight = height;
	bminfo.bmiHeader.biPlanes = 1;
	bminfo.bmiHeader.biBitCount = 24;
	bminfo.bmiHeader.biCompression = BI_RGB;
	bminfo.bmiHeader.biSizeImage = 0; //! unused for uncompressed image -> // (DWORD) width * height;
	bminfo.bmiHeader.biXPelsPerMeter = 0;
	bminfo.bmiHeader.biYPelsPerMeter = 0;
	bminfo.bmiHeader.biClrUsed = 0;
	bminfo.bmiHeader.biClrImportant = 0;

	hdcDisplay = CreateDC( "DISPLAY" ,NULL ,NULL ,NULL );

	// p->_hbitmap = CreateCompatibleBitmap( hdcDisplay ,width ,height );
	p->_hbitmap = CreateDIBSection( hdcDisplay ,&bminfo ,DIB_RGB_COLORS ,&bits ,NULL ,0 );

	DeleteDC( hdcDisplay );

	if( p->_hbitmap == NULL )
	{
		free( p );

		return ENOENT;
	}

	GetObject( p->_hbitmap ,sizeof(p->_bm) ,&p->_bm );

	SelectObject( p->_hdc ,p->_hbitmap );

	p->_guiSystem = _guiSystemGDI;

	*handle = (OsHandle) p;

	return ENOERROR;
}

TINYFUN void _GdiImageGetInfo( OsHandle handle ,struct OsGuiImageInfo *info )
{
	struct GuiImageHandle *p = CastGuiImageHandle( handle );

	_ASSERT( p != NULL && info != NULL );

	if( p == NULL || info == NULL ) return;

	info->width = p->_bm.bmWidth;
	info->height = p->_bm.bmHeight;
}

TINYFUN OsError _GdiImageWrite( OsHandle handle ,const uint8_t *pixelValues ,size_t size )
{
	struct GuiImageHandle *p = CastGuiImageHandle( handle );

	BITMAPINFO bminfo;

	int ierror;

	if( p == NULL || p->_bm.bmWidth == 0 || p->_bm.bmHeight == 0 ) return EINVAL;

	bminfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bminfo.bmiHeader.biWidth = p->_bm.bmWidth;
	bminfo.bmiHeader.biHeight = -p->_bm.bmHeight;
	bminfo.bmiHeader.biPlanes = 1;
	bminfo.bmiHeader.biBitCount = 24;
	bminfo.bmiHeader.biCompression = BI_RGB;
	bminfo.bmiHeader.biSizeImage = 0; //! NB unused for uncompressed data // -> // = (DWORD) size;
	bminfo.bmiHeader.biXPelsPerMeter = 0;
	bminfo.bmiHeader.biYPelsPerMeter = 0;
	bminfo.bmiHeader.biClrUsed = 0;
	bminfo.bmiHeader.biClrImportant = 0;

	ierror = SetDIBits( p->_hdc ,p->_hbitmap ,0 ,p->_bm.bmHeight ,pixelValues ,&bminfo ,DIB_RGB_COLORS );

	if( ierror <= 0 )
		return EINVAL;

	return ENOERROR;
}

TINYFUN void _GdiSetAlphaBlend( OsGuiContext context ,float alpha )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	_ASSERT( osContext != NULL && osContext->_hdc != NULL );

	osContext->_blendfunction.BlendOp = AC_SRC_OVER;
	osContext->_blendfunction.BlendFlags = 0;
	osContext->_blendfunction.AlphaFormat = 0;
	osContext->_blendfunction.SourceConstantAlpha = (alpha < 1.0) ? ((alpha > 0) ? (BYTE) (255*alpha) : 0) : 255;
}

//-- render
#define _TX(_x_)	(osgc->_offsetx+(int)((_x_)*osgc->_scalex))
#define _TY(_x_)	(osgc->_offsety+(int)((_x_)*osgc->_scaley))

TINYFUN void _GdiDrawImage( OsGuiContext context ,int left ,int top ,int right ,int bottom ,OsHandle handle ,int imageLeft ,int imageTop ,int imageRight ,int imageBottom )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	struct GuiImageHandle *p = CastGuiImageHandle( handle );

	int destWidth = right - left; //TODO _TX / _TY

	int destHeight = bottom - top;

	int srcWidth = imageRight - imageLeft;

	int srcHeight = imageBottom - imageTop;

	BOOL berror = TRUE;

	if( p == NULL ) { _ASSERT( FALSE ); return; }

	_ASSERT( osContext != NULL && osContext->_hdc != NULL );
	
	_ASSERT( osContext->_guiSystem == p->_guiSystem );

	if( right < 0 ) destWidth = p->_bm.bmWidth; else if( left > right ) left = right ,destWidth = -destWidth;

	if( bottom < 0 ) destHeight = p->_bm.bmHeight; else if( top > bottom ) top = bottom ,destHeight = -destHeight;

	if( imageRight < 0 ) srcWidth = p->_bm.bmWidth; else if( imageLeft > imageRight ) imageLeft = imageRight ,srcWidth = -srcWidth;

	if( imageBottom < 0 ) srcHeight = p->_bm.bmHeight; else if( imageTop > imageBottom ) imageTop = imageBottom ,srcHeight = -srcHeight;

	if( osContext->_blendfunction.SourceConstantAlpha != 255 )
		berror = AlphaBlend( osContext->_hdc ,left ,top ,destWidth ,destHeight ,p->_hdc ,imageLeft ,imageTop ,srcWidth ,srcHeight ,osContext->_blendfunction );

	else if( destWidth != srcWidth || destHeight != srcHeight )
		berror = StretchBlt( osContext->_hdc ,left ,top ,destWidth ,destHeight ,p->_hdc ,imageLeft ,imageTop ,srcWidth ,srcHeight ,SRCCOPY );

	else
		berror = BitBlt( osContext->_hdc ,left ,top ,destWidth ,destHeight ,p->_hdc ,imageLeft ,imageTop ,SRCCOPY );

	// _ASSERT( berror == TRUE );
}

TINYFUN void _GdiDrawRectangle( OsGuiContext context ,int left ,int top ,int right ,int bottom )
{
	struct GuiContextHandle *osgc = (struct GuiContextHandle*) context;

	_ASSERT( osgc != NULL && osgc->_hdc != NULL );

	Rectangle( osgc->_hdc ,_TX(left) ,_TY(top) ,_TX(right) ,_TY(bottom) );
}

TINYFUN void _GdiDrawEllipse( OsGuiContext context ,int left ,int top ,int right ,int bottom )
{
	struct GuiContextHandle *osgc = (struct GuiContextHandle*) context;

	_ASSERT( osgc != NULL && osgc->_hdc != NULL );

	Ellipse( osgc->_hdc ,_TX(left) ,_TY(top) ,_TX(right) ,_TY(bottom) );
}

TINYFUN void _GdiDrawLine( OsGuiContext context ,int left ,int top ,int right ,int bottom )
{
	struct GuiContextHandle *osgc = (struct GuiContextHandle*) context;

	_ASSERT( osgc != NULL && osgc->_hdc != NULL );

	MoveToEx( osgc->_hdc ,_TX(left) ,_TY(top) ,NULL );

	LineTo( osgc->_hdc ,_TX(right) ,_TY(bottom) );
}

static int _win_textalign_map[] = { DEFAULT_PITCH ,FIXED_PITCH ,VARIABLE_PITCH };

TINYFUN void _GdiDrawText( OsGuiContext context ,const char_t *text ,int left ,int top ,int right ,int bottom ,int align ,struct OsRect *rect )
{
	struct GuiContextHandle *osgc = (struct GuiContextHandle*) context;

	RECT winrect;

	UINT format = DT_SINGLELINE | (UINT) align;

	_ASSERT( osgc != NULL && osgc->_hdc != NULL );

	winrect.left = _TX(left); winrect.top = _TY(top); winrect.right = _TX(right); winrect.bottom = _TY(bottom);

	if( rect == NULL )
	{
		DrawText( osgc->_hdc ,text ,-1 ,&winrect ,align | DT_SINGLELINE );

		return;
	}

	DrawText( osgc->_hdc ,text ,-1 ,&winrect ,align | DT_SINGLELINE | DT_CALCRECT );

	rect->left = winrect.left;
	rect->top = winrect.top;
	rect->right = winrect.right;
	rect->bottom = winrect.bottom;
}

TINYFUN void _GdiDrawPolygon( OsGuiContext context ,int npoints ,const struct OsPoint *points )
{
	struct GuiContextHandle *osgc = (struct GuiContextHandle*) context;

	struct OsPoint p[32];

	int i;

	_ASSERT( osgc != NULL && osgc->_hdc != NULL );

	if( npoints > 32 ) npoints = 32;

	for( i=0; i<npoints; ++i )
	{
		p[i].x = _TX( points[i].x );
		p[i].y = _TY( points[i].y );
	}

	Polygon( osgc->_hdc ,(POINT*) p ,npoints );
}


//-- resource
TINYFUN int _GdiResourceGetType( OsHandle handle )
{
	struct GuiImageHandle *p = CastGuiImageHandle( handle );

	if( p != NULL ) return OS_GUIRESOURCETYPE_IMAGE;

	_ASSERT( FALSE );

	return 0;
}

TINYFUN OsError _GdiResourceLoadFromMemory( OsHandle *handle ,int resourceTypeHint ,uint8_t *memory ,const void *resourceInfo )
{
	_ASSERT( FALSE );

	return ENOSYS;
}

OsError _win_loadimage( OsHandle *handle ,HINSTANCE hinstance ,const char_t *name ,UINT fLoad )
{
	struct GuiImageHandle *p = NewGuiImageHandle();

	if( p == NULL ) return ENOMEM;

	p->_hdc = CreateCompatibleDC(NULL);

	if( p->_hdc == NULL )
	{
		free( p );

		return ENOMEM;
	}

	p->_hbitmap = LoadImage( hinstance ,name ,IMAGE_BITMAP ,0,0 ,fLoad );

	if( p->_hbitmap == NULL )
	{
		free( p );

		return ENOENT;
	}

	GetObject( p->_hbitmap ,sizeof(p->_bm) ,&p->_bm );

	SelectObject( p->_hdc ,p->_hbitmap );

	p->_guiSystem = _guiSystemGDI;

	*handle = (OsHandle) p;

	return ENOERROR;
}

TINYFUN OsError _GdiResourceLoadFromFile( OsHandle *handle ,int resourceTypeHint ,const char_t *filename )
{
	struct GuiImageHandle *p = NULL;

	OsError error = ENOERROR;

	if( handle == NULL || filename == NULL ) return EINVAL;

	if( *handle != OS_INVALID_HANDLE ) return EEXIST;

	if( resourceTypeHint == OS_GUIRESOURCETYPE_ANY ) return ENOSYS;

	if( resourceTypeHint == OS_GUIRESOURCETYPE_IMAGE )
		return _win_loadimage( handle ,NULL ,filename ,LR_LOADFROMFILE );

	_ASSERT( FALSE );

	return ENOSYS;
}

TINYFUN OsError _GdiResourceLoadFromApp( OsHandle *handle ,int resourceTypeHint ,int resourceId ,const char_t *application )
{
	struct GuiImageHandle *p = NULL;

	OsError error = ENOERROR;

	if( handle == NULL ) return EINVAL;

	if( *handle != OS_INVALID_HANDLE ) return EEXIST;

	if( resourceTypeHint == OS_GUIRESOURCETYPE_ANY ) return ENOSYS;

	if( resourceTypeHint == OS_GUIRESOURCETYPE_IMAGE )
		return _win_loadimage( handle ,GetModuleHandle(application) ,MAKEINTRESOURCE(resourceId) ,LR_DEFAULTCOLOR );

	_ASSERT( FALSE );

	return ENOSYS;
}

//////////////////////////////////////////////////////////////////////////
struct GdiAnyHandle 
{
	uint32_t _magic;

	struct OsGuiSystemTable *_guiSystem;

	uint32_t _handleType;

	HANDLE _handle;
};

OsError GdiDestroyWindow( OsHandle *handle )
{
	struct GdiAnyHandle *p = * (struct GdiAnyHandle **) handle;

	if( *handle == OS_INVALID_HANDLE ) return ENOERROR;

	if( p == NULL ) return EINVAL;

	if( p->_handle != INVALID_HANDLE_VALUE ) DestroyWindow( (HWND) p->_handle );

	*handle = OS_INVALID_HANDLE;

	free( p );

	return ENOERROR;
}

OsError GdiDeleteObject( OsHandle *handle )
{
	struct GdiAnyHandle *p = * (struct GdiAnyHandle **) handle;

	if( *handle == OS_INVALID_HANDLE ) return ENOERROR;

	if( p == NULL ) return EINVAL;

	if( p->_handle != INVALID_HANDLE_VALUE ) DeleteObject( (HGDIOBJ) p->_handle );

	*handle = OS_INVALID_HANDLE;

	free( p );

	return ENOERROR;
}

OsError GdiCloseHandle( OsHandle *handle )
{
	struct GdiAnyHandle *p = * (struct GdiAnyHandle **) handle;

	if( *handle == OS_INVALID_HANDLE ) return ENOERROR;

	if( p == NULL ) return EINVAL;

	if( p->_handle != INVALID_HANDLE_VALUE ) CloseHandle( p->_handle );

	*handle = OS_INVALID_HANDLE;

	free( p );

	return ENOERROR;
}

TINYFUN enum OsHandleType _GdiHandleGetType( OsHandle handle )
{
	struct GdiAnyHandle *p = (struct GdiAnyHandle *) handle;

	if( p == NULL ) return osNotAnHandle;

	switch( p->_handleType )
	{
	case GDI_HANDLETYPE_WINDOW: return osGuiWindowHandle;
	case GDI_HANDLETYPE_FONT: return osGuiFontHandle;
	case GDI_HANDLETYPE_IMAGE: return osGuiImageHandle;
	default:
		return osNotAnHandle;
	}
}

TINYFUN OsError _GdiHandleDestroy( OsHandle *handle )
{
	struct GdiAnyHandle *p = * (struct GdiAnyHandle **) handle;

	if( p == NULL ) return EINVAL;

	//TODO check magic

	switch( p->_handleType )
	{
	case GDI_HANDLETYPE_WINDOW:
		return GdiDestroyWindow( handle );

	case GDI_HANDLETYPE_FONT:
		return GdiDeleteObject( handle );
		// return GdiCloseHandle( handle );

	case GDI_HANDLETYPE_IMAGE: 
		return DeleteGuiImageHandle( handle );

	default:
		return EINVAL;
	}
}

OsError _GdiSwapBuffers( OsHandle* handle )
{
	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
struct OsGuiSystemTable _systemGDI =
{
	_GdiWindowCreate 
	,_GdiWindowRefresh
	,_GdiSwapBuffers
	,_GdiWindowShow
	,_GdiMouseSetCursor
	,_GdiMouseCapture
	,_GdiMouseRelease
	,_GdiGetClientArea
	,_GdiSetColor
	,_GdiRegionSetArea
	,_GdiRegionGetArea
	,_GdiRegionSetOffset
	,_GdiRegionSetScale
	,_GdiRegionGetSize
	,_GdiPointToCoords
	,_GdiFontCreate
	,_GdiFontCalcSize
	,_GdiSetFont
	,_GdiImageCreate
	,_GdiImageGetInfo
	,_GdiImageWrite
	,_GdiSetAlphaBlend
	,_GdiDrawImage
	,_GdiDrawRectangle
	,_GdiDrawEllipse
	,_GdiDrawLine
	,_GdiDrawText
	,_GdiDrawPolygon
	,_GdiResourceGetType
	,_GdiResourceLoadFromMemory
	,_GdiResourceLoadFromFile
	,_GdiResourceLoadFromApp
	,_GdiHandleGetType
	,_GdiHandleDestroy
};

//////////////////////////////////////////////////////////////////////////
struct OsGuiSystemTable *_guiSystemGDI = &_systemGDI;

//////////////////////////////////////////////////////////////////////////
//