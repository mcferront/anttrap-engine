#include "EnginePch.h"

#include "Database.h"
#include "IOStreams.h"
#include "ResourceWorld.h"
#include "Asset.h"
#include "MemoryStreams.h"
#include "FileStreams.h"
#include "CompressedStreams.h"
#include "UtilityClock.h"

HashTable<Id, Database::AssetRef*> Database::s_Assets;
List<Database*> Database::s_Databases;

void Database::BeginInstancing( void )
{
    s_Databases.Create( );
    s_Assets.Create( 16, 16, IdHash, IdCompare );
}

void Database::EndInstancing( void )
{
    s_Assets.Destroy( );
    s_Databases.Destroy( );
}

void Database::ReloadResourcesOfType(
    const char *pPath,
    const ResourceType &type
    )
{
    Debug::Print( Debug::TypeInfo, "Reloading Types %s\n", type.ToString( ) );

    FileInputStream stream;

    if ( stream.Open( pPath ) )
        ReloadResourcesOfType( &stream, type );
    else
        Debug::Assert( Condition( false ), "Could not load %s", pPath );
}

void Database::ReloadResourcesOfType(
    IInputStream *pInputStream,
    const ResourceType &type
    )
{
    uint32 i, count, compressed;

    pInputStream->Read( &compressed, sizeof( compressed ), NULL );
    pInputStream->Read( &count, sizeof( count ), NULL );

    HeaderTable *pHeaders = ReadHeaders( count, pInputStream );

    CompressedInputStream compressedStream( NULL );

    uint32 assetStartPos = pInputStream->Seek( 0, SeekCurrent );

    Clock clock;
    clock.Start( );

    for ( i = 0; i < count; i++ )
    {
        HeaderTable *pHeader = &pHeaders[ i ];

        AssetRef *pAssetRef;

        if ( true == s_Assets.Get( Id( pHeader->id ), &pAssetRef ) )
        {
            Resource *pResource = NULL;

            bool resourceBound = false;

            if ( pAssetRef->pResource && pAssetRef->pResource->GetType( ) == type )
            {
                resourceBound = IsResourceLoaded( pAssetRef->pResource->GetHandle( ) );

                pAssetRef->pResource->RemoveFromScene( );
                pAssetRef->pResource->Destroy( );
                pAssetRef->pResource = NULL;

                pInputStream->Seek( pHeader->startPosition + assetStartPos, SeekBegin );

                if ( 0 != compressed )
                {
                    compressedStream.Bind( pInputStream, pHeader->size );
                    pResource = DeserializeResource( &compressedStream );
                }
                else
                    pResource = DeserializeResource( pInputStream );

                pAssetRef->pResource = pResource;

                if ( true == resourceBound && NULL != pAssetRef->pResource )
                {
                    pAssetRef->pResource->AddToScene( );
                    pAssetRef->pResource->Bind( );
                }
            }
        }
    }

    float result = clock.TestSample( );
    Debug::Print( Debug::TypeInfo, "Database ReloadResourcesOfType in %.4f seconds\n", result );

    free( pHeaders );
}

bool Database::GetResourceLocation(
    Id resource,
    uint32 *pOffset,
    uint32 *pSize
    )
{
    for ( uint32 i = 0; i < m_NumHeaders; i++ )
    {
        if ( Id( m_pHeaders[ i ].id ) == resource )
        {
            *pOffset = m_pHeaders[ i ].startPosition;
            *pSize = m_pHeaders[ i ].size;

            return true;
        }
    }

    return false;
}

void Database::LoadResources(
    const char *pPath
    )
{
    Debug::Print( Debug::TypeInfo, "Loading Database\n" );

    FileInputStream stream;

    if ( stream.Open( pPath ) )
        Deserialize( &stream );
    else
        Debug::Assert( Condition( false ), "Could not load %s", pPath );
}

bool Database::Deserialize(
    IInputStream *pInputStream
    )
{
    Clock clock;
    clock.Start( );

    uint32 i, count, compressed;

    pInputStream->Read( &compressed, sizeof( compressed ), NULL );
    pInputStream->Read( &count, sizeof( count ), NULL );

    CompressedInputStream compressedStream( NULL );

    HeaderTable *pHeaders = ReadHeaders( count, pInputStream );

    List<Resource*> newResources;
    newResources.Create( );

    for ( i = 0; i < count; i++ )
    {
        HeaderTable *pHeader = &pHeaders[ i ];

        if ( true == m_RequestAll || m_RequestedIds.Contains( Id( pHeader->id ) ) )
        {
            AssetRef *pAssetRef;

            ResourceHandle handle( pHeader->id );

            if ( false == s_Assets.Get( pHeader->id, &pAssetRef ) )
            {
                Resource *pResource = NULL;

                pInputStream->Seek( pHeader->startPosition, SeekBegin );

                if ( 0 != compressed )
                {
                    compressedStream.Bind( pInputStream, pHeader->size );
                    pResource = DeserializeResource( &compressedStream );
                }
                else
                    pResource = DeserializeResource( pInputStream );

                handle.Bind( pHeader->name.ToString(), pResource );
                pResource->AddToScene( );

                newResources.Add( pResource );
                
                pAssetRef = (AssetRef *) malloc( sizeof( AssetRef ) );
                pAssetRef->refCount = 0;
                pAssetRef->id = Id( pHeader->id );
                pAssetRef->pResource = pResource;

                s_Assets.Add( pAssetRef->id, pAssetRef );
            }

            if ( pAssetRef->pResource )
                m_Resources.Add( pAssetRef );

            pAssetRef->refCount++;
        }
    }

    {
        List<Resource*>::Enumerator e = newResources.GetEnumerator( );

        while ( e.EnumNext( ) )
            e.Data( )->Bind( );
    }

    newResources.Destroy( );

    free( pHeaders );

    float result = clock.TestSample( );

    Debug::Print( Debug::TypeInfo, "Database Loaded in %.4f seconds\n", result );

    return true;
}

void Database::UnloadResources( void )
{
    uint32 i, count = m_Resources.GetSize( );

    for ( i = 0; i < count; i++ )
    {
        //pointer to the asset we want to remove
        AssetRef *pAssetRef = m_Resources.GetAt( i );

        pAssetRef->refCount--;

        if ( 0 == pAssetRef->refCount )
        {
            if ( NULL != pAssetRef->pResource )
            {
                pAssetRef->pResource->RemoveFromScene( );
                pAssetRef->pResource->Destroy( );
                delete pAssetRef->pResource;
            }

            s_Assets.Remove( pAssetRef->id );
            free( pAssetRef );
        }
    }

    m_Resources.Clear( );
}

void Database::GetResources(
   const ResourceType &type,
   IdList *pList
)
{
   uint32 i, count = m_Resources.GetSize( );

   for ( i = 0; i < count; i++ )
   {
      AssetRef *pAssetRef = m_Resources.GetAt( i );

      if ( pAssetRef->pResource->GetType() == type)
         pList->Add( pAssetRef->id );
   }
}

Database::HeaderTable *Database::ReadHeaders(
    uint32 count,
    IInputStream *pInputStream
    )
{
    HeaderTable *pHeaders = (HeaderTable *) malloc( count * sizeof( HeaderTable ) );

    uint32 i;

    for ( i = 0; i < count; i++ )
    {
        pHeaders[ i ].id = Id::Deserialize( pInputStream );
        pHeaders[ i ].name = Id::Deserialize( pInputStream );
        pInputStream->Read( &pHeaders[ i ].startPosition, sizeof( pHeaders[ i ].startPosition ) );
        pInputStream->Read( &pHeaders[ i ].size, sizeof( pHeaders[ i ].size ) );
    }

    return pHeaders;
}

Resource *Database::DeserializeResource(
    IInputStream *pInputStream
    )
{
    Serializer serializer( pInputStream );

    Resource *pResource = (Resource *) serializer.Deserialize( NULL );

    return pResource;
}

