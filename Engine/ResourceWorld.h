#pragma once

#include "EngineGlobal.h"
#include "HashTable.h"
#include "ChannelSystem.h"

#define GetResource(handle, type)\
    ((type*)((handle).TestCast( __FILE__, __LINE__, typeid(type), NULL != dynamic_cast<type*>((handle).GetResourceFromHandle(__FILE__, __LINE__)))))

#define IsResourceLoaded(handle)\
    ((handle).IsResourceLoadedToHandle( __FILE__, __LINE__ ))

class ResourceRef;
class ResourceHandleList;

class ResourceWorld
{
public:
    static ResourceWorld &Instance( void );

public:
    typedef List<ResourceRef *> ResourceList;
    typedef HashTable<const char *, ResourceRef *> ResourceHash;

private:
    ResourceList    m_ResourceList;
    ResourceList    m_FrameTickList;
    ResourceHash    m_ResourceHash;
    ResourceHash    m_ResourceTypeHash;
    ResourceHash    m_ResourceAliasHash;

    Channel *m_pChannel;
    Lock     m_Lock;
    bool     m_RunMaintenance;
    bool     m_IsReady;

public:   
    ResourceWorld( void ) { m_IsReady = false; }

    void Create ( void );
    void Destroy( void );

    void CreateResourceType(
        const char *pTypeString,
        const char *pBaseTypeString
        );

    const ResourceType &GetResourceType(
        const char *pType
        );

    ResourceHandle GetHandle(
        Id id
        );

    ResourceHandle GetHandle(
        const char *pId
        );

    ResourceHandle GetHandleFromAlias(
        const char *pAlias
        );

    ResourceHandle GetTypeHandle(
        const char *pTypeString
        );

    void GetHandles(
        ResourceHandleList *pList,
        const char *pSearchString
        );

    void GetHandles(
        ResourceHandleList *pList,
        const ResourceType &type
        );

    void BuildTickList( void );

    void Tick(
        float deltaSeconds
        );

    void PostTick( void );

    void Final( void );

    void UpdateAliasHash(
        const char *pName,
        ResourceRef *pRef
        );

    void RunMaintenance( void ) { m_RunMaintenance = true; }

    Channel *GetChannel( void ) const { return m_pChannel; }

    bool DoesResourceTypeExist( const char *pTypeString ) { return m_ResourceTypeHash.Contains(pTypeString);    }  

    bool IsReady( void ) const { return m_IsReady; }

private:
    void FlushRefs( void );

};
