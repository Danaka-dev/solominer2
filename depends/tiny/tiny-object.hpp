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
    //TODO redef macro here for this

///////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//! Reference

    //!IE reference counted pointer (NB object enables reference counting (!= std::share_ptr)

typedef ASYNCH_TYPE ref_t;

///////////////////////////////////////////////////////////////////////////////
#define RefOf ref_

template <class T>
class ref_ {
protected:
    T *m_ptr;

public:
    //! Initializing with a new ptr (use along with new, taking ownership of the initial reference, no AddRef)
    explicit ref_( T *p=NullPtr ) : m_ptr(p) {}

    //! Initializing with an existing object (adding a reference)
    ref_( T &p ) : m_ptr(&p) { AddRef(); }

    //! copy constructor (adding a reference)
    ref_( const ref_<T> &ref ) : m_ptr(ref.m_ptr) { AddRef(); }

    //! move constructor (adding a reference, ... )
    //! @note while it doesn't seems logical to AddRef on a move, destructor will be called off the donating object
    //! which will Release() and remove a reference... so we need to AddRef here )
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

    const T &get() const { assert(m_ptr); return *m_ptr; }
    T &get() { assert(m_ptr); return *m_ptr; } //! @note no check, only use if certain the pointer will not be null

    explicit operator bool() const { return !isNull(); }

    const T * operator *() const { return m_ptr; }
    const T * operator ->() const { return m_ptr; }

    T * operator *() { return m_ptr; }
    T * operator ->() { return m_ptr; }

    bool isNull() const { return m_ptr == NullPtr; }

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
        return this->operator =( ref.template As_<T>() ); return *this;
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
    virtual ~IReferable() DEFAULT;

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
//////////////////////////////////////////////////////////////////////////////
//! Ptr

#define INSTANCEID uint64_t

#define INSTANCE_NOID   0

// template <> getStore() => singleton

    //... TODO
    // ptr_ indirection + relink of serialized object instance (define instance id ...)
    //TODO
    //=> getObject( INSTANCEID id );

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Definitions

#define IOBJECT_UUID    0x0fffa000100010001
    //? TODO UUID from GUID ? so ok to use both ?

#ifndef UUID
 #define UUID uint64_t
#endif

// typedef UUID uuid_t; //TODO @note and solve, clashing UUID from libuuid ...

//////////////////////////////////////////////////////////////////////////////
//! Declaration

template <typename T>
const UUID classId_();

template <typename T>
const char *className_();

#define DECLARE_TCLASS(__class,__uid) \
    template <> inline const UUID classId_<__class>() { return __uid; } \
    template <> inline const char *className_<__class>() { return TINY_STR(__class); }

//--

//TODO change all declaration from below macro to above template

//! @brief declare an object class id
#define DECLARE_CLASSID(__uid) \
    static const UUID classId() { return __uid; }

//! @brief declare an object class name
#define DECLARE_CLASSNAME(__name) \
    static const char *className() { return __name; }

//! @brief declare an object class identity (id & name)
#define DECLARE_CLASS(__class,__uid) \
    DECLARE_CLASSID(__uid) \
    DECLARE_CLASSNAME( TINY_STR(__class) ) \

//! @brief declare an IObject class identity (i.e. with dynamic UUID get)
#define DECLARE_ICLASS(__class,__uid) \
    DECLARE_CLASS(__class,__uid) \
    IAPI_IMPL getClassId( UUID &id ) IOVERRIDE { id = classId(); return IOK; }

//////////////////////////////////////////////////////////////////////////////
//! Class identity

bool registerClassName( const UUID &id ,const char *name );

//TODO findClassIdByName ?

bool getClassIdFromName( const char *name ,UUID &id );
bool getClassNameFromId( const UUID &id ,String &name );

//--
template <typename T>
bool RegisterTClassName_() {
    return registerClassName( classId_<T>() ,className_<T>() );
}

//--
#define REGISTER_TCLASSNAME(__class) \
    static bool g_##__class##_registered = RegisterTClassName_<__class>();

//--
template <typename T>
bool RegisterClassName_() {
    return registerClassName( T::classId() ,T::className() );
}

//--
#define REGISTER_CLASSNAME(__class) \
    static bool g_##__class##_registered = RegisterClassName_<__class>();

//////////////////////////////////////////////////////////////////////////////
//! IObject

class IObject : public virtual IReferable {
public:
    virtual ~IObject() DEFAULT;

    static const UUID classId() { return IOBJECT_UUID; };

    bool operator == ( IObject &object ) const { return this == &object; }
    bool operator != ( IObject &object ) const { return this != &object; }

public:
    IAPI_DECL getClassId( UUID &id ) { return classId(); };
    IAPI_DECL getInterface( UUID id ,void **ppv ) = 0;

public: //-- identity
    UUID getMyClassId() {
        UUID id; getClassId(id); return id;
    }

    bool getClassName( String &name ) {
        return getClassNameFromId( getMyClassId() ,name );
    }

    String myClassName() {
        String name; getClassName(name); return name;
    }

public: //-- interface
    template <class T> T *getInterface_() {
        void *pv = NullPtr; return (this->getInterface( T::classId() ,&pv ) == IOK) ? (T*) pv : NullPtr;
    }

    template <class T> T *As_() {
        return getInterface_<T>();
    }

public: //-- name resolution
        /*
    // static void listClassNames( ListOf<String> &names ); //TODO
    static bool registerClassName( const UUID &id ,const char *name );

    static bool getClassIdFromName( const char *name ,UUID &id );
    static bool getClassNameFromId( const UUID &id ,String &name );
    */

protected:
    static MapOf<String,UUID> m_classNames;
};

typedef RefOf<IObject> IObjectRef;

#define IOBJECT_PARENT    virtual public IObject

#define MyUUID  this->getMyClassId()
#define MyNAME  this->getClassName()

//-- helper to honor an interface for getInterface implementation
template <class T>
bool honorInterface_( T *object ,UUID id ,void **ppv ) {
    if( T::classId() == id ) {
        *ppv = object; SAFECALL(object)->AddRef(); return true;
    }

    return false;
}

//-- out of class function to get an object's interface (NB template inference identify the object)
template <class T>
bool getInterface_( T &object ,UUID iid ,void **ppv ) {
    return object.getInterface( iid ,ppv ) == IOK;
}

//-- idem, for a pointer
template <class T>
bool getInterface_( T *object ,UUID iid ,void **ppv ) {
    return object && getInterface_<T>( *object ,iid ,ppv );
}

//-- idem, using template type to infer uuid and return proper typed pointer
    //! NB this is the safer and preferred way to get an interface from an object

template <class T ,class Ty>
Ty *getInterface_( T &object ) {
    void *pv = NullPtr; return object.getInterface( Ty::getClassId() ,&pv ) ? (T*) pv : NullPtr;
}

template <class T ,class Ty>
T *getInterface_( T *object ) {
    return object ? getInterface_<T,Ty>( *object ) : NullPtr;
}

//-- register class
template <class T>
bool registerClassName_() {
    return registerClassName( T::classId() ,T::className() );
}

//////////////////////////////////////////////////////////////////////////////
//! IException

class IException : public Exception {
public:
    IException( IObject *object=NullPtr ,iresult_t id=IERROR ,const char *msg=NullPtr ) :
        Exception(id,msg) ,m_object(object)
    {}

    IObject *object() const { return m_object; }

protected:
    IObject *m_object;
};

//////////////////////////////////////////////////////////////////////////////
//! CObject

    //! @brief base for IObject implementation

class CObject : IOBJECT_PARENT ,public CReferable {
public:
/* //LATER see ptr
    INSTANCEID m_id;

    CObject( INSTANCEID id=NO_INSTANCE_ID );

    ~CObject() override;

    INSTANCEID getId() { return m_id; }
*/

public: ///-- IObject
    IAPI_IMPL getInterface( UUID id ,void **ppv ) IOVERRIDE {
        return (!ppv || *ppv) ? IBADARGS : honorInterface_<IObject>(this,id,ppv) ? IOK : INODATA;
    }
};

#define COBJECT_PARENT    public virtual CObject

//TODO remove IMPORT_IOBJECT_API and only use DECLARE_xxx
#define IMPORT_IOBJECT_API(__uid) \
    static UUID classId() { return __uid; } \
    IAPI_IMPL getInterface( UUID id ,void **ppv ) IOVERRIDE

///--
/* #define DECLARE_OBJECT_API(__class,__uid) \
    DECLARE_CLASS(__class,__uid ) \
    IAPI_IMPL getInterface( UUID id ,void **ppv ) IOVERRIDE */

#define DECLARE_OBJECT(__class,__uid) \
    DECLARE_ICLASS(__class,__uid ) \
    IAPI_IMPL getInterface( UUID id ,void **ppv ) IOVERRIDE

#define DECLARE_OBJECT_IOBJECT(__class,__uid) \
    DECLARE_ICLASS(__class,__uid ) \
    IAPI_IMPL getInterface( UUID id ,void **ppv ) IOVERRIDE { \
        return (!ppv || *ppv) ? IBADARGS : honorInterface_<__class>(this,id,ppv) ? IOK : CObject::getInterface( id ,ppv ); \
    }

#define DECLARE_OBJECT_STD(__super,__class,__uid) \
    DECLARE_OBJECT(__class,__uid) { \
        return (!ppv || *ppv) ? IBADARGS : honorInterface_<__class>(this,id,ppv) ? IOK : __super::getInterface( id ,ppv ); \
    }

///--
#define DECLARE_FACTORY_API(__class) \
    template <class T> static T *Create( const char *name=NullPtr ) { return (T*) new __class(); } \
    static bool m_registered; \
    static bool RegisterClass()

#define DECLARE_FACTORY_IOBJECT(__class) \
    DECLARE_FACTORY_API(__class) { \
        IObject::registerClassName( getClassId() ,getClassName() ); \
        return RegisterClass_<IObject>( getClassId() ,Create<IObject> ); \
    }

#define DECLARE_FACTORY_STD(__super,__class) \
    DECLARE_FACTORY_API(__class) { \
        registerClassName( classId() ,className() ); \
        return RegisterClass_<__super>( classId() ,Create<__super> ) && RegisterClass_<IObject>( classId() ,Create<IObject> ); \
    }

///--
#define REGISTER_CLASS(__class) \
    bool __class::m_registered = __class::RegisterClass();

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//! View ... => ptr

    //TODO ... see ptr note above

/* template <class T>
struct Viewable_ {
    typedef unsigned long id_t;

    void Register();
    void Release();
};

template <class T>
class View_ {
protected:
    Viewable_<T>::id_t m_viewId;

public:
    void set( Viewable_<T> &viewable ) {
        m_viewId = viewable.id();
    }

    T *ptr() {
        return g_viewRoot.get( m_viewId );
    }
};

#define View_ ViewOf

ViewOf<myobject> m_myview;

struct View { // ViewRoot .. Views_
    //? observer pattern built-in ?

    // Tree to view_ ...
    //Ok for view should only be obejct UUID classes (ie not any pointer) ...

    typedef unsigned int id_t; //! or void* .. a pointer to ptr ... but then not compact + not recyclable and not indexable ...
    typedef void* ptr_t;

    static ArrayOf<uint32_t,ptr_t> root;

///--
    id_t Register( ptr_t p );
    void Release( id_t id ) { objects[id] = NullPtr; }

    ptr_t Get( id_t id ) { return objects[id]; }

    template <class T>
    T *Get_( id_t id ) { return (T*) objects[id]; }

    id_t Dig( ptr_t p );
};

struct Observer {
    virtual void onUpdate( IBase &object );

    virtual void onRelease( IBase &object );
};

// template <class T>
struct Observable {
    //id from viewable

    static ArrayOf<id_t,Observer> observers;
}; */

///////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

///////////////////////////////////////////////////////////////////////////////
#endif //TINY_OBJECT_H