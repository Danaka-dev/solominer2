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

#ifndef TINY_OBJECT_H
#define TINY_OBJECT_H

///////////////////////////////////////////////////////////////////////////////
//! tiny implementation of fundamental interface

    //! @note this header can be detached from tiny-for-c++ to be used on tis own
    //! (requires definitions from 'tiny-defs.h')

///////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//! Reference

    //!IE reference counted pointer (@note it is the object that enables reference
    //!     counting, so ref != std::share_ptr

typedef ASYNCH_TYPE ref_t;

///////////////////////////////////////////////////////////////////////////////
#define RefOf ref_

template <class T>
class ref_ {
protected:
    T *m_ptr;

public:
    ref_() : m_ptr(NullPtr) {}

    //! Initializing with a new ptr (use along with new, taking ownership of the initial reference, no AddRef)
    explicit ref_( T *p ) : m_ptr(p) {}

    //! Initializing with an existing object (adding a reference)
    ref_( T &p ) : m_ptr(&p) { AddRef(); }

    //! copy constructor (adding a reference)
    ref_( const ref_<T> &ref ) : m_ptr(ref.m_ptr) { AddRef(); }

    //! move constructor (adding a reference, ... )
        //! @note while it doesn't seems logical to AddRef on a move, destructor will be called off the donating object
        //! which will Release() and remove a reference... so AddRef is needed here
    ref_( ref_<T> &&ref ) NoExcept : m_ptr(ref.m_ptr) { AddRef(); }

    //! destructor
    ~ref_() { Release(); }

public:
    ref_t AddRef() {
        return m_ptr ? m_ptr->AddRef() : 0;
    }

    ref_t Release() {
        ref_t refs = Dismiss(); m_ptr = NullPtr; return refs;
    }

public: ///-- accessors
    const T *ptr() const { return m_ptr; }
    T *ptr() { return m_ptr; }

    //! @note get doesn't check if ptr is non null, should only be used if certain the pointer is set
    const T &get() const { assert(m_ptr); return *m_ptr; }
    T &get() { assert(m_ptr); return *m_ptr; }

    explicit operator bool() const { return !isNull(); }

    const T * operator *() const { return m_ptr; }
    const T * operator ->() const { return m_ptr; }

    T * operator *() { return m_ptr; }
    T * operator ->() { return m_ptr; }

    NoDiscard bool isNull() const { return m_ptr == NullPtr; }

public: ///-- operators
    ref_<T> &operator =( T *p ) { Assign( p ); return *this; } //! NB adds a reference, for 'new' assignment use constructor
    ref_<T> &operator =( T &p ) { Assign( &p ); return *this; }
    ref_<T> &operator =( const ref_<T> &p ) { Assign( (T*) p.ptr() ); return *this; }
    ref_<T> &operator =( ref_<T> &&p ) NoExcept { Assign( (T*) p.ptr() ); return *this; }

    bool operator ==( const T *ptr ) const { return m_ptr == ptr; }
    bool operator !=( const T *ptr ) const { return m_ptr != ptr; }

    bool operator ==( const T &p ) const { return m_ptr == &p; }
    bool operator !=( const T &p ) const { return m_ptr != &p; }

    bool operator ==( const ref_<T> &ref ) const { return m_ptr == ref.ptr(); }
    bool operator !=( const ref_<T> &ref ) const { return m_ptr != ref.ptr(); }

public: ///-- object interface query

    //!NB: requires Ty::As_<..>() object style interface)

    template <class Ty> Ty *As_() {
        return m_ptr ? m_ptr->template As_<Ty>() : NullPtr;
    }

    //--
    template <class Ty> //! NO
    ref_<T>( ref_<Ty> &ref ) : m_ptr(NullPtr) {
        Assign( ref.template As_<T>() );
    }

    template <class Ty>
    ref_<T> &operator =( Ty *p ) {
        return this->operator =( p ? p->template As_<T>() : NullPtr );
    }

    template <class Ty>
    ref_<T> &operator =( Ty &p ) {
        return this->operator =( p.template As_<T>() );
    }

    template <class Ty>
    ref_<T> &operator =( ref_<Ty> &ref ) {
        return this->operator =( ref.template As_<T>() );
    }

public:
//! CARE !// => functions below break reference counting : only use to set to uninitialized memory etc...
    void Attach( T *p ) { m_ptr = p; }
    void Detach() { m_ptr = NullPtr; }

protected:
    void Assign( T *p ) {
        if( m_ptr != p ) { Dismiss(); m_ptr = p; AddRef(); }
    }

    ref_t Dismiss() { //! NB this doesn't set ptr to null
        return m_ptr ? m_ptr->Release() : 0;
    }
};

///////////////////////////////////////////////////////////////////////////////
///-- interface

struct IReferable {
    virtual ~IReferable() DEFAULT

    API_DECL(ref_t) AddRef() = 0;
    API_DECL(ref_t) Release() = 0;
};

///////////////////////////////////////////////////////////////////////////////
///-- base implementation

class CReferable : virtual IReferable {
public:
    CReferable() : m_refs(1) {}

    virtual ~CReferable() {
        //TODO // assert(m_refs==0);
    }

///--
    API_IMPL(ref_t) AddRef() IOVERRIDE {
        ASYNCH_INCREMENT(m_refs); return m_refs;
    }

    API_IMPL(ref_t) Release() IOVERRIDE {
        ref_t refs = ASYNCH_DECREMENT(m_refs);

        if( refs == 0 )
            delete this;

        return refs;
    }

private:
    volatile ref_t m_refs; //! reference count
};

//////////////////////////////////////////////////////////////////////////////
//! Shared Ptr

template <class T>
struct Ptr_ {
protected:
    T *m_ptr;

    volatile ref_t m_refs;

public:
    Ptr_() : m_ptr(NullPtr) ,m_refs(0)
    {}

    ~Ptr_()
    {}

    //TODO
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Definitions

//TODO use
// #define GUIDtoPUID(__guid__)        ( ((PUID*) __guid) ^ ((PUID*) (__guid+8)) )
//TODO OR
// PUID MakePUID( const guid_t &guid ,const guid_t &appGuid = APPLICATION_GUID );

typedef uint64_t puid_t;

///-- class

    //! @note program unique id

#ifndef PUID //TODO rename to PUID
 #define PUID puid_t
#endif

#define PUID_NIL    ((PUID) 0 )
#define PUID_MAX    ((PUID) 0xffffffffffffffff )
#define PUID_OMNI   ((PUID) 0xffffffffffffffff )

//-- instance

    //! @note program instance id

#define PIID puid_t

#define PIID_NOINSTANCE  ((PIID) 0 )

//////////////////////////////////////////////////////////////////////////////
//! Class identity

///-- common registry
bool registerClassName( const PUID &id ,const char *name );

// void listClassNames( ListOf<String> &names );
// void listClassIds( ListOf<PUID> &ids );

bool findClassIdByName( const char *name ,PUID &id );
bool findClassNameById( const PUID &id ,String &name );

///-- structure

    //! @brief structure identity declared with an out of scope template declaration

template <typename T>
PUID classId_();

template <typename T>
const char *className_();

//! @brief declare a structure id
#define DECLARE_STRUCTID(__class,__uid) \
    template <> inline PUID classId_<__class>() { return __uid; }

//! @brief declare a structure's name
#define DECLARE_STRUCTNAME(__class,__name) \
    template <> inline const char *className_<__class>() { return __name; }

//! @brief declare a structure's identity (id & name)
#define DECLARE_STRUCT(__class,__uid) \
    DECLARE_STRUCTID(__class,__uid) \
    DECLARE_STRUCTNAME(__class,TINY_STR(__class) )

//! @brief register a structure's identity
template <typename T>
bool registerStructName_() {
    return registerClassName( classId_<T>() ,className_<T>() );
}

#define REGISTER_STRUCTNAME(__class) \
    static bool g_##__class##_registered = registerStructName_<__class>();

///-- classes

    //! @brief class identity declared with an in scope member declaration

//! @brief declare a class's id
#define DECLARE_CLASSID(__uid) \
    static PUID classId() { return __uid; }

//! @brief declare a class's name
#define DECLARE_CLASSNAME(__name) \
    static const char *className() { return __name; }

//! @brief declare an class's identity (id & name)
#define DECLARE_CLASS(__class,__uid) \
    DECLARE_CLASSID(__uid) \
    DECLARE_CLASSNAME( TINY_STR(__class) ) \

    //! @note using 'DECLARE' an not 'MEMBERS' as it is a static function declaration, not member functions

//! @brief register a class's identity
template <typename T>
bool registerClassName_() {
    return registerClassName( T::classId() ,T::className() );
}

#define REGISTER_CLASSNAME(__class) \
    static bool g_##__class##_registered = RegisterClassName_<__class>();

//////////////////////////////////////////////////////////////////////////////
//! IObject

#define IOBJECT_PUID    0x0fffa000100010001

class IObject : public virtual IReferable {
public:
    API_IMPL() ~IObject() IOVERRIDE DEFAULT;

    static PUID classId() { return IOBJECT_PUID; };

    bool operator == ( IObject &object ) const { return this == &object; }
    bool operator != ( IObject &object ) const { return this != &object; }

public:
    IAPI_DECL getClassId( PUID &id ) { id = classId(); return IOK; };
    IAPI_DECL getInterface( PUID id ,void **ppv ) = 0;

public: //-- identity
    PUID getMyClassId() {
        PUID id; getClassId(id); return id;
    }

    bool getMyClassName( String &name ) {
        return findClassNameById( getMyClassId() ,name );
    }

    String myClassName() {
        String name; getMyClassName( name ); return name;
    }

public: //-- interface
    template <class T> T *getInterface_() {
        void *pv = NullPtr; return (this->getInterface( T::classId() ,&pv ) == IOK) ? (T*) pv : NullPtr;
    }

    template <class T> T *As_() {
        return getInterface_<T>();
    }

protected:
    static MapOf<String,PUID> m_classNames;
};

typedef RefOf<IObject> IObjectRef;

//! @brief support macro to derive from IObject
#define IOBJECT_PARENT    virtual public IObject

//! @brief shorthands to get own id and name from an object member function
#define MyPUID  this->getMyClassId()
#define MyNAME  this->getMyClassName()

//////////////////////////////////////////////////////////////////////////////
//! interface support helpers

//! @brief helper function to honor an interface in getInterface implementation
template <class T>
bool honorInterface_( T *object ,PUID id ,void **ppv ) {
    if( T::classId() == id ) {
        *ppv = (T*) object; SAFECALL(object)->AddRef(); return true;
    }

    return false;
}

//! @brief out of class function to get an object's interface (NB template inference identify the object)
template <class T>
bool getInterface_( T &object ,PUID iid ,void **ppv ) {
    return object.getInterface( iid ,ppv ) == IOK;
}

//! idem, for a pointer
template <class T>
bool getInterface_( T *p ,PUID iid ,void **ppv ) {
    return p && getInterface_<T>( *p ,iid ,ppv );
}

//! idem, typed
    //! @note this is the safer and preferred way to get an interface from an object

template <class T ,class Ty>
Ty *getInterface_( T &object ) {
    void *pv = NullPtr; return object.getInterface( Ty::getClassId() ,&pv ) ? (T*) pv : NullPtr;
}

template <class T ,class Ty>
T *getInterface_( T *p ) {
    return p ? getInterface_<T,Ty>( *p ) : NullPtr;
}

//////////////////////////////////////////////////////////////////////////////
//! CObject

    //! @brief base for a IObject implementation

class CObject : IOBJECT_PARENT ,public CReferable {
public:
    CObject() DEFAULT

public: ///-- IObject
    IAPI_IMPL getInterface( PUID id ,void **ppv ) IOVERRIDE {
        return (!ppv || *ppv) ? IBADARGS :
            honorInterface_<IObject>(this,id,ppv)
            ? IOK : INODATA
        ;
    }
};

//! @brief support macro to derive from CObject
#define COBJECT_PARENT    public virtual CObject

//! @brief declare IObject class identity (static & dynamic gets)
#define DECLARE_ICLASS(__class,__uid) \
    DECLARE_CLASS(__class,__uid) \
    IAPI_IMPL getClassId( PUID &id ) IOVERRIDE { id = classId(); return IOK; }

//! @brief declare IObject class identity and interface
#define DECLARE_OBJECT(__class,__uid) \
    DECLARE_ICLASS(__class,__uid )    \
    IAPI_IMPL getInterface( PUID id ,void **ppv ) IOVERRIDE

//! @brief declare IObject class identity and default getInterface for single inheritance
#define DECLARE_OBJECT_STD(__super,__class,__uid) \
    DECLARE_OBJECT(__class,__uid) { \
        return (!ppv || *ppv) ? IBADARGS : honorInterface_<__class>(this,id,ppv) ? IOK : __super::getInterface( id ,ppv ); \
    }

//////////////////////////////////////////////////////////////////////////////
//! Named Pointers

void registerInstance( PIID &id ,IObject *p );
void revokeInstance( PIID &id );

IObject *getInstance( PIID &id );
PIID digInstanceId( IObject *p );

template <class T>
inline T *getInstance( PIID &id ) {
    IObject *p = getInstance( id );

    return p ? p->As_<T>() : NullPtr;
}

//////////////////////////////////////////////////////////////////////////////
//! Locking Pointers

/* static MapOf<PIID,int> g_objectLocks;

struct PtrLock {
    explicit PtrLock( PIID id ) : m_piid(id) {}

    IObject *exclusive();

    IObject *lock() {
        auto it = g_objectLocks.find( m_piid );

        // CS.lock
        if( it->second < 0 ) return NullPtr;

        ++it->second;
    }

    IObject *unlock();

    // static CriticalSection m_cs;

    PIID m_piid;
};

template <class T>
struct PtrLock_ : PtrLock {
};
*/

//////////////////////////////////////////////////////////////////////////////
//! Instance

//! @note base class for object with identifiable instance
class CWithInstance : IOBJECT_PARENT {
public:
    CWithInstance( PIID id=PIID_NOINSTANCE ) : m_piid(id) {
        registerInstance( id ,this );
    }

    API_IMPL() ~CWithInstance() IOVERRIDE {
        revokeInstance( m_piid );
    }

    NoDiscard PIID getInstanceId() const { return m_piid; }

protected:
    PIID m_piid; //! instance id
};

//////////////////////////////////////////////////////////////////////////////
//! Factory

//? ALSO / LATER

/* struct IFactory : IOBJECT_PARENT {
    // DECLARE_CLASS(IFactory,IFACTORY_PUID);

    API_DECL(IObject*) CreateInstance( PIID id ) = 0;
};

API_IMPL(IObject*) CreateInstance( PIID id ) { return (IObject*) new __class(); }
*/

#define DECLARE_FACTORY(__class) \
    template <class T> static T *Create_( PIID id=PIID_NOINSTANCE ) { return (T*) new __class(); } \
    static bool m_registered; \
    static bool RegisterClass()

#define DECLARE_FACTORY_IOBJECT(__class) \
    DECLARE_FACTORY(__class) { \
        IObject::registerClassName( getClassId() ,getClassName() ); \
        return RegisterClass_<IObject>( getClassId() ,Create<IObject> ); \
    }

#define DECLARE_FACTORY_STD(__super,__class) \
    DECLARE_FACTORY(__class) { \
        registerClassName( classId() ,className() ); \
        return RegisterClass_<__super>( classId() ,Create_<__super> ) && RegisterClass_<IObject>( classId() ,Create_<IObject> ); \
    }

//! @brief register a class with factory capabilities
#define REGISTER_CLASS(__class) \
    bool __class::m_registered = __class::RegisterClass();

//////////////////////////////////////////////////////////////////////////////
//! IException

class IException : public Exception {
public:
    IException( IObject *object=NullPtr ,iresult_t id=IERROR ,const char *msg=NullPtr ) :
            Exception(id,msg) ,m_object(object)
    {}

    NoDiscard IObject *object() const { return m_object; }

protected:
    IObject *m_object;
};

///////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

///////////////////////////////////////////////////////////////////////////////
#endif //TINY_OBJECT_H