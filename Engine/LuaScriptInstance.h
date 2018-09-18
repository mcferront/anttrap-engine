#pragma once

#include "EngineGlobal.h"
#include "ResourceWorld.h"
#include "ScriptInstance.h"
#include "LuaObject.h"

class LuaArgList
{
public:
    ArgList list;

    LuaArgList() {};

    void AddString(
        const char *pString
    ) 
    {
        list.Pack(typeid(const char *), &pString, sizeof(pString));
    }
    
    void AddFloat(
        float value
    ) 
    {
        list.Pack(typeid(float), &value, sizeof(value));
    }
    
    void AddInt(
        int value
    ) 
    {
        list.Pack(typeid(int), &value, sizeof(value));
    }

    void AddBool(
        bool value
    ) 
    {
        list.Pack(typeid(bool), &value, sizeof(value));
    }

    void AddVector(
        Vector value
    ) 
    {
        list.Pack(typeid(Vector), &value, sizeof(value));
    }

    void AddResourceHandle(
        ResourceHandle value
    ) 
    {
        list.Pack(typeid(ResourceHandle), &value, sizeof(value));
    }

    ArgList *GetList( ) { return &list; }
};

class LuaScriptInstance : public ScriptInstance
{
private:
    LuaContext    *m_pLuaContext;
    LuaObject     *m_pLuaObject;
    bool           m_NeedsCreate;
    bool           m_ReadyForTick;
    bool           m_ScriptOk;

public:
    LuaScriptInstance(
        Component *pParent,
        ResourceHandle script
    );

    virtual void Bind( void );
    
    virtual void Load( void );
    virtual void Unload( void );

    virtual void Tick(
        float deltaSeconds
    );

    virtual void PostTick( void );

    virtual void Final( void );

    virtual void EditorRender( void );

    bool ExecuteMemberFunction(
        const char *pFunctionName, 
        bool mustExist,
        const char *pSignature, ...
    );    
 
    virtual const char *GetObjectName( void ) const { return m_pLuaObject->GetName( ); }

private:
    void OnEvent(
        const char *pScriptMethod,
        const Channel *pSender,
        const char *pEvent,
        const ArgList &list
    );
};
