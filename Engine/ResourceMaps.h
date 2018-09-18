#pragma once

#include "EngineGlobal.h"
#include "List.h"
#include "HashTable.h"
#include "ISerializable.h"
#include "StringPool.h"

#if defined LINUX || defined IOS || defined MAC
   #include <cxxabi.h>
#endif

#ifdef ENABLE_RTTI_CHECK
   //check the signature of a function
   //with another arglist and assert if different
   #define CheckSignature(list, sig2)\
   {\
      Debug::Assert( Condition(list.GetArgSig( )->IsEqualTo((sig2))), "Signatures do not match!" );\
   }
#else
   #define CheckSignature(list, sig2)
   #define ArgSignaturesEqual(list, types, addressOfResult) *(addressOfResult) = false;
#endif

#define SZ_TYPE(type) #type
#define TypeRegistry_RegisterTypeInfo(type)\
   TypeRegistry::RegisterTypeInfo( SZ_TYPE(type), typeid(type) );\
   TypeRegistry::RegisterTypeInfo( SZ_TYPE(const type *), typeid(const type *) );\
   TypeRegistry::RegisterTypeInfo( SZ_TYPE(type *), typeid(type*) );

class TypeRegistry
{
public:
   class TypeInfo
   {
   public:
      TypeInfo(
         const char *pTypeString,
         const type_info &_type
      ) : type(_type)
      {      
         String::Copy( name, pTypeString, sizeof(name) );
      }

      const type_info &type;
      char name[ 32 ];
   };

private:
   static HashTable<const char *, const TypeInfo *> m_TypesByIdentifier;
   static HashTable<const char *, const TypeInfo *> m_TypesByTypeId;

public:
   static void Create ( void );
   static void Destroy( void );

   static const TypeInfo *GetTypeInfo( 
      const char *pName
   );
   
   static const TypeInfo *GetTypeInfo( 
      const type_info &typeInfo
   );

   static void RegisterTypeInfo(
      const char *pStringType,
      const type_info &
   );
};

class ArgSig
{
public:
   static const uint32 MaxArgCount = 7;

private:
   uint32 m_Count;
   const type_info *m_pTypes[ MaxArgCount ];

public:
   ArgSig( void )
   {
      m_Count = 0;
   }
   
   ArgSig(const type_info &a)
   {
      m_Count = 0;

      AddType( a );
   }

   ArgSig(const type_info &a, const type_info &b)
   {
      m_Count = 0;
      AddType( a );
      AddType( b );
   }

   ArgSig(const type_info &a, const type_info &b, const type_info &c)
   {
      m_Count = 0;

      AddType( a );
      AddType( b );
      AddType( c );
   }

   ArgSig(const type_info &a, const type_info &b, const type_info &c, const type_info &d)
   {
      m_Count = 0;

      AddType( a );
      AddType( b );
      AddType( c );
      AddType( d );
   }

   ArgSig(const type_info &a, const type_info &b, const type_info &c, const type_info &d, const type_info &e)
   {
      m_Count = 0;

      AddType( a );
      AddType( b );
      AddType( c );
      AddType( d );
      AddType( e );
   }

   ArgSig(const type_info &a, const type_info &b, const type_info &c, const type_info &d, const type_info &e, const type_info &f)
   {
      m_Count = 0;

      AddType( a );
      AddType( b );
      AddType( c );
      AddType( d );
      AddType( e );
      AddType( f );
   }

   ArgSig(const type_info &a, const type_info &b, const type_info &c, const type_info &d, const type_info &e, const type_info &f, const type_info &g)
   {
      m_Count = 0;

      AddType( a );
      AddType( b );
      AddType( c );
      AddType( d );
      AddType( e );
      AddType( f );
      AddType( g );
   }

   void Clear( )
   {
      m_Count = 0;
   }

   void AddType(
      const type_info &type
   )
   {
      Debug::Assert( Condition(m_Count < MaxArgCount), "ArgSig count exceeded" );
      m_pTypes[ m_Count++ ] = &type;
   }

   const type_info &GetType(
      uint32 index
   ) const
   {
      Debug::Assert( Condition(index <= m_Count), "ArgSig index exceeded" );
      return *m_pTypes[ index ];
   }

   uint32 GetCount( void ) const { return m_Count ;}
   
   bool IsEqualTo(
      const ArgSig &sig
   ) const
   {
      if ( m_Count != sig.m_Count ) return false;

      uint32 i;

      for ( i = 0; i < m_Count; i++ )
      {
         if ( m_pTypes[ i ] != sig.m_pTypes[ i ] ) return false;
      }

      return true;
   }
};

class ArgList : public ISerializable
{
public:
   DeclareSerializableType(ArgList);

public:
   static const uint32 MaxOffsetCount = ArgSig::MaxArgCount + 1;

public:
   ArgSig m_Sig;

   uint32 m_Offsets[ MaxOffsetCount ];
   char   m_Buffer[ 1024 ];

public:
   ArgList( void )
   {
      StartPack( );
   }

   template <typename t_a>
   ArgList( t_a a ) 
   {
      StartPack( );
      Pack( typeid(a), &a, sizeof(a) );
   }

   template <typename t_a, typename t_b>
   ArgList( t_a a, t_b b ) 
   {
      StartPack( );
      Pack( typeid(a), &a, sizeof(a) );
      Pack( typeid(b), &b, sizeof(b) );
   }

   template <typename t_a, typename t_b, typename t_c>
   ArgList( t_a a, t_b b, t_c c ) 
   {
      StartPack( );
      Pack( typeid(a), &a, sizeof(a) );
      Pack( typeid(b), &b, sizeof(b) );
      Pack( typeid(c), &c, sizeof(c) );
   }

   template <typename t_a, typename t_b, typename t_c, typename t_d>
   ArgList( t_a a, t_b b, t_c c, t_d d ) 
   {
      StartPack( );
      Pack( typeid(a), &a, sizeof(a) );
      Pack( typeid(b), &b, sizeof(b) );
      Pack( typeid(c), &c, sizeof(c) );
      Pack( typeid(d), &d, sizeof(d) );
   }

   template <typename t_a, typename t_b, typename t_c, typename t_d, typename t_e>
   ArgList( t_a a, t_b b, t_c c, t_d d, t_e e ) 
   {
      StartPack( );
      Pack( typeid(a), &a, sizeof(a) );
      Pack( typeid(b), &b, sizeof(b) );
      Pack( typeid(c), &c, sizeof(c) );
      Pack( typeid(d), &d, sizeof(d) );
      Pack( typeid(e), &e, sizeof(e) );
   }

   template <typename t_a, typename t_b, typename t_c, typename t_d, typename t_e, typename t_f>
   ArgList( t_a a, t_b b, t_c c, t_d d, t_e e, t_f f ) 
   {
      StartPack( );
      Pack( typeid(a), &a, sizeof(a) );
      Pack( typeid(b), &b, sizeof(b) );
      Pack( typeid(c), &c, sizeof(c) );
      Pack( typeid(d), &d, sizeof(d) );
      Pack( typeid(e), &e, sizeof(e) );
      Pack( typeid(f), &f, sizeof(f) );
   }

   template <typename t_a, typename t_b, typename t_c, typename t_d, typename t_e, typename t_f, typename t_g>
   ArgList( t_a a, t_b b, t_c c, t_d d, t_e e, t_f f, t_g g ) 
   {
      StartPack( );
      Pack( typeid(a), &a, sizeof(a) );
      Pack( typeid(b), &b, sizeof(b) );
      Pack( typeid(c), &c, sizeof(c) );
      Pack( typeid(d), &d, sizeof(d) );
      Pack( typeid(e), &e, sizeof(e) );
      Pack( typeid(f), &f, sizeof(f) );
      Pack( typeid(g), &g, sizeof(g) );
   }

   void StartPack( void )
   {
      m_Sig = ArgSig( );
      m_Offsets[ 0 ] = 0;
   }

   void Pack( const type_info &type, void *pData, size_t size )
   {
      size_t offset = m_Offsets[ m_Sig.GetCount( ) ];

      Debug::Assert( Condition(offset + size <= sizeof(m_Buffer)), "ArgList buffer size exceeded.  It is %d but needs to be %d", sizeof(m_Buffer), size + offset );
      Debug::Assert( Condition(m_Sig.GetCount( ) < ArgSig::MaxArgCount), "MaxArgCount exceeded for ArgList.  It is %d but needs to be %d", ArgSig::MaxArgCount, m_Sig.GetCount( ) + 1 );
      
      memcpy( m_Buffer + offset, pData, size );
      
      m_Offsets[ m_Sig.GetCount( ) + 1 ] = (uint32) (size + offset);

      m_Sig.AddType( type );
   }

   uint32 GetSize(
      uint32 index
   ) const
   {
      return m_Offsets[ index + 1 ] - m_Offsets[ index ];
   }

   void *GetRawPointer(
      uint32 index
   ) const
   {
      Debug::Assert( Condition(index < m_Sig.GetCount( )), "Index out of range: %d, count: %d", index, m_Sig.GetCount( ) );
      nuint location = *(nuint *) (m_Buffer + m_Offsets[ index ]);
      return (void *) location;
   }

   void *GetRawBuffer(
      uint32 index
   ) const
   {
      Debug::Assert( Condition(index < m_Sig.GetCount( )), "Index out of range: %d, count: %d", index, m_Sig.GetCount( ) );
      return (void *) (m_Buffer + m_Offsets[ index ]);
   }

   template <typename t_a>
   void GetArg(
      uint32 index,
      t_a *pArg
   ) const
   {
      if (index < m_Sig.GetCount( ))
      {
         Debug::Assert( Condition(m_Sig.GetType(index) == typeid(t_a)), "Arg type mismatch" );
         *pArg = *(t_a *) (m_Buffer + m_Offsets[ index ]);
      }
      else
      {
         Debug::Print( Debug::TypeWarning, "Index out of range: %d, count: %d\n", index, m_Sig.GetCount( ) );
      }
   }

   const ArgSig *GetArgSig( void ) const { return &m_Sig; }
};

class ResourceHandle;
class IOutputStream;

class SerializedArgList
{
public:
   ArgList m_ArgList;

public:
   void Destroy( void );
};

class IMethodMap
{
public:
   virtual ~IMethodMap( void )
   {}

   virtual void Method( const char *pMethod, const ArgList &list ) = 0;

   virtual const char *GetName( void ) const = 0;

   virtual IMethodMap *Clone( void ) const = 0;
};

class ISetPropertyMap
{
private:
   char m_Name[ MaxNameLength ];

public:
   virtual ~ISetPropertyMap( void ) {}

   virtual void Property( const ArgList &list ) = 0;

   virtual const char *GetName( void ) const = 0;

   virtual ISetPropertyMap *Clone( void ) const = 0;

protected:
   void SetName( const char *pName ) 
   { 
      String::Copy( m_Name, pName, sizeof(m_Name) );
   }
};

class IGetPropertyMap
{
private:
   char m_Name[ MaxNameLength ];

public:
   virtual ~IGetPropertyMap( void ) {}

   virtual void Property( ArgList &list ) = 0;

   virtual const char *GetName( void ) const = 0;

   virtual IGetPropertyMap *Clone( void ) const = 0;

protected:
   void SetName( const char *pName ) 
   { 
      String::Copy( m_Name, pName, sizeof(m_Name) );
   }
};

class Channel;

class IEventMap
{
public:
   virtual ~IEventMap( void ) {}

   virtual void Event( const Channel *pSender, const ArgList &list ) = 0;

   virtual const char *GetName( void ) const = 0;

   virtual IEventMap *Clone( void ) const = 0;

   virtual bool operator == (IEventMap &rhs) const = 0;
};

#define FunctorTypeDef(variable) void (t_object::*variable) (const char *, const ArgList & )
#define FunctorTypeDefChannel(variable) void (t_object::*variable) (const Channel *pSender, const char *, const ArgList & )

#define FunctorGetTypeDef(variable) void (t_object::*variable) (const char *, ArgList & )
#define FunctorGetTypeDefChannel(variable) void (t_object::*variable) (const Channel *pSender, const char *, ArgList & )

template <typename t_object>
class MethodMap : public IMethodMap
{
private:
   t_object *m_pObject;
   FunctorTypeDef(m_pMethod);
   const char *m_pName;

public:
   MethodMap(
      const char *pName,
      t_object *pObject,
      FunctorTypeDef(pMethod)
   )
   {
      m_pObject = pObject;
      m_pMethod = pMethod;

      m_pName = StringPool::Instance( ).Alloc( pName );
   }

   ~MethodMap( void )
   {
      StringPool::Instance( ).Free( m_pName );
   }

   virtual void Method( const char *pName, const ArgList &list )
   {
      (m_pObject->*m_pMethod)( pName, list );
   }

   const char *GetName( void ) const { return m_pName; }

   virtual IMethodMap *Clone( void ) const
   {
      return new MethodMap( m_pName, m_pObject, m_pMethod );
   }
};

template <typename t_object>
class GetPropertyMap : public IGetPropertyMap
{
private:
   t_object *m_pObject;
   FunctorGetTypeDef(m_pProperty);
   const char *m_pName;

public:
   GetPropertyMap(
      const char *pName,
      t_object *pObject,
      FunctorGetTypeDef(pProperty)
   )
   {
      m_pObject = pObject;
      m_pProperty = pProperty;
      
      m_pName = StringPool::Instance( ).Alloc( pName );
   }

   ~GetPropertyMap( void )
   {
      StringPool::Instance( ).Free( m_pName );
   }

   virtual void Property( ArgList &list )
   {
      (m_pObject->*m_pProperty)( m_pName, list );
   }

   virtual const char *GetName( void ) const { return m_pName; }

   virtual IGetPropertyMap *Clone( void ) const
   {
      return new GetPropertyMap( m_pName, m_pObject, m_pProperty );
   }
};

template <typename t_object>
class SetPropertyMap : public ISetPropertyMap
{
private:
   t_object *m_pObject;
   FunctorTypeDef(m_pProperty);
   const char *m_pName;

public:
   SetPropertyMap(
      const char *pName,
      t_object *pObject,
      FunctorTypeDef(pProperty)
   )
   {
      m_pObject = pObject;
      m_pProperty = pProperty;
      
      m_pName = StringPool::Instance( ).Alloc( pName );
   }

   ~SetPropertyMap( void )
   {
      StringPool::Instance( ).Free( m_pName );
   }

   virtual void Property( const ArgList &list )
   {
      (m_pObject->*m_pProperty)( m_pName, list );
   }

   virtual const char *GetName( void ) const { return m_pName; }

   virtual ISetPropertyMap *Clone( void ) const
   {
      return new SetPropertyMap( m_pName, m_pObject, m_pProperty );
   }
};

template <typename t_object>
class EventMap : public IEventMap
{
private:
   t_object *m_pObject;
   FunctorTypeDefChannel(m_pEvent);
   const char *m_pName;

public:
   EventMap(
      const char *pName,
      t_object *pObject,
      FunctorTypeDefChannel(pEvent)
   )
   {
      m_pObject = pObject;
      m_pEvent  = pEvent;

      m_pName = StringRef(pName);
   }

   ~EventMap( void )
   {
      StringRel( m_pName );
   }

   virtual void Event( const Channel *pSender, const ArgList &list )
   {
      (m_pObject->*m_pEvent)( pSender, m_pName, list );
   }

   virtual const char *GetName( void ) const { return m_pName; }

   virtual IEventMap *Clone( void ) const
   {
      return new EventMap( m_pName, m_pObject, m_pEvent );
   }

   virtual bool operator == (IEventMap &rhs) const
   {
      if ( typeid(*this) != typeid(rhs) ) return false;

      EventMap *pEventMap = (EventMap *) &rhs;
      
      return ( pEventMap->m_pObject == m_pObject &&
               pEventMap->m_pEvent  == m_pEvent &&
               0 == strcmp(m_pName, m_pName) );
   }
};

class NullMethodMap : public IMethodMap
{
public:
   NullMethodMap( void )
   {}

   virtual void Method( const char *pName, const ArgList &list )
   {}

   virtual const char *GetName( void ) const { return ""; }

   virtual IMethodMap *Clone( void ) const
   {
      return new NullMethodMap;
   }
};


