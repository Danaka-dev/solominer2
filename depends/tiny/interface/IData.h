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

#ifndef TINY_IDATA_H
#define TINY_IDATA_H

//////////////////////////////////////////////////////////////////////////////
#include <tiny.h>

//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
#define IDATASOURCE_PUID        0x01699144b333044c7
#define IDATAEVENT_PUID         0x095335c0c34260acc

///--
struct IDataSource;
struct IDataEvents;

//////////////////////////////////////////////////////////////////////////////
//! IDataEvent interface

    //! @brief interface for data client to receive data source events

struct IDataEvents : IOBJECT_PARENT {
    DECLARE_CLASS(IDataEvents,IDATAEVENT_PUID);

    //! from data source to client (edit)

    IAPI_DECL onDataCommit( IDataSource &source ,Params &data ) = 0;
    //! transact with client to commit data
    //! (source call on commit) : client should update &data with their own data (if any)

    IAPI_DECL onDataChanged( IDataSource &source ,const Params &data ) = 0;
    //! advise client to update their data
    //! (source call on initial,cancel) : client should discard any data/edit and refresh with &data provided
};

//////////////////////////////////////////////////////////////////////////////
//! IDataSource interface

    //! @biref interface for data provider

struct IDataSource : CPublisher_<IDataEvents> ,IOBJECT_PARENT {
    DECLARE_CLASS(IDataSource,IDATASOURCE_PUID);

    //! query/advise from client (edit) to source

//-- source
    IAPI_DECL Connect( const char *source ,const Params &params ) = 0;

    IAPI_DECL Bind( const char *table ) = 0;

    IAPI_DECL getInfo( Params &params ) = 0;

//-- data
    //! absolute or relative seek to data record
    //! id content interpretation is implementation specific
    IAPI_DECL Seek( const char *id ) = 0;

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
} //TINY_NAMEPSACE

//////////////////////////////////////////////////////////////////////////////
#endif //TINY_IDATA_H