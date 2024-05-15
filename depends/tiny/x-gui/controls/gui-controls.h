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

#ifndef TINY_GUI_CONTROLS_H
#define TINY_GUI_CONTROLS_H

//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

    using namespace gui; //TODO temp

//////////////////////////////////////////////////////////////////////////////
//! components

#define TINY_GUIMARGIN_UUID       0x07b216a8921ce93d3
#define TINY_GUILABEL_UUID        0x0b7c677e34e3740ef
#define TINY_GUILINK_UUID         0x0e96c0d38a60f2771
#define TINY_GUIBUTTON_UUID       0x04be1712b20e346a9
#define TINY_GUICHECK_UUID        0x0d4b7db7290465d99
#define TINY_GUICHECKBOX_UUID     0x0f193ede975a3ce07
#define TINY_GUILIST_UUID         0x0b8c46f78f21b2922
#define TINY_GUIIMAGEBOX_UUID     0x062cc65c4ed9d1117
#define TINY_GUITHUMBNAIL_UUID    0x062665840a2be5276
#define TINY_GUITHUMBWALL_UUID    0x03b06f796cbd706d7

class GuiMargin;
class GuiLabel;
class GuiLink;
class GuiButton;
class GuiCheckBox;
class GuiList;
class GuiThumbnail;
class GuiThumbwall;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! decorative

//////////////////////////////////////////////////////////////////////////////
//! GuiWithShortcuts

class GuiWithShortcut : GUICONTROL_PARENT {
public:
    void onKey( OsKeyAction keyAction ,OsKeyState keyState ,OsKeyCode keyCode ,char_t c ) override;

protected:
    ListOf<GuiShortcut> m_shortcuts;
};

//////////////////////////////////////////////////////////////////////////////
//! GuiWithFont

class GuiWithFont : GUICONTROL_PARENT {
public:
    GuiWithFont( GuiFont *font=NullPtr );

    DECLARE_GUIPROPERTIES;

    const GuiFont *getFont() const { return m_font.ptr(); }

    void setFont( GuiFont &font ) { m_font = font; }
    void setFont( GuiFont *font=NullPtr ) { m_font = font; }

    bool hasFont() const { return !m_font.isNull(); }

public:
    void onDraw( const OsRect &updateArea ) override;

protected:
    GuiFontRef m_font;
};

//////////////////////////////////////////////////////////////////////////////
//! GuiWithText

class GuiWithText : public GuiWithFont {
public:
    GuiWithText( const char *text="" ,TextAlign textAlign=textalignNormal ,GuiFont *font=NullPtr ) :
        GuiWithFont(font) ,m_text(text?text:"") ,m_textAlign(textAlign)
    {}

    DECLARE_GUIPROPERTIES;

    const String &text() const { return m_text; }
    String &text() { return m_text; }

    const TextAlign &textAlign() const { return m_textAlign; }
    TextAlign &textAlign() { return m_textAlign; }

public:
    const char *getText() const { return m_text.c_str(); }
    void setText( const char *text ) { m_text = text; }

    bool hasText() const { return !m_text.empty(); }

protected:
    void Draw( const OsRect &r ); //! @note not onDraw, we do not know how the text will be placed here

protected:
    String m_text;
    TextAlign m_textAlign;
};

//////////////////////////////////////////////////////////////////////////////
//! GuiWithImage

    //TODO dito as GuiWithXXX ...

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! controls

//////////////////////////////////////////////////////////////////////////////
//! GuiMargin

class GuiMargin : GUICONTROL_PARENT {
public:
    GuiMargin( int borderSize=0 )
    {}

    DECLARE_GUICONTROL(GuiControl,GuiMargin,TINY_GUIMARGIN_UUID);
    DECLARE_GUIPROPERTIES;

    Rect &margin() { return m_margin; }

public:
    API_IMPL(void) onLayout( const OsRect &clientArea ,OsRect &placeArea ) IOVERRIDE {
        GuiControl::onLayout( clientArea ,placeArea );

        placeArea = Rect( placeArea ).Deflate( m_margin );
    }

protected:
    Rect m_margin;
};

//////////////////////////////////////////////////////////////////////////////
//! GuiLabel

    //TODO use GuiWithText instead of with font
class GuiLabel : public GuiWithFont ,GUICONTROL_PARENT {
public:
    GuiLabel( const char *text="label" ,TextAlign textAlign=textalignNormal ,GuiFont *font=NullPtr ) :
        GuiWithFont(font) ,m_text(text?text:"") ,m_textAlign(textAlign)
    {}

    DECLARE_GUICONTROL(GuiControl,GuiLabel,TINY_GUILABEL_UUID);
    DECLARE_GUIPROPERTIES;

    const TextAlign &textAlign() const { return m_textAlign; }
    TextAlign &textAlign() { return m_textAlign; }

    const String &text() const { return m_text; }
    String &text() { return m_text; }

public:
    void setText( const char *text ) { m_text = text; }
    const char *getText() const { return m_text.c_str(); }

public:
    API_IMPL(void) onDraw( const OsRect &updateArea ) IOVERRIDE;

protected:
    TextAlign m_textAlign;

    String m_text;
};

//////////////////////////////////////////////////////////////////////////////
//! GuiLink

class GuiLink : public GuiCommandOnClick ,public GuiLabel {
public:
    GuiLink( const char *text="link" ,IGuiCommandEvent *listener=NullPtr ) :
        GuiLabel(text) ,m_textHooverColor(OS_COLOR_BLUE)
    {
        if( listener ) GuiCommandPublisher::Subscribe(*listener);

        m_textNormalColor = colors().textColor;

        ColorQuad hoover = theTheme().getColors( MyUUID ,"hoover" );

        m_textHooverColor = hoover.textColor;
    }

    DECLARE_GUICONTROL(GuiControl,GuiLink,TINY_GUILINK_UUID);
    DECLARE_GUIPROPERTIES;

    ColorRef &normalColor() { return m_textNormalColor; }
    ColorRef &hooverColor() { return m_textHooverColor; }

public: ///-- control interface
    void onMouseEnter( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) override {
        colors().textColor = m_textHooverColor;
        root().Refresh();
    }

    void onMouseLeave( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) override {
        colors().textColor = m_textNormalColor;
        root().Refresh();
    }

protected:
    ColorRef m_textNormalColor;
    ColorRef m_textHooverColor;
};

//////////////////////////////////////////////////////////////////////////////
//! GuiButton

class GuiButton : public GuiCommandOnClick ,GUICONTROL_PARENT {
public:
    GuiButton( const char *label="button" ,IGuiCommandEvent *listener=NullPtr ) :
        m_label( label ,textalignCenter )
    {
        if( listener ) GuiCommandPublisher::Subscribe(*listener);

        theTheme().makeColorSet( MyUUID ,m_colorset );

        setStateAndColor( highlightNormal );
    }

    DECLARE_GUICONTROL(GuiControl,GuiButton,TINY_GUIBUTTON_UUID);
    DECLARE_GUIPROPERTIES;

    const String &text() const { return m_label.text(); }
    String &text() { return m_label.text(); }

    ColorSet &colorSet() { return m_colorset; }

    //TODO this causes issue when set enabled from GuiControl interface, find a better way to synch enabled & state
    void setState( Highlight state ) {
        setStateAndColor( state );

        enabled() = (state != highlightDisabled);
    }

protected: ///-- control interface
    API_IMPL(void) onMouseEnter( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) IOVERRIDE {
        setStateAndColor( highlightHoover ); root().Refresh();
    }

    API_IMPL(void) onMouseLeave( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) IOVERRIDE {
        setStateAndColor( highlightNormal ); root().Refresh();
    }

    API_IMPL(void) onMouseDown( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) IOVERRIDE {
        setStateAndColor( highlightPushed ); root().Refresh();
    }

    API_IMPL(void) onMouseUp( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) IOVERRIDE {
        setStateAndColor( highlightNormal ); root().Refresh();
    }

protected:
    API_IMPL(void) onLayout( const OsRect &clientArea ,OsRect &placeArea ) IOVERRIDE;
    API_IMPL(void) onDraw( const OsRect &updateArea ) IOVERRIDE;

protected:
    void setStateAndColor( Highlight state ) {
        ColorQuad color = m_colorset.getColors( m_state = state );

        colors() = color;
        m_label.colors() = color;
    }

    GuiLabel m_label;
    ColorSet m_colorset;
    Highlight m_state;
};

//////////////////////////////////////////////////////////////////////////////
//! GuiCheck

//TODO
// void OsGuiDrawTick( IGuiDisplay &display ,const Rect &rect );

//--
class GuiCheck : GUICONTROL_PARENT {
public:
    enum Style {
        styleTick=0 ,styleCross ,styleRound
    };

public:
    GuiCheck( Style style=styleTick ,bool checked=false ) :
        m_style(style) ,m_checked(checked)
    {
        m_outterColor = theTheme().getColors( MyUUID ,"outter" );
        m_size = 6; //TODO get from theme

        setStyle(style);
    }

    DECLARE_GUICONTROL(GuiControl,GuiCheck,TINY_GUICHECK_UUID);
    // DECLARE_GUIPROPERTIES;

    Style &setStyle( Style style ) {
        switch( style ) {
            case styleTick: m_checkColor = theTheme().getColors( MyUUID ,"tick" ); break;
            case styleCross: m_checkColor = theTheme().getColors( MyUUID ,"cross" ); break;
            case styleRound: m_checkColor = theTheme().getColors( MyUUID ,"round" ); break;
        }

        Refresh();

        return m_style = style;
    }

    Style &style() { return m_style; }
    bool &checked() { return m_checked; }

public:
    API_IMPL(void) onClick( const OsPoint &p ,OsMouseButton mouseButton ,OsKeyState keyState ) IOVERRIDE;

    API_IMPL(void) onDraw( const OsRect &updateArea ) IOVERRIDE;

protected:
    void drawTick( const Rect &r );
    void drawCross( const Rect &r );
    void drawRound( const Rect &r );

protected:
    int m_size; //! check size

    Style m_style;
    bool m_checked;

    ColorQuad m_outterColor;
    ColorQuad m_checkColor;
};

//////////////////////////////////////////////////////////////////////////////
//! GuiCheckBox

class GuiCheckBox : public GuiCommandOnClick ,public GuiArray_<2> {
public:
    GuiCheckBox( const char *label="label" ,GuiCheck::Style style=GuiCheck::styleTick ,bool checked=false ) :
        m_label(label) ,m_check(style,checked)
    {
        m_label.coords() = { 0 ,0 ,90.f ,100.f };
        m_label.align() = GuiAlign::alignRight;
        m_label.textAlign() = (TextAlign) (textalignLeft | textalignCenterV);
        m_controls[0] = &m_label;

        m_check.coords() = { 0 ,0 ,10.f ,100.f };
        m_check.align() = GuiAlign::alignLeft;
        m_controls[1] = &m_check;
    }

    DECLARE_GUICONTROL(GuiControl,GuiCheckBox,TINY_GUICHECKBOX_UUID);
    // DECLARE_GUIPROPERTIES;

    GuiCheck &check() { return m_check; }
    GuiLabel &label() { return m_label; }

    GuiCheck::Style &style() { return m_check.style(); }
    bool &checked() { return m_check.checked(); }
    String &text() { return m_label.text(); }

public:
    API_IMPL(bool) makeCommandParam( params_t &params ) IOVERRIDE {
        params.param = (long) m_check.checked();
        return true;
    }

protected:
    GuiCheck m_check;
    GuiLabel m_label;
};

//////////////////////////////////////////////////////////////////////////////
//! GuiList

    //TODO select, multi-select ...

class GuiList : public GuiGroup ,GUICONTROL_PARENT {
public:
    enum Placement {
        placeLine ,placeZigZag ,placeGrid ,placeDiamond ,placeSpiral
    };

public:
    GuiList() DEFAULT

    DECLARE_GUICONTROL(GuiGroup,GuiList,TINY_GUILIST_UUID);
    // DECLARE_GUIPROPERTIES;

    Placement &placement() { return m_placer.placement; }
    Direction &direction() { return m_placer.direction; }
    RectPoint &origin() { return m_placer.origin; }

    Point &itemSize() { return m_placer.size; }

public:
    API_IMPL(void) onLayout( const OsRect &clientArea ,OsRect &placeArea ) IOVERRIDE {
        GuiGroup::onLayout( clientArea ,placeArea );

        int n = controls().size();

        int i=0; for( auto &it : controls() ) {
            Rect area ,r;

            m_placer.Emplace( i ,n ,clientArea ,area );

            it->onLayout( area ,r );

            ++i;
        }
    }

protected:
    struct Placer {
        Placement placement;
        Direction direction;
        RectPoint origin;
        Point size;

        void Emplace( int i ,int n ,const Rect &client ,Rect &area );

        void EmplaceLine( int i ,int n ,const Rect &client ,Rect &area );
    };

    Placer m_placer;
};

//////////////////////////////////////////////////////////////////////////////
//! GuiImageBox

    //TODO stretch / center / ...

class GuiImageBox : public GuiCommandOnClick ,public GuiWithText {
public:
    GuiImageBox( GuiImage *image=NullPtr ,const char *text="" ,IGuiCommandEvent *listener=NullPtr );

    DECLARE_GUICONTROL(GuiWithText,GuiImageBox,TINY_GUIIMAGEBOX_UUID);
    DECLARE_GUIPROPERTIES;

    const GuiImage *getImage() const { return m_image.ptr(); }
    GuiImage *getImage() { return m_image.ptr(); }

    void setImage( GuiImage &image ) {
        size() = Point( image.GetWidth() ,image.GetHeight() );
        m_thumbnail = Rect( Point() ,size() );
        m_image = &image;
    }

    void setThumbnail( int thumbId ) {
        if( m_image.isNull() ) return;

        m_thumbnail = m_image->getThumbRect( m_thumbId = thumbId );
        size() = m_thumbnail.getSize();
    }

    Rect &thumbnail() { return m_thumbnail; }

    GuiAlign &textPlacement() { return m_textPlacement; }

    //! @note image is centered in the image box
    Rect &getImageArea( Rect &r ) const;
    Rect &getTextArea( Rect &r ) const;

public:
    API_IMPL(void) onLayout( const OsRect &clientArea ,OsRect &placeArea ) IOVERRIDE;
    API_IMPL(void) onDraw( const OsRect &updateArea ) IOVERRIDE;

protected:
    GuiImageRef m_image;
    Rect m_thumbnail; //! thumbnail in the source image
    int m_thumbId; //! thumbnail in the image

    GuiAlign m_textPlacement;
    Rect m_textArea;
};

//////////////////////////////////////////////////////////////////////////////
class GuiThumbnail : public GuiCommandOnClick ,GUICONTROL_PARENT {
protected:
    GuiImageBox m_image; //TODO use GuiArray_
    GuiLabel m_label;

public:
    GuiThumbnail( GuiImage *image=NullPtr ,const char *label="" ,GuiFont *font=NullPtr ,IGuiCommandEvent *listener=NullPtr ) :
        m_image( image ,label )
        ,m_label( label ,(TextAlign) (textalignCenterH | textalignCenterV) ,font )
    {
        if( listener ) GuiCommandPublisher::Subscribe(*listener);
    }

    DECLARE_GUICONTROL(GuiControl,GuiThumbnail,TINY_GUITHUMBNAIL_UUID);
    // DECLARE_GUIPROPERTIES; //TODO

    GuiImageBox &image() { return m_image; }
    GuiLabel &label() { return m_label; }

public:
    void onLayout( const OsRect &clientArea ,OsRect &placeArea ) override;
    void onDraw( const OsRect &updateArea ) override;
};

//////////////////////////////////////////////////////////////////////////

    //? TODO move function of 'wall' to generic list

class GuiThumbwall : public GuiCommandPublisher ,public IGuiCommandEvent ,public GuiWithFont ,public GuiGroup  {
protected:
    Point m_thumbSize;

    //TODO selection border ... multi select

public:
    GuiThumbwall( const OsPoint &thumbSize ,GuiFont *font=NullPtr) :
        GuiWithFont(NullPtr) ,m_thumbSize(thumbSize)
    {}

    IMPORT_IOBJECT_API(TINY_GUITHUMBWALL_UUID);

    DECLARE_GUIPROPERTIES;

    void setThumbSize( const OsPoint &p ) { m_thumbSize = p; }

    int getThumbCount() const { return getControlCount(); }
    int addThumb( GuiImage *image=NullPtr ,const char *label="" );
    void removeThumb( int index );

protected: ///-- events
    virtual void onItemSelected( GuiControl &item ,int index ,bool selected ) {}

protected: ///-- override
    void onLayout( const OsRect &clientArea ,OsRect &placeArea ) override;
    void onDraw( const OsRect &updateArea ) override;

    void onCommand( GuiControl &source ,uint32_t commandId ,long param ,Params *params ,void *extra ) override;
};

//////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////
#endif //TINY_GUI_CONTROLS_H