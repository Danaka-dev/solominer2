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

#ifndef TINY_GUI_THEME_H
#define TINY_GUI_THEME_H

//////////////////////////////////////////////////////////////////////////////
TINY_GUI_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
//! Theme

class VisualTheme {
public:
    typedef unsigned int index_t;

public: ///-- dictionary
    VisualTheme( const char *name ) : m_name(name)
    {
        m_defaultColors = { OS_COLOR_DARKGRAY ,OS_COLOR_BLACK ,OS_COLOR_LIGHTGRAY ,OS_COLOR_BLACK };
    }

    DECLARE_MANIFEST_API(VisualTheme,Params)

    const char *name() const { return m_name.c_str(); }

    index_t index( const char *name ) {
        static index_t i = 0;

        auto *it = m_names.findItem( name );

        return it ? *it : (m_names[ name ] = ++i);
    }

    Params &vars() { return m_vars; }

public: ///-- global
    OsColorRef getColor( index_t i ) {
        auto *it = m_palette.findItem(i); return it ? *it : OS_COLOR_NONE;
    }

    GuiImage *getImage( index_t i ) {
        auto *it = m_images.findItem(i); return it ? it->ptr() : NullPtr;
    }

    GuiFont *getFont( index_t i ) {
        auto *it = m_fonts.findItem(i); return it ? it->ptr() : NullPtr;
    }

    const char *getProperty( index_t i ) {
        auto *it = m_properties.findItem(i); return it ? it->c_str() : NullPtr;
    }

    //--
    ColorRef getColor( const char *name ) {
        auto *it = m_names.findItem(name); return it ? getColor( *it ) : OS_COLOR_NONE;
    }

    GuiImage *getImage( const char *name ) {
        auto *it = m_names.findItem(name); return it ? getImage( *it ) : NullPtr;
    }

    GuiFont *getFont( const char *name ) {
        auto *it = m_names.findItem(name); return it ? getFont( *it ) : NullPtr;
    }

    const char *getProperty( const char *name ) {
        auto *it = m_names.findItem(name); return getProperty( *it );
    }

protected:
    String m_name; //! theme name
    Params m_vars; //! replacement variable for theme manifest
        //! @note colors are automatically added to variables

    Map_<String,index_t> m_names; //! name to index map (perf, all index dico...)

    Map_<index_t,OsColorRef> m_palette; //! Theme color collection
    Map_<index_t,GuiImageRef> m_images; //! Theme image collection
    Map_<index_t,GuiFontRef> m_fonts; //! Theme font collection
        //! @note Theme to hold a (named) selection of assets
        //! (i.e. assets are referenced here, not declared)
    Map_<index_t,String> m_properties; //! Theme font collection

public: ///-- per control
    struct Controls {
        Controls( VisualTheme &a_theme ) : theme(a_theme) {}

        DECLARE_MANIFEST_API(Controls,Params);

        VisualTheme &theme;

        Map_<index_t,ColorQuad> colors;
        Map_<index_t,Point> sizes;
        Map_<index_t,Params> properties;
    };

    Map_<UUID,Controls*> m_controls;
    ColorQuad m_defaultColors;

    const ColorQuad &getColors( const UUID &id ,index_t i ) {
        auto **control = m_controls.findItem(id);

        auto *p = control && *control ? (*control)->colors.findItem(i) : NullPtr;

        return p ? *p : m_defaultColors;
    }

    const Point &getSize( const UUID &id ,index_t i ) {
        static Point dummy;

        auto **control = m_controls.findItem(id);

        auto *p = control ? (*control)->sizes.findItem(i) : NullPtr;

        return p ? *p : dummy;
    }

    const Params &getProperties( const UUID &id ,index_t i ) {
        static Params dummy;

        auto **control = m_controls.findItem(id);

        auto *p = control ? (*control)->properties.findItem(i) : NullPtr;

        return p ? *p : dummy;
    }

    //--
    const ColorQuad &getColors( const UUID &id ,const char *name ) {
        static ColorQuad dummy;

        auto *it = m_names.findItem(name);

        return it ? getColors( id ,*it ) : dummy;
    }

    const Point &getSize( const UUID &id ,const char *name  ) {
        static Point dummy;

        auto *it = m_names.findItem(name);

        return it ? getSize( id ,*it ) : dummy;
    }

    const Params &getProperties( const UUID &id ,const char *name ) {
        static Params dummy;

        auto *it = m_names.findItem(name);

        return it ? getProperties( id ,*it ) : dummy;
    }

    //! using named color from theme
    void makeColorSet( const UUID &id ,ColorSet &colorSet );
};

//////////////////////////////////////////////////////////////////////////////
//! Themes

class ThemeStore : public Store_<String,VisualTheme*> ,public Singleton_<ThemeStore> {
public:
    ThemeStore();

public:
    VisualTheme &getDefault() { return m_default; }
    VisualTheme &getCurrent() { return *m_current; }

    void selectCurrent( const char *name ) {
        auto *it = getTheme(name);

        if( it ) m_current = it;
    }

    VisualTheme *getTheme( const char *name ) {
        auto **it = m_themes.findItem(name);

        return it ? *it : NullPtr;
    }

public:
    bool addThemeFromManifest( const Params &params );
    bool addThemeFromManifest( const char *properties );

protected:
    Map_<String,VisualTheme*> m_themes;
        //! @note themes belong to the ThemeStore which manage their lifetime

    VisualTheme m_default;
    VisualTheme *m_current;
};

inline ThemeStore &theThemeStore() { return ThemeStore::getInstance(); }
inline VisualTheme &theTheme() { return theThemeStore().getCurrent(); }

//////////////////////////////////////////////////////////////////////////////
} //TINY_GUI_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
//! String

    //...

//////////////////////////////////////////////////////////////////////////////
//! Manifest

CLASS_MANIFEST(TINY_NAMESPACE_GUI::VisualTheme::Controls,Params);
CLASS_MANIFEST(TINY_NAMESPACE_GUI::VisualTheme,Params);

//////////////////////////////////////////////////////////////////////////////
//! Binary

    //...

//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
#endif //TINY_GUI_THEME_H