#ifndef DEF_H
#define DEF_H

#include <functional>
#include <QMutex>
#include <QMutexLocker>

/**********************************************************************************************************************
@brief: 
***********************************************************************************************************************/
#define _EXE_COMMAND_REQUEST_EVENT "___EXE_COMMAND_REQUEST__"
#define _UPDATE_UI_REQUEST_EVENT   "___UPDATE_UI_REQUEST__"

/**********************************************************************************************************************
@brief: concatenating multiple args into one
***********************************************************************************************************************/
#define CONCATE(...) __VA_ARGS__

/**********************************************************************************************************************
@brief: Q_PROPERTY getter and setter generator
***********************************************************************************************************************/
#define ADD_QPROP_RW_INIT(type, name, getter, setter, init) \
    public: \
        Q_PROPERTY(type name MEMBER m_##name READ getter WRITE setter NOTIFY name##Changed) \
        type& getter() { return m_##name; } \
        type const & getter() const{ return m_##name; } \
        void setter(type name) { m_##name = name; emit name##Changed(name);} \
        Q_SIGNAL void name##Changed(type& name); \
    private: \
        type m_##name = init;

/**********************************************************************************************************************
@brief:
***********************************************************************************************************************/
#define ADD_QPROP_RW(type, name, getter, setter) \
    public: \
        Q_PROPERTY(type name MEMBER m_##name READ getter WRITE setter NOTIFY name##Changed) \
        type& getter() { return m_##name; } \
        type const & getter() const{ return m_##name; } \
        void setter(type name) { m_##name = name; emit name##Changed(name);} \
        Q_SIGNAL void name##Changed(type& name); \
    private: \
        type m_##name;

/**********************************************************************************************************************
@brief:
***********************************************************************************************************************/
#define ADD_QPROP_RO_INIT(type, name, getter, init) \
    public: \
        Q_PROPERTY(type name MEMBER m_##name READ getter) \
        type& getter() { return m_##name; } \
        type const & getter() const{ return m_##name; } \
    private: \
        type m_##name = init;

/**********************************************************************************************************************
@brief:
***********************************************************************************************************************/
#define ADD_QPROP_RO(type, name, getter) \
    public: \
        Q_PROPERTY(type name MEMBER m_##name READ getter) \
        type& getter() { return m_##name; } \
        type const & getter() const{ return m_##name; } \
    private: \
        type m_##name;

/**********************************************************************************************************************
@brief:
***********************************************************************************************************************/
#define ADD_QPROP_PR_INIT(type, name, init) \
    public: \
        Q_PROPERTY(type name MEMBER m_##name) \
    private: \
        type m_##name = init;

/**********************************************************************************************************************
@brief:
***********************************************************************************************************************/
#define ADD_QPROP_PR(type, name) \
    public: \
        Q_PROPERTY(type name MEMBER m_##name) \
    private: \
        type m_##name;

/**********************************************************************************************************************
@brief: getter and setter generator for class member
***********************************************************************************************************************/
#define ADD_FIELD_INIT(type, name, getter, setter, init) \
    public: \
        type& getter() { return m_##name; } \
        type const & getter() const{ return m_##name; } \
        void setter(type name) { m_##name = name; } \
    private: \
        type m_##name = init;

/**********************************************************************************************************************
@brief:
***********************************************************************************************************************/
#define ADD_FIELD(type, name, getter, setter) ADD_CLASS_FIELD(type, name, getter, setter)
#define ADD_CLASS_FIELD(type, name, getter, setter) \
    public: \
        type& getter() { return m_##name; } \
        type const & getter() const{ return m_##name; } \
        void setter(type name) { m_##name = name; } \
    private: \
        type m_##name;

/**********************************************************************************************************************
@brief:
***********************************************************************************************************************/
#define ADD_FIELD_NOSETTER_INIT(type, name, getter, init) \
    public: \
        type& getter() { return m_##name; } \
        type const & getter() const{ return m_##name; } \
    private: \
        type m_##name = init;

/**********************************************************************************************************************
@brief:
***********************************************************************************************************************/
#define ADD_FIELD_NOSETTER(type, name, getter) ADD_CLASS_FIELD_NOSETTER(type, name, getter)
#define ADD_CLASS_FIELD_NOSETTER(type, name, getter) \
    public: \
        type& getter() { return m_##name; } \
        type const & getter() const{ return m_##name; } \
    private: \
        type m_##name;

/**********************************************************************************************************************
@brief:
***********************************************************************************************************************/
#define ADD_FIELD_PRIVATE_INIT(type, name, init ) \
    private: \
        type m_##name = init;

#define ADD_FIELD_PRIVATE(type, name ) ADD_CLASS_FIELD_PRIVATE(type, name )
#define ADD_CLASS_FIELD_PRIVATE(type, name ) \
    private: \
        type m_##name;


/**********************************************************************************************************************
@brief: SINGLETON DESIGN PATTERN (Thread Safe) 
***********************************************************************************************************************/
#define SINGLETON_PATTERN_DECLARE(classname)\
    public: \
        static classname* getInstance() { QMutexLocker cLocker(&m_cGetInstanceMutex); if(m_instance==nullptr) m_instance=new classname(); return m_instance; } \
    private: \
        static classname* m_instance; \
        static QMutex m_cGetInstanceMutex;

#define SINGLETON_PATTERN_IMPLIMENT(classname)\
    classname* classname::m_instance = nullptr; \
    QMutex classname::m_cGetInstanceMutex;

/**********************************************************************************************************************
@brief: PROTOTYPE PATTERN
***********************************************************************************************************************/
#define CLONABLE(classname)\
    public:\
        virtual classname* clone() const { return new classname(*this); }

/**********************************************************************************************************************
@brief: CLIP c BETWEEN a AND b
***********************************************************************************************************************/
#define VALUE_CLIP(min,max,value) ( ((value)>(max))?(max):((value)<(min))?(min):(value) )

/**********************************************************************************************************************
@brief: SCOPE GUARD
***********************************************************************************************************************/
template <typename F>
struct ScopeExit
{
    ScopeExit(F f) : f(f) {}
    ~ScopeExit() { f(); }
    F f;
};

template <typename F>
ScopeExit<F> MakeScopeExit(F f) 
{
    return ScopeExit<F>(f);
}

#define SCOPE_EXIT(code) \
    auto scope_exit_##__LINE__ = MakeScopeExit([=](){code;})


/**********************************************************************************************************************
@brief: make callback functor
***********************************************************************************************************************/
#define MAKE_CALLBACK(memberFunc) \
    std::bind( &memberFunc, this, std::placeholders::_1 )

#define MAKE_CALLBACK_OBJ(object, memberFunc) \
    std::bind( &memberFunc, &(object), std::placeholders::_1 )

#endif
