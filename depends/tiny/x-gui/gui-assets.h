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

#ifndef TINY_GUI_ASSETS_H
#define TINY_GUI_ASSETS_H

//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE_GUI {

//////////////////////////////////////////////////////////////////////////////
//! Asset

template <class TBase ,class T>
struct AssetStore_ : Store_<String,RefOf<T> > ,Singleton_<TBase> {

    GuiImage *get( const char *name ) {
        if( name && *name ) {} else return NullPtr;

        RefOf<T> *p = this->getInstance().findItem( name );

        return (p && !p->isNull()) ? p->ptr() : NullPtr;
    }

    bool addFromManifest( const Params &manifest ) {
        NameType decl;
        Params props;

        for( const auto &it : manifest ) {
            props.clear();

            fromString( decl ,it.first );
            fromString( props ,it.second );

            RefOf<T> p( new T() );

            if( !p ) continue; //! log this

            fromManifest( *p.ptr() ,props );

            this->RegisterItem( decl.name ,p );
        }

        return true;
    }

    bool addFromManifest( const char *str ) {
        Params params;

        fromString( params ,str );

        return addFromManifest( params );
    }

//--
    bool RegisterObject( const char *name ,IObject *p ) {
        RefOf<T> object( p ? p->getInterface_<T>() : NullPtr );

        return object && RegisterItem( name ,object );
    }
};

///--
struct ImageStore : AssetStore_<ImageStore,GuiImage> {};
struct FontStore : AssetStore_<FontStore,GuiFont> {};
// struct StringStore : AssetStore_<StringStore,String> {};
    //! TODO need string as objects

//////////////////////////////////////////////////////////////////////////////
struct Assets {
    static ImageStore &Image() { return ImageStore::getInstance(); }
    static FontStore &Font() { return FontStore::getInstance(); }
    // static StringStore &String() { return StringStore::getInstance(); }
};
#

/*
const String assetManifest_exemple =
    "header:image = { file=assets/icons/solo-icons.png; }"
    "small:font = { facename=clean; italic=true; ...}"
;
*/

//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE_GUI

//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
#endif //TINY_GUI_ASSETS_H