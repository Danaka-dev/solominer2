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

#ifndef TINY_XDATA_SOURCE_H
#define TINY_XDATA_SOURCE_H

//////////////////////////////////////////////////////////////////////////
#include "../interface/IData.h"

//////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
//! Data Interface helpers

template <typename T>
T getInfoField_( IDataSource *source ,const char *field ) {
    assert(source && field && *field);

    Params params = {{field,""}};

    source->getInfo(params);

    T info;

    fromString( info ,params[field] );

    return info;
}

//////////////////////////////////////////////////////////////////////////////
//! CDataSource

class CDataSource : public IDataSource ,COBJECT_PARENT {
public:
    DECLARE_OBJECT_STD(CObject,IDataSource,IDATASOURCE_PUID);

public:
//-- source
    IAPI_IMPL Connect( const char *source ,const Params &params ) IOVERRIDE { return INOEXEC; }
    IAPI_IMPL Bind( const char *table ) IOVERRIDE { return INOEXEC; }
    IAPI_IMPL getInfo( Params &params ) IOVERRIDE { return INOEXEC; }

//-- data
    IAPI_IMPL Seek( const char *id ) IOVERRIDE { return INOEXEC; }
    IAPI_IMPL Commit() IOVERRIDE { return INOEXEC; }
    IAPI_IMPL Discard() IOVERRIDE { return INOEXEC; }

//-- client
    IAPI_IMPL readHeader( Params &data ,bool requireValues=false ) IOVERRIDE { return INOEXEC; }
    IAPI_IMPL readData( Params &data ) IOVERRIDE { return INOEXEC; }
    IAPI_IMPL onDataEdit( Params &data ) IOVERRIDE { return INOEXEC; }

protected:
    void adviseDataChanged( Params &data ) {
        for( auto &it : m_subscribers ) {
            it->onDataChanged( *this ,data );
        }
    }
};

//////////////////////////////////////////////////////////////////////////////
//! CDataSource

    /// @brief generic data source support

template <class TSource ,class TEntry>
class CDataSource_ : public CDataSource ,COBJECT_PARENT {
public:
    CDataSource_( TSource &source  ) :
        m_source(source) ,m_haveEdit(false)
    {}

    TSource &Source() { return m_source; }

    bool &HaveEdit() { return m_haveEdit; }

    IAPI_IMPL getInterface( PUID id ,void **ppv ) IOVERRIDE {
        return CDataSource::getInterface(id,ppv);
    }

public: ///-- IDataSource
    IAPI_IMPL Connect( const char *source ,const Params &params ) IOVERRIDE {
        if( getMember_<bool>(params,"commit","false") && m_haveEdit ) Commit();

        Close();

        if( source && *source ) {} else {
            //! disconnecting only
            return IOK;
        }

        if( *source == '$' ) {
            //! get from store
            // m_config = Store_<Config>::getInstance();
            //TODO reform Store for this
            return INOEXEC;
        } else {
            //! get from file

            return Open( source ,params );
        }
    }

    IAPI_IMPL Bind( const char *table ) IOVERRIDE {
        if( !hasData() ) return INODATA;

        if( table && *table ) {} else return IBADARGS;

        return IOK;
    }

//--
    IAPI_IMPL Seek( const char *id ) IOVERRIDE {
        if( id && *id ) {} else return IBADARGS;

        if( m_haveEdit ) Commit();

        //TODO process id "next" ,"prev" ,"first" ,"end" ...

        if( !readValues( id ) ) return INODATA;

        adviseDataChanged(m_values);

        return IOK;
    }

    IAPI_IMPL Commit() IOVERRIDE {
        if( !hasData() ) return INODATA;

        for( auto &it : m_subscribers ) {
            IRESULT result = it->onDataCommit( *this ,m_values );
            IF_IFAILED_RETURN(result);
        }

        return writeValues() ? IOK : IERROR;
    }

    IAPI_IMPL Discard() IOVERRIDE {
        if( !hasData() ) return INODATA;

        if( !readValues() ) return INODATA;

        adviseDataChanged(m_values);

        return IOK;
    }

    //--
    IAPI_IMPL readHeader( Params &data ,bool requireValues=false ) IOVERRIDE {
        if( !hasData() ) return INODATA;

        data = m_values;

        return IOK;
    }

    IAPI_IMPL readData( Params &data ) IOVERRIDE {
        if( !hasData() ) return INODATA;

        for( auto &it : data ) {
            auto field = m_values.find( it.first );

            if( field != m_values.end() ) {
                it.second = field->second;
            }
        }

        return IOK;
    }

    IAPI_IMPL onDataEdit( Params &data ) IOVERRIDE {
        if( !hasData() ) return INODATA;

        if( data.empty() ) {
            //! @note client can datasource edit has started but doesn't want to transact the data yet
            //! (e.g. an edit control received a char ...)

            adviseDataChanged(data);
            m_haveEdit = true;
            return IOK;
        }

        bool haveEdit = false;

        IRESULT result = IOK;

        for( auto &entry : data ) {
            auto it = m_values.find( entry.first );

            if( it == m_values.end() ) {
                result = IPARTIAL; continue;
            }

            haveEdit |= (it->second != entry.second);

            it->second = entry.second;
        }

        if( haveEdit ) {
            adviseDataChanged(data);
            m_haveEdit = true;
        }

        return result;
    }

public: ///-- CDataSource_
    API_DECL(bool) hasData() = 0;

    IAPI_DECL Open( const char *source ,const Params &params ) = 0;
    API_DECL(void) Close() = 0;

    API_DECL(bool) getEntry( TEntry &entry ) = 0;
    API_DECL(bool) setEntry( TEntry &entry ) = 0;

protected:
    bool readValues( const char *id ) {
        fromString( m_id ,id );

        return readValues();
    }

    API_DECL(bool) readValues() {
        TEntry entry;

        if( !getEntry( entry ) ) return false;

        toManifest( entry ,m_values );

        m_haveEdit = false; //! @note if any edit it is canceled here

        //! @note no data change advise here,
        //! its done IDataSource implementation

        return true;
    }

    API_DECL(bool) writeValues() {
        TEntry entry;

        fromManifest( entry ,m_values );

        return setEntry( entry );
    }

protected:
    RefOf<TSource> m_source;

    //-- data
    size_t m_id;
    Params m_values;

    //-- edition
    bool m_haveEdit;
};

//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
#endif //TINY_XDATA_SOURCE_H