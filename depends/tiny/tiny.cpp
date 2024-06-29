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
//! DEVNOTE check timers in tiny

/* struct ITimerListener {
    virtual void onTimer( uint32_t id ,OsEventTime now ,OsEventTime last ,void *userdata ) = 0;
};

struct TimerInfo {
    ITimerListener *listener;
    OsEventTime delay;
    OsEventTime last;
    void *userdata;
}; */

//////////////////////////////////////////////////////////////////////////////
//! String helpers

std::string &toupper( std::string &s ) {
    std::transform( s.begin() ,s.end() ,s.begin()
        ,[](unsigned char c){ return std::toupper(c); }
    );

    return s;
}

std::string &tolower( std::string &s ) {
    std::transform( s.begin() ,s.end() ,s.begin()
        ,[](unsigned char c){ return std::tolower(c); }
    );

    return s;
}

std::string &ltrim( std::string &s ) {
    s.erase( s.begin() ,std::find_if(
            s.begin() ,s.end()
            ,[](unsigned char ch) { return !std::isspace(ch); }
        )
    );

    return s;
}

std::string &rtrim( std::string &s ) {
    s.erase( std::find_if(
            s.rbegin(), s.rend()
            ,[](unsigned char ch) { return !std::isspace(ch); }
        ).base()
        , s.end()
    );

    return s;
}

//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Objects

//////////////////////////////////////////////////////////////////////////////
//! Class identity

static MapOf<String,PUID> &getClassNameStore() {
    static MapOf<String,PUID> g_classNames;

    return g_classNames;
}

bool registerClassName( const PUID &id ,const char *name ) {
    getClassNameStore()[name] = id; return true;
}

bool findClassIdByName( const char *name ,PUID &id ) {
    const auto &it = getClassNameStore().find( name );

    if( it == getClassNameStore().end() ) return false;

    id = it->second;

    return true;
}

bool findClassNameById( const PUID &id ,String &name ) {
    for( const auto &it : getClassNameStore() ) {
        if( it.second == id ) {
            name = it.first;
            return true;
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////
//! Named Pointers

static MapOf<PIID,IObject*> g_objectInstances;

void registerInstance( PIID &id ,IObject *p ) {
    assert( g_objectInstances.find(id) == g_objectInstances.end() );

    g_objectInstances[id] = p;
}

void revokeInstance( PIID &id ) {
    auto it = g_objectInstances.find(id);

    if( it != g_objectInstances.end() )
        g_objectInstances.erase(it);
}

IObject *getInstance( PIID &id ) {
    auto it = g_objectInstances.find(id);

    if( it == g_objectInstances.end() ) return NullPtr;

    return it->second;
}

PIID digInstanceId( IObject *p ) {
    for( auto &it : g_objectInstances ) {
        if( it.second == p ) return it.first;
    }

    return PIID_NOINSTANCE;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Base

//////////////////////////////////////////////////////////////////////////////
//! String (native support)

template <>
double &fromString( double &p ,const String &s ,size_t &size ) {
    const char *str = s.c_str();

    char *str_end = (char*) (str + s.size());

    double v = strtod( str ,&str_end );

    size = str_end - str;

    if( size ) p = v;

    return p;
}

template <>
float &fromString( float &p ,const String &s ,size_t &size ) {
    double v; fromString( v ,s ,size ); if( size ) p = (float) v; return p;
}

template <>
long &fromString( long &p ,const String &s ,size_t &size ) {
    const char *str = s.c_str();

    char *str_end = (char*) (str + s.size());

    long v = strtol( str ,&str_end ,10 );

    size = str_end - str;

    if( size ) p = v;

    return p;
}

template <>
int &fromString( int &p ,const String &s ,size_t &size ) {
    long v; fromString( v ,s ,size ); if( size ) p = (int) v; return p;
}

template <>
short &fromString( short &p ,const String &s ,size_t &size ) {
    long v; fromString( v ,s ,size ); if( size ) p = (short) v; return p;
}

template <>
bool &fromString( bool &p ,const String &s ,size_t &size ) {
    if( iMatch(s,"true") ) p = true; else if( iMatch(s,"false") ) p = false;
    return p;
}

template <> unsigned long &fromString( unsigned long &p ,const String &s ,size_t &size ) {
    long v; fromString( v ,s ,size ); if( size ) p = (unsigned long) v; return p;
}

template <> unsigned int &fromString( unsigned int &p ,const String &s ,size_t &size ) {
    long v; fromString( v ,s ,size ); if( size ) p = (unsigned int) v; return p;
}

template <> unsigned short &fromString( unsigned short &p ,const String &s ,size_t &size ) {
    long v; fromString( v ,s ,size ); if( size ) p = (unsigned short) v; return p;
}

///--
template <>
String &toString( const bool &p ,String &s ) {
    s = (p ? "true" : "false"); return s;
}

//////////////////////////////////////////////////////////////////////////////
//! Parse / Format

size_t Format( String &s ,const char *format ,size_t maxsize ,... ) {
    va_list varg; int len=0;

    va_start( varg ,maxsize );
    {
        s.resize( maxsize+1 ,0 );

        if( (len = vsnprintf( &s.front() ,maxsize ,format ,varg )) < 0 )
            s = "";
        else
            s.resize( len );
    }
    va_end( varg );

    return len;
}

//////////////////////////////////////////////////////////////////////////////
//! NameType

template <>
NameType &fromString( NameType &p ,const String &s ,size_t &size ) {
    const char *str = s.c_str();
    const char *s0 = str;

    if( ParseToken( str ,&p.name ) && SkipSymbol( str ,':' ) && ParseToken( str ,&p.type ) ) {} else return p;

    trim(p.name); trim(p.type);

    size = str - s0;

    return p;
}

template <>
std::string &toString( const NameType &p ,std::string &s ) {
    Format( s ,_T("%s:%s;") ,p.name.size() + p.type.size() + 4 ,(const char*) p.name.c_str() ,(const char*) p.type.c_str() );

    return s;
}

//////////////////////////////////////////////////////////////////////////////
//! KeyValue

template <>
KeyValue &fromString( KeyValue &p ,const String &s ,size_t &size ) {
    const char *str = s.c_str();
    const char *s0 = str;

    if( !ParseField( str ,'=' ,&p.key ,false ) ) return p;

    trim(p.key); //TODO key should not contain space ?

    NoSpace( str );

    if( !ParseBlock( str ,'{' ,'}' ,&p.value ,false ) && !ParseField( str ,';' ,&p.value ,false ) ) {
        return p;
    }

    rtrim(p.value);

    size = str - s0;

    return p;
}

template <>
std::string &toString( const KeyValue &p ,std::string &s ) {
    if( p.value.find(';') != String::npos ) {
        Format( s ,_T("%s={%s}") ,p.key.size() + p.value.size() + 4 ,(const char*) p.key.c_str() ,(const char*) p.value.c_str() );
    } else {
        Format( s ,_T("%s=%s;") ,p.key.size() + p.value.size() + 4 ,(const char*) p.key.c_str() ,(const char*) p.value.c_str() );
    }

    return s;
}

//////////////////////////////////////////////////////////////////////////////
//! Params

template <>
Params &fromString( Params &p ,const String &s ,size_t &size ) {
    KeyValue kv;

    const char *str = s.c_str();

    if( str ) while( str[0] && !fromString( kv ,str ,size ).key.empty() ) {
        p[kv.key] = kv.value;

        str += size;
    }

    return p;
}

template <>
String &toString( const Params &p ,String &s ) {
    KeyValue kv; String entry;

    int i=0; for( auto it=p.begin(); it!=p.end(); ++it ,++i ) {
        kv.key = it->first;
        kv.value = it->second;

        toString( kv ,entry );

        if( i > 0 ) s += ' ';

        s += entry;
    }

    return s;
}

//--
bool hasMember( const Params &p ,const char *key ) {
    return p.find(key) != p.end();
}

const String *peekMember( const Params &p ,const char *key ) {
    const auto &it = p.find(key);

    return it != p.end() ? &(it->second) : NullPtr;
}

const char *getMember( const Params &p ,const char *key ,const char *defaultValue ) {
    const auto &it = p.find(key);

    return it!=p.end() ? it->second.c_str() : (defaultValue ? defaultValue : "" );
}

Params &addMembers( Params &p ,const Params &params ,bool addEmpty ) {
    for( const auto &it : params ) if( addEmpty || !it.second.empty() ) {
        p[ it.first ] = it.second;
    }

    return p;
}

bool getDecl( const Params &params ,const char *name ,NameType &decl ,String &value ) {
    for( const auto &it : params ) {
        fromString( decl ,it.first );

        if( decl.name == name ) {
            value = it.second;
            return true;
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////
//! ParamList

template <>
ParamList &fromString( ParamList &p ,const String &s ,size_t &size ) {
    KeyValue kv;

    const char *str = s.c_str();

    if( str ) while( str[0] && !fromString( kv ,str ,size ).key.empty() ) {
        p.emplace_back( kv );

        str += size;
    }

    return p;
}

template <>
String &toString( const ParamList &p ,String &s ) {
    KeyValue kv; String entry;

    int i=0; for( auto it=p.begin(); it!=p.end(); ++it ,++i ) {
        kv = *it;

        toString( kv ,entry );

        if( i > 0 ) s += ' ';

        s += entry;
    }

    return s;
}

//////////////////////////////////////////////////////////////////////////////
//! StringList

template <>
StringList &fromString( StringList &p ,const String &s ,size_t &size ) {
    const char *str = s.c_str();
    const char *s0 = str;

    String value;

    while( ( NoSpace(str) OPTIONAL ) &&
        ( ParseBlock( str ,'{' ,'}' ,&value ,false ) && ( NoSpace(str) OPTIONAL ) && ( SkipSymbol(str,',') OPTIONAL )
        || ParseField( str ,',' ,&value ,false )
    )) {
        trim(value);

        p.emplace_back( value );
    }

    //TODO support quoted fields

    size = s0 - str;

    return p;
}

template <>
String &toString( const StringList &p ,String &s ) {
    s.clear();

    int i=0; for( const auto &it : p ) {
        if( i>0 ) s += ',';

        if( it.find(',') != String::npos ) {
            s += "{"; s += it; s += "}";
        } else {
            s += it;
        }

        ++i;
    }

    return s;
}

//////////////////////////////////////////////////////////////////////////////
//! Schema

Schema Schema::schema;

template <>
Schema &fromString( Schema &p ,const String &s ,size_t &size ) {
    StringList list;

    fromString( list ,s ,size );

    NameType decl;

    for( const auto &it : list ) {
        fromString( decl ,it );

        p.emplace_back( decl );
    }

    return p;
}

template <>
String &toString( const Schema &p ,String &s ) {
    StringList list;
    String decl;

    for( const auto &it : p ) {
        toString( it ,decl );

        list.emplace_back( decl );
    }

    toString( list ,s );
    return s;
}

int getMemberId( const char *name ,const Schema &schema ) {
    int i=0; for( auto &it : schema ) {
        if( it.name == name ) return i;

        ++i;
    }

    return -1;
}

//--
Params &listToParams( Params &manifest ,const Schema &schema ,const StringList &values ) {
    size_t n = schema.size() ,n2 = values.size();

    for( int i=0; i<n; ++i ) {
        manifest[ schema[i].name ] = ( (i < n2 ) ? values[i] : "" );
    }

    return manifest;
}

StringList &paramsToList( StringList &values ,const Schema &schema ,Params &manifest ) {
    NameType name;

    for( auto &it : schema ) {
        values.emplace_back( getMember( manifest ,it.name.c_str() ) );
    }

    return values;
}

//////////////////////////////////////////////////////////////////////////////
//! Arguments

//! @brief white space separated list // no ending // quoted text // no block

template <>
Arguments &fromString( Arguments &p ,const String &s ,size_t &size ) {
    _TODO; return p; //TODO
}

template <>
String &toString( const Arguments &p ,String &s ) {
    _TODO; return s; //TODO
}

//////////////////////////////////////////////////////////////////////////////
//! Serialization

///-- String
template <>
std::ostream &operator <<( std::ostream &out ,Bits<const String&> p ) {
    int size = (int) p().size();

    out << tobits( size ); //! @note not stored with null terminator

    return out.write( p().c_str() ,size );
}

template <>
std::istream &operator >>( std::istream &in ,Bits<String&> p ) {
    int size = 0;

    in >> tobits(size); if( in && size ) {} else return in;

    std::vector<char> buf(size+1); //! @note no portable way to directly write in String when using std::string

    char *str = &buf[0];

    assert( str && buf.size() >= size+1 );

    in.read( str ,size ); str[size] = 0;

    p().assign( str );

    return in;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Core (tiny-os wrapper)

//////////////////////////////////////////////////////////////////////////////
//! text

//TODO in text ?
const char *Parse( const char *s ,cacceptA accept ,char limit ,String &field ) {
    int i=0; if( !s ) return s;

    while( s[i] && accept(s[i]) && (s[i] != limit) ) ++i;

    field = String( s ,i ); //! @note not including terminator, refused or limit char

    return s + i + ((s[i] == limit) ? 1 : 0); //! @note skipping limit if found
}

const char *Parse( const char *s ,char limit ,String &field ) {
    return Parse( s ,[](char ch){ return true; } ,limit ,field );
}

//////////////////////////////////////////////////////////////////////////////
//! struct wrappers

//-- point
template <> OsPoint &fromString( OsPoint &p ,const String &s ,size_t &size ) {
    const char *str = s.c_str(); String field;

    str = Parse( str ,cisdigit ,',' ,field ); if( str ) fromString( p.x ,field ); else return p;
    str = Parse( str ,cisdigit ,0 ,field ); if( str ) fromString( p.y ,field ); else return p;

    return p;
}

template <> String &toString( const OsPoint &p ,String &s ) {
    Format( s ,"%d,%d" ,(int) p.x ,(int) p.y ); return s;
}

//-- rect
template <> OsRect &fromString( OsRect &p ,const String &s ,size_t &size ) {
    const char *str = s.c_str(); String field;

    str = Parse( str ,cisdigit ,',' ,field ); if( str ) fromString( p.left ,field ); else return p;
    str = Parse( str ,cisdigit ,',' ,field ); if( str ) fromString( p.top ,field ); else return p;
    str = Parse( str ,cisdigit ,',' ,field ); if( str ) fromString( p.right ,field ); else return p;
    str = Parse( str ,cisdigit ,0 ,field ); if( str ) fromString( p.bottom ,field ); else return p;

    return p;
}

template <> String &toString( const OsRect &p ,String &s ) {
    Format( s ,"%d,%d,%d,%d" ,(int) p.left ,(int) p.top ,(int) p.right ,(int) p.bottom ); return s;
}

//-- color
REGISTER_STRUCTNAME( ColorRef );

static const MapOf<String,ColorRef> g_colorNames = {
    { "none" ,OS_COLOR_NONE }
    ,{ "white" ,OS_COLOR_WHITE }
    ,{ "red" ,OS_COLOR_RED }
    ,{ "green" ,OS_COLOR_GREEN }
    ,{ "blue" ,OS_COLOR_BLUE }
};

bool findColorValueByName( const char *name ,ColorRef &color ) {
    return findMap( g_colorNames ,name ,color );
}

bool matchColorByName( const String &s ,ColorRef &color ,size_t *size ) {
    for( auto &it : g_colorNames ) {
        if( iMatch( s ,it.first.c_str() ) ) {
            if( size ) *size = it.first.size();
            color = it.second;
            return true;
        }
    }

    return false;
}

bool findColorNameByValue( ColorRef color ,String &name ) {
    return digMap( g_colorNames ,color ,name );
}

//--
template <> ColorRef &fromString( ColorRef &p ,const String &s ,size_t &size ) {
    int hex; if( s.empty() ) return p;

    const char *str = s.c_str();

    if( *str != '#' ) {
        if( matchColorByName( s ,p ,&size ) ) return p;

        fromString<uint32_t>( p ,s ,size );

        return p;
    } else
        ++str;

    int i=0; while( (hex = chexvalue(*str)) >= 0 ) {
        p.color = (p.color << 4) | hex; ++str; ++i;
    }

    p.color = OS_BGR2RGB( p.color );

    if( i <= 6 )
        p.color |= 0x0FF000000; //! if not specified alpha = max

    return p;
}

template <> String &toString( const ColorRef &p ,String &s ) {
    if( p.color != OS_COLOR_NONE )
        Format( s ,"#%08X" ,10 ,(unsigned int) OS_BGR2RGB(p) );
    else
        s = "0"; //! or none

    return s;
}

///-- native
REGISTER_STRUCTNAME( bool );

//////////////////////////////////////////////////////////////////////////////
//! class wrappers

OsError ThreadFunction( const struct OsEventMessage *msg ,void *userData ) {
	if( msg->eventType != osExecuteEvent ) return EINVAL;

	Thread *t = (Thread*) userData;

	if( t==NULL ) return EINVAL;

	if( msg->executeMessage.executeAction == osExecuteStart )
	{
		OsError error = t->Main();

		t->Destroy(); //! NB one time start

		return error;
	}

	if( msg->executeMessage.executeAction == osExecuteStop )
		return t->OnStop();

	return ENOSYS;
}

OsError Thread::Create( bool suspended/*=true*/ ) {
	if( _hthread != OS_INVALID_HANDLE ) return EEXIST; 

	uint32_t flag = (suspended ? OS_THREAD_CREATESUSPENDED : 0);

	OsError error = OsThreadCreate( &_hthread ,flag ,ThreadFunction ,(void*) this );

	if( error != ENOERROR ) return error;

	OsThreadGetId( _hthread ,&_id );

	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#ifndef TINY_NO_GUI

//////////////////////////////////////////////////////////////////////////////
//! TextAlign

    //TODO merge TextAlign and GuiAlign ...

template <>
const char *Enum_<TextAlign>::names[] = {
    "normal" ,"left" ,"centerh" ,"right" ,"top" ,"centerv" ,"bottom" ,"center"
};

template <>
const TextAlign Enum_<TextAlign>::values[] = {
     textalignNormal ,textalignLeft ,textalignCenterH
     ,textalignRight ,textalignTop ,textalignCenterV
     ,textalignBottom ,textalignCenter
};

//////////////////////////////////////////////////////////////////////////////
//! RefreshFlags

template <>
const char *Enum_<RefreshFlags>::names[] = {
    "normal" ,"resized" ,"content" ,"background" ,"update"
};

template <>
const RefreshFlags Enum_<RefreshFlags>::values[] = {
    refreshNormal ,refreshResized ,refreshContent ,refreshBackground ,refreshUpdate
};

//////////////////////////////////////////////////////////////////////////////
//! GuiFont

template <>
const char *Enum_<FontWeight>::names[] = {
    "any" ,"thin" ,"ultralight" ,"light" ,"normal" ,"medium" ,"semibold" ,"bold" ,"ultrabold" ,"heavy"
};

template <>
const FontWeight Enum_<FontWeight>::values[] = {
    fontWeightAny
    ,fontWeightThin
    ,fontWeightUltraLight
    ,fontWeightLight
    ,fontWeightNormal
    ,fontWeightMedium
    ,fontWeightSemiBold
    ,fontWeightBold
    ,fontWeightUltraBold
    ,fontWeightHeavy
};

template <>
const char *Enum_<FontStyle>::names[] = {
    "any" ,"italic" ,"underline" ,"strikeout"
};

template <>
const FontStyle Enum_<FontStyle>::values[] = {
    fontStyleNormal
    ,fontStyleItalic
    ,fontStyleUnderline
    ,fontStyleStrikeout
};

template <>
const char *Enum_<FontPitch>::names[] = {
    "any" ,"fixed" ,"variable"
};

template <>
const FontPitch Enum_<FontPitch>::values[] = {
    fontPitchAny
    ,fontPitchFixed
    ,fontPitchVariable
};

template <>
GuiFont &fromManifest( GuiFont &p ,const Params &manifest ) {
    String facename;
    int pointSize = OS_GUIFONT_DEFAULTSIZE;
    FontWeight weight = fontWeightNormal;
    FontStyle style = fontStyleNormal;
    FontPitch pitch = fontPitchAny;

    facename = getMember( manifest ,"facename" );
    fromString( pointSize ,getMember( manifest ,"size" ) );
    enumFromString( weight ,getMember( manifest ,"weight" ) );
    enumFromString( style ,getMember( manifest ,"style" ) );
    enumFromString( pitch ,getMember( manifest ,"pitch" ) );

    p.Create( facename.c_str() ,pointSize ,(int) weight ,(int) style ,(int) pitch );

    return p;
}

//////////////////////////////////////////////////////////////////////////////
//! ThumbMap

void addThumbmapRegular( ThumbMap &p ,const OsRect &canvas ,int nx ,int ny ,int stepx ,int stepy ) {
    //! @note enum x, then y

    Rect r = canvas; //! origin + size
    int w = r.getWidth();
    int h = r.getHeight();

    for( int y=0; y<ny; ++y ) {
        Rect ry = r + Point(0,(h+stepy)*y);

        for( int x = 0; x < nx; ++x ) {
            p.rects.emplace_back( ry + Point((w+stepx)*x,0) );
        }
    }
}

void addThumbmapFromPoints( ThumbMap &p ,const OsRect &canvas ,const ListOf<Point> &points ) {
    Rect r = canvas; //! @note points offset canvas origin (i.e. not 0,0)

    for( const auto &it : points ) {
        p.rects.emplace_back( r + it );
    }
}

template <>
ThumbMap &fromString( ThumbMap &p ,const String &s ,size_t &size ) {
    StringList list;

    fromString( list ,s ,size );

    if( list.empty() ) return p;

    // s = regular,{10,10,75,75},4,4 //! regular
    // s = rect,{10,10,75,75},{85,85,155,155} //! rect list
    // s = point,{10,10,75,75},{10,10}

    if( iMatch( list[0] ,"regular" ) ) {
        if( list.size() < 4 ) return p;

        Rect canvas; int nx ,ny ,sx=0 ,sy=0;

        fromString( canvas ,list[1] );
        fromString( nx ,list[2] ,size );
        fromString( ny ,list[3] );

        if( list.size() >= 6 ) {
            fromString( sx ,list[4] );
            fromString( sy ,list[5] );
        }

        addThumbmapRegular( p ,canvas ,nx ,ny ,sx ,sy );

        return p;
    }

    return p;
}

//////////////////////////////////////////////////////////////////////////////
//! GuiImage

template <>
GuiImage &fromManifest( GuiImage &p ,const Params &manifest ) {
    String s;

    s = getMember( manifest ,"thumbmap" );
    if( !s.empty() ) {
        fromString( p.thumbmap() ,s );
    }

    s = getMember( manifest ,"file" );
    if( !s.empty() ) {
        p.LoadFromFile( s.c_str() );
        return p;
    }

    s = getMember( manifest ,"size" );
    if( !s.empty() ) {
        OsPoint size;

        fromString( size ,s );

        p.Create( size.x ,size.y );

        return p;
    }

    return p;
}

//////////////////////////////////////////////////////////////////////////////
//! GuiWindow

OsError GuiWindowFunction( const struct OsEventMessage *msg ,void *userData ) {
	GuiWindow *window = (GuiWindow*) userData;

	if( window==NULL ) return EINVAL;

	switch( msg->eventType )
	{
	case osExecuteEvent:
		if( msg->executeMessage.executeAction == osExecuteStart )
			return window->onCreate();

		else if( msg->executeMessage.executeAction == osExecuteStop )
			return window->onClose();

		else if( msg->executeMessage.executeAction == osExecuteTerminate )
		{
			window->onDestroy();
			
			window->_hwindow = OS_INVALID_HANDLE;

			return ENOERROR;
		}

		return ENOSYS;

	case osRenderEvent:
		window->_context = msg->renderMessage.context;
		{
			if( window->_lastLayoutTime==0 || msg->renderMessage.resized > 0 ) {
				OsPoint size;

				window->GetClientSize( size );

				OsRect clientRect;

                clientRect.left = clientRect.top = 0;
                clientRect.right = size.x;
                clientRect.bottom = size.y;

				window->onLayout( clientRect );
			}

			window->onRender( *msg->renderMessage.updateRect );
		}
		window->_context = OS_INVALID_HANDLE;

		return ENOERROR;

	case osMouseEvent:
		window->onMouse( msg->mouseMessage.mouseAction ,msg->mouseMessage.keyState ,msg->mouseMessage.mouseButton ,msg->mouseMessage.points ,msg->mouseMessage.pos );
		return ENOERROR;

	case osKeyboardEvent:
		window->onKey( msg->keyboardMessage.keyAction ,msg->keyboardMessage.keyState ,msg->keyboardMessage.keyCode ,msg->keyboardMessage.c );
		return ENOERROR;

    case osTimerEvent:
        window->onTimer( msg->timerMessage.timerAction ,msg->timerMessage.now ,msg->timerMessage.last );
        return ENOERROR;

	default:
		return ENOSYS;
	}
}

//--
IRESULT GuiWindow::getInterface( puid_t id ,void **ppv ) {
    if( !ppv || *ppv) return IBADARGS;

    return
        honorInterface_<GuiWindow>(this,id,ppv)
        || honorInterface_<IGuiEvents>(this,id,ppv) || honorInterface_<IGuiMessage>(this,id,ppv)
        || honorInterface_<IGuiDisplay>(this,id,ppv) || honorInterface_<IGuiSurface>(this,id,ppv) || honorInterface_<IGuiContext>(this,id,ppv)
        ? IOK : CObject::getInterface( id ,ppv )
    ;
}

//--
OsError GuiWindow::Create( int guiSystemId ,bool visible ) {
	if( _hwindow != OS_INVALID_HANDLE ) return EEXIST;
	
	assert( visible == true ); //TODO create hidden

	struct OsGuiSystemTable *guiSystem;

	OsError error = OsGuiGetSystemTable( guiSystemId ,&guiSystem );

	if( OS_FAILED(error) ) return error;

	if( guiSystem == NULL ) return ENOSYS;

	return guiSystem->_WindowCreate( &_hwindow ,_name.c_str() ,&_properties ,GuiWindowFunction ,(void*) this ) ;
}

//-- timer
uint32_t GuiWindow::addTimer( IGuiEvents &listener ,OsEventTime delay ,void *userdata ) {
    GuiTimerInfo p = { &listener ,delay ,(OsEventTime) 0 };

    m_timers[m_timerIds] = p;

    return m_timerIds++;
}

void GuiWindow::setTimer( uint32_t id ,OsEventTime delay ) {
    auto timer = m_timers.find(id);

    if( timer == m_timers.end()  )
        return;

    timer->second.delay = delay;
}

void GuiWindow::removeTimer( uint32_t id ) {
    m_timers.erase(id);
}

void GuiWindow::removeTimers( IGuiEvents &listener ) {
    for( auto it=m_timers.begin(); it!=m_timers.end(); ) {
        if( it->second.listener == &listener ) {
            m_timers.erase(it);
        } else {
            ++it;
        }
    }
}

//--
void GuiWindow::onTimer( OsTimerAction timeAction ,OsEventTime now ,OsEventTime last ) {
    for( auto &it : m_timers ) {
        auto &it2 = it.second;

        if( last == 0 || it2.last == 0 || it2.last + it2.delay < now ) {
            it2.listener->onTimer( (OsTimerAction) it.first ,now ,last );
            it2.last = now;
        }
    }
}

//-- static
Point_<int> GuiWindow::_offset;
Point_<float> GuiWindow::_scale;

//////////////////////////////////////////////////////////////////////////////
#endif //TINY_NO_GUI

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
//EOF