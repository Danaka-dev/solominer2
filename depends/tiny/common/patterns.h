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

#ifndef TINY_PATTERNS_H
#define TINY_PATTERNS_H

//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Patterns

//////////////////////////////////////////////////////////////////////////////
//! Singleton

template<class T>
class Singleton_ {
public:
    static T &getInstance() {
        static T g_instance;
        return g_instance;
    }
};

//////////////////////////////////////////////////////////////////////////////
//! Enums

template <typename T>
struct Enum_ {

///-- static interface
    static const char *names[];
    static const T values[]; //! @note values needs to be set only if they are irregular

    static size_t getCount() {
        static size_t count = []() -> size_t { size_t n=0; while( names[n] ) ++n; return n; } ();

        assert(count>0);

        return count;
    }

    static T getMaxValue() {
        static size_t n = getCount(); return (T) (n ? n-1 : n);
    }

    static T getValue( const char *s ) {
        T v = (T) 0; findValue(s,v); return v;
    }

    static size_t isValid( T key ) {
        return (size_t) key < getCount();
    }

    static size_t Index( T key ) {
        return CLAMP( key ,0 ,getMaxValue() );
    }

//-- @note following function assuming there is only a single match possible (use from/to StringList for combination)
    static const char *findName( T v ) {
        int n = getCount();

        for( int i=0; i<n; ++i ) {
            if( values[i] == v ) return names[i];
        }

        return "";
    }

    static bool findValue( const char *s ,T &v ,size_t &size ) {
        if( s && *s ) {} else return false;

        int i=0; const char *p; while( (p = names[i]) != NullPtr ) {
            if( *p && iMatch( s ,p ) ) {
                size = strlen( p );

                v = values[i]; return true;
            }

            ++i;
        }

        return false;
    }

    static bool findValue( const char *s ,T &v ) {
        size_t size; return findValue( s ,v ,size );
    }
};

//--
template <typename T>
T getByName( const char *s ,T &keyInList ) {
    keyInList = Enum_<T>::getValue( s ); return keyInList;
}

template <typename T>
T getByName( const char *s ) {
    return Enum_<T>::getValue( s );
}

template <typename T>
ListOf<String> &listEnum( ListOf<String> &list ) {
    size_t n = Enum_<T>::getCount();

    for( int i=0; i<(int) n; ++i )
        list.emplace_back( Enum_<T>::names[i] );

    return list;
}

//--
template <typename T>
T &enumFromString( T &v ,const String &s ,size_t &size ) {
    Enum_<T>::findValue( tocstr(s) ,v ,size );

    return v;
}

template <typename T>
T &enumFromString( T &v ,const String &s ) {
    size_t size; return enumFromString( v ,s ,size );
}

template <typename T>
String &enumToString( const T &v ,String &s ) {
    s = Enum_<T>::findName(v);

    return s;
}

//-- Params
template <typename T>
T &enumFromMember( T &v ,const Params &p ,const char *key ) {
    const String *s = peekMember( p ,key );

    if( s ) enumFromString( v ,*s );

    return v;
}

template <typename T>
Params &enumToMember( const T &v ,Params &p ,const char *key ) {
    String s; enumToString( v ,s );

    if( key && *key && !s.empty() )
        p[key] = tocstr(s);

    return p;
}

//-- StringList
template <typename T>
T &enumFromStringList( T &v ,const StringList &list ) {
    if( list.empty() ) return v;

    T ev; v = (T) 0;

    for( const auto &it : list ) {
        enumFromString( ev ,it );

        v = (T) (v | ev );
    }

    return v;
}

template <typename T>
StringList &enumToStringList( const T &v ,StringList &list ) {
    int n = (int) Enum_<T>::getCount();

    T vi = v;

    for( int i=n-1; i>=0; --i ) if( vi ) {
        if( ((Enum_<T>::values[i] & vi) == vi) ) {
            list.emplace_back( Enum_<T>::names[i] );
            vi = (T) (vi - i);
        }
    }

    if( list.empty() )
        list.emplace_back( Enum_<T>::names[0] );

    return list;
}

//-- StringList given as String
template <typename T>
T &enumFromStringList( T &v ,const String &s ,size_t &size ) {
    StringList list;
    String str = s;

    replaceChar( str ,'+' ,',' );

    fromString( list ,str ,size );

    return enumFromStringList( v ,list );
}

template <typename T>
T &enumFromStringList( T &v ,const String &s ) {
    size_t size; return enumFromStringList( v ,s ,size );
}

template <typename T>
String &enumToStringList( const T &v ,String &s ) {
    StringList list;

    enumToStringList( v ,list );

    return toString( list ,s );
}

//////////////////////////////////////////////////////////////////////////////
//! List

template<typename T>
class List_ {
public:
    typedef ListOf<T> list_t;

public:
    size_t getCount() const { return m_list.size(); }

    const list_t &getList() const { return m_list; }

    list_t &list() { return m_list; }

public:
    bool getItem( int i ,T &p ) {
        if( i < 0 || i >= getCount() ) return false;

        p = list().at(i); return true;
    }

    T &getItem( int i ) {
        assert( i >= 0 && i < getCount() );
        return list().at(i);
    }

    template <typename F>
    bool findItemWith( F &&filter ,T &p ) {
        for( auto &it : list() ) {
            if( filter( it ) ) {
                p = it; return true;
            }
        }

        return false;
    }

public:
    bool addItem( T &&p ) {
        list().emplace_back( p ); return true;
    }

    void removeItem( int i ) {
        list().erase(i);
    }

    void Clear() {
        list().clear();
    }

protected:
    list_t m_list;
};

//////////////////////////////////////////////////////////////////////////////
//! Map

template<typename TKey ,typename TValue>
class TMap_ {
public:
    typedef ListOf<TKey> index_t;
    typedef ListOf<TValue> list_t;
    typedef MapOf<TKey,TValue> map_t;

public:
    TMap_( map_t &map ) : m_tmap(map) {}

    const map_t &map() const { return m_tmap; };
    map_t &map() { return m_tmap; };

    //--
    template <typename T>
    TValue &operator []( T i ) { return map()[i]; }

public:
    size_t getCount() const {
        return map().size();
    }

    void listIndex( ListOf<TKey> &list ) const {
        for( const auto &it : map() ) {
            list.emplace_back( it.first );
        }
    }

    void listItems( ListOf<TValue> &list ) const {
        for( const auto &it : map() ) {
            list.emplace_back( it.second );
        }
    }

//-- get
    //! @note use if certain requested item is in the collection, or to rely on exception mechanism

    TValue &getItem( const TKey &index ) {
        auto it = map().find( index );

        if( it == map().end() )
            throw Exception( INODATA ,"Index not found in map" );

        return it->second;
    }

//-- find & dig
    //! @note use when unsure if request is in the collection, or to use return value based checking

    bool findItem( const TKey &index ,TValue &p ) NoExcept {
        auto it = map().find( index );

        if( it == map().end() ) return false;

        p = it->second; return true;
    }

    TValue *findItem( const TKey &index ) NoExcept {
        auto it = map().find( index );

        if( it == map().end() ) return NullPtr;

        return & it->second;
    }

    const TKey *digItem( const TValue &item ) {
        for( auto &it : map() ) {
            if( it.second == item ) return &(it.first);
        }

        return NullPtr;
    }

//-- filter
    template <typename F>
    void eachItem( F &&filter ) {
        for( auto &it : map() ) {
            if( !filter( it.first ,it.second ) ) return;
        }
    }

    template <typename F>
    TValue *findWith( F &&filter ) {
        for( auto &it : map() ) {
            if( filter( it.second ) ) {
                return & it.second;
            }
        }

        return NullPtr;
    }

    template <typename F>
    bool findWith( TValue &p ,F &&filter ) {
        TValue *find = findWith( filter );

        if( !find ) return false;

        p = *find; return true;
    }

    template <typename F>
    void lisIndexWith( ListOf<TKey> &list ,F &&filter ) {
        for( auto &it : map() ) {
            if( filter( it.second ) ) {
                list.emplace_back( it.first );
            }
        }
    }

    template <typename F>
    void listItemsWith( ListOf<TValue> &list ,F &&filter ) {
        for( auto &it : map() ) {
            if( filter( it.second ) ) {
                list.emplace_back( it.second );
            }
        }
    }

public:
    bool addItem( const TKey &index ,const TValue &p ) {
        map()[index] = p; return true;
    }

    void delItem( const TKey &index ) {
        map().erase( index );
    }

    void Clear() {
        map().clear();
    }

private:
    MapOf<TKey,TValue> &m_tmap;
};

//--
template<typename TKey ,typename TValue>
class Map_ : public TMap_<TKey,TValue> {
public:
    typedef ListOf<TKey> index_t;
    typedef ListOf<TValue> list_t;
    typedef MapOf<TKey,TValue> map_t;

public:
    Map_() : TMap_<TKey,TValue>(m_map) {}

protected:
    map_t m_map;
};

//////////////////////////////////////////////////////////////////////////////
//! Chain_

    //! @brief linked list

//TODO

//////////////////////////////////////////////////////////////////////////////
//! Tape_

    //! @brief double linked list
    //! @note T class/struct needs to define 'next()' ,'prev()' and 'index()' member and be nullable (0)
    //! @note T needs to define static TIndex &getItem( TIndex i )

/* template <class T ,typename TIndex>
class Tape_ {
    struct Link {
        TIndex m_prev ,m_next;
    };

public:
    void Insert( T item ) { //! first
        if( first ) first.prev() = item.index();

        item.next() = first;
        first = item.index();
    }

    void InsertBefore( T at ,T item ) {
        item.next() = at.index();
        item.prev() = at.prev();

        T::getItem( item.prev() ).next() = item.index();
        item.next().prev() = item.index();

        if( first && first == at.index() ) first = item.index();
    }

    void InsetAfter( T at ,T item ) {
        item.prev() = at.index();
        item.next() = at.next();

        item.prev().next() = item.index();
        item.next().prev() = item.index();

        if( last && last == at.index() ) first = item.index();
    }

    void Append( T item ) {

    }

    void Remove( T item ) {

    }

    void Clear() {

    }

    template <typename F>
    void ForEach( F &&lambda ) {

    }

protected:
    void setIndex( TIndex &index ,const TIndex i ) {
        if( &index != (TIndex) 0 ) index = i;
    }

    TIndex first ,last;
}; */

//////////////////////////////////////////////////////////////////////////////
//! Cache

template<typename TKey ,typename TValue>
class Cache_ : public Map_<TKey,TValue> {
public:
    //TODO handle cache hit and limit quantity of index in the map
};

//////////////////////////////////////////////////////////////////////////////
//! Store

    //TODO this is a Map_, but as a singleton per type object

template<typename TIndex ,typename TObject>
class Store_ { //! STORE ALWAYS HAVE A SINGLETON PER OBEJCT => defined as global registry of object ?
public:
    typedef ListOf<TIndex> index_t;
    typedef ListOf<TObject> list_t;
    typedef MapOf<TIndex,TObject> map_t;
        //TODO derive from Map_

public:
    size_t getCount() const { return m_map.size(); }

    const map_t &getMap() const { return m_map; }

    map_t &map() { return m_map; }

public:
    void getIndex( ListOf<TIndex> &index ) {
        for( auto &it : m_map ) {
            index.emplace_back( it.second );
        }
    }

    index_t getIndex() {
        index_t index; getIndex( index ); return index;
    }

    void getList( ListOf<TObject> &list ) {
        for( auto &it : m_map ) {
            list.emplace_back( it.second );
        }
    }

    list_t getList() {
        list_t list; getList( list ); return list;
    }

    template <typename F>
    void getListWith( ListOf<TObject> &list ,F &&filter ) {
        for( auto &it : m_map ) {
            if( filter( it.second ) ) {
                list.emplace_back( it.second );
            }
        }
    }

    TObject &getItem( const TIndex &index ) {
        TObject *p = findItem(index);

        if( !p ) throw IException( this ,IBADARGS ,"Bad index in call to Store::getItem" );

        return *p;
    }

    bool digItem( const TObject &object ,TIndex &i ) {
        for( auto &it : m_map ) {
            if( it.second == object ) {
                i = it.first; return true;
            }
        }

        return false;
    }

    bool findItem( const TIndex &index ,TObject &p ) {
        auto it = m_map.find( index );

        if( it == m_map.end() ) return false;

        p = it->second; return true;
    }

    TObject *findItem( const TIndex &index ) {
        auto it = m_map.find( index );

        return ( it != m_map.end() ) ? &(it->second) : NullPtr;
    }

    template <typename F>
    bool findItemWith( F &&filter ,TObject &p ) {
        for( auto &it : m_map ) {
            if( filter( it.second ) ) {
                p = it.second; return true;
            }
        }

        return false;
    }

public:
    bool RegisterItem( const TIndex &index ,TObject &&p ) {
        m_map[index] = p; return true;
    }

    bool RegisterItem( const TIndex &index ,TObject &p ) {
        m_map[index] = p; return true;
    }

    void RevokeItem( const TIndex &index ) {
        m_map.erase( index );
    }

    void Clear() {
        m_map.clear();
    }

protected:
    map_t m_map;
};

//////////////////////////////////////////////////////////////////////////////
//! Registry

/* template <class T>
class Registry_ : public Store_<PUID,RefOf<T> > { //! NB used in GuiGroup

protected:
    bool RegisterItem( PUID *id ,const char *name ,const RefOf<T> &p ) {
        if( id && *id ) m_ids[*id] = p;
        if( name && *name ) m_named[*id] = p;
    }

    void RevokeItem( const PUID TA &index ) {
        m_map.erase( index );
    }

protected:
    // MapOf<PUID,RefOf<T> > m_ids;
    MapOf<PUID,String> m_ids;
    MapOf<String,PUID> m_named;
}; */

//////////////////////////////////////////////////////////////////////////////
//! Factory

template <class T>
class Factory_ : public Singleton_< Factory_<T> >{
public:
    using Constructor = T *(*)( PIID );

    void listClasses( ListOf<PUID> &classes ) {
        for( const auto &it : m_constructors ) {
            classes.emplace_back( it.first );
        }
    }

    virtual bool RegisterClass( const PUID &id ,Constructor factor ) {
        m_constructors[id] = factor; return true;
    }

    virtual T *Create( const PUID &id ,PIID instanceId=PIID_NOINSTANCE ) {
        const auto &it = m_constructors.find(id);

        return (it != m_constructors.end() ) ? it->second(instanceId) : NullPtr;
    }

protected:
    MapOf<PUID,Constructor> m_constructors;
};

//--
template <class T>
bool RegisterClass_( const PUID &id ,typename Factory_<T>::Constructor constructor ) {
    return Factory_<T>::getInstance().RegisterClass( id ,constructor );
}

template <class T>
T *CreateClass_( const PUID &id ,PIID instanceId=PIID_NOINSTANCE ) {
    return Factory_<T>::getInstance().Create( id ,instanceId );
}

///-- IObject

/* struct CBase {
    DECLARE_CLASSID(1010);
};

struct CClass : CBase ,COBJECT_PARENT {

    DECLARE_CLASSID(1012);
    DECLARE_CLASSNAME("CClass");

    // IMPORT_FACTORY_API;
    DECLARE_FACTORY_API(CBase) {
        IObject::classNames[ getClassName() ] = getClassId();

        return
            RegisterClass_<IObject>( getClassId() ,Create<IObject> )
            && RegisterClass_<CBase>( getClassId() ,Create<CBase> )
        ;
    }
}; */

//? keep here ? where ?

template <class T>
T *ICreateObject_( const char *name ) {
    PUID id;

    if( !findClassIdByName( name ,id ) ) return NullPtr;

    return Factory_<T>::getInstance().Create( id );
}

//////////////////////////////////////////////////////////////////////////////
//! Publisher/Subscriber

template<class TEvents>
struct IPublisher_ { // : IOBJECT_PARENT
    IAPI_DECL Subscribe( TEvents &listener ) = 0;
    IAPI_DECL Revoke( TEvents &listener ) = 0;
};

///--
template<class TEvents>
class CPublisher_ : public virtual IPublisher_<TEvents> {
public:
    typedef ListOf< RefOf<TEvents> > subscribers_t;

protected:
    subscribers_t m_subscribers;

    void RevokeAll() {
        for( auto it = m_subscribers.begin(); it != m_subscribers.end(); ) {
            m_subscribers.erase( it );
        }
    }

public:
    ~CPublisher_() {
        RevokeAll();
    }

    const subscribers_t &subscribers() const { return m_subscribers; }

    subscribers_t &subscribers() { return m_subscribers; }

public: ///-- IEventSource interface

    IAPI_IMPL Subscribe( TEvents &listener ) IOVERRIDE {
        for( auto &it : m_subscribers ) {
            if( it == listener ) return IOK; //! already subscribed
        }

        m_subscribers.emplace_back( listener );

        return IOK;
    }

    IAPI_IMPL Revoke( TEvents &listener ) IOVERRIDE {
        for( auto it = m_subscribers.begin(); it != m_subscribers.end(); ) {
            if( *it == listener ) {
                m_subscribers.erase( it );
            } else {
                ++it;
            }
        }

        return IOK;
    }

    /*
public: ///-- TEvents interface
    //! @note TEvents interface to be implemented for event dispatching by final Source
    //...
     */
};

//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
#endif //TINY_PATTERNS_H