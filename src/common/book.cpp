// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include <tiny.h>

#include "book.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
typedef BookEntry::Id entryid_t;

template<>
const char *Enum_<BookMode>::names[] = {
    "text" ,"binary"
};

template<>
const BookMode Enum_<BookMode>::values[] = {
    BookMode::bookText ,BookMode::bookBinary
};

//////////////////////////////////////////////////////////////////////////////
//! BookHeader

struct BookHeader {
    char title[32]; //! original title (e.g. 'earnings' )
    char volume[32]; //! periodic label (e.g. '2024-04' for an archive)
    BookMode mode;

    Udbn udbn; //! book serial number
    PUID uuid; //! entry type uuid

    int sizeOfEntry;
    int entryPerPage;

    uint32_t nPage; //! page count
    uint32_t nEntry; //! entry count

    byte user[44]; //! user info (e.g. for totals)
};

//////////////////////////////////////////////////////////////////////////////
//! BookPage

//TODO in common
// typedef int32_t checksum32_t;
// typedef int64_t checksum64_t;

struct BookPage {
    typedef uint32_t offset_t;
    typedef uint16_t page_t;

    struct Header {
        size_t footer; // offset of previous page

        // page_t type; //! 0->15 reserved, 16->default as defined by book ,>16 user defined
        uint16_t nEntry; // count of entry in this page (NB could use bit map of free entries)
    };

    //! NB entry might be binary or text
    struct Entry : BookEntry {
        /// BookEntry::Id id; //! from BookEntry (id not required but used for safe checking)

        ///-- HEAD
        //..

        ///-- CONTENT (fixed size, use reference in payload for extra variable size data)
        //=> ptr to payload (DataRef)
        //=> ptr to other book/entry (Bookmark)

        ///-- FOOT
        // Hash hash;
    };

    //LATER
    /* struct Payload { //! data clipped to this page
        size_t size;
        unsigned char bytes; //! first byte, other bytes follow ...
    } payload; */

    struct Footer {

        offset_t header; //! offset back to header, allow reading from back (NB file is always written with complete pages)

        //! @note next page is right after, previous page via header
    };

//-- reference
    //LATER
    /* struct DataRef {
        offset_t offset;
        size_t size;
    }; */
};

bool isPageFull( const BookHeader &book ,const BookPage::Header &page ) {
    assert( page.nEntry <= book.entryPerPage );

    return page.nEntry == book.entryPerPage;
}

//////////////////////////////////////////////////////////////////////////////
//! PageInfo

#define SIZEOF_BOOKHEADER   sizeof(BookHeader)
#define SIZEOF_PAGEHEADER   sizeof(BookPage::Header)
#define SIZEOF_PAGEFOOTER   sizeof(BookPage::Footer)

#define ENTRY_PER_PAGE      32 //! default entry count

struct PageInfo {
    BookPage::Header header;

    int pageId;
    size_t offset;
};

//////////////////////////////////////////////////////////////////////////////
//! CBookFile (private)

template<>
struct Private_<CBookFile> {
    explicit Private_<CBookFile>( CBookFile &a_book ) : book(a_book)
    {}

    CBookFile &book;

    File &file() { return book.m_file; }

//--
    bool addPage( PageInfo &page ) {
        BookPage::Footer footer;

        page.pageId = (int) bookHeader.nPage;
        page.offset = (size_t) file().GetSize();

        size_t bodySize = bookHeader.sizeOfEntry * bookHeader.entryPerPage;
        size_t bodyOffset = page.offset + SIZEOF_PAGEHEADER;
        size_t footerOffset = bodyOffset + bodySize;

        page.header.footer = footerOffset;
        page.header.nEntry = 0;

        footer.header = page.offset;

        ++ bookHeader.nPage;

        return
            writePageHeader( page.pageId ,page.offset ,page.header )
            && writePageBody( bodyOffset ,bodySize )
            && writePageFooter( footerOffset ,footer )
            && writeBookHeader()
        ;
    }

    bool findPage( int pageId ,PageInfo &page ) {
        if( !findPageStart( pageId ,page ) ) return false;

        int n = pageId - page.pageId;

        if( n > 0 ) {
            while( page.pageId != pageId && flickPageForward( page ) ) {}
        } else {
            while( page.pageId != pageId && flickPageBackward( page ) ) {}
        }

        return true;
    }

    bool findPageStart( int pageId ,PageInfo &page ) {
        if( pageId == 0 ) return getFirstPage(page);

        //-- closest in cache
        page.pageId = -1;

        pageHeaders.eachItem( [pageId,&page]( int id ,PageInfo &info ) -> bool {
            if( abs(pageId-id) < abs(page.pageId-id) ) {
                page = info;
            }

            return page.pageId != pageId;
        });

        if( page.pageId >= 0 )
            return true;

        //-- first or last
        int n = bookHeader.nPage;

        return (pageId < (n - pageId) ) ? getFirstPage( page ) : getLastPage( page );
    }

    bool getFirstPage( PageInfo &page ) {
        page.pageId = 0;
        page.offset = SIZEOF_BOOKHEADER;

        return readPageHeader( page.pageId ,page.offset ,page.header );
    }

    bool getLastPage( PageInfo &page ) {
        page.pageId = (int) bookHeader.nPage;
        page.offset = (size_t) file().GetSize();

        return flickPageBackward( page );
    }

    bool flickPageForward( PageInfo &page ) {
        if( page.header.nEntry < bookHeader.entryPerPage ) return false;

        ++page.pageId;

        return readPageHeader( page.pageId ,page.offset = page.header.footer + SIZEOF_PAGEFOOTER ,page.header );
    }

    bool flickPageBackward( PageInfo &page ) {
        if( page.pageId == 0 ) return false;

        --page.pageId;

        BookPage::Footer footer;

        return
            readPageFooter( page.offset - SIZEOF_PAGEFOOTER ,footer )
            && readPageHeader( page.pageId ,page.offset = footer.header ,page.header )
        ;
    }

//-- book
    bool readBookHeader() {
        return file().Seek( 0 ,SEEK_SET ) == ENOERROR && file().Read( (byte*) &bookHeader ,SIZEOF_BOOKHEADER ) == ENOERROR;
    }

    bool writeBookHeader() {
        return file().Seek( 0 ,SEEK_SET ) == ENOERROR && file().Write( (byte*) &bookHeader ,SIZEOF_BOOKHEADER ) == ENOERROR;
    }

//-- page
    bool readPageHeader( int pageId ,size_t atOffset ,BookPage::Header &header ) {
        if( file().Seek( atOffset ,SEEK_SET ) == ENOERROR && file().Read( (byte*) &header ,SIZEOF_PAGEHEADER ) == ENOERROR ) {} else {
            return false;
        }

        pageHeaders[pageId] = PageInfo({ header ,pageId ,atOffset });

        return true;
    }

    bool writePageHeader( int pageId ,size_t atOffset ,BookPage::Header &header ) {
        if( file().Seek( atOffset ,SEEK_SET ) == ENOERROR && file().Write( (byte*) &header ,SIZEOF_PAGEHEADER ) == ENOERROR ) {} else {
            return false;
        }

        pageHeaders[pageId] = PageInfo({ header ,pageId ,atOffset });

        return true;
    }

    bool readPageFooter( size_t atOffset ,BookPage::Footer &footer ) {
        return file().Seek( atOffset ,SEEK_SET ) == ENOERROR && file().Read( (byte*) &footer ,SIZEOF_PAGEFOOTER ) == ENOERROR;
    }

    bool writePageFooter( size_t atOffset ,BookPage::Footer &footer ) {
        return file().Seek( atOffset ,SEEK_SET ) == ENOERROR && file().Write( (byte*) &footer ,SIZEOF_PAGEFOOTER ) == ENOERROR;
    }

    bool writePageBody( size_t atOffset ,size_t size ) {
        if( file().Seek( atOffset ,SEEK_SET ) != ENOERROR )
            return false;

        Bytes z( 0 ,size );

        return file().Write( z.ptr() ,size ) == ENOERROR;
    }

///-- members
    BookHeader bookHeader;

    Map_<int,PageInfo> pageHeaders; //! page headers location cache { pageId => offset }
        //MAYBE use cache from pattern

    // Bytes buffer; //! entry read/write buffers
};

//////////////////////////////////////////////////////////////////////////////
//! CBookFile

CBookFile::CBookFile() :
    WithPrivate_<CBookFile>( new Private_<CBookFile>(*this) )
    ,m_uuid( 0 ) ,m_sizeofEntry( 0 ) ,m_mode( bookText )
{}

CBookFile::CBookFile( PUID uuid ,int sizeofEntry ,BookMode mode ) :
    WithPrivate_<CBookFile>( new Private_<CBookFile>(*this) )
    ,m_uuid( uuid ) ,m_sizeofEntry( sizeofEntry ) ,m_mode( mode )
{}

CBookFile::~CBookFile() {
    Close();

    delete m_private;
}

byte *CBookFile::getUserData( size_t &size ) {
    size = 44; return priv().bookHeader.user;
}

bool CBookFile::updateUserData() {
    return priv().writeBookHeader() && m_file.Flush() == ENOERROR;
}

bool CBookFile::setUserData( byte *data ,size_t size ) {
    if( !data || size > 44 ) return false;

    byte *user = priv().bookHeader.user;

    if( data != user ) {
        memcpy( user ,data ,size );
    }

    return updateUserData();
}

///-- file
IRESULT CBookFile::makeFilepath( const char *title ,const char *path ,String &filepath ) {
    if( title && *title && strlen(title) < 31 ) {} else return IBADARGS;

    filepath = path ? path : "";
    filepath += title;
    filepath += ".book";

    return IOK;
}

IRESULT CBookFile::makeFilepath( const char *title ,const char *path ) {
    return makeFilepath( title ,path ,m_filepath );
}

//--
IRESULT CBookFile::Archive( const char *volume ,entryid_t untilId ) {
    _TODO; return INOEXEC; //TODO
}

IRESULT CBookFile::Create( const char *title ,const char *path ,PUID uuid ,int sizeofEntry ,BookMode mode ) {
    if( uuid && sizeofEntry ) {} else return IBADARGS;

    IRESULT result = makeFilepath( title ,path ); IF_IFAILED_RETURN(result);

    m_uuid = uuid;
    m_sizeofEntry = sizeofEntry;

    return Create( title ,path );
}

IRESULT CBookFile::Create( const char *title ,const char *path ) {
    IRESULT result = makeFilepath( title ,path ); IF_IFAILED_RETURN(result);

    if( m_file.isOpen()) return IALREADY;

    OsError error = m_file.Open( m_filepath.c_str() ,OS_ACCESS_ALL ,OS_SHARE_READ ,OS_CREATE_NOEXIST );

    if( error != ENOERROR )
        return IERROR;

    return makeBookHeader( title ) && priv().writeBookHeader() ? IOK : IERROR;
}

IRESULT CBookFile::Open( const char *title ,const char *path ,bool createIfNotExit ) {
    IRESULT result = makeFilepath( title ,path ); IF_IFAILED_RETURN(result);

    if( m_file.isOpen()) return IALREADY;

    OsError error = m_file.Open( m_filepath.c_str() ,OS_ACCESS_ALL ,OS_SHARE_READ ,OS_CREATE_DONT );

    if( error != ENOERROR ) { //! doesn't exist
        return createIfNotExit ? Create( title ,path ) : IERROR;
    }

    if( m_file.GetSize() == 0 ) { //! exist but is empty
        return makeBookHeader( title ) && priv().writeBookHeader() ? IOK : IERROR;
    }

    return priv().readBookHeader() && matchBookHeader(title) ? IOK : IERROR;
}

void CBookFile::Close() {
    m_file.Close();
}

///-- pages
size_t CBookFile::getPageCount() const {
    return priv().bookHeader.nPage;
}

bool CBookFile::getPageIndices( int pageId ,entryid_t &a ,entryid_t &b ) const {
    if( pageId >= getPageCount() ) return false;

    size_t n = priv().bookHeader.entryPerPage;

    a = n * (pageId / n);
    b = a + n-1;

    return true;
}

///-- entries
size_t CBookFile::getEntryCount() const {
    return priv().bookHeader.nEntry;
}

entryid_t CBookFile::addEntry( const byte *entry ,size_t size ) {
    size_t offset;
    entryid_t id;

    if( size > m_sizeofEntry || !makeNewEntry( id ,offset ) )
        return INVALID_ENTRY_VALUE;

//-- write line
    //! devnote may want to 0 pad entry write
    return m_file.Seek( offset ,SEEK_SET ) == ENOERROR && m_file.Write( entry ,size ) == ENOERROR;
}

bool CBookFile::readEntry( entryid_t id ,byte *entry ,size_t size ) {
    size_t offset;

    if( size > m_sizeofEntry || !getEntryOffset( id ,offset ) )
        return false;

//-- read line
    return m_file.Seek( offset ,SEEK_SET ) == ENOERROR && m_file.Read( entry ,size ) == ENOERROR;
}

bool CBookFile::writeEntry( entryid_t id ,const byte *entry ,size_t size ) {
    size_t offset;

    if( size > m_sizeofEntry || !getEntryOffset( id ,offset ) )
        return false;

//-- write line
    //! devnote may want to 0 pad entry write
    return m_file.Seek( offset ,SEEK_SET ) == ENOERROR && m_file.Write( entry ,size ) == ENOERROR;
}

///-- protected

bool CBookFile::makeBookHeader( const char *title ) {
    if( strlen( title ) > 31 ) {
        assert(false); //! should not happen, test before
        return false;
    }

//--
    BookHeader &header = priv().bookHeader;

    memset( (byte*) &header ,0 ,SIZEOF_BOOKHEADER );

    strncpy( header.title ,title ,32 );
    // header.volume; //! @note intentionally blank, for archives
    header.mode = m_mode;

    header.udbn.Make(); //! book serial number
    header.uuid = m_uuid; //! entry type uuid

    header.sizeOfEntry = m_sizeofEntry;
    header.entryPerPage = ENTRY_PER_PAGE;

//--
    m_title = title;
    m_volume.clear();
    m_udbn = header.udbn;

    return true;
}

bool CBookFile::matchBookHeader( const char *title ) {
    BookHeader &header = priv().bookHeader;

    if( strncmp( header.title ,title ,31 ) == 0
        && header.uuid == m_uuid
        && header.sizeOfEntry >= m_sizeofEntry
    ) {} else {
        return false;
    }

//--
    m_title = header.title;
    m_volume = header.volume;
    m_mode = header.mode;

    m_udbn = header.udbn;
    // uuid //! already have it

    m_sizeofEntry = header.sizeOfEntry;

    return true;
}

//-- entries
bool CBookFile::makeNewEntry( entryid_t &id ,size_t &offset ) {
    BookHeader &header = priv().bookHeader;

    PageInfo page;

    if( ( priv().getLastPage( page ) && !isPageFull( header ,page.header ) ) || priv().addPage( page ) ) {} else {
        return false;
    }

    assert(page.header.nEntry <= header.entryPerPage);

    id = page.pageId + page.header.nEntry;

    ++ page.header.nEntry;

    if( !priv().writePageHeader( page.pageId ,page.offset ,page.header ) )
        return false;

    ++ header.nEntry;

    if( !priv().writeBookHeader() )
        return false; //! Yikes, book would be in some invalid state here

    offset = page.offset + SIZEOF_PAGEHEADER + (page.header.nEntry-1) * m_sizeofEntry;

    return true;
}

bool CBookFile::getEntryOffset( entryid_t id ,size_t &offset ) {
    PageInfo page;

    int n = (int) priv().bookHeader.entryPerPage; //! IE n = entry per page

    int pageId = (int) (id / n);

    if( !priv().findPage( pageId ,page ) )
        return false;

    int firstId = pageId * n; //! IE first id in page

    int i = (int) (id - firstId);

    if( i >= page.header.nEntry ) return false; //! id above entries in page

    offset = page.offset + SIZEOF_PAGEHEADER + (i) * m_sizeofEntry;

    return true;
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//EOF