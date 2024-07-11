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
#include "data-source.h"

//////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
//! ConfigDataSource

    //! @brief a Config file as data source

class ConfigDataSource : public CDataSource ,COBJECT_PARENT {
public:
    ConfigDataSource( Config &config ,const char *section=NullPtr ) :
        m_config(config) ,m_section(NullPtr) ,m_haveEdit(false)
    {
        if( section ) Bind( section );
    }

    IAPI_IMPL getInterface( PUID id ,void **ppv ) IOVERRIDE {
        return CDataSource::getInterface(id,ppv);
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

    IAPI_IMPL getInfo( Params &params ) IOVERRIDE {
        if( hasMember(params,"haveedit") ) toString( m_haveEdit ,params["haveedit"] );

        return IOK;
    }

//--
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
            //! @note client can advise datasource edit has started but doesn't want to transact the data yet
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

public:
    void Update() { //! @note friendly name for updating using Discard
        Discard();
    }

protected:
    API_DECL(bool) readValues( const char *id ) {
        m_entry = id;

        return readValues();
    }

    API_DECL(bool) readValues() {
        auto it = m_section->params.find( m_entry );
        if( it == m_section->params.end() ) return false;

        fromString( m_values ,it->second );
        m_haveEdit = false;

        return true;
    }

    API_DECL(bool) writeValues() {
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