#include "EnginePch.h"

#include "ResourceWorld.h"
#include "Resource.h"

ResourceWorld &ResourceWorld::Instance( void )
{
    static ResourceWorld s_instance;
    return s_instance;
}

void ResourceWorld::Create( void )
{
    m_ResourceTypeHash.Create( );
    m_ResourceList.Create( );
    m_FrameTickList.Create( );
    m_ResourceHash.Create( );
    m_ResourceAliasHash.Create( );

    ResourceRef::GetNullRef( )->AddRef( );

    m_pChannel = new Channel;
    m_pChannel->Create( Id("ResourceWorld"), NULL );

    ChannelSystem::Instance( ).Add( m_pChannel );

    m_RunMaintenance = false;
    m_IsReady = true;
}

void ResourceWorld::Destroy( void )
{
    List<ResourceRef*> refs;
    refs.Create( );

    {
        //remove resource type resources
        Enumerator<const char *, ResourceRef *> e = m_ResourceTypeHash.GetEnumerator( );

        while ( e.EnumNext( ) )
        {
            refs.Add( e.Data( ) );
        }

        int i, size = refs.GetSize( );

        for ( i = 0; i < size; i++ )
        {
            Resource *pResource = GetResource( ResourceHandle(refs.GetAt(i)), Resource );         
            pResource->RemoveFromScene( );

            pResource->Destroy( );
            delete pResource;
        }

        refs.Clear( );
    }

    {
        //grab a list of all the handles
        Enumerator<const char *, ResourceRef *> e = m_ResourceHash.GetEnumerator( );

        while ( e.EnumNext( ) )
        {
            refs.Add( e.Data( ) );
        }
    }

    m_ResourceHash.Clear( );
    m_ResourceTypeHash.Clear( );

    ResourceRef::GetNullRef( )->Release( );


    //destroy any remaining resource
    //and refs
    int i, size = refs.GetSize( );

    for ( i = 0; i < size; i++ )
    {
        ResourceRef *pRef = refs.GetAt( i );
        Debug::Assert( Condition(pRef->GetRefCount() == 0), "ResourceWorld shutting down but Resource %s %s has a ref count of %d", pRef->GetId( ).ToString( ), pRef->GetName(), pRef->GetRefCount( ) );
        Debug::Assert( Condition(NULL == pRef->GetResourceFromRef()), "ResourceWorld shutting down but Resource %s %s is still in use", pRef->GetId( ).ToString( ), pRef->GetName() );

        delete pRef;
    }

    refs.Destroy( );

    ChannelSystem::Instance( ).Remove( m_pChannel );

    m_pChannel->Destroy( );
    delete m_pChannel;

    m_ResourceAliasHash.Destroy( );
    m_ResourceTypeHash.Destroy( );
    m_ResourceList.Destroy( );
    m_FrameTickList.Destroy( );
    m_ResourceHash.Destroy( );
}

void ResourceWorld::CreateResourceType(
    const char *pTypeString,
    const char *pBaseTypeString
    )
{
    pTypeString = StringRef(pTypeString);
    pBaseTypeString = StringRef(pBaseTypeString);

    Debug::Assert( Condition(false == DoesResourceTypeExist(pTypeString)), "Resource Type %s already exists", pTypeString );

    const ResourceType *pBaseType = NULL;
    Channel *pBaseChannel = NULL;

    if ( NULL != pBaseTypeString )
    {
        pBaseType = &GetResourceType( pBaseTypeString );
        pBaseChannel = pBaseType->GetChannel( );
    }

    ResourceType resourceType( pTypeString, pBaseType );

    ResourceTypeResource *pTypeResource = new ResourceTypeResource;
    pTypeResource->Create( resourceType, pBaseChannel );

    ResourceHandle handle = GetHandle( Id(pTypeString) );
    handle.Bind( NULL, pTypeResource );
    pTypeResource->AddToScene();

    m_ResourceTypeHash.Add( pTypeResource->GetType( ).ToString( ), handle.m_pRef );
}

const ResourceType &ResourceWorld::GetResourceType(
    const char *pType
    )
{
    return GetResource( GetTypeHandle(pType), Resource )->GetType( );
}

ResourceHandle ResourceWorld::GetHandle(
    const char *pId
    )
{
    return GetHandle( Id(pId) );
}


ResourceHandle ResourceWorld::GetHandle(
    Id id
    )
{
    ScopeLock lock( m_Lock );

    ResourceRef *pRef;

    if ( false == m_ResourceHash.Get(id.ToString( ), &pRef) )
    {
        pRef = new ResourceRef( id );
        m_ResourceHash.Add( pRef->GetId( ).ToString( ), pRef );
        m_ResourceList.Add( pRef );
    }

    return ResourceHandle( pRef );
}

ResourceHandle ResourceWorld::GetHandleFromAlias(
    const char *pAlias
    )
{
    ScopeLock lock( m_Lock );

    ResourceRef *pRef;

    pAlias = StringRef(pAlias);

    ResourceHandle handle;

    if ( m_ResourceAliasHash.Get( pAlias, &pRef ) )
       handle = ResourceHandle( pRef );
    else
    {
       Debug::Print( Debug::TypeWarning, "Resource %s never loaded, not even once\n", pAlias );
       handle = NullHandle;
    }

    StringRel( pAlias );
    
    return handle;    
}

ResourceHandle ResourceWorld::GetTypeHandle(
    const char *pTypeString
    )
{
    ScopeLock lock( m_Lock );

    ResourceRef *pRef;
    bool success;

    success = m_ResourceTypeHash.Get( pTypeString, &pRef );
    Debug::Assert( Condition(success), "ResourceType %s has not been added to ResourceManager", pTypeString );

    return ResourceHandle( pRef );
}

void ResourceWorld::GetHandles(
    ResourceHandleList *pList,
    const char *pSearchString
    )
{
    ScopeLock lock( m_Lock );

    size_t length = strlen(pSearchString);

    //grab a list of all the handles
    Enumerator<const char *, ResourceRef *> e = m_ResourceHash.GetEnumerator( );

    while ( e.EnumNext( ) )
    {
        if ( 0 == strncmp(pSearchString, e.Key( ), length) )
        {
            pList->Add( ResourceHandle(e.Data( )) );
        }
    }
}

void ResourceWorld::GetHandles(
    ResourceHandleList *pList,
    const ResourceType &type
    )
{
    ScopeLock lock( m_Lock );

    Enumerator<const char *, ResourceRef *> e = m_ResourceHash.GetEnumerator( );

    while ( e.EnumNext( ) )
    {
        if ( NULL != e.Data( )->GetResourceFromRef( ) )
        {
            if ( e.Data( )->GetResourceFromRef( )->GetType( ).IsTypeOf(type) )
            {
                //filter out the ResourceTypeResource which
                //will return the same type
                ResourceRef *pTypeRef;
                m_ResourceTypeHash.Get( e.Data( )->GetResourceFromRef( )->GetType( ).ToString(), &pTypeRef );

                if (pTypeRef != e.Data())
                {
                    pList->Add( ResourceHandle(e.Data( )) );
                }
            }
        }
    }
}

void ResourceWorld::BuildTickList( void )
{
    MainThreadCheck;

    //copy only valid resources to be ticked to a 
    //list for tick/posttick/final this frame
    //
    //by using a copy of only valid ones
    //it will prevent resources
    //from being added, removed or rebound with a handle
    //in the middle of a tick, posttick or final
    uint32 i, size  = m_ResourceList.GetSize( );

    m_FrameTickList.Clear( );

    for ( i = 0; i < size; i++ )
    {
        ResourceRef *pRef = m_ResourceList.GetAt( i );
        if ( pRef->GetResourceFromRef() && pRef->GetResourceFromRef()->IsTickable() ) m_FrameTickList.Add( pRef );
    }
}

void ResourceWorld::Tick(
    float deltaSeconds                        
    )
{
    MainThreadCheck;

    uint32 i, size = m_FrameTickList.GetSize( );

    for ( i = 0; i < size; i++ )
    {
        ResourceRef *pRef = m_FrameTickList.GetAt( i );
        if ( pRef->GetResourceFromRef() ) pRef->GetResourceFromRef()->Tick( deltaSeconds );
    }
}

void ResourceWorld::PostTick( void )
{
    MainThreadCheck;

    uint32 i, size = m_FrameTickList.GetSize( );

    for ( i = 0; i < size; i++ )
    {
        ResourceRef *pRef = m_FrameTickList.GetAt( i );
        if ( pRef->GetResourceFromRef() ) pRef->GetResourceFromRef()->PostTick( );
    }
}

void ResourceWorld::Final( void )
{
    uint32 i, size = m_FrameTickList.GetSize( );

    for ( i = 0; i < size; i++ )
    {
        ResourceRef *pRef = m_FrameTickList.GetAt( i );
        if ( pRef->GetResourceFromRef() ) pRef->GetResourceFromRef()->Final( );
    }

    m_FrameTickList.Clear( );

    if ( true == m_RunMaintenance )
    {
        FlushRefs( );

        m_RunMaintenance = false;
    }
}

void ResourceWorld::UpdateAliasHash(
    const char *pName,
    ResourceRef *pRef
    )
{
    ScopeLock lock( m_Lock );

    pName = StringRef(pName);

    m_ResourceAliasHash.Remove( pName );
    m_ResourceAliasHash.Add( pName, pRef );
}

void ResourceWorld::FlushRefs( void )
{
    MainThreadCheck;
    ScopeLock lock( m_Lock );

    //use the frame tick list to flush resoure refs because it should be
    //clear when running maintenance and then we don't have to allocate another list
    Debug::Assert( Condition(m_FrameTickList.GetSize() == 0), "Flushing refs but resources are still in the frame's tick list" );

    {
        //grab a list of all the handles
        Enumerator<const char *, ResourceRef *> e = m_ResourceHash.GetEnumerator( );

        while ( e.EnumNext( ) )
        {
            m_FrameTickList.Add( e.Data( ) );
        }
    }

    int i, size = m_FrameTickList.GetSize( );

    for ( i = 0; i < size; i++ )
    {
        ResourceRef *pRef = m_FrameTickList.GetAt( i ); 

        //if the ref has a channel which has event listeners
        //don't delete it or they would need to reregister
        //(don't call GetChannel in the if because that will
        //create a channel)
        if ( NULL != pRef->m_pChannel )
        {
            if ( true == pRef->GetChannel( )->HasEventListeners( ) )
            {
                continue;
            }
        }

        //channel is not in use, so delete it if no one else is using us
        if ( 0 == pRef->GetRefCount( ) && NULL == pRef->GetResourceFromRef( ) )
        {
            m_ResourceHash.Remove( pRef->GetId( ).ToString( ) );
            m_ResourceList.RemoveSorted( pRef );

            delete pRef;
        }
    }

    m_FrameTickList.Clear( );
}
