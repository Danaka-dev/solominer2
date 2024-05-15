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
#include <X11/X.h>
#include <X11/Xlib.h>

//////////////////////////////////////////////////////////////////////////
//#include <GL/glx.h>
//#include <GL/glxext.h>

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <EGL/egl.h>

// Mali SDK
#include "Platform.h"
#include "Shader.h"
#include "Matrix.h"

#include "linux/tinyc-linux-defs.h"


//////////////////////////////////////////////////////////////////////////
#include "tinyc-os.h"
#include "tinyc-linux-x11.h"

extern struct OsGuiSystemTable *_guiSystemOPENGLES;

using namespace MaliSDK;


// Display access
extern Display *_linux_peekdisplay( void );

extern Display *_linux_getdisplay( void );

/*typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

glXCreateContextAttribsARBProc glXCreateContextAttribsARB;*/

// Note: this function could be shared among different OS
static Bool isExtensionSupported(const char *extList, const char *extension)
{
  const char *start;
  const char *where, *terminator;
  
  /* Extension names should not have spaces. */
  where = strchr(extension, ' ');
  
  if (where || *extension == '\0')
    return False;

  /* It takes a bit of care to be fool-proof about parsing the
     OpenGL extensions string. Don't be fooled by sub-strings,
     etc. */
  for (start=extList;;) {
    where = strstr(start, extension);

    if (!where)
      break;

    terminator = where + strlen(extension);

    if ( where == start || *(where - 1) == ' ' )
      if ( *terminator == ' ' || *terminator == '\0' )
        return True;

    start = terminator;
  }

  return False;
}


//////////////////////////////////////////////////////////////////////////
#define GLWINDOWHANDLE_MAGIC	0x06F5C89FF

struct GLWindowHandle
{
	struct GuiWindowHandle _X11Window;

    // Extra data needed for OpenGL
	//GLXContext _xglcontext;
};

extern OsError LinuxAddWindowToThread( Window window ,struct GLWindowHandle *handle );

// Frame Buffer config
/*static int visual_attribs[] =
{
  GLX_X_RENDERABLE    , True,
  GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
  GLX_RENDER_TYPE     , GLX_RGBA_BIT,
  GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
  GLX_RED_SIZE        , 8,
  GLX_GREEN_SIZE      , 8,
  GLX_BLUE_SIZE       , 8,
  GLX_ALPHA_SIZE      , 8,
  GLX_DEPTH_SIZE      , 24,
  GLX_STENCIL_SIZE    , 8,
  GLX_DOUBLEBUFFER    , True,
  //GLX_SAMPLE_BUFFERS  , 1,
  //GLX_SAMPLES         , 4,
  None
};*/

static struct GLWindowHandle *NewGLWindowHandle( void )
{
	const size_t size = sizeof(struct GLWindowHandle);

	struct GLWindowHandle *p = (struct GLWindowHandle*) malloc(size);

	if( p == NULL ) return NULL;

	memset( p ,0 ,size );

	p->_X11Window._magic = GLWINDOWHANDLE_MAGIC;
	
	p->_X11Window._guiSystem = _guiSystemOPENGLES;

	return p;
}

static struct GLWindowHandle *CastGLWindowHandle( OsHandle handle )
{
	struct GLWindowHandle *p = (struct GLWindowHandle *) handle;

	if( p == NULL ) return NULL;

	if( p->_X11Window._magic != GLWINDOWHANDLE_MAGIC ) return NULL;

	return p;
}

// Initialize Display and load necessary OpenGL extensions
/*OsError GLInit()
{
	const char *glxExts;
    
	static Bool alreadyinit = False;
	
	if( alreadyinit ) return ENOERROR;
	
	Display *xdisplay = _linux_getdisplay();
	if (!xdisplay)
	{
		printf("Failed to open X display\n");
		return EFAILED;
	}

	// Get the default screen's GLX extension list
	glxExts = glXQueryExtensionsString( xdisplay,
                                   DefaultScreen( xdisplay ) );

	glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC) glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );
	// Check for the GLX_ARB_create_context extension string and the function.
	// If either is not present, use GLX 1.3 context creation method.
	if ( !isExtensionSupported( glxExts, "GLX_ARB_create_context" ) ||
       !glXCreateContextAttribsARB )
	{
		printf( "glXCreateContextAttribsARB() not found"
            " ... using old-style GLX context\n" );
	}
	
	alreadyinit = True;
	return ENOERROR;
}*/

/*OsError GLClose()
{
	Display *xdisplay = _linux_getdisplay();

	XCloseDisplay( xdisplay );
	
	return ENOERROR;
}*/

OsError _GLWindowCreate( OsHandle *handle ,const char_t *name ,const struct OsGuiWindowProperties *properties ,OsEventFunction eventFunction ,void *userData )
{
	//XSetWindowAttributes swa;
	struct GLWindowHandle* hwndGL = NULL;
	//GLXFBConfig *fbc = NULL;
	//XVisualInfo *vi = NULL;
	//GLXFBConfig bestFbc;
	int width = 100, height = 100;
	//int glx_major, glx_minor;
	//int fbcount, best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
	//Bool ctxErrorOccurred = False;
	Display *xdisplay = _linux_getdisplay();
	
	//if (GLInit() != ENOERROR) return EFAILED;
	
    if( properties != NULL )
    {
        width = properties->defaultWidth;
        height = properties->defaultHeight;
    }

	/* Intialise the Platform object for platform specific functions. */
    Platform* platform = Platform::getInstance();

    if(platform != NULL)
    {
        /* Initialize windowing system. */
        platform->createWindow(width, height);

        /* Initialize EGL. */
        EGLRuntime::initializeEGL(EGLRuntime::OPENGLES3, NULL);//xdisplay);
	}
    else
    {
        LOGE("Could not create platform.");
    }

	hwndGL = NewGLWindowHandle();
	hwndGL->_X11Window._function = eventFunction;
	hwndGL->_X11Window._userData = userData;
	hwndGL->_X11Window._xwindow = Platform::getInstance()->window;
	
	EGL_CHECK(eglMakeCurrent(EGLRuntime::display, EGLRuntime::surface, EGLRuntime::surface, EGLRuntime::context));

	*handle = (OsHandle)hwndGL;
	
	return ENOERROR;
}

OsError _GLWindowDestroy( OsHandle *handle )
{
	/*Display *xdisplay = _linux_getdisplay();
	struct GLWindowHandle* hwndGL = (struct GLWindowHandle*)handle;
	assert (hwndGL->_X11Window._magic == GLWINDOWHANDLE_MAGIC);
	if (hwndGL->_X11Window._magic != GLWINDOWHANDLE_MAGIC) return EFAILED;
	
	glXMakeCurrent( xdisplay, 0, 0 );
	glXDestroyContext( xdisplay, hwndGL->_xglcontext );
	XDestroyWindow( xdisplay, hwndGL->_X11Window._xwindow );
	XFreeColormap( xdisplay, hwndGL->_X11Window._xcolormap );*/

	// TODO: destroy resources
	/* Delete textures. */
/*	for (int i = 0; i < numberOfTextures; i++)
	{
		GL_CHECK(glDeleteTextures(1, &imageArray[i].textureId));
	}*/

	/* Delete buffers. */
//	GL_CHECK(glDeleteBuffers(2, bufferObjectIds));

	/* Delete vertex array object. */
//	GL_CHECK(glDeleteVertexArrays(1, &vertexArrayId));

	/* Release text object instance. */
/*	if (internalformatTextDisplayer != NULL)
	{
		delete internalformatTextDisplayer;
		internalformatTextDisplayer = NULL;
	}*/

	/* Shut down EGL. */
	EGLRuntime::terminateEGL();

    Platform* platform = Platform::getInstance();

    if(platform != NULL)
    {
		/* Shut down windowing system. */
		platform->destroyWindow();

		/* Shut down the Platform object. */
		delete platform;
	}
	return ENOERROR;
}

void _GLWindowRefresh( OsHandle handle, const struct OsRect *updateRect ,int refresh)
{
	struct GLWindowHandle *p = CastGLWindowHandle( handle );

    //Display *xdisplay = _linux_peekdisplay();
	Display *xdisplay = Platform::getInstance()->display;
    
    XEvent xevent;
    
	if( p == NULL || xdisplay == NULL ) return/* EFAILED*/;

    xevent.type = Expose;
    xevent.xexpose.window = p->_X11Window._xwindow;
    
    Status test = XSendEvent( xdisplay ,p->_X11Window._xwindow ,False ,ExposureMask ,&xevent);
    XFlush( xdisplay );

	return;
}

OsError _GLWindowSwapBuffers( OsHandle* handle )
{
	/*struct GLWindowHandle *p = CastGLWindowHandle( handle );
	Display *xdisplay = _linux_getdisplay();

	glXSwapBuffers ( xdisplay, p->_X11Window._xwindow );*/
	
	/* 
	 * Push the EGL surface color buffer to the native window.
	 * Causes the rendered graphics to be displayed on screen.
	 */
	eglSwapBuffers(EGLRuntime::display, EGLRuntime::surface);
    
	return ENOERROR;
}

void _GLWindowShow( OsHandle handle, int show)
{
	//OsGLWindowResize(handle);// TODO check if not called twice

	return /*ENOERROR*/;
}

//////////////////////////////////////////////////////////////////////////
//! Window common

//-- state
void _GLGetClientArea( OsHandle handle ,struct OsPoint *size )
{
	/*struct GLWindowHandle *p = CastGLWindowHandle( handle );
    
    Display *xdisplay = _linux_peekdisplay();

    XWindowAttributes xattributes;
    
	if( p == NULL || size == NULL || xdisplay == NULL ) return;

    XGetWindowAttributes( xdisplay ,p->_X11Window._xwindow ,&xattributes );
    
    size->x = xattributes.width;
    
    size->y = xattributes.height;*/
}

void _GLSetClipArea( OsHandle handle ,int left ,int top ,int right ,int bottom )
{
	assert( 0 );
}



//////////////////////////////////////////////////////////////////////////
void _GLSetForeColor( OsGuiContext context ,OsColorRef c )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	struct OsColor *C = (struct OsColor*) &c;

    assert( osContext != NULL && osContext->_xdisplay != NULL /*&& osContext->_xgc != 0*/ );
    
    osContext->_xforecolor.red = C->r * 256;
    osContext->_xforecolor.green = C->g * 256;
    osContext->_xforecolor.blue = C->b * 256;
    osContext->_xforecolor.flags = DoRed|DoGreen|DoBlue;
    
    /*XAllocColor( osContext->_xdisplay ,osContext->_hwindow->_xcolormap ,&osContext->_xforecolor );
    
    XSetForeground( osContext->_xdisplay ,osContext->_xgc ,osContext->_xforecolor.pixel );*/
	assert(0);
}

void _GLSetFillColor( OsGuiContext context ,OsColorRef c )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	struct OsColor *C = (struct OsColor*) &c;

    assert( osContext != NULL && osContext->_xdisplay );
    
    osContext->_xfillcolor.red = C->r * 256;
    osContext->_xfillcolor.green = C->g * 256;
    osContext->_xfillcolor.blue = C->b * 256;
    osContext->_xfillcolor.flags = DoRed|DoGreen|DoBlue;
    
    /*XAllocColor( osContext->_xdisplay ,osContext->_hwindow->_xcolormap ,&osContext->_xfillcolor );
    
    osContext->_xnofill = (C->a == 0) ? True : False;*/
	assert(0);
}

//>text color
void _GLSetTextColor( OsGuiContext context ,OsColorRef c )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	struct OsColor *C = (struct OsColor*) &c;

    assert( osContext != NULL && osContext->_xdisplay != NULL );
    
    osContext->_xtextcolor.red = C->r * 256;
    osContext->_xtextcolor.green = C->g * 256;
    osContext->_xtextcolor.blue = C->b * 256;
    osContext->_xtextcolor.flags = DoRed|DoGreen|DoBlue;
	
	assert(0);
    
    //XAllocColor( osContext->_xdisplay ,osContext->_hwindow->_xcolormap ,&osContext->_xtextcolor );
}

void _GLSetBackColor( OsGuiContext context ,OsColorRef c )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	struct OsColor *C = (struct OsColor*) &c;

    assert( osContext != NULL && osContext->_xdisplay != NULL /*&& osContext->_xgc != 0*/ );
    
    osContext->_xbackcolor.red = C->r * 256;
    osContext->_xbackcolor.green = C->g * 256;
    osContext->_xbackcolor.blue = C->b * 256;
    osContext->_xbackcolor.flags = DoRed|DoGreen|DoBlue;
    
	assert(0);
    /*XAllocColor( osContext->_xdisplay ,osContext->_hwindow->_xcolormap ,&osContext->_xbackcolor );
    
    osContext->_xnoback = (C->a == 0) ? True : False;
    
    XSetBackground( osContext->_xdisplay ,osContext->_xgc ,osContext->_xbackcolor.pixel );*/
}

void _GLSetColor( OsGuiContext context ,int selectColor ,OsColorRef color )
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
void _GLRegionSetArea( OsGuiContext context ,int left ,int top ,int right ,int bottom ,int useOffset )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;
    	
    assert( osContext != NULL && osContext->_xdisplay != NULL /*&& osContext->_xgc != 0*/ );
    
	assert(0);
    /*XRectangle rect;
    
    rect.x = rect.y = 0;
    rect.width = right - left;
    rect.height = bottom - top;
    
    XSetClipRectangles( osContext->_xdisplay ,osContext->_xgc ,left ,top ,&rect ,1 ,Unsorted );*/
}

void _GLRegionGetArea( OsGuiContext context ,struct OsRect *area )
{
	assert(0);
	/*struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	assert( osContext != NULL && osContext->_xdisplay != NULL && osContext->_xgc != 0 );

	if( area != NULL )
	{
		area->left = osContext->_region.left;
		area->top = osContext->_region.top;
		area->right = osContext->_region.right;
		area->bottom = osContext->_region.bottom;
	}*/
}

void _GLRegionSetOffset( OsGuiContext context ,int x ,int y )
{
	assert(0);
	/*struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	assert( osContext != NULL && osContext->_xdisplay != NULL && osContext->_xgc != 0 );

	osContext->_offsetx = x;
	osContext->_offsety = y;*/
}

void _GLRegionSetScale( OsGuiContext context ,float x ,float y )
{
	assert(0);
	/*struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	assert( osContext != NULL && osContext->_xdisplay != NULL && osContext->_xgc != 0 );

	osContext->_scalex = x;
	osContext->_scaley = y;*/
}

void _GLRegionGetSize( OsGuiContext context ,int *width ,int *height )
{
	_ASSERT( 0 );
}

void _GLPointToCoords( OsGuiContext context ,int surfaceForP ,const struct OsPoint *p ,int surfaceForCoords ,struct OsPoint *coords )
{
	_ASSERT( 0 );
}

//-- Mouse
void _GLMouseSetCursor( OsHandle handle ,int cursorId )
{
	assert(0);
}

void _GLMouseCapture( OsHandle handle )
{
	assert( 0 );
}

void _GLMouseRelease( OsHandle handle )
{
	assert( 0 );
}

OsError _GLWindowResize( OsHandle *handle )
{
	int width, height;
	struct GLWindowHandle* hwndGL = (struct GLWindowHandle*)handle;
	assert (hwndGL->_X11Window._magic == GLWINDOWHANDLE_MAGIC);
	if (hwndGL->_X11Window._magic != GLWINDOWHANDLE_MAGIC) return EFAILED;
	
	width = hwndGL->_X11Window._width;
	height = hwndGL->_X11Window._height;
	
	if (height == 0) height = 1;
	if (width == 0) width = 1;

	/*
	glViewport(0, 0, width, height);					// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
	*/

	return ENOERROR;
}


//////////////////////////////////////////////////////////////////////////
OsError _GLFontCreate( OsHandle *handle ,const char_t *faceName ,int pointSize ,int weight ,int style ,int family )
{
	assert( 0 );
	return ENOERROR;
}

void _GLFontCalcSize( OsHandle handle ,const char_t *text ,struct OsPoint *size )
{
	assert(0);
	//return ENOERROR;
}

void _GLSetFont( OsGuiContext context ,OsHandle fontHandle )
{
	assert(0);
	//return ENOERROR;
}


//////////////////////////////////////////////////////////////////////////
//-- image 
#define GLIMAGEHANDLE_MAGIC	0x06F5CF45C // 0x079A3F45C
// TODO: same ID as GUIIMAGEHANDLE_MAGIC to allow dirty cast, should be refactored


struct GLImageHandle
{
	uint32_t _magic;
	struct OsGuiSystemTable *_guiSystem; //! must be second
	
	unsigned int _width, _height;
	const unsigned char* _buffer;
	
	// Extra info needed for OpenGL
	unsigned int textureId;
	unsigned int bufferObjectId[2];// 0 for vertices and 1 for texture coordinates
	unsigned int arrayObjectId;
	unsigned int programId;
	unsigned int vertexShaderId;
	unsigned int fragmentShaderId;
	int			 modelViewMatrixLocation;
	int 		 positionLocation;
	int 		 textureCoordinateLocation;
	int 		 textureLocation;
};

static struct GLImageHandle *NewGLImageHandle( void )
{
	const size_t size = sizeof(struct GLImageHandle);

	struct GLImageHandle *p = (struct GLImageHandle*) malloc(size);

	if( p == NULL ) return NULL;

	memset( p ,0 ,size );

	p->_magic = GLIMAGEHANDLE_MAGIC;
	
	p->_guiSystem = _guiSystemOPENGLES;
	
	p->textureId = 0;

	return p;
}

static struct GLImageHandle *CastGLImageHandle( OsHandle handle )
{
	struct GLImageHandle *p = (struct GLImageHandle *) handle;

	if( p == NULL ) return NULL;

	if( p->_magic != GLIMAGEHANDLE_MAGIC ) return NULL;

	return p;
}

//////////////////////////////////////////////////////////////////////////
OsError _GLImageCreate( OsHandle *handle ,int width ,int height )
{
	struct GLImageHandle *p = NULL;

    Display *xdisplay = _linux_getdisplay();
    
    // XImage *ximage;
    
    // void *data;
    
    if( handle == NULL ) return EINVAL;
    
    if( *handle != OS_INVALID_HANDLE ) return EEXIST;
    
    p = NewGLImageHandle();
    
    /*if( p == NULL ) return ENOMEM;
	
    int rootDepth = DefaultDepth( xdisplay ,DefaultScreen(xdisplay) );
    
	p->_X11Image._xpixmap = XCreatePixmap( xdisplay ,DefaultRootWindow(xdisplay) ,width ,height ,rootDepth );
    
    // data = malloc( width*3*height );
    // ximage = XCreateImage( xdisplay ,CopyFromParent ,24 ,ZPixmap ,0 ,data ,width ,height ,32 ,0 );
    
    if( p->_X11Image._xpixmap == 0 )
    {
         //? get status 
         free( p );
         
         return EFAILED;
    }
    
    p->_X11Image._width = width; p->_X11Image._height = height;*/
	
	p->_width = width;
	p->_height = height;
    
    *handle = (OsHandle) p;
    
	return ENOERROR;
}


void _GLImageGetInfo( OsHandle handle ,struct OsGuiImageInfo *info )
{
	struct GLImageHandle *p = CastGLImageHandle( handle );

//	_ASSERT( p != NULL && info != NULL );

	if( p == NULL || info == NULL ) return;

	info->width = (int) p->_width;
	info->height = (int) p->_height;
}

OsError _GLImageWrite( OsHandle handle ,const uint8_t *pixelValues ,size_t size )
{
	struct GLImageHandle *p = CastGLImageHandle( handle );
	
	p->_buffer = pixelValues;// Should make a copy of the buffer?
	
	return ENOERROR;
}

void _GLSetAlphaBlend( OsGuiContext context ,float alpha )
{
/*	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

	_ASSERT( osContext != NULL && osContext->_hdc != NULL );

	osContext->_blendfunction.BlendOp = AC_SRC_OVER;
	osContext->_blendfunction.BlendFlags = 0;
	osContext->_blendfunction.AlphaFormat = 0;
	osContext->_blendfunction.SourceConstantAlpha = (alpha < 1.0) ? ((alpha > 0) ? (BYTE) (255*alpha) : 0) : 255;*/
}

void orthographicMatrix(int windowWidth, int windowHeight, float* matrix)
{
	float scalingFactor = 1.f;
	Matrix scaleMatrix       = Matrix::createScaling     (scalingFactor * windowWidth, scalingFactor * windowHeight, 1.0f);
    Matrix ortographicMatrix = Matrix::matrixOrthographic(float(-windowWidth), float(windowWidth), float(-windowHeight), 
                                                          float(windowHeight), -1.0f,              1.0f);
	Matrix res = ortographicMatrix * scaleMatrix;

	memcpy(matrix, res.getAsArray(), 16 * sizeof(float));
}


void _GLDrawImage( OsGuiContext context ,int left ,int top ,int right ,int bottom ,
		OsHandle himg ,int imageLeft ,int imageTop ,int imageRight ,int imageBottom )
/*OsError _GLDrawImage( OsHandle hwin, int left, int top, int right, int bottom,
	OsHandle himg, int imageLeft, int imageTop, int imageRight, int imageBottom )*/
{
	GLenum glError = glGetError();
	
	if( glError != GL_NO_ERROR ) assert(0);
	
	Display *xdisplay = _linux_getdisplay();
    
	struct GLImageHandle *p = CastGLImageHandle( himg );
	XImage* xim = NULL;
	/* Array of coordinates describing quad. */
	float vertexData[] = {-1.0f, -1.0f, 0.0f,
						   1.0f, -1.0f, 0.0f,
						  -1.0f,  1.0f, 0.0f,
						  -1.0f,  1.0f, 0.0f,
						   1.0f, -1.0f, 0.0f,
						   1.0f,  1.0f, 0.0f};
	/* Array of texture coordinates used for mapping texture to a quad. */
	float textureCoordinatesData[] = {0.0f, 1.0f,
									  1.0f, 1.0f,
									  0.0f, 0.0f,
									  0.0f, 0.0f,
									  1.0f, 1.0f,
									  1.0f, 0.0f};

	/*Modelview matrix*/
	float modelViewMatrix[16];

	float imageSize[2];
	imageSize[0] = (float)p->_width;
	imageSize[1] = (float)p->_height;

	if( p == NULL ) return/* EFAILED*/;
	
	EGL_CHECK(eglMakeCurrent(EGLRuntime::display, EGLRuntime::surface, EGLRuntime::surface, EGLRuntime::context));

	glClearColor(0.f, .75f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Create the texture if it does not exist
	if (p->textureId == 0)
	{
		glGenTextures(1, &p->textureId);
		glBindTexture(GL_TEXTURE_2D, p->textureId);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		/* Generate and bind vertex array. */
		glGenVertexArrays(1, &p->arrayObjectId);
		GL_CHECK(glBindVertexArray(p->arrayObjectId));

		/* Generate buffers. */
		GL_CHECK(glGenBuffers(2, p->bufferObjectId));

		/* Fill buffer object with vertex data. */
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, p->bufferObjectId[0]));
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW));

		/* Fill buffer object with texture coordinates data. */
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, p->bufferObjectId[1]));
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoordinatesData), textureCoordinatesData, GL_STATIC_DRAW));

		/* Do everything to create program. */
		Shader::processShader(&p->vertexShaderId,   "image_shader.vert",   GL_VERTEX_SHADER);
		Shader::processShader(&p->fragmentShaderId, "image_shader.frag", GL_FRAGMENT_SHADER);

		p->programId = GL_CHECK(glCreateProgram());

		GL_CHECK(glAttachShader(p->programId, p->vertexShaderId));
		GL_CHECK(glAttachShader(p->programId, p->fragmentShaderId));

		GL_CHECK(glLinkProgram(p->programId));
		GL_CHECK(glUseProgram(p->programId));

		/* Get attributes and uniforms locations from shaders attached to the program. */
		p->modelViewMatrixLocation   = GL_CHECK(glGetUniformLocation(p->programId, "modelViewMatrix"));
		p->positionLocation          = GL_CHECK(glGetAttribLocation (p->programId, "attributePosition"));
		p->textureCoordinateLocation = GL_CHECK(glGetAttribLocation (p->programId, "attributeTextureCoordinate"));
		p->textureLocation           = GL_CHECK(glGetUniformLocation(p->programId, "uniformTexture"));
	}
	else
	{
		// Program Object
		GL_CHECK(glUseProgram(p->programId));
	}

	/* Set up texture. */
	GL_CHECK(glActiveTexture(GL_TEXTURE0));
	GL_CHECK(GL_CHECK(glBindTexture(GL_TEXTURE_2D, p->textureId)););
	GL_CHECK(glUniform1i(p->textureLocation, 0));

	/* Set up model-view matrix. */
	orthographicMatrix(right - left, bottom - top, modelViewMatrix);
	GL_CHECK(glUniformMatrix4fv(p->modelViewMatrixLocation, 1, GL_FALSE, modelViewMatrix));
	
	// Vertices array
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, p->bufferObjectId[0]));
	GL_CHECK(glEnableVertexAttribArray(p->positionLocation));
	GL_CHECK(glVertexAttribPointer(p->positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0));

	// Texture Coordinates
	if (p->textureCoordinateLocation >= 0)
	{
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, p->bufferObjectId[1]));
		GL_CHECK(glEnableVertexAttribArray(p->textureCoordinateLocation));
		GL_CHECK(glVertexAttribPointer(p->textureCoordinateLocation, 2, GL_FLOAT, GL_FALSE, 0, 0));
	}

	// Get Access to the pixmap image
	//xim = XGetImage(xdisplay, p->_X11Image._xpixmap, 0, 0, p->_X11Image._width, p->_X11Image._height, AllPlanes, ZPixmap);
	
	// Copy the image buffer into the texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, p->_width , p->_height,
				0, GL_RGB, GL_UNSIGNED_BYTE, (void*)p->_buffer/*(&(xim->data[0]))*/);
	glError = glGetError();
	if( glError != GL_NO_ERROR ) assert(0);


	/* Test with a fake texture */
	/*unsigned char fakeTexture[] = { 0, 255, 255, 
								0, 255, 255, 
								0, 0, 0, 
								0, 0, 0, 
								0, 255, 255,
								0, 255, 255,
								0, 0, 0, 
								0, 0, 0, 
								0, 0, 0,
								0, 0, 0,
								0, 255, 255, 
								0, 255, 255, 
								0, 0, 0,
								0, 0, 0,
								0, 255, 255, 
								0, 255, 255 };

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4, 4, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)fakeTexture);
	glError = glGetError();
	if(glError != GL_NO_ERROR) {
		exit(1);
	}*/

    /* Draw quad with texture image. */
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 6));

	return /*ENOERROR*/;
}




void _GLDrawRectangle( OsGuiContext context ,int left ,int top ,int right ,int bottom )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

    int width = right - left;
    
    int height = bottom - top;
    
    if( width < 1 || height < 1 ) return;
    
    assert( osContext != NULL && osContext->_hwindow != NULL /*&& osContext->_hwindow->_xgc != 0*/ );
       
    //! filling 
    /*if( !osContext->_xnofill && width > 2 && height > 2 )
    {
        XSetForeground( osContext->_xdisplay ,osContext->_xgc ,osContext->_xfillcolor.pixel );
    
        XFillRectangle( osContext->_xdisplay ,osContext->_xdrawable ,osContext->_xgc
            ,_TX(left+1) ,_TY(top+1) ,_TX(width-2) ,_TY(height-2)
            );        

        XSetForeground( osContext->_xdisplay ,osContext->_xgc ,osContext->_xforecolor.pixel );
    } 
    
    //! border 
    XDrawRectangle( osContext->_xdisplay ,osContext->_xdrawable ,osContext->_xgc
        ,_TX(left) ,_TY(top) ,_TX(right-left) ,_TY(bottom-top)
        );*/
}

void _GLDrawEllipse( OsGuiContext context ,int left ,int top ,int right ,int bottom )
{
//	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

//	_ASSERT( osContext != NULL && osContext->_hdc != NULL );

//	Ellipse( osContext->_hdc ,left ,top ,right ,bottom );
}

void _GLDrawLine( OsGuiContext context ,int left ,int top ,int right ,int bottom )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

    assert( osContext != NULL && osContext->_hwindow != NULL /*&& osContext->_hwindow->_xgc != 0*/ );
    
 /*   XDrawLine( osContext->_xdisplay ,osContext->_xdrawable ,osContext->_xgc
        ,_TX(left) ,_TY(top) ,_TX(right) ,_TY(bottom)
        );*/
}

// static int _win_textalign_map[] = { DEFAULT_PITCH ,FIXED_PITCH ,VARIABLE_PITCH };

void _GLDrawText( OsGuiContext context ,const char_t *text ,int left ,int top ,int right ,int bottom ,int align ,struct OsRect *rect )
{
	struct GuiContextHandle *osContext = (struct GuiContextHandle*) context;

    assert( osContext != NULL && osContext->_hwindow != NULL /*&& osContext->_hwindow->_xgc != 0*/ );
    
    /*XSetForeground( osContext->_xdisplay ,osContext->_xgc ,osContext->_xtextcolor.pixel );
    
    XCharStruct toverall;
    
    int tdirection ,tascent ,tdescent;
    
    XFontStruct *xfont = (osContext->_currentFont != NULL) ? osContext->_currentFont->_xfont : osContext->_hwindow->_xfont;
    // XLoadQueryFont( osContext->_xdisplay ,"10x20" );
    
    XTextExtents( xfont ,text ,strlen(text) ,&tdirection ,&tascent ,&tdescent ,&toverall );
    
    //XFreeFont( osContext->_xdisplay ,xfont );
    
    int x = left ,y = top;
    
    if( align & OS_ALIGN_CENTER ) x = left + (right - left - toverall.width)/2;
    else if( align & OS_ALIGN_RIGHT ) x = right - toverall.width;
    
    if( align & OS_ALIGN_VCENTER ) y = top + (bottom - top - toverall.ascent)/2;
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
*/
/*	RECT rect;

	_ASSERT( osContext != NULL && osContext->_hdc != NULL );

	rect.left = left; rect.top = top; rect.right = right; rect.bottom = bottom;

	DrawText( osContext->_hdc ,text ,-1 ,&rect ,align | DT_SINGLELINE );*/
}

void _GLDrawPolygon( OsGuiContext context ,int npoints ,const struct OsPoint *points )
{
	assert(0);
}

//-- resource
int _GLResourceGetType( OsHandle handle )
{
	struct GLImageHandle *p = CastGLImageHandle( handle );

	if( p != NULL ) return OS_GUIRESOURCETYPE_IMAGE;

//	_ASSERT( FALSE );

	return 0;
}

OsError _GLResourceLoadFromMemory( OsHandle *handle ,int resourceTypeHint ,uint8_t *memory ,const void *resourceInfo )
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

TINYFUN OsError _GLResourceLoadFromFile( OsHandle *handle ,int resourceTypeHint ,const char_t *filename )
{
	/*struct GLImageHandle *p = NULL;

	// OsError error = ENOERROR;
    
    Display *xdisplay = _linux_getdisplay();

    // int xhot=0 ,yhot=0;
    
    int status;

	if( handle == NULL || filename == NULL ) return EINVAL;

	if( *handle != OS_INVALID_HANDLE ) return EEXIST;

	if( resourceTypeHint == OS_GUIRESOURCETYPE_ANY ) return ENOSYS;
    
	if( resourceTypeHint == OS_GUIRESOURCETYPE_IMAGE )
    {
        p = NewGLImageHandle();
        
        status = XReadBitmapFile( xdisplay ,DefaultRootWindow(xdisplay) ,"./RabbitFire-icon.xbm" ,&(p->_X11Image._width) ,&(p->_X11Image._height) ,&(p->_X11Image._xpixmap) ,NULL ,NULL ); // &xhot ,&yhot );
        
        if( status == 0 )
        {           
            *handle = (OsHandle) p;
            
            return ENOERROR;
        }
        
        if( status == BadAlloc ) return ENOMEM;
        
        return EFAILED;
    }*/

	return ENOSYS;
}

TINYFUN OsError _GLResourceLoadFromApp( OsHandle *handle ,int resourceTypeHint ,int resourceId ,const char_t *application )
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



//////////////////////////////////////////////////////////////////////////

#define GL_HANDLETYPE_WINDOW	1
#define GL_HANDLETYPE_FONT		2
#define GL_HANDLETYPE_IMAGE		3

struct GLAnyHandle 
{
	uint32_t _magic;

	struct OsGuiSystemTable *_guiSystem;

	uint32_t _handleType;
};

OsError GLCloseHandle( OsHandle *handle )
{
	return ENOSYS;
}

TINYFUN enum OsHandleType _GLHandleGetType( OsHandle handle )
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
	struct GLAnyHandle *p = (struct GLAnyHandle *) handle;

	if( p == NULL ) return EINVAL;

	//TODO check magic

	switch( p->_handleType )
	{
	case GL_HANDLETYPE_WINDOW:
	case GL_HANDLETYPE_FONT:
	case GL_HANDLETYPE_IMAGE: 
    //TODO
		return ENOSYS;

	default:
		return EINVAL;
	}
}






struct xGLWindowNode
{
    Window _xwindow;
    
    struct GLWindowHandle *_handle;
    
    struct xWindowNode *_next;
};

#ifdef _OPENGL_GUI_

__thread struct xGLWindowNode *_xWindowHead = NULL;

/* OsError LinuxAddWindowToThread( Window window ,struct GLWindowHandle *handle )
{
    struct xGLWindowNode *p = _xWindowHead;  
    
    struct xGLWindowNode *n = (struct xGLWindowNode*) malloc( sizeof(struct xGLWindowNode) );
    
    if( n == NULL ) return ENOMEM;
    
    n->_xwindow = window;
    n->_handle = handle;
    n->_next = p;
    
    _xWindowHead = n;
    
    return ENOERROR;
} */

OsError _linux_glwindowproc( struct GLWindowHandle *handle ,XEvent *event );


#endif


//////////////////////////////////////////////////////////////////////////
/**
 * @brief 
 * @param handle
 */
struct OsGuiSystemTable _systemGLES =
{
	_GLWindowCreate 
	,_GLWindowRefresh
	,_GLWindowSwapBuffers
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
struct OsGuiSystemTable *_guiSystemOPENGLES = &_systemGLES;

//////////////////////////////////////////////////////////////////////////
//
