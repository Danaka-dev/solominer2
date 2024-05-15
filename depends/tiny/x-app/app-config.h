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

#ifndef TINY_XAPP_CONFIG_H
#define TINY_XAPP_CONFIG_H

//////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
class Config {
public: ///-- definitions
    struct Section {
        int index; //! index in m_index
        String name; //! name of this section
        Params params; //! params under this section

        //! @note could track change with hash
        //! @note DEV might be limitative to force section to only hold unique maps ..
        //! @note cannot transact per params, only entire section //TODO make it possible to edit only one entry ?
    };

public: ///-- instance
    Config() : def(&cmake_like_comment) ,m_saveOnExit(false) {}

    Config( const char *filename ,bool saveOnExit=false ) :
        def(&cmake_like_comment) ,m_saveOnExit(saveOnExit)
    {
        LoadFile( filename );
    }

    ~Config() {
        if( m_saveOnExit ) SaveFile();

        CloseFile();
    }

    File &file() { return m_file; }

public: ///-- file
    bool LoadFile( const char *filename ,bool dontLock=false ,bool dontCreate=false );
    bool SaveFile();
    void CloseFile();

public: ///-- section
    Section *peekSection( const char *name );
    Section &getSection( const char *name );

    bool readSection( Section &section ); //! load /reload section content
    bool commitSection( Section &section ); //! save section, doesn't write file

protected: ///-- content
    void IndexContent();
    void ParseContent();
    void UpdateContent();

protected: ///-- members
    struct Segment {
        String name;
        int start ,end;

        int size() const { return end - start; }
    };

    const CommentDef *def;

    File m_file;
    String m_content; //! file content as from last load
    bool m_saveOnExit;

    MapOf<String,Section> m_sections;

    ListOf<Segment> m_index; //! section index (from after [name] to before [next_name]
};

//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
#endif //TINY_XAPP_CONFIG_H