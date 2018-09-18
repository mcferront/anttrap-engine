#pragma once

#include "EngineGlobal.h"
#include "Resource.h"
#include "ScriptInstance.h"

class Component;

class ScriptInstance
{
private:
    Component     *m_pComponent;
    ResourceHandle m_Handle;

public:
    ScriptInstance(
        Component *pComponent,
        ResourceHandle script
        )
    {
        m_Handle  = script;
        m_pComponent = pComponent;
    }

    virtual ~ScriptInstance( void )
    {
        m_Handle = NullHandle;
    }

    Component     *GetComponent( void )    const { return m_pComponent; }
    ResourceHandle GetScriptHandle( void ) const { return m_Handle; }

    virtual void Bind( void ){};
    
    virtual void Load( void ){};
    virtual void Unload( void ){};

    virtual void Tick(
        float deltaSeconds
        ){};

    virtual void PostTick( void ) {};

    virtual void Final( void ) {};

    virtual void EditorRender( void ) {};

    virtual void OnEvent(
        const char *pScriptMethod,
        const Channel *pSender,
        const char *pEvent,
        const ArgList &list
        ){}

    virtual const char *GetObjectName( void ) const { return NULL; }
};
