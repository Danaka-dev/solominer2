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

#ifndef TINY_XDATA_CONFIG_H
#define TINY_XDATA_CONFIG_H

//////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
//! Data Interface (later in interface)

//////////////////////////////////////////////////////////////////////////////
//! IData interface

    //TODO in its own file

struct IDataSource;

struct IDataEvent : IOBJECT_PARENT {
    //! from data source to client (edit)

    IAPI_DECL onDataCommit( IDataSource &source ,Params &data ) = 0;
    //! transact with client to commit data
    //! (source call on commit) : client should update &data with their own data (if any)

    IAPI_DECL onDataChanged( IDataSource &source ,const Params &data ) = 0;
    //! advise client to update their data
    //! (source call on initial,cancel) : client should discard any data/edit and refresh with &data provided
};

struct IDataSource : CPublisher_<IDataEvent> ,IOBJECT_PARENT {
    //! query/advise from client (edit) to source

//-- source
    IAPI_DECL Connect( const char *source ,const Params &params ) = 0;

    IAPI_DECL Bind( const char *table ) = 0;

    //! absolute or relative seek to data record
    //! id content interpretation is implementation specific
    IAPI_DECL Seek( const char *id ) = 0;

//-- control
    //! commit pending edits to storage
    IAPI_DECL Commit() = 0;

    //! discard pending edits
    IAPI_DECL Discard() = 0;

//-- client
    //! client request source to list all data header (optionally with current values)
    IAPI_DECL readHeader( Params &data ,bool requireValues=false ) = 0;

    //! client request source to fill in data values
    IAPI_DECL readData( Params &data ) = 0;

    //! client advise data was edited
    //! source confirm with return value and validated &data
    IAPI_DECL onDataEdit( Params &data ) = 0;
};

typedef RefOf<IDataSource> IDataSourceRef;

//////////////////////////////////////////////////////////////////////////////
//! CDataSource

class CDataSource : public IDataSource {
public:
//-- source
    IAPI_IMPL Connect( const char *source ,const Params &params ) IOVERRIDE { return INOEXEC; }
    IAPI_IMPL Bind( const char *table ) IOVERRIDE { return INOEXEC; }
    IAPI_IMPL Seek( const char *id ) IOVERRIDE { return INOEXEC; }

//-- control
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
//! ConfigDataSource

    //! @brief a Config file as data source

class ConfigDataSource : public CDataSource ,COBJECT_PARENT {
public:
    ConfigDataSource( Config &config ,const char *section=NullPtr ) :
            m_config(config) ,m_section(NullPtr) ,m_haveEdit(false)
    {
        if( section ) Seek( section );
    }

public: ///-- IDataSource
    IAPI_IMPL Connect( const char *source ,const Params &params ) IOVERRIDE {
        if( getMember_<bool>(params,"commit","false") && m_haveEdit ) Commit();

        if( source && *source ) {} else {
            //! disconnecting
            m_config.CloseFile();
            return IOK;
        }

        if( *source == '$' ) {
            //! get from store
            // m_config = Store_<Config>::getInstance(); //TODO reform Store for this
            _TODO; //TODO
            return INOEXEC;
        } else {
            //! get from file
            return m_config.LoadFile( source ) ? IOK : IERROR;
        }
    }

    IAPI_IMPL Bind( const char *table ) IOVERRIDE {
        if( table && *table ) {} else return IBADARGS;

        m_section = m_config.peekSection( table );

        return m_section ? IOK : INODATA;
    }

    IAPI_IMPL Seek( const char *id ) IOVERRIDE {
        if( id && *id ) {} else return IBADARGS;
        if( !m_section ) return IBADENV;

        if( m_haveEdit ) Commit();

        if( !readValues( id ) ) return INODATA;

        adviseDataChanged(m_values);

        return IOK;
    }

    IAPI_IMPL Commit() IOVERRIDE {
        if( !m_section ) return INODATA;
        if( !m_haveEdit ) return INOTHING;

        for( auto &it : m_subscribers ) {
            IRESULT result = it->onDataCommit( *this ,m_values );
            IF_IFAILED_RETURN(result);
        }

        return writeValues() ? IOK : IERROR;
    }

    IAPI_IMPL Discard() IOVERRIDE {
        if( !m_section ) return IOK;

        if( !readValues() ) return INODATA;

        adviseDataChanged(m_values);

        return IOK;
    }

    //--
    IAPI_IMPL readHeader( Params &data ,bool requireValues=false ) IOVERRIDE {
        if( !m_section ) return INODATA;

        for( auto &it : m_values ) {
            data[ it.first ] = it.second;
        }

        return IOK;
    }

    IAPI_IMPL readData( Params &data ) IOVERRIDE {
        if( !m_section ) return INODATA;

        for( auto &it : data ) {
            auto field = m_values.find( it.first );

            if( field != m_values.end() ) {
                it.second = field->second;
            }
        }

        return IOK;
    }

    IAPI_IMPL onDataEdit( Params &data ) IOVERRIDE {
        if( !m_section ) return INODATA;

        if( data.empty() ) {
            //! @note client can datasource edit has started but doesn't want to transact the data yet
            //! (e.g. an edit control received a char ...)

            adviseDataChanged(data);
            m_haveEdit = true;
            return IOK;
        }

        auto &params = m_section->params;

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

protected:
    bool readValues( const char *id ) {
        auto it = m_section->params.find( id );
        if( it == m_section->params.end() ) return false;

        m_entry = id;
        fromString( m_values ,it->second );
        m_haveEdit = false;

        return true;
    }

    bool readValues() {
        return readValues( m_entry.c_str() );
    }

    bool writeValues() {
        String s;

        toString( m_values ,s );
        m_section->params[ m_entry ] = s;

        return m_config.commitSection( *m_section ) && m_config.SaveFile();
    }

protected:
    Config &m_config; //TODO, this to a pointer

    Config::Section *m_section;
    String m_entry;
    Params m_values;

    bool m_haveEdit;
};

//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
#endif //TINY_XDATA_CONFIG_H