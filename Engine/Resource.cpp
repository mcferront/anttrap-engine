#include "EnginePch.h"

#include "Resource.h"
#include "ResourceWorld.h"

const Id Id::Empty;

ResourceType Resource::_internal_ResourceType;
SerializableType Resource::_internal_SerializableType;

ResourceHandleEnumerator::ResourceHandleEnumerator(const ResourceHandleHash *pHash) :
enumerator(&pHash->m_Hash)
{}
   
ResourceRef::~ResourceRef( void )
{
    MainThreadCheck;

    //assert if the refcount is valid and this isn't a null ref
    Debug::Assert( Condition(0 == m_RefCount || this == GetNullRef( )), "ResourceRef %s being deleted but still has a refcount!", m_Id.ToString( ) );

    if ( NULL != m_pChannel )
    {
        ChannelSystem::Instance( ).Remove( m_pChannel ); 
        m_pChannel->Destroy( );

        delete m_pChannel;
        m_pChannel = NULL;
    }
}

ResourceRef *ResourceRef::GetNullRef( void )
{
    static ResourceRef s_pNullRef(Id("NULL"));
    return &s_pNullRef;
}

Channel *ResourceRef::GetChannel( void )
{
    ScopeLock lock( m_Lock );

    if ( NULL == m_pChannel )
    {
        if ( NULL == m_pResource )
        {
            m_pChannel = ChannelSystem::Instance( ).TryGetChannel( GetId() );
    
            if ( NULL == m_pChannel )
            {
                m_pChannel = new Channel;
                m_pChannel->Create( GetId( ), NULL );
                ChannelSystem::Instance( ).Add( m_pChannel ); 
            }
        }
        else
        {
            m_pChannel = m_pResource->CreateChannel( );
            ChannelSystem::Instance( ).Add( m_pChannel ); 
        }
    }

    return m_pChannel;
}

void ResourceRef::Bind( 
    const char *pName,
    Resource *pResource
    )
{
    pResource->BindRef( this );

    if ( NULL != pName )
    {
        m_pName = StringRef(pName);
        ResourceWorld::Instance( ).UpdateAliasHash( pName, this );
    }
}

ResourceHandle::ResourceHandle(Resource *pResource)
{
    if ( NULL == pResource )
    {
        m_pRef = ResourceRef::GetNullRef( );
        m_pRef->AddRef( );
    }
    else
    {
        m_pRef = ResourceRef::GetNullRef( );
        m_pRef->AddRef( );

        ResourceHandle handle = ResourceWorld::Instance( ).GetHandle( pResource->GetId( ) );
        *this = handle;
    }
}

void ResourceHandle::Bind(
    const char *pName, 
    Resource *pResource
    )
{
    m_pRef->Bind( pName, pResource );
}

Resource *ResourceHandle::TestCast(
    const char *pFile,
    int line,
    const type_info &type,
    bool testCast
    ) const
{
    Debug::Assert( Condition(NULL != m_pRef), "ResourceRef not loaded: caller: %s, %d", pFile, line );

    Debug::Assert( Condition(testCast),
        "Resource cast does not match.  Resource: %s is trying to be cast to %s.  caller: %s, %d",
        typeid(*(m_pRef->GetResourceFromRef())).name( ), type.name( ), pFile, line );

    return m_pRef->GetResourceFromRef( );
}

void Resource::AddToScene( void )
{
    MainThreadCheck;

    Debug::Assert( Condition(NULL != m_pRef), "ResourceRef not loaded" );

    m_pRef->m_pResource = this;
}

void Resource::RemoveFromScene( void )
{
    MainThreadCheck;

    if ( NULL != m_pRef )
      m_pRef->m_pResource = NULL;
}

Channel *Resource::GetChannel( void )
{ 
    ResourceHandle handle( this );
    return handle.GetChannel( );
}

void Resource::Register( void )
{
    ResourceWorld::Instance( ).CreateResourceType( StringRef("Resource"), NULL );

    _internal_ResourceType = LoadResourceType( );
    _internal_SerializableType = LoadSerializableType( );
}

const ResourceType &Resource::LoadResourceType( void )
{
    return ResourceWorld::Instance( ).GetResourceType( StringRef("Resource") );
}

const SerializableType &Resource::LoadSerializableType( void )\
{
    static SerializableType s_Type( "Resource" );
    return s_Type;
}

Channel *Resource::CreateChannel( void )
{
    //the type channel is the base channel so
    //events go to listeners of this instance channel
    //and the type channel
    Channel *pChannel = new Channel;
    pChannel->Create( GetId(), GetType().GetChannel() );

    return pChannel;
}

DefineResourceType(SystemResource, Resource, NULL);

Channel *ResourceType::GetChannel( void ) const
{
    ResourceHandle handle = ResourceWorld::Instance( ).GetTypeHandle( ToString() );
    return GetResource( handle, Resource )->GetChannel( );
}

Channel *ResourceTypeResource::CreateChannel( void )
{
    const ResourceType *pBase = m_EmbeddedType.GetBaseType( );

    Channel *pChannel = new TypeChannel;
    pChannel->Create( GetId(), pBase ? pBase->GetChannel( ) : NULL );

    return pChannel;
}

void ResourceTypeResource::Create(
    const ResourceType &resourceType,
    Channel *pBaseChannel
    )
{
    m_EmbeddedType = resourceType;
}
