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
#include <X11/X.h>
#include <X11/Xlib.h>

//////////////////////////////////////////////////////////////////////////
struct GuiFontHandle;

struct GuiContextHandle
{
	struct OsGuiSystemTable *_guiSystem;
    
	struct GuiWindowHandle *_hwindow;

    Display *_xdisplay;
    
    Drawable _xdrawable;
    
    GC _xgc;
    
    XColor _xforecolor;
    
    XColor _xfillcolor;
    
    Bool _xnofill;
    
    XColor _xtextcolor;
    
    XColor _xbackcolor;
    
    Bool _xnoback;
    
    struct GuiFontHandle *_currentFont;
       
	// BLENDFUNCTION _blendfunction;
    
    //! region
	struct OsRect _region;

	int _offsetx ,_offsety;

	float _scalex ,_scaley;    
};

//! NB GuiContextHandle is not cast protected as it is intended to be used in fast GUI code, use with care

//////////////////////////////////////////////////////////////////////////
//-- window handle

#define GUIWINDOWHANDLE_MAGIC	0x06F5C89F4 // 0x01ACD89F4

struct GuiWindowHandle
{
	uint32_t _magic; //! GuiMagic tag

	struct OsGuiSystemTable *_guiSystem; //! must be second

	OsEventFunction _function;
	
	uint32_t _handleType;

    int _resized;

    Window _xwindow;
    
    Pixmap _xpixmap;
    
    GC _xgc;
    
    Colormap _xcolormap;
    
    XFontStruct *_xfont;

    int _width ,_height ,_depth;

    enum OsMouseButton _button;

    void *_userData;
};


//////////////////////////////////////////////////////////////////////////
//-- image 
#define GUIIMAGEHANDLE_MAGIC	0x06F5CF45C // 0x079A3F45C

struct GuiImageHandle
{
	uint32_t _magic;
	
	struct OsGuiSystemTable *_guiSystem; //! must be second
    
    Pixmap _xpixmap;
    
    unsigned int _width ,_height;
};

//////////////////////////////////////////////////////////////////////////
//EOF