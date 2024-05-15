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

#ifndef TINY_APP_TEST_H
#define TINY_APP_TEST_H

//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
//! @file "x-gui/gui.h"
//! @brief GUI - tiny extension
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
#define TEST_LOG( heading ,color ,format ,...) _root.GetLogger().Log( _depth ,heading ,color ,format ,__VA_ARGS__ );
#define TEST_LOG2( header ,format ,...) _root.GetLogger().Log( _depth ,header ,format ,__VA_ARGS__ );
#define TEST_PASS_FAIL(cond)		((cond) ? "[PASSED]" : "[FAILED]") ,((cond) ? OS_COLOR_GREEN : OS_COLOR_RED)
#define TEST_OK_FAILED(cond)		((cond) ? "[--OK--]" : "[FAILED]") ,((cond) ? OS_COLOR_GREEN : OS_COLOR_RED)

//////////////////////////////////////////////////////////////////////////////
struct TestSuite {

    struct Logger {
        virtual void Initialize( const char *title ,const char_t *outdir=NULL );
        virtual void Close( void );

        virtual void Log( int depth ,const char *heading ,OsColorRef color ,const char *format ,... );

        File _file;

        bool _hasFile;
    };

    struct TestUnit;

    struct TestCase {
        TestCase( TestSuite &root ) : _root( root ) { _depth = root.GetDepth(); }
        // TestCase( TestSuite &root ,const char *title ) : _root( root ) { _depth = root.GetDepth(); Start( title ); }

        TestSuite &_root;

        TestSuite &GetRoot() { return _root; }

        virtual Logger &GetLogger() { return _root.GetLogger(); }

        int _depth;

        int GetDepth() { return _depth+1; }

        int _units ,_fails;

        virtual void Start( const char *title ) {
            TEST_LOG( "[-CASE-]" ,OS_COLOR_DARKGREEN ,"%s..." ,title ); _units = _fails = 0;
        }

        TestUnit &StartTestUnit( const char *title ) {
            TestUnit *p = new TestUnit( *this );

            p->Start(title);

            return *p;
        }

        virtual void ReportUnit( bool result ) {
            ++_units; if( !result ) ++_fails;
        }

        virtual void Done( void ) {
            TEST_LOG2( TEST_PASS_FAIL(_fails==0) ,"(%d/%d) => " ,_units-_fails ,_units );
        }
    };

    struct TestUnit {
        TestUnit( TestCase &root ) : _root( root ) { _depth = root.GetDepth(); }
        // TestUnit( TestCase &root ,const char *title ) : _root( root ) { _depth = root.GetDepth(); Start( title ); }

        TestCase &_root;

        TestCase &GetRoot() { return _root; }

        virtual void Start( const char *title ) {
            _title = title; _result = true; StartTiming();

            TEST_LOG( "[-UNIT-]" ,OS_COLOR_DARKGREEN ,"%s" ,(const char*) _title.c_str() );
        }

        virtual void Assert( const char *checkname ,bool result ) {
            ++_assertCount; if( !result ) ++_assertFailed; _result &= result;

            TEST_LOG2( TEST_OK_FAILED(result) ," (ASSERT) %s" ,(const char *) checkname );
        }

        virtual void StartTiming() { _tm0 = OsTimerGetTimeNow(); _hasTiming = false; }
        virtual void StopTiming() { _tm1 = OsTimerGetTimeNow(); _hasTiming = true; }

        virtual void Done( const char *valueTitle ,double value ,bool result ) {
            if( !_hasTiming ) StopTiming();

            _result &= result;

            double tms = _root.GetRoot().GetMs(_tm0,_tm1);

            if( valueTitle && valueTitle[0] != 0 ) {
                TEST_LOG2( TEST_OK_FAILED(result) ," RESULT %s=%f (%.02fms)" ,(const char*) valueTitle ,(double) value ,(double) tms  );
            }
            else {
                TEST_LOG2( TEST_OK_FAILED(result) ," RESULT: (%.02fms)" ,(double) tms );
            }

            GetRoot().ReportUnit( result );

            delete this;
        }

        int _depth;

        String _title;

        OsTimerTime _tm0 ,_tm1;

        bool _hasTiming;

        int _assertCount;
        int _assertFailed;

        bool _result;
    };

///--
    Logger _defaultLogger;

    int _depth;

    OsTimerTime _frequencyMs;

    TestSuite( const char_t *title="tiny" ,const char_t *dir=NULL ) : _depth(0) {
        _frequencyMs = OsTimerGetResolution();

        GetLogger().Initialize( title ,dir );

        GetLogger().Log( 0 ,"[TEST-SUITE]" ,OS_COLOR_GREEN ,"%s...\r\n" ,title );
    }

    ~TestSuite() { GetLogger().Close(); }

    double GetMs( OsTimerTime tm0 ,OsTimerTime tm1 ) {
        return (double) (tm1 - tm0) / (double) _frequencyMs;
    }

    virtual Logger &GetLogger() { return _defaultLogger; }

    int &GetDepth() { return _depth; }

    TestCase *StartTestCase( const char *title ) {
        TestCase *p = new TestCase( *this );

        p->Start( title );

        return p;
    }
};

//////////////////////////////////////////////////////////////////////////////
#undef TEST_LOG
#undef TEST_PASS_FAIL
#undef TEST_OK_FAILED

//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
#endif //TINY_APP_TEST_H