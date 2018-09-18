#pragma once

#include "EngineGlobal.h"
#include "Serializer.h"
#include "Resource.h"

class ResourceHandle;

class Database
{
private:
    struct AssetRef
    {
        Id id;
        Resource *pResource;
        int refCount;
    };

    static HashTable<Id, AssetRef*> s_Assets;
    static List<Database*> s_Databases;

public:
    static Resource *RemoveResource(
        Id id
        )
    {
        AssetRef *pRef;
        Resource *pResource = NULL;

        if ( true == s_Assets.Get(id, &pRef) )
        {
            pResource = pRef->pResource;
            pRef->pResource = NULL;
        }

        return pResource;
    }

    static void BeginInstancing( void );
    static void EndInstancing  ( void );

    static void ReloadResourcesOfType(
        const char *pPath,
        const ResourceType &type
        );

    static void ReloadResourcesOfType(
        IInputStream *pInputStream,
        const ResourceType &type
        );

    static Database *Create( 
        bool persistent = false 
        )
    {
        return new Database(persistent);
    }

    static void Destroy( Database *pDatabase )
    {
        delete pDatabase;
    }

    static void UnloadDatabases( 
        bool keepPersistent 
        )
    {
        List<Database*> databases;
        databases.Create( );
        databases.CopyFrom( s_Databases );

        {
            List<Database*>::Enumerator e = databases.GetEnumerator( );

            while ( e.EnumNext( ) )
            {
                if (true == keepPersistent &&
                    true == e.Data()->m_Persistent)
                    continue;

                e.Data()->UnloadResources( );
                Database::Destroy( e.Data( ) );
            }
        }

        databases.Destroy( );
    }

public:   
    struct HeaderTable
    {
        Id id;
        Id name;
        uint32 startPosition;
        uint32 size;
    };

private:
    HeaderTable            *m_pHeaders;
    uint32                  m_NumHeaders;
    List<AssetRef*>         m_Resources;
    HashTable<Id, int>      m_RequestedIds;
    bool                    m_RequestAll;
    bool                    m_Persistent;

public:
    void RequestAll( void )
    {
        m_RequestAll = true;
    }

    void RequestIds(
        IdList requestedIds
        )
    {
        int i, count = requestedIds.GetSize( );

        for ( i = 0; i < count; i++ )
        {  
            if ( false == m_RequestedIds.Contains(requestedIds.GetAt(i)) )
            {
                m_RequestedIds.Add( requestedIds.GetAt(i), 0 );
            }
        }
    }

    bool RequestResource(
        const Id &id
        )
    {
        if ( false == m_RequestedIds.Contains(id) )
        {
            m_RequestedIds.Add( id, 0 );
            return true;
        }

        return false;
    }

    bool GetResourceLocation(
        Id resource,
        uint32 *pOffset,
        uint32 *pSize
        );

    void LoadResources(
        const char *pPath
        );

    bool Deserialize(
        IInputStream *pInputStream
        );

    void UnloadResources( void );

    void GetResources(
        const ResourceType &type,
        IdList *pList
        );

private:
    Database( 
        bool persistent 
        ) 
    {
        m_RequestedIds.Create( 16, 16, IdHash, IdCompare );
        m_Resources.Create( );

        m_pHeaders = NULL;
        m_RequestAll = false;
    
        s_Databases.Add( this );
    
        m_Persistent = persistent;
    }

    ~Database( void )
    {
        s_Databases.Remove( this );

        free( m_pHeaders );

        m_RequestedIds.Destroy( );
        m_Resources.Destroy( );
    }

    Resource *AddResource(
        IInputStream *pInputStream
        );

private:
    static HeaderTable *ReadHeaders(
        uint32 count,
        IInputStream *pInputStream
        );

    static Resource *DeserializeResource(
        IInputStream *pInputStream
        );  
};
