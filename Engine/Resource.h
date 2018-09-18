#pragma once

#include "EngineGlobal.h"
#include "HashTable.h"
#include "ResourceMaps.h"
#include "SystemId.h"
#include "Channel.h"
#include "ResourceType.h"
#include "Serializer.h"
#include "Identifiable.h"
#include "ResourceWorld.h"

//helper macros to create standard resource type declarations
//and register resources with ResourceWorld and Serializer loaders
#define DeclareResourceType(className)\
    virtual const ResourceType &GetType( void ) const { return className::StaticType( ); }\
    virtual const SerializableType &GetSerializableType( void ) const { return className::StaticSerializableType( ); }\
    static const ResourceType &LoadResourceType( void ); \
    static const SerializableType &LoadSerializableType( void ); \
    static void Register( void ); \
    static ResourceType _internal_ResourceType; \
    static SerializableType _internal_SerializableType; \
    static const ResourceType &StaticType( void )\
{\
    return _internal_ResourceType; \
}\
    static const SerializableType &StaticSerializableType( void )\
{\
    return _internal_SerializableType; \
}

//copy serializerAddr here incase it is a memory allocation (new ISerializer)
#define DefineResourceType(className, baseClassName, serializerAddr)\
    ResourceType className::_internal_ResourceType; \
    SerializableType className::_internal_SerializableType; \
    void className::Register( void )\
{\
    TypeRegistry_RegisterTypeInfo( className ); \
    ISerializer *pSerializer = serializerAddr; \
    ResourceWorld::Instance( ).CreateResourceType( StringRef(#className), StringRef(#baseClassName) ); \
    _internal_ResourceType = LoadResourceType( ); \
    _internal_SerializableType = LoadSerializableType( ); \
if ( pSerializer ) SerializerWorld::Instance( ).AddISerializer( TypeRegistry::GetTypeInfo( typeid( className* ) ), pSerializer ); \
}\
    const ResourceType &className::LoadResourceType( void )\
{\
    return ResourceWorld::Instance( ).GetResourceType( StringRef(#className) ); \
}\
    const SerializableType &className::LoadSerializableType( void )\
{\
    static SerializableType s_Type( #className ); \
    return s_Type; \
}

class Resource;
class Channel;
class ArgList;

class ResourceRef
{
   friend class Resource;
   friend class ResourceHandle;
   friend class ResourceWorld;

private:
   static ResourceRef *GetNullRef( void );

private:
   Id          m_Id;
   const char *m_pName;
   Resource   *m_pResource;
   Channel    *m_pChannel;
   uint32      m_RefCount;
   Lock        m_Lock;

public:
   ResourceRef(
      Id id
   )
   {
      m_RefCount = 0;
      m_pResource = NULL;
      m_Id = id;
      m_pChannel = NULL;
      m_pName = NULL;
   }

   ~ResourceRef( void );

   void AddRef( void )
   {
      AtomicIncrement( &m_RefCount );
   }

   void Release( void )
   {
      int r = (int) AtomicDecrement( &m_RefCount );
      Debug::Assert( Condition( r >= 0 ), "ResourceRef %s(%s) being released but refcount was already 0", m_Id.ToString( ), m_pName ? m_pName : "" );
   }

   Channel *GetChannel( void );

   uint32 GetRefCount( void ) { return m_RefCount; }
   Id GetId( void ) { return m_Id; }

   const char *GetName( void ) const { return m_pName; }

   Resource *GetResourceFromRef( void ) const { return m_pResource; }

private:
   void Bind(
      const char *pName,
      Resource *pResource
   );
};

class ResourceHandle
{
   friend class ResourceWorld;

public:
   ResourceHandle( const ResourceHandle &rhs )
   {
      m_pRef = ResourceRef::GetNullRef( );
      m_pRef->AddRef( );
      *this = rhs;
   }

   ResourceHandle( const char *pId )
   {
      if ( NULL == pId )
      {
         m_pRef = ResourceRef::GetNullRef( );
         m_pRef->AddRef( );
      }
      else
      {
         m_pRef = ResourceRef::GetNullRef( );
         m_pRef->AddRef( );

         ResourceHandle handle = ResourceWorld::Instance( ).GetHandle( Id( pId ) );
         *this = handle;
      }
   }

   ResourceHandle( Id id )
   {
      m_pRef = ResourceRef::GetNullRef( );
      m_pRef->AddRef( );

      ResourceHandle handle = ResourceWorld::Instance( ).GetHandle( id );
      *this = handle;
   }

   ResourceHandle( Resource *pResource );

   ResourceHandle( int null )
   {
      Debug::Assert( Condition( 0 == null ), "ResourceHandle has invalid value (%d) instead of null", null );

      m_pRef = ResourceRef::GetNullRef( );
      m_pRef->AddRef( );
   }

   ResourceHandle( void )
   {
      m_pRef = ResourceRef::GetNullRef( );
      m_pRef->AddRef( );
   }

   ~ResourceHandle( void )
   {
      m_pRef->Release( );
   }

   ResourceHandle & operator = ( const ResourceHandle &rhs )
   {
      m_pRef->Release( );
      m_pRef = rhs.m_pRef;

      m_pRef->AddRef( );
      return *this;
   }

   const char *GetName( void ) const { return m_pRef->GetName( ); }

   void Bind(
      const char *pName,
      Resource *pResource
   );

   void AddRef( void )
   {
      m_pRef->AddRef( );
   }

   void Release( void )
   {
      m_pRef->Release( );
   }

   Resource *GetResourceFromHandle(
      const char *pFile,
      int line
   )  const
   {
      Debug::Assert( Condition( NULL != this ), "ResourceHandle not loaded: caller: %s, %d", pFile, line );
      Debug::Assert( Condition( NULL != m_pRef ), "ResourceRef not loaded: caller: %s, %d", pFile, line );
      Debug::Assert( Condition( NULL != m_pRef->GetResourceFromRef( ) ), "Resource %s %s not loaded: caller: %s, %d", m_pRef->GetId( ).ToString( ), m_pRef->GetName( ), pFile, line );

      return m_pRef->GetResourceFromRef( );
   }

   Resource *TestCast(
      const char *pFile,
      int line,
      const type_info &type,
      bool testCast
   ) const;

   bool IsResourceLoadedToHandle(
      const char *pFile,
      int line
   ) const
   {
      Debug::Assert( Condition( NULL != this ), "ResourceHandle not loaded: caller: %s, %d", pFile, line );
      Debug::Assert( Condition( NULL != m_pRef ), "ResourceRef not loaded: caller: %s, %d", pFile, line );

      return NULL != m_pRef->GetResourceFromRef( );
   }

   Channel *GetChannel( void )
   {
      Debug::Assert( Condition( NULL != this ), "ResourceHandle not loaded" );
      Debug::Assert( Condition( NULL != m_pRef ), "ResourceRef not loaded" );

      return m_pRef->GetChannel( );
   }

   Id GetId( void ) const
   {
      Debug::Assert( Condition( NULL != m_pRef ), "ResourceRef not loaded" );
      return m_pRef->GetId( );
   }

   const char *ToString( void ) const
   {
      Debug::Assert( Condition( NULL != m_pRef ), "ResourceRef not loaded" );
      return m_pRef->GetId( ).ToString( );
   }

   bool operator == ( const ResourceHandle &rhs ) const
   {
      return m_pRef == rhs.m_pRef;
   }

   bool operator != ( const ResourceHandle &rhs ) const
   {
      return m_pRef != rhs.m_pRef;
   }

public:
   static ResourceHandle FromAlias(
      const char *pAlias
   )
   {
      return ResourceWorld::Instance( ).GetHandleFromAlias( pAlias );
   }

private:
   ResourceHandle(
      ResourceRef *pRef
   )
   {
      m_pRef = pRef;
      m_pRef->AddRef( );
   }

private:
   ResourceRef *m_pRef;
};


class Resource : public ISerializable
{
   friend class ResourceWorld;
   friend class ResourceRef;

public:
   DeclareResourceType( Resource );

public:
   Resource( void )
   {
      m_IsTickable = false;
      m_pRef = NULL;
   }

   virtual void Destroy( void )
   {
      if ( NULL != m_pRef )
      {
         m_pRef->m_pResource = NULL;
         m_pRef->Release( );
      }
   }

   virtual void Bind( void ) {}

   virtual void AddToScene( void );
   virtual void RemoveFromScene( void );

   virtual void Tick(
      float deltaSeconds
   ) {}

   virtual void PostTick( void ) {}
   virtual void Final( void ) {}

   virtual void EditorRender( void ) {}

   Channel *GetChannel( void );

   const char *GetName( void ) const { return m_pRef->GetName( ); }
   Id GetId( void ) const { return m_pRef->GetId( ); }
   ResourceHandle GetHandle( void ) const { return ResourceHandle( m_pRef->GetId( ) ); }
   void SetTickable( bool tickable ) { m_IsTickable = tickable; }
   bool IsTickable( void ) const { return m_IsTickable; }

   bool HasRef( void ) const { return NULL != m_pRef; }

protected:
   virtual Channel *CreateChannel( void );

private:
   void BindRef(
      ResourceRef *pRef
   )
   {
      if ( NULL != m_pRef )
         m_pRef->Release( );

      m_pRef = pRef;
      m_pRef->AddRef( );
   }
private:
   bool m_IsTickable;
   ResourceRef *m_pRef;
};

class SystemResource : public Resource
{
public:
   DeclareResourceType( SystemResource );

};

class ResourceTypeResource : public Resource
{
private:
   ResourceType m_EmbeddedType;

public:
   virtual void Create(
      const ResourceType &resourceType,
      Channel *pBaseChannel
   );

   virtual const ResourceType &GetType( void ) const { return m_EmbeddedType; }

protected:
   virtual Channel *CreateChannel( void );
};


typedef List<Id>  IdList;
typedef List<Resource*> ResourceList;


class ResourceHandleList
{
private:
   List<ResourceHandle> m_List;

public:
   ResourceHandleList( ) {}

   void Create( void )
   {
      m_List.Create( );
   }

   void CopyFrom( const ResourceHandleList &copyFrom )
   {
      Clear( );

      uint32 i, size = copyFrom.GetSize( );

      for ( i = 0; i < size; i++ )
         Add( copyFrom.Get( i ) );
   }

   void CopyFrom( const ResourceHandle handles[], uint32 count )
   {
      Clear( );

      for ( uint32 i = 0; i < count; i++ )
         Add( handles[i] );
   }

   void Destroy( void )
   {
      Clear( );
      m_List.Destroy( );
   }

   void Clear( void )
   {
      int i, size = GetSize( );

      for ( i = 0; i < size; i++ )
      {
         ResourceHandle handle = Get( i );
         handle.Release( );
      }

      m_List.Clear( );
   }

   uint32 GetSize( void ) const
   {
      return m_List.GetSize( );
   }

   int Add(
      ResourceHandle handle
   )
   {
      handle.AddRef( );
      return m_List.Add( handle );
   }

   int AddUnique(
      ResourceHandle handle
   )
   {

      int index = m_List.AddUnique( handle );

      if ( List<ResourceHandle>::InvalidIndex != index )
         handle.AddRef( );

      return index;
   }

   void Insert(
      ResourceHandle handle,
      uint32 indexPosition
   )
   {
      handle.AddRef( );
      m_List.Insert( handle, indexPosition );
   }

   void Replace(
      uint32 index,
      ResourceHandle value
   )
   {
      ResourceHandle handle = m_List.GetAt( index );
      handle.Release( );

      value.AddRef( );
      m_List.Replace( index, value );
   }

   void Remove(
      ResourceHandle handle
   )
   {
      Remove( m_List.GetIndex( handle ) );
   }

   void Remove(
      uint32 index
   )
   {
      ResourceHandle handle = m_List.GetAt( index );
      handle.Release( );

      m_List.Remove( index );
   }

   void RemoveSorted(
      ResourceHandle handle
   )
   {
      RemoveSorted( m_List.GetIndex( handle ) );
   }

   void RemoveSorted(
      uint32 index
   )
   {
      ResourceHandle handle = m_List.GetAt( index );
      handle.Release( );

      m_List.RemoveSorted( index );
   }

   ResourceHandle Get(
      uint32 index
   ) const
   {
      return m_List.GetAt( index );
   }

   uint32 GetIndex(
      ResourceHandle handle
   ) const
   {
      return m_List.GetIndex( handle );
   }
};

class ResourceHandleHash;

class ResourceHandleEnumerator
{
   friend class ResourceHandleHash;

public:
   bool EnumNext( void ) { return enumerator.EnumNext( ); }

   Id Key( void ) { return enumerator.Key( ); }
   const ResourceHandle Data( void ) { return ResourceHandle( enumerator.Data( ) ); }

private:
   ResourceHandleEnumerator( const ResourceHandleHash *pHash );

private:
   Enumerator<Id, Id> enumerator;
};

//behind the scenes... hash system ids instead of resource handles
//because resource handle refcounting doesn't work
//with the hash table's memcpy and memory pools
class ResourceHandleHash
{
   friend class ResourceHandleEnumerator;

private:
   HashTable<Id, Id> m_Hash;

public:
   ResourceHandleHash( )
   {
      m_Hash.Create( 16, 16, IdHash, IdCompare );
   }

   ~ResourceHandleHash( )
   {
      m_Hash.Destroy( );
   }

   void Clear( void )
   {
      m_Hash.Clear( );
   }

   void Copy(
      const ResourceHandleHash &copyFrom
   )
   {
      m_Hash.Copy( copyFrom.m_Hash );
   }

   ResourceHandleEnumerator GetEnumerator( void ) const
   {
      return ResourceHandleEnumerator( this );
   }

   bool Get(
      Id id,
      ResourceHandle *pHandle
   ) const
   {
      bool result = m_Hash.Get( id, &id );

      if ( true == result )
      {
         *pHandle = ResourceHandle( id );
      }

      return result;
   }

   bool Contains(
      Id id
   ) const
   {
      return m_Hash.Contains( id );
   }

   void Add(
      Id id,
      const ResourceHandle &handle
   )
   {
      m_Hash.Add( id, handle.GetId( ) );
   }

   void Remove(
      Id id
   )
   {
      m_Hash.Remove( id );
   }
};

