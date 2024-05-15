#include "tiny.h"

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

//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
void TestSuite::Logger::Initialize( const char *title ,const char_t *outdir ) {
    String filename;

    Format( filename ,"%s%s.log" ,1024 ,(const char_t*) (outdir!=NULL) ? outdir : _T("") ,title );

    _hasFile = OS_SUCCEED( _file.Open( filename.c_str() ,OS_ACCESS_WRITE ,OS_SHARE_READ ,OS_CREATE_ALWAYS ) );
}

void TestSuite::Logger::Close( void ) {
    _file.Close();
}

void TestSuite::Logger::Log( int depth ,const char *heading ,OsColorRef color ,const char *format ,... ) {
    String filelog;

    for( int i=0;i<depth;++i ) { printf(_T("\t")); filelog += _T("\t"); }

	OsConsoleInfo console_infos;
	OsConsoleGetInfo(&console_infos);
    if( heading )
    {
        OsConsoleSetColor( OS_SELECT_TEXTCOLOR ,color );

        printf( "%s " ,(const char*) heading );
        filelog += heading; filelog += _T(" ");

        OsConsoleSetColor( OS_SELECT_TEXTCOLOR , OS_COLOR_WHITE );
    }

    char report[1024];

    va_list args;

    va_start( args ,format );
    {
#pragma warning(disable:4996)
        vsprintf( report ,format ,args );
#pragma warning(default:4996)
    }
    va_end( args );

    printf( "%s", report );
    filelog += report;

    printf("\r\n");
    filelog += _T("\r\n");

    if( _hasFile )
        _file.Write( (uint8_t*) filelog.c_str() ,filelog.size() );

	OsConsoleSetColor( OS_SELECT_TEXTCOLOR ,console_infos.forecolor );
}

//////////////////////////////////////////////////////////////////////////////
}; //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
//EOF