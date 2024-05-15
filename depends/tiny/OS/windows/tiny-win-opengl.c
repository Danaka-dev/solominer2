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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

//////////////////////////////////////////////////////////////////////////
#include <windows.h>		// Header File For Windows
#include <windowsx.h>		// Header File For Windows
#include <dwmapi.h>

#include "imgui/imgui_c.h"

typedef enum { false, true } bool;

//////////////////////////////////////////////////////////////////////////
/*#include <GL/gl.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/glext.h>
#include <GL/glu.h>*/

#include "gl/glad.h"


//////////////////////////////////////////////////////////////////////////
#include "../tiny-os.h"
#include "tiny-win-defs.h"


//Keep this code to pass to glad !!
// https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions#Windows
static void *GetProcAddressWindows(const char* name)
{
	void *p = (void *)wglGetProcAddress(name);
	if (p == 0 ||
		(p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
		(p == (void*)-1))
	{
		HMODULE module = LoadLibraryA("opengl32.dll");
		p = (void *)GetProcAddress(module, name);
	}

	return p;
}



extern struct OsGuiSystemTable *_guiSystemGL;

#define GL_HANDLETYPE_WINDOW	1
#define GL_HANDLETYPE_FONT		2
#define GL_HANDLETYPE_IMAGE		3


struct GLContextHandle
{
	struct OsGuiSystemTable *_guiSystem;

	struct GLWindowHandle *_hwindow;

	HDC _hdc;

	HRGN _hrgn;

	HPEN _hpen;

	HBRUSH _hbrush;

	int _ownedGLObj; //! HPEN + HBRUSH

	BLENDFUNCTION _blendfunction;

	//! region
	struct OsRect _region;

	int _offsetx ,_offsety;

	float _scalex ,_scaley;

};


//////////////////////////////////////////////////////////////////////////
#define GLWINDOWHANDLE_MAGIC	0x06F5CC4BD // 0x01ACD89F4

//////////////////////////////////////////////////////////////////////////
struct GLWindowHandle
{
	uint32_t _magic;

	struct OsGuiSystemTable *_guiSystem;

	uint32_t _handleType;

	//HANDLE _handle;
	

	HDC			_hDC;		// Private GDI Device Context
	HGLRC		_hRC;		// Permanent Rendering Context
	HWND		_hWnd;		// Holds Our Window Handle
	HINSTANCE	_hInstance;		// Holds The Instance Of The Application

    int _width ,_height ,_depth;

	int _resized;

	int _doubleBuffer;

	COLORREF _backgroundColor;

	BYTE _backgroundAlpha;

	void* _imguiContext;
		
	OsEventFunction _function;
	
	void *_userData;
};


static struct GLWindowHandle *NewGLWindowHandle( void )
{
	const size_t size = sizeof(struct GLWindowHandle);

	struct GLWindowHandle *p = (struct GLWindowHandle*) malloc(size);

	if( p == NULL ) return NULL;

	memset( p ,0 ,size );

	p->_magic = GLWINDOWHANDLE_MAGIC;

	return p;
}

static struct GLWindowHandle *CastGLWindowHandle( OsHandle handle )
{
	struct GLWindowHandle *p = (struct GLWindowHandle *) handle;

	if( p == NULL ) return NULL;

	if( p->_magic != GLWINDOWHANDLE_MAGIC ) return NULL;

	return p;
}


//-- win procedure
LRESULT CALLBACK _win_glwindowproc( HWND hWnd ,UINT message ,WPARAM wParam ,LPARAM lParam )
{
	struct GLWindowHandle *p = (struct GLWindowHandle*) GetWindowLongPtr( hWnd ,GWLP_USERDATA );

	CREATESTRUCT *createStruct = NULL;

	struct OsEventMessage eventMessage;

	struct OsPoint points[OS_MAX_MOUSEPOS];

	struct OsRect updateRect;

	struct GLContextHandle context;

	OsError error;

	PAINTSTRUCT ps; HDC hdc;
	
	RECT windowRect;

	WORD htcode;

	int width, height;

	if (p != NULL)
	{
		wglMakeCurrent(p->_hDC, p->_hRC);

		if (p->_imguiContext)
		{
			imgui_setcontext_c(p->_imguiContext);
			imgui_ImplWin32_WndProcHandler_c(hWnd , message , wParam , lParam );
		}
	}
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

		p = (struct GLWindowHandle *) createStruct->lpCreateParams;

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
		context._guiSystem = _guiSystemGL;
		context._blendfunction.SourceConstantAlpha = 255;

		GetClientRect( hWnd ,&windowRect );
		context._region.left = windowRect.left;
		context._region.top = windowRect.top;
		context._region.right = windowRect.right;
		context._region.bottom = windowRect.bottom;

		context._offsetx = context._offsety = 0;
		context._scalex = context._scaley = 1.0;
		
		eventMessage.eventType = osRenderEvent;
		hdc = BeginPaint( hWnd ,&ps );
		eventMessage.renderMessage.updateRect = &updateRect;
		eventMessage.renderMessage.updateRect->left = ps.rcPaint.left;
		eventMessage.renderMessage.updateRect->top = ps.rcPaint.top;
		eventMessage.renderMessage.updateRect->right = ps.rcPaint.right;
		eventMessage.renderMessage.updateRect->bottom = ps.rcPaint.bottom;

		eventMessage.renderMessage.resized = (p->_resized > 0) ? 1 : 0;

		context._hwindow = p;
		context._hdc = hdc;
		context._hpen = NULL;
		context._hbrush = NULL;

		eventMessage.renderMessage.context = (OsGuiContext) &context;

		width = ps.rcPaint.right-ps.rcPaint.left;
		height = ps.rcPaint.bottom-ps.rcPaint.top;
		glViewport(0,0, width, height);						// Reset The Current Viewport

		glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
		glLoadIdentity();									// Reset The Projection Matrix

		// Calculate The Aspect Ratio Of The Window
		//gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);
		glOrtho(0.f, 1.f, 1.f, 0.f, -1.f, 1.f);

		glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
		glLoadIdentity();

		p->_function( &eventMessage ,p->_userData );


		EndPaint( hWnd ,&ps );

		SwapBuffers(hdc);

		/*if( p->_doubleBuffer != 0 )
		{
			BitBlt( hdc ,0,0 ,width,height ,p->_hdcBitmap ,0,0, SRCCOPY );
		}*/

		//! cleanup
		p->_resized = 0;

		/*if( context._hrgn != NULL )
			DeleteObject( context._hrgn );
		if( (context._ownedGLObj & OS_GDIOBJ_OWNBRUSH) != 0 )
			DeleteObject( context._hbrush );
		if( (context._ownedGLObj & OS_GDIOBJ_OWNPEN) != 0 )
			DeleteObject( context._hpen );

		context._ownedGLObj = 0;*/

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

		/*
		Removed because delete mouse button pressed in Up event*/
		if (eventMessage.mouseMessage.mouseAction == osMouseMove)
		{
			eventMessage.keyboardMessage.mouseButton = (enum OsMouseButton) (
				(((wParam & MK_LBUTTON) != 0) ? osLeftMouseButton : osNoMouseButton)
				| (((wParam & MK_MBUTTON) != 0) ? osMiddleMouseButton : osNoMouseButton)
				| (((wParam & MK_RBUTTON) != 0) ? osRightMouseButton : osNoMouseButton)
				);
		}
// 		eventMessage.keyboardMessage.mouseButton |= ((wParam & MK_MBUTTON) != 0) ? osMiddleMouseButton : osNoMouseButton;
// 		eventMessage.keyboardMessage.mouseButton |= ((wParam & MK_RBUTTON) != 0) ? osRightMouseButton : osNoMouseButton;

		points[0].x = (int) GET_X_LPARAM(lParam);
		points[0].y = (int) GET_Y_LPARAM(lParam);

		//eventMessage.mouseMessage.points = 1;
		eventMessage.mouseMessage.pos = points;

		p->_function( &eventMessage ,p->_userData );
	}

	return 0;
}


// Initialize Display and load necessary OpenGL extensions
OsError GLInit()
{
	static bool alreadyinit = false;
	
	if( alreadyinit) return ENOERROR;

	if (!gladLoadGLLoader((GLADloadproc)GetProcAddressWindows))
	{
		MessageBox(NULL,"Failed to initialize GLAD","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		return ERROR;
	}
	
	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping ( NEW )
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	
	alreadyinit = true;

	assert(glGetError() == GL_NO_ERROR);

	return ENOERROR;
}

OsError _GLWindowDestroy( OsHandle *handle )
{
	struct GLWindowHandle *hwndGL = CastGLWindowHandle( handle );
	if (false)//fullscreen)										// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL,0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	if (hwndGL->_hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL,NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hwndGL->_hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hwndGL->_hRC=NULL;										// Set RC To NULL
	}

	if (hwndGL->_hDC && !ReleaseDC(hwndGL->_hWnd, hwndGL->_hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hwndGL->_hDC=NULL;										// Set DC To NULL
	}

	if (hwndGL->_hWnd && !DestroyWindow(hwndGL->_hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hwndGL->_hWnd=NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL", hwndGL->_hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hwndGL->_hInstance=NULL;									// Set hInstance To NULL
	}

	return ENOERROR;
}

OsError _GLWindowCreate( OsHandle *handle ,const char_t *name ,const struct OsGuiWindowProperties *properties ,OsEventFunction eventFunction ,void *userData )
{
	GLuint		pixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASSEX	wc;						// Windows Class Structure
	struct GLWindowHandle *p = NULL;

	DWM_BLURBEHIND bb = {0};
	HRGN hRgn;


	//HINSTANCE hInstance = GetModuleHandle(NULL);

	DWORD style ,exStyle;

	bool fullScreen = properties->style & OS_WINDOWSTYLE_FULLSCREEN;			// Set The Global Fullscreen Flag
	bool transparent = (properties->style & OS_WINDOWSTYLE_TRANSPARENT) != 0;

	//RECT clientRect = { 0 ,0 ,properties->defaultWidth ,properties->defaultHeight };
	int left = 0, right = properties->defaultWidth, 
		top = 0, bottom = properties->defaultHeight;

	RECT screenRect; // , rc;

	unsigned int er;

	static int posx = 100, posy = 50;

	static HWND firstHwnd = NULL;

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_SUPPORT_COMPOSITION |					// Format Must Support Composition
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		32,											// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		8,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	static HGLRC firstRC = NULL;

	SystemParametersInfo(SPI_GETWORKAREA, 0, &screenRect, 0);
	if (fullScreen || transparent)
	{
		left = screenRect.left + 50;
		right = screenRect.right - 100;
		top = screenRect.top + 20;
		bottom = screenRect.bottom - 40;
	}

	if( handle == NULL ) return EINVAL;

	if( *handle != OS_INVALID_HANDLE ) return EEXIST;

	p = NewGLWindowHandle();

	if( p == NULL ) return ENOMEM;

	p->_function = eventFunction;

	p->_userData = userData;

	p->_hInstance = GetModuleHandle(NULL);				// Grab An Instance For Our Window


	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) _win_glwindowproc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= p->_hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hIconSm			= LoadIcon( NULL ,IDI_APPLICATION ); // LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= name;								// Set The Class Name

	if (!RegisterClassEx(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}


	if (false)//fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= properties->defaultWidth;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= properties->defaultHeight;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= 32;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullScreen=FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	if (fullScreen || transparent)												// Are We Still In Fullscreen Mode?
	{
		exStyle = WS_EX_APPWINDOW | WS_EX_TOOLWINDOW;				// Window Extended Style
		style = WS_POPUP;											// Windows Style
		//ShowCursor(FALSE);									// Hide Mouse Pointer
	}
	else
	{
		exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		//style = WS_OVERLAPPEDWINDOW;							// Windows Style
		style = WS_OVERLAPPEDWINDOW ^ ((! (properties->style & OS_WINDOWSTYLE_SIZEABLE)) ? WS_THICKFRAME|WS_MAXIMIZEBOX : 0);
	}

	if (properties->style & OS_WINDOWSTYLE_ONTOP)
		exStyle |= WS_EX_TOPMOST;

	/*if( properties->style & OS_WINDOWSTYLE_TOOLBOX )
		exStyle = WS_EX_TOOLWINDOW;
	else
		exStyle = 0;*/
	//
	//SystemParametersInfo
	p->_hWnd = CreateWindowEx(	exStyle,							// Extended Style For The Window
		name,							// Class Name
		properties->title,								// Window Title
		style |							// Defined Window Style
		WS_CLIPSIBLINGS |					// Required Window Style
		WS_CLIPCHILDREN,					// Required Window Style
		left, top,								// Window Position
		right,	// Calculate Window Width
		bottom,	// Calculate Window Height
		NULL,								// No Parent Window
		NULL,								// No Menu
		p->_hInstance,					// Instance
		(void*) p);						// Don't Pass Anything To WM_CREATE

	// Create The Window
	if (!p->_hWnd)
	{
		free( p );
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	/*if (firstHwnd != NULL)
	{
		SetParent(p->_hWnd, firstHwnd); //  (b, a): a will be the new parent b
		style = GetWindowLong(p->_hWnd, GWL_STYLE); //get the b style
		//style &= ~(WS_POPUP|WS_CAPTION); //reset the "caption" and "popup" bits
		style |= WS_CHILD; //set the "child" bit
		SetWindowLong(p->_hWnd,GWL_STYLE,style); //set the new style of b
		
		GetClientRect(firstHwnd, &rc); //the "inside border" rectangle for a
		MoveWindow(p->_hWnd, rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top, true); //place b at (x,y,w,h) in a
		UpdateWindow(firstHwnd);
	}
	else firstHwnd = p->_hWnd;*/

	if (transparent)
	{
		hRgn = CreateRectRgn(0, 0, -1, -1);
		bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
		bb.hRgnBlur = hRgn;
		bb.fEnable = TRUE;
		DwmEnableBlurBehindWindow(p->_hWnd, &bb);
	}

	//-- set, show & update window
	SetWindowLongPtr( p->_hWnd ,GWLP_USERDATA ,(LONG_PTR) p );

	//p = (struct GLWindowHandle*) GetWindowLongPtr( p->_hWnd ,GWLP_USERDATA );


	p->_hDC = GetDC(p->_hWnd);
	if (!p->_hDC)							// Did We Get A Device Context?
	{
		free( p );
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	pixelFormat = ChoosePixelFormat(p->_hDC,&pfd);
	if (!pixelFormat)	// Did Windows Find A Matching Pixel Format?
	{
		free( p );
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!SetPixelFormat(p->_hDC, pixelFormat, &pfd))		// Are We Able To Set The Pixel Format?
	{
		free( p );
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	p->_hRC = wglCreateContext(p->_hDC);
	if (!p->_hRC)				// Are We Able To Get A Rendering Context?
	{
		free( p );
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (firstRC == NULL)
		firstRC = p->_hRC;
	else
	{
		if (!wglShareLists(firstRC, p->_hRC))
		{
			MessageBox(NULL,"Could not share context", "ERROR", MB_OK | MB_ICONINFORMATION);
		}
	}

	if(!wglMakeCurrent(p->_hDC,p->_hRC))					// Try To Activate The Rendering Context
	{
		free( p );
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}


	p->_guiSystem = _guiSystemGL;

	p->_handleType = GL_HANDLETYPE_WINDOW;

	p->_resized = 1; //! we want a resize on first render event

	p->_backgroundColor = (COLORREF) (properties->backgroundColor & 0x0ffffff);

	p->_backgroundAlpha = ((struct OsColor*) &(properties->backgroundColor))->a;

	p->_doubleBuffer = (properties->flags & OS_WINDOWFLAG_DIRECTDRAW) ? 0 : 1;

	*handle = p;


	if (!OS_SUCCEED(GLInit()))										// Initialize Our Newly Created GL Window
	{
		_GLWindowDestroy((OsHandle*)p);						// Reset The Display
		free(p);
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}


	//-- set, show & update window
	/*SetWindowLongPtr( hWnd ,GWLP_USERDATA ,(LONG_PTR) p );

	AdjustWindowRect( &clientRect ,style^WS_OVERLAPPED ,FALSE );*/


	assert(glGetError() == GL_NO_ERROR);
	if (!transparent && !fullScreen)
	{
		SetWindowPos( p->_hWnd ,HWND_TOP ,posx, posy ,right-left ,bottom-top ,SWP_NOMOVE|SWP_NOZORDER|SWP_NOREDRAW );
		//AdjustWindowRect( &clientRect ,style^WS_OVERLAPPED ,FALSE );
		posx+=20; posy+=20;
	}
	assert(glGetError() == GL_NO_ERROR);
	p->_imguiContext = NULL;
	if (properties->flags & OS_WINDOWFLAG_IMGUI)
		p->_imguiContext = imgui_init_c(p->_hWnd);

	ShowWindow(p->_hWnd,SW_SHOW);						// Show The Window
	assert(glGetError() == GL_NO_ERROR);

	SetForegroundWindow(p->_hWnd);						// Slightly Higher Priority
	assert(glGetError() == GL_NO_ERROR);


	SetFocus(p->_hWnd);									// Sets Keyboard Focus To The Window
	assert(glGetError() == GL_NO_ERROR);

	transparent = UpdateWindow( p->_hWnd );
	er = glGetError();
	assert(glGetError() == GL_NO_ERROR);

	return ENOERROR;
}

TINYFUN void _GLWindowRefresh( OsHandle handle, const struct OsRect *updateRect ,int flags )
{
	struct GLWindowHandle *p = CastGLWindowHandle( handle );

	BOOL bErase = ((flags & OS_REFRESH_BACKGROUND) != 0) ? TRUE : FALSE;

	RECT rect;

	if( p == NULL ) return;

	if( flags & OS_REFRESH_RESIZED )
		++p->_resized;

	if( updateRect != NULL )
	{
		rect.left = updateRect->left; rect.top = updateRect->top;
		rect.right = updateRect->right; rect.bottom = updateRect->bottom;

		InvalidateRect( p->_hWnd ,&rect ,bErase );
	}
	else
		InvalidateRect( p->_hWnd ,NULL ,bErase );

	if( flags & OS_REFRESH_UPDATE )
		UpdateWindow( p->_hWnd );
}

TINYFUN void _GLWindowShow( OsHandle handle, int show)
{
	int width, height;
	struct GLWindowHandle* hwndGL = CastGLWindowHandle(handle);

	width = hwndGL->_width;
	height = hwndGL->_height;

	if (height == 0) height = 1;

	glViewport(0, 0, width, height);					// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

//-- input
TINYFUN void _GLMouseSetCursor( OsHandle handle ,int cursorId )
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

TINYFUN void _GLMouseCapture( OsHandle handle )
{
	struct GLWindowHandle *p = CastGLWindowHandle( handle );

	if( p == NULL ) return;

	SetCapture( p->_hWnd ); // _TODO CHECK THIS
}

TINYFUN void _GLMouseRelease( OsHandle handle )
{
	struct GLWindowHandle *p = CastGLWindowHandle( handle );

	if( p == NULL ) return;

	ReleaseCapture(); //  p->_handle );
}

//-- properties
TINYFUN void _GLGetClientArea( OsHandle handle ,struct OsPoint *size )
{
	struct GLWindowHandle *p = CastGLWindowHandle( handle );

	RECT rect;

	if( p == NULL ) return;

	if( size == NULL ) return;

	GetClientRect( p->_hWnd ,&rect );

	size->x = rect.right - rect.left;

	size->y = rect.bottom - rect.top;
}

//////////////////////////////////////////////////////////////////////////
TINYFUN void _GLSetForeColor( OsGuiContext context ,OsColorRef c )
{
	struct GLContextHandle *osContext = (struct GLContextHandle*) context;

	struct OsColor *C = (struct OsColor*) &c;

	assert( osContext != NULL && osContext->_hdc != NULL );

	/*if( (osContext->_ownedGLObj & OS_GDIOBJ_OWNPEN) != 0 )
	{
		DeleteObject( osContext->_hpen );
		osContext->_ownedGLObj ^= OS_GDIOBJ_OWNPEN;
	}

	if( C->a != 0 )
	{
		osContext->_hpen = CreatePen(  PS_SOLID ,1 ,(COLORREF) (c&0x0FFFFFF) );
		osContext->_ownedGLObj |= OS_GDIOBJ_OWNPEN;
	}
	else 
		osContext->_hpen = GetStockPen( NULL_PEN );

	SelectObject( osContext->_hdc ,osContext->_hpen );*/

}

TINYFUN void _GLSetFillColor( OsGuiContext context ,OsColorRef c )
{
	struct GLContextHandle *osContext = (struct GLContextHandle*) context;

	struct OsColor *C = (struct OsColor*) &c;

	assert( osContext != NULL && osContext->_hdc != NULL );

	/*if( (osContext->_ownedGLObj & OS_GDIOBJ_OWNBRUSH) != 0 )
	{
		DeleteObject( osContext->_hbrush );
		osContext->_ownedGLObj ^= OS_GDIOBJ_OWNBRUSH;
	}

	if( C->a != 0 )
	{
		osContext->_hbrush = CreateSolidBrush( (COLORREF) (c&0x0FFFFFF) );
		osContext->_ownedGLObj |= OS_GDIOBJ_OWNBRUSH;
	}
	else 
		osContext->_hbrush = GetStockBrush( NULL_BRUSH );

	SelectObject( osContext->_hdc ,osContext->_hbrush );*/
}

//>text color
TINYFUN void _GLSetTextColor( OsGuiContext context ,OsColorRef c )
{
	struct GLContextHandle *osContext = (struct GLContextHandle*) context;

	assert( osContext != NULL && osContext->_hdc != NULL );

	SetTextColor( osContext->_hdc ,(COLORREF) (c&0x0FFFFFF) );
}

TINYFUN void _GLSetBackColor( OsGuiContext context ,OsColorRef c )
{
	struct GLContextHandle *osContext = (struct GLContextHandle*) context;

	struct OsColor *C = (struct OsColor*) &c;

	assert( osContext != NULL && osContext->_hdc != NULL );

	SetBkColor( osContext->_hdc ,(COLORREF) (c&0x0FFFFFF) );

	SetBkMode( osContext->_hdc ,(C->a != 0) ? OPAQUE : TRANSPARENT );
}

TINYFUN void _GLSetColor( OsGuiContext context ,int selectColor ,OsColorRef color )
{
	switch( selectColor )
	{
	case OS_SELECT_FORECOLOR: _GLSetForeColor( context ,color ); break;
	case OS_SELECT_FILLCOLOR: _GLSetFillColor( context ,color ); break;
	case OS_SELECT_TEXTCOLOR: _GLSetTextColor( context ,color ); break;
	case OS_SELECT_BACKCOLOR: _GLSetBackColor( context ,color ); break;
	default: break;
	}
}

//////////////////////////////////////////////////////////////////////////
TINYFUN void _GLRegionSetArea( OsGuiContext context ,int left ,int top ,int right ,int bottom ,int useOffset )
{
	struct GLContextHandle *osContext = (struct GLContextHandle*) context;

	BOOL setRectRgn;

	int selectClipRgn;

	assert( osContext != NULL && osContext->_hdc != NULL );

	osContext->_region.left = left;
	osContext->_region.top = top;
	osContext->_region.right = MAX(left,right);
	osContext->_region.bottom = MAX(top,bottom);

	if( osContext->_hrgn == NULL )
		osContext->_hrgn = CreateRectRgn( left ,top ,right ,bottom );

	if( osContext->_hrgn == NULL )
		return;

	setRectRgn = SetRectRgn( osContext->_hrgn ,left ,top ,right ,bottom ); assert( setRectRgn == TRUE );

	selectClipRgn = SelectClipRgn( osContext->_hdc ,osContext->_hrgn ); assert( selectClipRgn != ERROR );

	if( useOffset != 0 )
	{
		osContext->_offsetx = left;
		osContext->_offsety = top;
	}
}

TINYFUN void _GLRegionGetArea( OsGuiContext context ,struct OsRect *area )
{
	/*struct GLContextHandle *osContext = (struct GLContextHandle*) context;

	assert( osContext != NULL && osContext->_hdc != NULL );

	if( area != NULL )
	{
		area->left = osContext->_region.left;
		area->top = osContext->_region.top;
		area->right = osContext->_region.right;
		area->bottom = osContext->_region.bottom;
	}*/

	return;
}

TINYFUN void _GLRegionSetOffset( OsGuiContext context ,int x ,int y )
{
	/*struct GLContextHandle *osContext = (struct GLContextHandle*) context;

	assert( osContext != NULL && osContext->_hdc != NULL );

	osContext->_offsetx = x;
	osContext->_offsety = y;*/

	return;
}

TINYFUN void _GLRegionSetScale( OsGuiContext context ,float x ,float y )
{
	/*struct GLContextHandle *osContext = (struct GLContextHandle*) context;

	assert( osContext != NULL && osContext->_hdc != NULL );

	osContext->_scalex = x;
	osContext->_scaley = y;*/

	return;
}

TINYFUN void _GLRegionGetSize( OsGuiContext context ,int *width ,int *height )
{
	assert( FALSE );
	return;
}

TINYFUN void _GLPointToCoords( OsGuiContext context ,int surfaceForP ,const struct OsPoint *p ,int surfaceForCoords ,struct OsPoint *coords )
{
	assert( FALSE );
	return;
}

//////////////////////////////////////////////////////////////////////////
#define GLFONTHANDLE_MAGIC	0x06F5CC4BD // 0x04D7EFC35

struct GLFontHandle
{
	uint32_t _magic;

	struct OsGuiSystemTable *_guiSystem;

	uint32_t _handleType;

	HANDLE _handle;
};

static struct GLFontHandle *NewGLFontHandle( void )
{
	const size_t size = sizeof(struct GLFontHandle);

	struct GLFontHandle *p = (struct GLFontHandle*) malloc(size);

	if( p == NULL ) return NULL;

	memset( p ,0 ,size );

	p->_magic = GLFONTHANDLE_MAGIC;

	p->_handleType = GL_HANDLETYPE_FONT;

	p->_handle = INVALID_HANDLE_VALUE;

	return p;
}

static struct GLFontHandle *CastGLFontHandle( OsHandle handle )
{
	struct GLFontHandle *p = (struct GLFontHandle *) handle;

	if( p == NULL ) return NULL;

	if( p->_magic != GLFONTHANDLE_MAGIC ) return NULL;

	return p;
}

//--
static int _win_fontpitch_map[] = { DEFAULT_PITCH ,FIXED_PITCH ,VARIABLE_PITCH };

static int _win_fontweight_map[] = { FW_DONTCARE ,FW_THIN ,FW_ULTRALIGHT ,FW_LIGHT ,FW_NORMAL ,FW_MEDIUM ,FW_SEMIBOLD ,FW_BOLD ,FW_ULTRABOLD ,FW_HEAVY };

#define HASFONTSTYLE(__p_,__style_)		(((__p_&__style_)!=0)?TRUE:FALSE)

TINYFUN OsError _GLFontCreate( OsHandle *handle ,const char_t *faceName ,int pointSize ,int weight ,int style ,int family )
{
	struct GLFontHandle *p = NewGLFontHandle();

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

		return ERROR;//GetLastOsError();
	}

	p->_guiSystem = _guiSystemGL;

	p->_handle = hfont;

	*handle = (OsHandle) p;

	return ENOERROR;
}

TINYFUN void _GLFontCalcSize( OsHandle handle ,const char_t *text ,struct OsPoint *size )
{
	struct GLFontHandle *p = CastGLFontHandle( handle );

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

TINYFUN void _GLSetFont( OsGuiContext context ,OsHandle fontHandle )
{
	struct GLContextHandle *osContext = (struct GLContextHandle*) context;

	struct GLFontHandle *p = CastGLFontHandle( fontHandle );

	if( p == NULL ) return;

	assert( osContext != NULL && osContext->_hdc != NULL );

	SelectObject( osContext->_hdc ,p->_handle );
}

//////////////////////////////////////////////////////////////////////////
//-- image 
#define GUIIMAGEHANDLE_MAGIC	0x06F5CC4BD // 0x079A3F45C

struct GLImageHandle
{
	uint32_t _magic;

	struct OsGuiSystemTable *_guiSystem;

	uint32_t _handleType;

	HBITMAP _hbitmap;

	BITMAP _bm;

	HDC _hdc;
};

static struct GLImageHandle *NewGLImageHandle( void )
{
	const size_t size = sizeof(struct GLImageHandle);

	struct GLImageHandle *p = (struct GLImageHandle*) malloc(size);

	if( p == NULL ) return NULL;

	memset( p ,0 ,size );

	p->_magic = GUIIMAGEHANDLE_MAGIC;

	p->_handleType = GL_HANDLETYPE_IMAGE;

	return p;
}

static struct GLImageHandle *CastGLImageHandle( OsHandle handle )
{
	struct GLImageHandle *p = (struct GLImageHandle *) handle;

	if( p == NULL ) return NULL;

	if( p->_magic != GUIIMAGEHANDLE_MAGIC ) return NULL;

	return p;
}

static OsError DeleteGLImageHandle( OsHandle *handle )
{
	struct GLImageHandle *p = * (struct GLImageHandle **) handle;

	if( *handle == OS_INVALID_HANDLE ) return ENOERROR;

	if( p == NULL ) return EINVAL;

	if( p->_hbitmap != NULL ) DeleteObject( p->_hbitmap );

	*handle = OS_INVALID_HANDLE;

	free( p );

	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
//> GUI image function
TINYFUN OsError _GLImageCreate( OsHandle *handle ,int width ,int height )
{
	struct GLImageHandle *p = NewGLImageHandle();

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

	p->_guiSystem = _guiSystemGL;

	*handle = (OsHandle) p;

	return ENOERROR;
}

TINYFUN void _GLImageGetInfo( OsHandle handle ,struct OsGuiImageInfo *info )
{
	struct GLImageHandle *p = CastGLImageHandle( handle );

	assert( p != NULL && info != NULL );

	if( p == NULL || info == NULL ) return;

	assert(false);
	/*info->width = p->_bm.bmWidth;
	info->height = p->_bm.bmHeight;*/
}

TINYFUN OsError _GLImageWrite( OsHandle handle ,const uint8_t *pixelValues ,size_t size )
{
	struct GLImageHandle *p = CastGLImageHandle( handle );

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

TINYFUN void _GLSetAlphaBlend( OsGuiContext context ,float alpha )
{
	struct GLContextHandle *osContext = (struct GLContextHandle*) context;

	assert( osContext != NULL && osContext->_hdc != NULL );

	osContext->_blendfunction.BlendOp = AC_SRC_OVER;
	osContext->_blendfunction.BlendFlags = 0;
	osContext->_blendfunction.AlphaFormat = 0;
	osContext->_blendfunction.SourceConstantAlpha = (alpha < 1.0) ? ((alpha > 0) ? (BYTE) (255*alpha) : 0) : 255;

	return;
}

//-- render
#define _TX(_x_)	(osgc->_offsetx+(int)((_x_)*osgc->_scalex))
#define _TY(_x_)	(osgc->_offsety+(int)((_x_)*osgc->_scaley))

TINYFUN void _GLDrawImage( OsGuiContext context ,int left ,int top ,int right ,int bottom ,OsHandle handle ,int imageLeft ,int imageTop ,int imageRight ,int imageBottom )
{
	/*	Display *xdisplay = _linux_getdisplay();
    
	struct GLWindowHandle* hwndGL = CastGLWindowHandle(hwin);
	struct GLImageHandle *img = CastGLImageHandle( himg );
	XImage* xim = NULL;
	
	unsigned char fakeTexture[] = { 255, 255, 0, 
								255, 255, 0, 
								0, 0, 0, 
								0, 0, 0, 
								255, 255, 0,
								255, 255, 0,
								0, 0, 0, 
								0, 0, 0, 
								0, 0, 0,
								0, 0, 0,
								255, 255, 0, 
								255, 255, 0, 
								0, 0, 0,
								0, 0, 0,
								255, 255, 0, 
								255, 255, 0 };

	assert (hwndGL->_magic == GLWINDOWHANDLE_MAGIC);
	if (hwndGL->_magic != GLWINDOWHANDLE_MAGIC) return EFAILED;

	if( p == NULL ) return EFAILED;

	glEnable(GL_TEXTURE_RECTANGLE_ARB);
	glClearColor(1.f, 1.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(left, right, bottom, top);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Update the texture
	//////////////////////
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_RECTANGLE_ARB);
	glDisable(GL_LIGHTING);
	
	// Create the texture if it does not exist
	if (textureIndex == 0)
	{
		glGenTextures(1, &textureIndex);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB/, textureIndex);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, 4, 4, 0, GL_RGB, GL_UNSIGNED_BYTE, fakeTexture);
	}
	else
	{
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, textureIndex);
	}
	
	// Get Access to the pixmap image
	xim = XGetImage(xdisplay, p->_xpixmap, 0, 0, p->_width, p->_height, AllPlanes, ZPixmap);
	
	// Copy the image buffer into the texture
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, p->_width, p->_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)(&(xim->data[0])));

	glColor3f(0.f, .8f, 0.f);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(left, top);
		
		glTexCoord2f(p->_width, 0);
		glVertex2f(right, top);
		
		glTexCoord2f(p->_width, p->_height);
		glVertex2f(right, bottom);
		
		glTexCoord2f(0, p->_height);
		glVertex2f(left, bottom);
	glEnd();*/

	/*struct GLContextHandle *osContext = (struct GLContextHandle*) context;

	struct GLImageHandle *p = CastGLImageHandle( handle );

	int destWidth = right - left; //TODO _TX / _TY

	int destHeight = bottom - top;

	int srcWidth = imageRight - imageLeft;

	int srcHeight = imageBottom - imageTop;

	BOOL berror = TRUE;

	if( p == NULL ) { assert( FALSE ); return; }

	assert( osContext != NULL && osContext->_hdc != NULL );
	
	assert( osContext->_guiSystem == p->_guiSystem );

	if( right < 0 ) destWidth = p->_bm.bmWidth; else if( left > right ) left = right ,destWidth = -destWidth;

	if( bottom < 0 ) destHeight = p->_bm.bmHeight; else if( top > bottom ) top = bottom ,destHeight = -destHeight;

	if( imageRight < 0 ) srcWidth = p->_bm.bmWidth; else if( imageLeft > imageRight ) imageLeft = imageRight ,srcWidth = -srcWidth;

	if( imageBottom < 0 ) srcHeight = p->_bm.bmHeight; else if( imageTop > imageBottom ) imageTop = imageBottom ,srcHeight = -srcHeight;

	if( osContext->_blendfunction.SourceConstantAlpha != 255 )
		berror = AlphaBlend( osContext->_hdc ,left ,top ,destWidth ,destHeight ,p->_hdc ,imageLeft ,imageTop ,srcWidth ,srcHeight ,osContext->_blendfunction );

	else if( destWidth != srcWidth || destHeight != srcHeight )
		berror = StretchBlt( osContext->_hdc ,left ,top ,destWidth ,destHeight ,p->_hdc ,imageLeft ,imageTop ,srcWidth ,srcHeight ,SRCCOPY );

	else
		berror = BitBlt( osContext->_hdc ,left ,top ,destWidth ,destHeight ,p->_hdc ,imageLeft ,imageTop ,SRCCOPY );*/

	// assert( berror == TRUE );

	return;
}

TINYFUN void  _GLDrawRectangle( OsGuiContext context ,int left ,int top ,int right ,int bottom )
{
	struct GLContextHandle *osgc = (struct GLContextHandle*) context;

	assert( osgc != NULL && osgc->_hdc != NULL );

	Rectangle( osgc->_hdc ,_TX(left) ,_TY(top) ,_TX(right) ,_TY(bottom) );

	return;
}

TINYFUN void  _GLDrawEllipse( OsGuiContext context ,int left ,int top ,int right ,int bottom )
{
	struct GLContextHandle *osgc = (struct GLContextHandle*) context;

	assert( osgc != NULL && osgc->_hdc != NULL );

	Ellipse( osgc->_hdc ,_TX(left) ,_TY(top) ,_TX(right) ,_TY(bottom) );

	return;
}

TINYFUN void  _GLDrawLine( OsGuiContext context ,int left ,int top ,int right ,int bottom )
{
	struct GLContextHandle *osgc = (struct GLContextHandle*) context;

	assert( osgc != NULL && osgc->_hdc != NULL );

	MoveToEx( osgc->_hdc ,_TX(left) ,_TY(top) ,NULL );

	LineTo( osgc->_hdc ,_TX(right) ,_TY(bottom) );

	return;
}

static int _win_textalign_map[] = { DEFAULT_PITCH ,FIXED_PITCH ,VARIABLE_PITCH };

TINYFUN void  _GLDrawText( OsGuiContext context ,const char_t *text ,int left ,int top ,int right ,int bottom ,int align ,struct OsRect *rect )
{
	struct GLContextHandle *osgc = (struct GLContextHandle*) context;

	RECT winrect;

	UINT format = DT_SINGLELINE | (UINT) align;

	assert( osgc != NULL && osgc->_hdc != NULL );

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

	return;
}

TINYFUN void  _GLDrawPolygon( OsGuiContext context ,int npoints ,const struct OsPoint *points )
{
	struct GLContextHandle *osgc = (struct GLContextHandle*) context;

	struct OsPoint p[32];

	int i;

	assert( osgc != NULL && osgc->_hdc != NULL );

	if( npoints > 32 ) npoints = 32;

	for( i=0; i<npoints; ++i )
	{
		p[i].x = _TX( points[i].x );
		p[i].y = _TY( points[i].y );
	}

	Polygon( osgc->_hdc ,(POINT*) p ,npoints );

	return;
}


//-- resource
TINYFUN int _GLResourceGetType( OsHandle handle )
{
	struct GLImageHandle *p = CastGLImageHandle( handle );

	if( p != NULL ) return OS_GUIRESOURCETYPE_IMAGE;

	assert( FALSE );

	return 0;
}

TINYFUN OsError _GLResourceLoadFromMemory( OsHandle *handle ,int resourceTypeHint ,uint8_t *memory ,const void *resourceInfo )
{
	assert( FALSE );

	return ENOSYS;
}

static OsError _win_loadimage( OsHandle *handle ,HINSTANCE hinstance ,const char_t *name ,UINT fLoad )
{
	struct GLImageHandle *p = NewGLImageHandle();

	if( p == NULL ) return ENOMEM;

	/*p->_hdc = CreateCompatibleDC(NULL);

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

	p->_guiSystem = _guiSystemGL;

	*handle = (OsHandle) p;*/

	return ENOERROR;
}

TINYFUN OsError _GLResourceLoadFromFile( OsHandle *handle ,int resourceTypeHint ,const char_t *filename )
{
	struct GLImageHandle *p = NULL;

	OsError error = ENOERROR;

	if( handle == NULL || filename == NULL ) return EINVAL;

	if( *handle != OS_INVALID_HANDLE ) return EEXIST;

	if( resourceTypeHint == OS_GUIRESOURCETYPE_ANY ) return ENOSYS;

	if( resourceTypeHint == OS_GUIRESOURCETYPE_IMAGE )
		return _win_loadimage( handle ,NULL ,filename ,LR_LOADFROMFILE );

	assert( FALSE );

	return ENOSYS;
}

TINYFUN OsError _GLResourceLoadFromApp( OsHandle *handle ,int resourceTypeHint ,int resourceId ,const char_t *application )
{
	struct GLImageHandle *p = NULL;

	OsError error = ENOERROR;

	if( handle == NULL ) return EINVAL;

	if( *handle != OS_INVALID_HANDLE ) return EEXIST;

	if( resourceTypeHint == OS_GUIRESOURCETYPE_ANY ) return ENOSYS;

	if( resourceTypeHint == OS_GUIRESOURCETYPE_IMAGE )
		return _win_loadimage( handle ,GetModuleHandle(application) ,MAKEINTRESOURCE(resourceId) ,LR_DEFAULTCOLOR );

	assert( FALSE );

	return ENOSYS;
}

//////////////////////////////////////////////////////////////////////////
struct GLAnyHandle 
{
	uint32_t _magic;

	struct OsGuiSystemTable *_guiSystem;

	uint32_t _handleType;

	HANDLE _handle;
};

OsError GLDestroyWindow( OsHandle *handle )
{
	struct GLAnyHandle *p = * (struct GLAnyHandle **) handle;

	if( *handle == OS_INVALID_HANDLE ) return ENOERROR;

	if( p == NULL ) return EINVAL;

	if( p->_handle != INVALID_HANDLE_VALUE ) DestroyWindow( (HWND) p->_handle );

	*handle = OS_INVALID_HANDLE;

	free( p );

	return ENOERROR;
}

OsError GLDeleteObject( OsHandle *handle )
{
	struct GLAnyHandle *p = * (struct GLAnyHandle **) handle;

	if( *handle == OS_INVALID_HANDLE ) return ENOERROR;

	if( p == NULL ) return EINVAL;

	if( p->_handle != INVALID_HANDLE_VALUE ) DeleteObject( (HGDIOBJ) p->_handle );

	*handle = OS_INVALID_HANDLE;

	free( p );

	return ENOERROR;
}

OsError GLCloseHandle( OsHandle *handle )
{
	struct GLAnyHandle *p = * (struct GLAnyHandle **) handle;

	if( *handle == OS_INVALID_HANDLE ) return ENOERROR;

	if( p == NULL ) return EINVAL;

	if( p->_handle != INVALID_HANDLE_VALUE ) CloseHandle( p->_handle );

	*handle = OS_INVALID_HANDLE;

	free( p );

	return ENOERROR;
}

enum OsHandleType _GLHandleGetType( OsHandle handle )
{
	struct GLAnyHandle *p = (struct GLAnyHandle *) handle;

	if( p == NULL ) return osNotAnHandle;

	switch( p->_handleType )
	{
	case GL_HANDLETYPE_WINDOW: return osGuiWindowHandle;
	case GL_HANDLETYPE_FONT: return osGuiFontHandle;
	case GL_HANDLETYPE_IMAGE: return osGuiImageHandle;
	default:
		return osNotAnHandle;
	}
}

TINYFUN OsError _GLHandleDestroy( OsHandle *handle )
{
	struct GLAnyHandle *p = * (struct GLAnyHandle **) handle;

	if( p == NULL ) return EINVAL;

	//TODO check magic

	switch( p->_handleType )
	{
	case GL_HANDLETYPE_WINDOW:
		return GLDestroyWindow( handle );

	case GL_HANDLETYPE_FONT:
		return GLDeleteObject( handle );
		// return GLCloseHandle( handle );

	case GL_HANDLETYPE_IMAGE: 
		return DeleteGLImageHandle( handle );

	default:
		return EINVAL;
	}
}

OsError _GLSwapBuffers( OsHandle* handle)
{
	struct GLWindowHandle* hwndGL = CastGLWindowHandle(handle);

	SwapBuffers(hwndGL->_hDC);

	return ENOERROR;
}




























OsError GLWindowResize( OsHandle *handle )
{
	int width, height;
	struct GLWindowHandle* hwndGL = CastGLWindowHandle(handle);

	width = hwndGL->_width;
	height = hwndGL->_height;
	
	if (height == 0) height = 1;

	glViewport(0, 0, width, height);					// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
	
	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
/*static struct GLImageHandle *CastGLImageHandle( OsHandle handle )
{
	struct GLImageHandle *p = (struct GLImageHandle *) handle;

	if( p == NULL ) return NULL;

	if( p->_magic != GLIMAGEHANDLE_MAGIC ) return NULL;

	return p;
}*/

// TODO uniformize GUI and GL displays _linux_getdisplay();


//////////////////////////////////////////////////////////////////////////
struct OsGuiSystemTable _systemGL =
{
	_GLWindowCreate 
	,_GLWindowRefresh
	,_GLSwapBuffers
	,_GLWindowShow
	,_GLMouseSetCursor
	,_GLMouseCapture
	,_GLMouseRelease
	,_GLGetClientArea
	,_GLSetColor
	,_GLRegionSetArea
	,_GLRegionGetArea
	,_GLRegionSetOffset
	,_GLRegionSetScale
	,_GLRegionGetSize
	,_GLPointToCoords
	,_GLFontCreate
	,_GLFontCalcSize
	,_GLSetFont
	,_GLImageCreate
	,_GLImageGetInfo
	,_GLImageWrite
	,_GLSetAlphaBlend
	,_GLDrawImage
	,_GLDrawRectangle
	,_GLDrawEllipse
	,_GLDrawLine
	,_GLDrawText
	,_GLDrawPolygon
	,_GLResourceGetType
	,_GLResourceLoadFromMemory
	,_GLResourceLoadFromFile
	,_GLResourceLoadFromApp
	,_GLHandleGetType
	,_GLHandleDestroy
};



//////////////////////////////////////////////////////////////////////////
struct OsGuiSystemTable *_guiSystemGL = &_systemGL;
