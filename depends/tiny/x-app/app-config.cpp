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
//////////////////////////////////////////////////////////////////////////////
//! Config

bool Config::LoadFile( const char *filename ,bool dontLock ,bool dontCreate ) {
    if( !filename ) return false;

    if( m_file.Open( filename ,OS_ACCESS_ALL ,dontLock ? OS_SHARE_ALL : OS_SHARE_READ ,dontCreate ? OS_CREATE_DONT : OS_CREATE_NOEXIST ) != ENOERROR )
        return false;

    size_t size = m_file.GetSize();

    if( size == 0 )
        return true;

    Memory mem;

    if( !mem.Resize(size) || m_file.Read( mem.data() ,size ) != ENOERROR )
        return false;

    m_content = String( (char*) mem.data() ,size );

    IndexContent();
    ParseContent();

    return true;
}

bool Config::SaveFile() {
    if( !m_file.isOpen() ) return false;

    //TODO ? if not locked check if file was modified on disk before writing ?

    auto size = m_content.size();

    if( m_file.GetSize() > size ) {
        m_file.SetSize( size );
    }

    return m_file.Seek(0,OS_SEEK_BEGIN) == ENOERROR && m_file.Write( (byte*) m_content.data() ,size ) == ENOERROR && m_file.Flush() == ENOERROR;
}

void Config::CloseFile() {
    m_file.Close();
}

//--
Config::Section *Config::peekSection( const char *name ) {
    String key = name; tolower(key);

    auto it = m_sections.find( key );

    return ( it != m_sections.end() ) ? &it->second : NullPtr;
}

Config::Section &Config::getSection( const char *name ) {
    Config::Section *section = peekSection(name);

    if( section ) return *section;

    //! if segment not found create a new one at the end of content
    String key = name; tolower(key);

    Segment segment;

    segment.name = key;
    segment.start = (int) m_content.size();
    segment.end = (int) m_content.size();

    m_index.emplace_back( segment );

    return m_sections[name];
}

bool Config::readSection( Section &section ) {
    String block;

    Segment &segment = m_index[ section.index ];

    block = m_content.substr( segment.start ,segment.end-segment.start );

    Uncomment( block ,*def );

    // String key = segment.name; toupper(key);

    fromString( section.params ,block ); //LATER use stream to allow keeping comments ?

    return true;
}

bool Config::commitSection( Section &section ) {

    //-- generate
    String block ,prettyBlock;

    toString( section.params ,block );
    prettyPrintBlock( block.c_str() ,prettyBlock );

    //-- replace
    int i = section.index ,n = (int) m_sections.size();

    auto &index = m_index[ i ];

    m_content.replace( index.start ,index.size() ,prettyBlock );

    //-- reindex
    int delta = (int) (prettyBlock.size() - index.size());

    index.end += delta;

    for( ++i; i<n; ++i ) {
        auto &ix = m_index[ i ];

        ix.start += delta;
        ix.end += delta;
    }

    return true;
}

//--
void Config::IndexContent() {
    const char *s0 = m_content.c_str();
    String line ,header;

    Segment segment;

    const char *s = s0 ,*s1 = s0 ,*s2;

    int i=0; while( (s1=s) && (NoCommentNorSpace( s,*def ) || true) && ParseLine( s ,&line ) ) {
        if( !ParseHeader( s2 = line.c_str() ,"[" ,"]" ,&header ,false ) ) continue;

        if( i ) {
            m_index[i-1].end = (int) (s1 - s0);
        }

        NoCommentNorSpace( s,*def );

        segment.name = header; tolower(segment.name);
        segment.start = (int) (s - s0);
        segment.end = 0;

        m_index.emplace_back( segment );
        ++i;
    }

    if( i ) { //! fix last end
        m_index[i-1].end = (int) (s1 - s0);
    }
}

//--
void Config::ParseContent() {
    //! with index, from index to index make section

    String block;

    int i=0; for( auto &it : m_index ) {
        auto &section = m_sections[ it.name ];
        section.index = i;
        section.name = it.name;

        readSection( section );

        ++i;
    }
}

void Config::UpdateContent() {
    String block;

    for( auto &it : m_sections ) {
        commitSection( it.second );
    }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
//EOF