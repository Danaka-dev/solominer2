#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#ifndef SOLOMINER_BOOK_H
#define SOLOMINER_BOOK_H

//////////////////////////////////////////////////////////////////////////////
#include "common.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
#define TINY_CBOOKFILE_PUID      0x0f06f0f82974f18bb

class CBookFile;

//////////////////////////////////////////////////////////////////////////////
#define INVALID_ENTRY_VALUE     ((uint32_t) -1)

typedef Guid Udbn; //! Universal Digital Book Number

struct BookEntry {
    typedef uint32_t Id;

    Id sequence; //! book entry sequence number
};

struct Bookmark {
    Udbn udbn;
    BookEntry::Id sequence;
};

enum BookMode {
    bookText=0 ,bookBinary
};

//////////////////////////////////////////////////////////////////////////////
//! CBookFile

class CBookFile : private WithPrivate_<CBookFile> ,COBJECT_PARENT {
    friend class Private_<CBookFile>;

public:
    typedef BookEntry::Id entryid_t;

public:
    CBookFile();

    CBookFile( PUID uuid ,int sizeofEntry ,BookMode mode=bookText );

    virtual ~CBookFile();

    DECLARE_OBJECT_STD(CObject,CBookFile,TINY_CBOOKFILE_PUID);

    const char *title() const { return m_title.c_str(); }
    const char *volume() const { return m_volume.c_str(); }
    BookMode bookmode() const { return m_mode; }

    const Guid &udbn() const { return m_udbn; }
    const PUID &uuid() const { return m_uuid; }

    size_t sizeofEntry() const { return m_sizeofEntry; }

    byte *getUserData( size_t &size );
    bool updateUserData();
    bool setUserData( byte *data ,size_t size );

    bool isOpen() { return m_file.isOpen(); }

public:
    static IRESULT makeFilepath( const char *title ,const char *path ,String &filepath );

    IRESULT makeFilepath( const char *title ,const char *path );

//-- file
    IRESULT Archive( const char *volume ,entryid_t untilId );

    IRESULT Create( const char *title ,const char *path ,PUID uuid ,int sizeofEntry ,BookMode mode=bookText );
    IRESULT Create( const char *title ,const char *path );
    IRESULT Open( const char *title ,const char *path="" ,bool createIfNotExit=true );
    void Close();

//-- pages
    size_t getPageCount() const;

    bool getPageIndices( int pageId ,entryid_t &firstId ,entryid_t &lastId ) const;

//-- entries
    size_t getEntryCount() const;

    entryid_t addEntry( const byte *entry ,size_t size );
    bool readEntry( entryid_t id ,byte *entry ,size_t size );
    bool writeEntry( entryid_t id ,const byte *entry ,size_t size );
        //! @note size here because entry bytes may be shorter than sizeofEntry, if so written data is 0 padded

protected:
    bool makeBookHeader( const char *title );
    bool matchBookHeader( const char *title );

    bool makeNewEntry( entryid_t &id ,size_t &offset );
    bool getEntryOffset( entryid_t id ,size_t &offset );

protected:

//-- file info
    String m_filepath; //! complete path to load/save file
    File m_file; //! file object associated with the book

//-- header info
    String m_title; //! book title, normally the filename without extension
    String m_volume; //! archive volume label
    BookMode m_mode; //! what serialization format entries are recorded with

    Guid m_udbn; //! universal digital book number
    PUID m_uuid; //! entry uuid

    int m_sizeofEntry; //! size in byte of an entry
};

typedef RefOf<CBookFile> CBookFileRef;

//////////////////////////////////////////////////////////////////////////////
//! BookShelf

class CBookShelf : Singleton_<CBookShelf> {
public:
    CBookShelf() DEFAULT;

    CBookFile *getBookByTitle( const char *title );
    CBookFile *getBookByUdbn( const Guid &udbn );

//-- bookmark
    CBookFile *findBook( const Bookmark &bookmark );
    byte *findEntry( const Bookmark &bookmark );

//-- registry
    bool registerBook( CBookFile &book );
    bool releaseBook( CBookFile &book );

protected:
    Map_<Guid,String> _catalog; //! known book
        //! save to catalog file

    Map_<Guid,CBookFileRef> _books; //! opened book
};

//////////////////////////////////////////////////////////////////////////////
//! CBookFile_

#define BOOK_CACHE_NONE ((size_t) 0)
#define BOOK_CACHE_ALL  ((size_t) -1)

template <class T>
class CBookFile_ : public CBookFile {
public:
    CBookFile_( int sizeofEntry=T::sizeofEntry() ,BookMode mode=bookText ,size_t cacheCount=32 ) :
        CBookFile( T::classId() ,sizeofEntry ,mode )
    {}

public:
    template <typename THeader>
    THeader &getHeader() {
        size_t size; THeader &header = * (THeader*) getUserData( size );

        assert( size > sizeof(THeader) );

        return header;
    }

    bool updateHeader() {
        return updateUserData();
    }

public:
    //TODO properly use chache and commits
    void Commit() {
        for( auto &it : m_cache.map() ) if( it.second.touched ) {
            writeEntry( it.first ,it.second.entry );

            it.second.touched = false;
        }

        m_file.Flush();
    }

    void Rollback() {
        for( auto &it : m_cache.map() ) if( it.second.touched ) {
            readEntry( it.first ,it.second.entry );

            it.second.touched = false;
        }
    }

public:
    id_t addEntry( const T &entry ,bool commit=false ) {
        entryid_t id = getEntryCount();

        auto &it = m_cache[id];

        it = entry; //! @note forcing cache for commit
        it.touched = false;

        if( !writeEntry( id ,entry ,true ) )
            return INVALID_ENTRY_VALUE;

        Commit();

        return id;
    }

    bool updateEntry( entryid_t id ,const T &entry ,bool commit=true ) {
        m_cache[id] = entry; //! @note forcing cache for commit

        if( commit ) Commit();

        return true;
    }

    T *getEntry( entryid_t id ) {
        auto *p = m_cache.findItem( id );

        if( p ) {
            p->Hit(); return &(p->entry);
        }

        T entry;

        Zero( entry );

        if( !readEntry( id ,entry ) )
            return NullPtr;

        auto &it = m_cache[id]; //! @note forcing cache for return pointer

        it = entry; //! @note forcing cache for return pointer

        return &(it.entry);
    }

    bool getEntry( entryid_t id ,T &entry ) {
        auto *p = getEntry( id );

        if( !p ) return false;

        entry = *p;

        return true;
    }

    template <typename F>
    void eachEntry( F &&filter ,bool backward=true ) {
        auto n = (entryid_t) this->getEntryCount();

        for( entryid_t i=0; i<n; ++i ) {
            entryid_t id = backward ? (n-i-1) : i;

            T *p = getEntry( id );

            if( p && filter( id ,*p ) ) {} else return;
        }
    }

    template <typename F>
    T *findEntry( F &&filter ,bool backward=true ) {
        auto n = (entryid_t) this->getEntryCount();

        for( entryid_t i=0; i<n; ++i ) {
            entryid_t id = backward ? (n-i-1) : i;

            T *p = getEntry( id );

            if( p && filter( id ,*p ) ) return p;
        }

        return NullPtr;
    }

    template <typename F>
    bool listEntry( F &&filter ,ListOf<T*> &list ,bool backward=true ) {
        auto n = (entryid_t) this->getEntryCount();
        bool found = false;

        for( entryid_t i=0; i<n; ++i ) {
            entryid_t id = backward ? (n-i-1) : i;

            T *p = getEntry( id );

            if( p && filter( id ,*p ) ) {
                list.emplace_back( p );
                found = true;
            }
        }

        return found;
    }

protected:
    Bytes m_buffer;

    bool readEntry( entryid_t id ,T &entry ) {
        m_buffer.Reserve( sizeofEntry() );

        if( !CBookFile::readEntry( id ,m_buffer.ptr() ,sizeofEntry() ) )
            return false;

        entry.sequence = id;

        switch( m_mode ) {
            case bookText:
                return readTextEntry( m_buffer.ptr() ,entry );
            case bookBinary:
                return readBinaryEntry( m_buffer.ptr() ,entry );

            default:
                return false;
        }
    }

    bool writeEntry( entryid_t id ,const T &entry ,bool add=false ) {
        Bytes bytes; bool result;

        switch( m_mode ) {
            case bookText:
                result = writeTextEntry( entry ,bytes ); break;
            case bookBinary:
                result = writeBinaryEntry( entry ,bytes ); break;
            default:
                return false;
        }

        return add ?
            CBookFile::addEntry( bytes.ptr() ,bytes.count() ) != INVALID_ENTRY_VALUE
            : CBookFile::writeEntry( id ,bytes.ptr() ,bytes.count() )
        ;
    }

//-- mode text
    bool readTextEntry( const byte *p ,T &entry ) {
        assert( m_mode == bookText && p );

        const char *str = (const char*) p;

        String s( str ,strlen(str) );

        fromString( entry ,s );

        return true;
    }

    bool writeTextEntry( const T &entry ,Bytes &bytes ) {
        String s;

        toString( entry ,s );

        if( s.size() > sizeofEntry() ) {
            assert(false); //! should not happen
            return false;
        }

        bytes.copyFrom( (byte*) s.c_str() ,s.size()+1 );

        return true;
    }

//-- mode binary
    bool readBinaryEntry( const byte *p ,T &entry ) {
        _TODO; return false;
    }

    bool writeBinaryEntry( const T &entry ,Bytes &bytes ) {
        _TODO; return false;
    }

//-- cache
    void setCache( entryid_t id ,const T &entry ) {
        m_cache[id] = entry;
    }

    // bool getCache

    struct Cache {
        Cache &operator =( const T &other ) { entry = other; touched = true; return *this; }

        void Hit() { ++hit; }

        T entry;

        bool touched = false;
        int hit = 0;
    };

    Map_<entryid_t,Cache> m_cache; //TODO use cache from pattern

    size_t m_cacheCount; //! count of cache entries 0=none ,BOOK_CACHE_ALL=all
};

//////////////////////////////////////////////////////////////////////////////
//! BookDataSource

    //! @brief book file as data source

template <class TEntry ,class TEntry2>
class CBookDataSource_ : public CDataSource_<CBookFile_<TEntry>,TEntry2> ,COBJECT_PARENT {
public:
    CBookDataSource_( CBookFile_<TEntry> &source  ) : CDataSource_<CBookFile_<TEntry> ,TEntry2>( source )
    {}

public:
    IAPI_IMPL getInfo( Params &params ) IOVERRIDE {
        if( hasMember( params ,"count" ) )
            toString( this->m_source->getEntryCount() ,params["count"] );

        return IOK;
    }

public: ///-- CDataSource_
    API_IMPL(bool) hasData() IOVERRIDE {
        return this->m_source->isOpen();
    }

    IAPI_IMPL Open( const char *source ,const Params &params ) IOVERRIDE {
        return this->m_source->Open( source ) ? IOK : IERROR;
    }

    API_IMPL(void) Close() IOVERRIDE {
        this->m_source->Close();
    }

    API_IMPL(bool) getEntry( TEntry2 &entry ) IOVERRIDE {
        auto *p = this->m_source->getEntry( this->m_id );

        if( !p ) return false;

        entry = *p;

        return true;
    }

    API_IMPL(bool) setEntry( TEntry2 &entry ) IOVERRIDE {
        return this->m_source->updateEntry( this->m_id ,entry ,false );
    }
};

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer {

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_BOOK_H