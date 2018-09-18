#include "EnginePch.h"

#include "LuaScriptInstance.h"
#include "LuaVM.h"

LuaScriptInstance::LuaScriptInstance(
    Component *pComponent,
    ResourceHandle script
) : ScriptInstance(pComponent, script)
{
    m_pLuaContext = NULL;
    m_pLuaObject  = NULL;
    m_ScriptOk    = true;
}

void LuaScriptInstance::Bind( void )
{
    ExecuteMemberFunction( "_Create", false,  "" );
    m_NeedsCreate = true;
    m_ReadyForTick = false;
}
    
void LuaScriptInstance::Load( void )
{
    Unload( );
 
    m_pLuaContext = new LuaContext( GetComponent( ), GetScriptHandle( ) );

    LuaVM::Instance().PushLuaContext(m_pLuaContext);
    m_pLuaObject  = new LuaObject( GetComponent( ), GetScriptHandle( ) );
    LuaVM::Instance().PopLuaContext();
}

void LuaScriptInstance::Unload( void )
{
    if ( NULL != m_pLuaObject )
    {
        if (true == m_ReadyForTick)
            ExecuteMemberFunction( "Destroy",false,  "" );
        
        ExecuteMemberFunction( "_Destroy", false,  "" );

        delete m_pLuaContext;
        delete m_pLuaObject;

        m_pLuaObject = NULL;
        m_pLuaContext= NULL;

        m_ReadyForTick = false;
        m_NeedsCreate = true;
    }
}

void LuaScriptInstance::Tick(
    float deltaSeconds
)
{
    if (true == m_NeedsCreate)
    {
        ExecuteMemberFunction( "Create", false,  "" );
        m_NeedsCreate = false;
    }
    else
    {
        m_ReadyForTick = true;
        ExecuteMemberFunction( "Tick", false, "f", deltaSeconds );
    }
}

void LuaScriptInstance::PostTick( void )
{
    if (true == m_ReadyForTick)
        ExecuteMemberFunction( "PostTick", false, "" );
}

void LuaScriptInstance::Final( void )
{
    if (true == m_ReadyForTick)
        ExecuteMemberFunction( "Final", false, "" );
}

void LuaScriptInstance::EditorRender( void )
{
    ExecuteMemberFunction( "EditorRender", false, "" );
}

bool LuaScriptInstance::ExecuteMemberFunction(
    const char *pFunctionName, 
    bool mustExist,
    const char *pSignature, ...
)
{
    if (true == m_ScriptOk && m_pLuaObject != NULL && m_pLuaObject->IsValid())
    {
        va_list list;
        va_start( list, pSignature );

        LuaVM::Instance().PushLuaContext(m_pLuaContext);

        //The ScriptOK prevents cascading errors from showing up by 
        //running a lua script which had previous errors.  When the owner of this file (the node) is updated
        //this will be reset and the script can run again
        m_ScriptOk = LuaVM::Instance().ExecuteLuaObjectV(m_pLuaObject, pFunctionName, mustExist, pSignature, list );

        LuaVM::Instance().PopLuaContext();

        va_end( list );
    }

    return m_ScriptOk;
}    
    
void LuaScriptInstance::OnEvent(
    const char *pScriptMethod,
    const Channel *pSender,
    const char *pEvent,
    const ArgList &list
)
{
    if (true == m_ScriptOk && m_pLuaObject != NULL && m_pLuaObject->IsValid())
    {
        LuaVM::Instance().PushLuaContext(m_pLuaContext);

        m_ScriptOk = LuaVM::Instance().PushLuaObjectEvent(m_pLuaObject, pScriptMethod, pSender, pEvent, list);

        LuaVM::Instance().PopLuaContext();
    }
}
