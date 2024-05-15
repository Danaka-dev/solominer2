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
#ifdef __cplusplus
extern "C" {
#endif

extern Display *_linux_peekdisplay( void );
extern  Display *_linux_getdisplay( void );

#ifdef __cplusplus
}
#endif

//////////////////////////////////////////////////////////////////////////
#define XTIME_EVENT 0x010AC

//////////////////////////////////////////////////////////////////////////
#define GUISYSTEMHANDLE_MAGIC	0x06F5C0000

struct AnyHandle
{
	uint32_t _magic;
};

struct GuiSystemHandle
{
	uint32_t _magic;

	struct OsGuiSystemTable *_guiTable;
};

//////////////////////////////////////////////////////////////////////////
//