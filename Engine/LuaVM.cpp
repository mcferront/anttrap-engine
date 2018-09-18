#include "EnginePch.h"
#include "LuaVM.h"
#include "LuaObject.h"
#include "LuaAsset.h"
#include "Debug.h"
#include "MemoryManager.h"
#include "Component.h"

extern "C" 
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

bool g_LuaVM_RequestFullCollection = false;
int  g_LuaVM_StepCount = 1;
int  g_LuaVM_StepSize = 50;

void FormatLuaError(const char *pNodeName, const char *pClassName, const char *pFuncName, const char *pError)
{
    char error[1024];

    const char *pBegin = strstr( pError, "//" );

    if ( NULL == pBegin )
        Debug::Print( Debug::TypeError, "Lua: %s %s:%s %s\n", pNodeName ? pNodeName : "", pClassName ? pClassName : "Global", pFuncName ? pFuncName : "", pError );
    else
    {
        String::Copy( error, pBegin, sizeof(error) );

        char *pEnd = strstr( error, ".lua" );
        if ( NULL != pEnd )
        {
            pEnd += 4;
            *pEnd = NULL;

            ++pEnd;

            String::Format( error, sizeof(error), "%s%s", error, pEnd += sizeof("_chunk") );
        }
        else
            String::Format( error, sizeof(error), "%s%s", error, pBegin );

        Debug::Print( Debug::TypeError, "Lua: %s %s:%s %s\n", pNodeName ? pNodeName : "", pClassName ? pClassName : "Global", pFuncName ? pFuncName : "", error );
    }
}

void FormatLuaError(const char *pError)
{
    FormatLuaError(NULL, NULL, NULL, pError);
}

extern "C" {
    int luaopen_Math(lua_State* L); // declare the wrapped module
    int luaopen_Core(lua_State* L); // declare the wrapped module
}


extern "C" {
    void CWPPrintLua(const char *pMsg)
    {
        Debug::Print( Debug::TypeScript, pMsg );
    }
}

LuaVM *LuaVM::s_Instance = 0;

void LuaVM::GarbageCollectionThread::OnThreadRun( void )
{
    //manually do this in app.cpp because
    //it's causing too much drain on single core machines
    //while ( ShouldRun( ) )
    //{
    //   LuaVM::Instance( ).OnThreadRun();

    //   //sleep instead of yield
    //   //because otherwise it consumes 1/2 the time
    //   //on a single core machine
    //   //maybe we should lower priority instead?
    //   Thread::Sleep( 1 );
    //}
}

void LuaVM::OnThreadRun( void )
{
    LuaVM::Instance().GarbageCollect();
}

LuaVM::LuaVM() : m_pLuaState(0)
{
}

LuaVM::~LuaVM()
{
}

void *LuaAlloc(
    void *ud, 
    void *ptr, 
    size_t osize,
    size_t nsize
    ) 
{
    (void)osize;  /* not used */

    MemoryHeap *pHeap = (MemoryHeap *) ud;

    if ( 0 == nsize ) 
    {
#ifdef ENABLE_DEBUGMEMORY
        pHeap->Free( ptr, __FILE__, __LINE__ );
#else
        pHeap->Free( ptr );
#endif
        return NULL;
    }
    else
    {
#ifdef ENABLE_DEBUGMEMORY
        void *pMemory = pHeap->Realloc( ptr, nsize, __FILE__, __LINE__ );
        return pMemory;      
#else
        return pHeap->Realloc( ptr, nsize );   
#endif
    }
}

void LuaVM::Create()
{
    m_Chunks.Create(16, 16, IdHash, IdCompare);

    m_LuaContexts.Create( );
    m_LuaObjects.Create( );
    m_ArgListStack.Create( );

    /* initialize Lua */
    m_pLuaState = lua_newstate( LuaAlloc, MemoryManager::Instance( ).GetHeap("Lua") );

    if( m_pLuaState != NULL )
    {
        /* load Lua base libraries */
        luaL_openlibs(m_pLuaState);

        if (!luaopen_Math((lua_State *)LuaVM::Instance().GetLuaState()))
        {
            Debug::Assert( Condition( false ), "Unable to open Engine lua library." );
        }
        if (!luaopen_Core((lua_State *)LuaVM::Instance().GetLuaState()))
        {
            Debug::Assert( Condition( false ), "Unable to open Engine lua library." );
        }
    }

    // Set the global variable for build config
#ifdef _DEBUG
    lua_pushstring(m_pLuaState, "_DEBUG");
#elif defined(_RELEASE)
    lua_pushstring(m_pLuaState, "_RELEASE");
#elif defined(_DISTRIBUTION)
    lua_pushstring(m_pLuaState, "_DISTRIBUTION");
#else
#error "Build configuration not defined"
#endif
    lua_setglobal(m_pLuaState, "DEFINE_BuildConfig");

    // Set the global variable for build config
#ifdef WIN32
    lua_pushstring(m_pLuaState, "WIN32");
#elif defined(IOS)
    lua_pushstring(m_pLuaState, "IOS");
#elif defined(LINUX)
    lua_pushstring(m_pLuaState, "LINUX");
#elif defined(MAC)
    lua_pushstring(m_pLuaState, "MAC");
#elif defined(ANDROID)
    lua_pushstring(m_pLuaState, "ANDROID");
#else
#error Platform Undefined
#endif
    lua_setglobal(m_pLuaState, "DEFINE_PlatformType");

    lua_gc(m_pLuaState, LUA_GCSTOP, 0);

    m_pCurrentFunctionName = NULL;

    ResourceWorld::Instance().GetChannel( )->AddEventListener( EventMap<LuaVM>("PostAssetReloaded", this, &LuaVM::OnAssetReloaded) );

    m_Thread.Create( );
    m_Thread.Run( );
}

void LuaVM::Destroy()
{
    m_Thread.Stop( );

    ResourceWorld::Instance().GetChannel( )->RemoveEventListener( EventMap<LuaVM>("PostAssetReloaded", this, &LuaVM::OnAssetReloaded) );

    if( m_pLuaState != NULL )
    {
        ForceFullGarbageCollect( );

        /* cleanup Lua */
        lua_close(m_pLuaState);
        m_pLuaState = NULL;
    }

    m_LuaContexts.Destroy( );
    m_LuaObjects.Destroy( );
    m_ArgListStack.Destroy( );

    m_Chunks.Destroy( );
}

bool LuaVM::ExecuteLuaAsset(ResourceHandle luaResourceHandle)
{
    char chunkName[96];

    String::Format(chunkName, sizeof(chunkName), "%s_Chunk", luaResourceHandle.GetId().ToString());

    return ExecuteFunction(chunkName, "");
}

bool LuaVM::ExecuteFunction(const char *pFunctionName, const char *pSignature, ...)
{
    bool retVal;

    va_list vaList;
    va_start(vaList, pSignature);
    retVal = ExecuteFunctionV(pFunctionName, pSignature, vaList);
    va_end(vaList);

    return retVal;
}

bool LuaVM::ExecuteFunctionV(const char *pFunctionName, const char *pSignature, va_list vlist)
{
    ScopeLock lock(m_Lock);

    pSignature = pSignature != NULL ? pSignature : "";

    int argCount, resultCount, errorHandlerIndex = 0;
    const char *pParsedSignature = pSignature;

    // Ensure we have stack space for the error handler and the function
    if (!lua_checkstack(m_pLuaState, 1))
    {
        Debug::Print( Debug::TypeError, "LuaVM::ExecuteFunctionV() - '%s(%s)' - No stack space for function arguments\n", pFunctionName, pSignature);
        return false;
    }

    // Push the function onto the stack
    lua_getglobal(m_pLuaState, pFunctionName);

    // Push the arguments onto the stack
    argCount = 0;
    while (*pParsedSignature)
    {
        // Ensure we have stack space for the next argument
        if (!lua_checkstack(m_pLuaState, 1))
        {
            Debug::Print( Debug::TypeError, "LuaVM::ExecuteFunctionV() - '%s(%s)' - No stack space for function arguments\n", pFunctionName, pSignature);
            return false;
        }

        switch (*pParsedSignature)
        {
        case 'f':
            {
                float v = (float) va_arg(vlist, double);
                lua_pushnumber(m_pLuaState, v);
                //lua_pushnumber(m_pLuaState, (float) va_arg(vlist, float));
                break;
            }
        case 'd':
            lua_pushnumber(m_pLuaState, (float)va_arg(vlist, double));
            break;

        case 'i':
            lua_pushinteger(m_pLuaState, va_arg(vlist, int));
            break;

        case 's':
            lua_pushstring(m_pLuaState, va_arg(vlist, char *));
            break;

        case 'b':
            lua_pushboolean(m_pLuaState, va_arg(vlist, int));
            break;

        case '>':
            pParsedSignature++;
            goto callFunction;

        default:
            Debug::Print( Debug::TypeError, "LuaVM::ExecuteFunctionV() - '%s(%s)' - invalid argument type '%c'\n", pFunctionName, pSignature, *pParsedSignature);
        }

        pParsedSignature++;
        argCount++;
    }

callFunction:

    // Determine the number of expected results
    resultCount = (int)strlen(pParsedSignature);

    // Bind the LuaContext
    //   if (m_LuaContexts.GetSize() > 0)
    //   {
    //      m_LuaContexts.Top()->Bind();
    //   }
    m_pCurrentFunctionName = pFunctionName;

    // Call the Lua function
    int retVal = lua_pcall(m_pLuaState, argCount, resultCount, errorHandlerIndex);

    // Unbind the LuaContext
    m_pCurrentFunctionName = NULL;
    //   if (m_LuaContexts.GetSize() > 0)
    //   {
    //      m_LuaContexts.Top()->Unbind();
    //   }

    // Check for errors from the function call
    if (retVal != 0)
    {
        const char *pErrorMessage = lua_tostring(m_pLuaState, -1);

        if (retVal == 1)
        {
            FormatLuaError(pErrorMessage);
        }
        else if (retVal == LUA_ERRRUN)
        {
            FormatLuaError(pErrorMessage);
        }
        else if (retVal == LUA_ERRMEM)
        {
            FormatLuaError(pErrorMessage);
        }
        else if (retVal == LUA_ERRERR)
        {
            FormatLuaError(pErrorMessage);
        }
        else
        {
            FormatLuaError(pErrorMessage);
        }

        //      StatusPrintf(SU_Engine, SI_Error, "LuaVM::ExecuteFunctionV() - '%s(%s)' - invalid result '%c'\n", pFunctionName, pSignature, *pParsedSignature);
        lua_pop(m_pLuaState, -1); // Pop the error message
        return false;
    }

    // Counter for string cacheing
    int returnStringCount = 0;

    // Parse the results
    int resultIndex = -resultCount;
    while (*pParsedSignature)
    {
        switch (*pParsedSignature)
        {
        case 'f':
            if (!lua_isnumber(m_pLuaState, resultIndex))
            {
                Debug::Print( Debug::TypeError, "LuaVM::ExecuteFunctionV() - '%s(%s)' - invalid result '%c'\n", pFunctionName, pSignature, *pParsedSignature);
                break;
            }
            *va_arg(vlist, float *) = (float)lua_tonumber(m_pLuaState, resultIndex);
            break;

        case 'd':
            if (!lua_isnumber(m_pLuaState, resultIndex))
            {
                Debug::Print( Debug::TypeError, "LuaVM::ExecuteFunctionV() - '%s(%s)' - invalid result '%c'\n", pFunctionName, pSignature, *pParsedSignature);
                break;
            }
            *va_arg(vlist, double *) = lua_tonumber(m_pLuaState, resultIndex);
            break;

        case 'i':
            if (!lua_isnumber(m_pLuaState, resultIndex))
            {
                Debug::Print( Debug::TypeError, "LuaVM::ExecuteFunctionV() - '%s(%s)' - invalid result '%c'\n", pFunctionName, pSignature, *pParsedSignature);
                break;
            }
            *va_arg(vlist, int *) = lua_tointeger(m_pLuaState, resultIndex);
            break;

        case 'b':
            if (!lua_isboolean(m_pLuaState, resultIndex))
            {
                Debug::Print( Debug::TypeError, "LuaVM::ExecuteFunctionV() - '%s(%s)' - invalid result '%c'\n", pFunctionName, pSignature, *pParsedSignature);
                break;
            }
            *va_arg(vlist, int *) = lua_toboolean(m_pLuaState, resultIndex);
            break;

        case 's':
            if (!lua_isstring(m_pLuaState, resultIndex))
            {
                Debug::Print( Debug::TypeError, "LuaVM::ExecuteFunctionV() - '%s(%s)' - invalid result '%c'\n", pFunctionName, pSignature, *pParsedSignature);
                break;
            }
            if (returnStringCount < MAX_RETURN_STRING_COUNT)
            {
                String::Copy(m_ReturnStringCache[returnStringCount].value, lua_tostring(m_pLuaState, resultIndex), MAX_RETURN_STRING_LENGTH);
                *va_arg(vlist, const char **) = m_ReturnStringCache[returnStringCount].value;
                returnStringCount++;
            }
            else
            {
                Debug::Print( Debug::TypeError, "Returning more than %d string values from a lua function is unsupported.", MAX_RETURN_STRING_COUNT );
            }
            break;

        default:
            Debug::Print( Debug::TypeError, "LuaVM::ExecuteFunctionV() - '%s(%s)' - invalid result type '%c'\n", pFunctionName, pSignature, *pParsedSignature);
            break;
        }

        // Remove the result from the stack      
        //lua_remove(m_pLuaState, -1);

        pParsedSignature++;
        resultIndex++;
    }
    va_end(vlist);

    // Remove the results from the stack      
    lua_pop(m_pLuaState, resultCount);
    return true;
}

bool LuaVM::ExecuteString(const char *pString)
{
    ScopeLock lock(m_Lock);

    int errorHandlerIndex = 0;

    if (!lua_checkstack(m_pLuaState, 1))
    {
        Debug::Print( Debug::TypeError, "LuaVM::ExecuteString() - No stack space for string\n");
        return false;
    }

    luaL_loadstring(m_pLuaState, pString);

    m_pCurrentFunctionName = NULL;

    // Call the Lua function
    int retVal = lua_pcall(m_pLuaState, 0, 0, errorHandlerIndex);

    // Unbind the LuaContext
    m_pCurrentFunctionName = NULL;

    // Check for errors from the function call
    if (retVal != 0)
    {
        const char *pErrorMessage = lua_tostring(m_pLuaState, -1);

        if (retVal == 1)
        {
            FormatLuaError(pErrorMessage);
        }
        else if (retVal == LUA_ERRRUN)
        {
            FormatLuaError(pErrorMessage);
        }
        else if (retVal == LUA_ERRMEM)
        {
            FormatLuaError(pErrorMessage);
        }
        else if (retVal == LUA_ERRERR)
        {
            FormatLuaError(pErrorMessage);
        }
        else
        {
            FormatLuaError(pErrorMessage);
        }

        lua_pop(m_pLuaState, -1); // Pop the error message
        return false;
    }

    return true;
}

bool LuaVM::PushLuaObjectEvent(const char *pClassName, const char *pObjectName, const char *pFunctionName, const Channel *pSender, const char *pEvent, const ArgList &list)
{
    ScopeLock lock(m_Lock);

    // Ensure we have stack space for the error handler and the function
    if (!lua_checkstack(m_pLuaState, 1))
    {
        Debug::Print( Debug::TypeError, "LuaVM::ExecuteFunctionV() - '%s(%s)' - No stack space for function arguments\n", pFunctionName, pEvent);
        return false;
    }

    // Table to be indexed
    lua_getfield(m_pLuaState, LUA_GLOBALSINDEX, pObjectName);
    // Push the function from the table on to the stack
    lua_getfield(m_pLuaState, -1, pFunctionName);

    // remove the table from the stack
    // which pushes the function to the calling spot
    lua_remove(m_pLuaState, -2);

    //put the table back into globals , I guess
    //this is so it has the table reference of 'self'?
    lua_getfield(m_pLuaState, LUA_GLOBALSINDEX, pObjectName);


    lua_pushstring(m_pLuaState, pSender->GetId().ToString());
    lua_pushstring(m_pLuaState, pEvent);


    // Push the arguments onto the stack
    for (uint32 i = 0; i < list.GetArgSig()->GetCount(); i++)
    {
        // Ensure we have stack space for the next argument
        if (!lua_checkstack(m_pLuaState, 1))
        {
            Debug::Print( Debug::TypeError, "LuaVM::ExecuteFunctionV() - '%s(%s)' - No stack space for function arguments\n", pFunctionName, pEvent);
            return false;
        }

        const type_info &type = list.GetArgSig( )->GetType(i);

        if ( typeid(Id) == type )
        {
            Id id;
            list.GetArg( i, &id );

            lua_pushstring(m_pLuaState, id.ToString());
        }
        else if ( typeid(float) == type )
        {
            float value;
            list.GetArg( i, &value );

            lua_pushnumber(m_pLuaState, value);
        }
        else if ( typeid(int) == type )
        {
            int value;
            list.GetArg( i, &value );

            lua_pushinteger(m_pLuaState, value);
        }
        else if ( typeid(char *) == type )
        {
            char *pValue;
            list.GetArg( i, &pValue );

            lua_pushstring(m_pLuaState, pValue);
        }
        else if ( typeid(const char *) == type )
        {
            const char *pValue;
            list.GetArg( i, &pValue );

            lua_pushstring(m_pLuaState, pValue);
        }
        else if ( typeid(bool) == type )
        {
            bool v;
            list.GetArg( i, &v );

            int value = v ? 1 : 0;

            lua_pushinteger(m_pLuaState, value);
        }
        else
            Debug::Print( Debug::TypeWarning, "Unrecognized event signature type: %s when sending event to Lua\n", type.name() );
    }

    m_pCurrentFunctionName = pFunctionName;

    // Call the Lua function
    int retVal = lua_pcall(m_pLuaState, list.GetArgSig()->GetCount() + 3, 0, 0);

    // Unbind the LuaContext
    m_pCurrentFunctionName = NULL;

    // Check for errors from the function call
    if (retVal != 0)
    {
        const char *pErrorMessage = lua_tostring(m_pLuaState, -1);

        if (retVal == 1)
        {
            FormatLuaError(pErrorMessage);
        }
        else if (retVal == LUA_ERRRUN)
        {
            FormatLuaError(pErrorMessage);
        }
        else if (retVal == LUA_ERRMEM)
        {
            FormatLuaError(pErrorMessage);
        }
        else if (retVal == LUA_ERRERR)
        {
            FormatLuaError(pErrorMessage);
        }
        else
        {
            FormatLuaError(pErrorMessage);
        }

        lua_pop(m_pLuaState, -1); // Pop the error message
        return false;
    }

    // Remove the results from the stack      
    lua_pop(m_pLuaState, 0);
    return true;
}

int LuaVM::GetInt(const char *pVarName)
{
    int value = 0;
    lua_getglobal(m_pLuaState, pVarName);

    value = (int)lua_tonumber(m_pLuaState, 1);
    lua_pop(m_pLuaState, 1);

    return value;
}

char ** LuaVM::GetString(const char *pVarName)
{
    lua_getglobal(m_pLuaState, pVarName);
    char **value = 0;

    *value = (char *)lua_tostring(m_pLuaState, 1);
    lua_pop(m_pLuaState, 1);

    return value;
}

void LuaVM::RegisterFunction(const char *pLuaFuncName, LUA_FUNC funcName)
{
    lua_register(m_pLuaState, pLuaFuncName, funcName);
}

LuaVM &LuaVM::Instance()
{
    static LuaVM s_Instance;
    return s_Instance;
}

bool LuaVM::LoadChunk(ResourceHandle luaResourceHandle, const char *pData, int size)
{
    int retVal;

    if (strlen(luaResourceHandle.GetId().ToString()) <= 0)
        return false;

    char chunkName[96];

    String::Format(chunkName, 96, "%s_Chunk", luaResourceHandle.GetId().ToString());

    // Load the Lua chunk
    {
        ScopeLock lock(m_Lock);
        retVal = luaL_loadbuffer(m_pLuaState, pData, (unsigned long)size, chunkName);
    }

    if (retVal != 0)
    {
        // Get the error message
        const char *pErrorMessage = lua_tostring(m_pLuaState, -1);

        if (retVal == LUA_ERRSYNTAX)
        {
            FormatLuaError(pErrorMessage);
        }  
        else if (retVal == LUA_ERRMEM)
        {
            FormatLuaError(pErrorMessage);
        }
        else
        {
            FormatLuaError(pErrorMessage);
        }

        lua_pop(m_pLuaState, -1);

        return false;
    }

    // Set the global function variable in Lua to the current chunk
    lua_setglobal(m_pLuaState, chunkName);

    AddChunkRef(luaResourceHandle);

    return true;
}

bool LuaVM::UnloadChunk(ResourceHandle luaResourceHandle)
{
    ScopeLock lock(m_Lock);

    RemoveChunkRef(luaResourceHandle);

    return true;
}

bool LuaVM::IsChunkLoaded(ResourceHandle luaResourceHandle)
{
    return m_Chunks.Contains(luaResourceHandle.GetId().ToString()) != 0;
}

void LuaVM::AddChunkRef(ResourceHandle luaResourceHandle)
{
    ChunkData chunkData;
    Id systemId = luaResourceHandle.GetId();

    if (!m_Chunks.Get(systemId, &chunkData))
    {
        m_Chunks.Add(systemId, chunkData);
    }

    chunkData.m_RefCount++;
    m_Chunks.Remove(systemId);
    m_Chunks.Add(systemId, chunkData);
}

void LuaVM::RemoveChunkRef(ResourceHandle luaResourceHandle)
{
    ChunkData chunkData;
    Id systemId = luaResourceHandle.GetId();
    const char *pLuaResourceName = systemId.ToString();

    if (m_Chunks.Get(systemId, &chunkData))
    {
        Debug::Assert( Condition(chunkData.m_RefCount > 0), "RefCount is already zero!\nAttempting to Remove Chunk Ref for: %s", luaResourceHandle.GetId().ToString() );
        chunkData.m_RefCount--;

        if (chunkData.m_RefCount <= 0)
        {
            //         m_Chunks.Add(systemId, chunkData);
            //      }
            //      else
            //      {
            // Set the global function variable in Lua to nil, forcing it to unload and be garbage collected
            char chunkName[96];
            String::Format(chunkName, 96, "%s_Chunk", pLuaResourceName);
            lua_pushnil(m_pLuaState);
            lua_setglobal(m_pLuaState, chunkName);

            m_Chunks.Remove(systemId);
        }
    }
}

void LuaVM::ForceFullGarbageCollect()
{
    ScopeLock lock(m_Lock);
    g_LuaVM_RequestFullCollection = true;
    GarbageCollect();
}

void LuaVM::GarbageCollect()
{
    if (m_pLuaState)
    {
        ScopeLock lock(m_Lock);

        if (g_LuaVM_RequestFullCollection == true)
        {
            g_LuaVM_RequestFullCollection = false;

            lua_gc(m_pLuaState, LUA_GCCOLLECT, 0);

            //int amountUsed = lua_gc(m_pLuaState, LUA_GCCOUNT, 0);
            //Debug::Print( Debug::TypeInfo, "Lua KB Used: %d", amountUsed );
        }
        else
        {
            for (int i = 0; i < g_LuaVM_StepCount; i++)
            {
                if (lua_gc(m_pLuaState, LUA_GCSTEP, g_LuaVM_StepSize) == 1)
                {
                    break;
                }
            }
        }
    }
}

bool LuaVM::ExecuteLuaObject(LuaObject *pLuaObject, const char *pFunctionName, bool mustExist, const char *pSignature, ...)
{
    ScopeLock lock(m_Lock);
    bool retVal;

    PushLuaObject(pLuaObject);

    va_list vaList;
    va_start(vaList, pSignature);
    retVal = ExecuteObjectFunctionV(pLuaObject->GetClassName(), pLuaObject->GetName(), pFunctionName, mustExist, pSignature, vaList);
    va_end(vaList);

    PopLuaObject();

    return retVal;
}

bool LuaVM::ExecuteLuaObjectV(LuaObject *pLuaObject, const char *pFunctionName, bool mustExist, const char *pSignature, va_list vaList)
{
    ScopeLock lock(m_Lock);
    bool retVal;

    PushLuaObject(pLuaObject);

    retVal = ExecuteObjectFunctionV(pLuaObject->GetClassName(), pLuaObject->GetName(), pFunctionName, mustExist, pSignature, vaList);

    PopLuaObject();

    return retVal;
}

bool LuaVM::PushLuaObjectEvent(LuaObject *pLuaObject, const char *pScriptMethod, const Channel *pSender, const char *pEvent, const ArgList &list)
{
    ScopeLock lock(m_Lock);
    bool retVal;

    PushLuaObject(pLuaObject);

    retVal = PushLuaObjectEvent(pLuaObject->GetClassName(), pLuaObject->GetName(), pScriptMethod, pSender, pEvent, list);

    PopLuaObject();

    return retVal;
}

bool LuaVM::ExecuteObjectFunction(const char *pClassName, const char *pObjectName, const char *pFunctionName, bool mustExist, const char *pSignature, ...)
{
    bool retVal;

    va_list vaList;
    va_start(vaList, pSignature);
    retVal = ExecuteObjectFunctionV(pClassName, pObjectName, pFunctionName, mustExist, pSignature, vaList);
    va_end(vaList);

    return retVal;
}

bool LuaVM::ExecuteObjectFunctionV(const char *pClassName, const char *pObjectName, const char *pFunctionName, bool mustExist, const char *pSignature, va_list vlist)
{
    ScopeLock lock(m_Lock);

    pSignature = pSignature != NULL ? pSignature : "";

    int argCount, resultCount, errorHandlerIndex = 0;
    const char *pParsedSignature = pSignature;

    // Ensure we have stack space for the error handler and the function
    if (!lua_checkstack(m_pLuaState, 1))
    {
        Debug::Print( Debug::TypeError, "LuaVM::ExecuteObjectFunctionV() - '%s %s:%s(%s)' - No stack space for function arguments\n", pClassName, pObjectName, pFunctionName, pSignature);
        return false;
    }

    // Table to be indexed
    lua_getfield(m_pLuaState, LUA_GLOBALSINDEX, pObjectName);
    // Push the function from the table on to the stack
    lua_getfield(m_pLuaState, -1, pFunctionName);

    if (false == mustExist)
    {
        int result = lua_isfunction(m_pLuaState, -1);
        if (0 == result) 
        {
            lua_pop(m_pLuaState, -1);
            return true;
        }
    }

    // remove the table from the stack
    // which pushes the function to the calling spot
    lua_remove(m_pLuaState, -2);

    //put the table back into globals , I guess
    //this is so it has the table reference of 'self'?
    lua_getfield(m_pLuaState, LUA_GLOBALSINDEX, pObjectName);


    // Push the arguments onto the stack
    argCount = 0;
    while (*pParsedSignature)
    {
        // Ensure we have stack space for the next argument
        if (!lua_checkstack(m_pLuaState, 1))
        {
            Debug::Print( Debug::TypeError, "LuaVM::ExecuteObjectFunctionV() - '%s %s:%s(%s)' - No stack space for function arguments\n", pClassName, pObjectName, pFunctionName, pSignature);
            return false;
        }

        switch (*pParsedSignature)
        {
        case 'f':
            {
                float v = (float) va_arg(vlist, double);
                lua_pushnumber(m_pLuaState, v);
                break;
            }
        case 'd':
            lua_pushnumber(m_pLuaState, (float)va_arg(vlist, double));
            break;

        case 'i':
            lua_pushinteger(m_pLuaState, va_arg(vlist, int));
            break;

        case 's':
            lua_pushstring(m_pLuaState, va_arg(vlist, char *));
            break;

        case 'b':
            lua_pushboolean(m_pLuaState, va_arg(vlist, int));
            break;

        case '>':
            pParsedSignature++;
            goto callFunction;

        default:
            Debug::Print( Debug::TypeError, "LuaVM::ExecuteObjectFunctionV() - '%s %s:%s(%s)' - invalid argument type '%c'\n", pClassName, pObjectName, pFunctionName, pSignature, *pParsedSignature);
            return false;
        }

        pParsedSignature++;
        argCount++;
    }

callFunction:

    // Determine the number of expected results
    resultCount = (int)strlen(pParsedSignature);

    m_pCurrentFunctionName = pFunctionName;

    // Call the Lua function
    int retVal = lua_pcall(m_pLuaState, argCount + 1, resultCount, errorHandlerIndex);

    m_pCurrentFunctionName = NULL;

    // Check for errors from the function call
    if (retVal != 0)
    {
        const char *pErrorMessage = lua_tostring(m_pLuaState, -1);
        const char *pNodeName = GetLuaContext()->GetParentComponent()->GetParent()->GetName();

        if (retVal == 1)
        {
            FormatLuaError(pNodeName, pClassName, pFunctionName, pErrorMessage);
        }
        else if (retVal == LUA_ERRRUN)
        {
            FormatLuaError(pNodeName, pClassName, pFunctionName, pErrorMessage);
        }
        else if (retVal == LUA_ERRMEM)
        {
            FormatLuaError(pNodeName, pClassName, pFunctionName, pErrorMessage);
        }
        else if (retVal == LUA_ERRERR)
        {
            FormatLuaError(pNodeName, pClassName, pFunctionName, pErrorMessage);
        }
        else
        {
            FormatLuaError(pNodeName, pClassName, pFunctionName, pErrorMessage);
        }

        //      StatusPrintf(SU_Engine, SI_Error, "LuaVM::ExecuteFunctionV() - '%s(%s)' - invalid result '%c'\n", pFunctionName, pSignature, *pParsedSignature);

        lua_pop(m_pLuaState, -1); // Pop the error message
        return false;
    }

    // Counter for string cacheing
    int returnStringCount = 0;

    // Parse the results
    int resultIndex = -resultCount;
    while (*pParsedSignature)
    {
        switch (*pParsedSignature)
        {
        case 'f':
            if (!lua_isnumber(m_pLuaState, resultIndex))
            {
                Debug::Print( Debug::TypeError, "LuaVM::ExecuteFunctionV() - '%s %s(%s)' - invalid result '%c'\n", pClassName, pFunctionName, pSignature, *pParsedSignature);
                break;
            }
            *va_arg(vlist, float *) = (float)lua_tonumber(m_pLuaState, resultIndex);
            break;

        case 'd':
            if (!lua_isnumber(m_pLuaState, resultIndex))
            {
                Debug::Print( Debug::TypeError, "LuaVM::ExecuteFunctionV() - '%s %s(%s)' - invalid result '%c'\n", pClassName, pFunctionName, pSignature, *pParsedSignature);
                break;
            }
            *va_arg(vlist, double *) = lua_tonumber(m_pLuaState, resultIndex);
            break;

        case 'i':
            if (!lua_isnumber(m_pLuaState, resultIndex))
            {
                Debug::Print( Debug::TypeError, "LuaVM::ExecuteFunctionV() - '%s %s(%s)' - invalid result '%c'\n", pClassName, pFunctionName, pSignature, *pParsedSignature);
                break;
            }
            *va_arg(vlist, int *) = lua_tointeger(m_pLuaState, resultIndex);
            break;

        case 'b':
            if (!lua_isboolean(m_pLuaState, resultIndex))
            {
                Debug::Print( Debug::TypeError, "LuaVM::ExecuteFunctionV() - '%s %s(%s)' - invalid result '%c'\n", pClassName, pFunctionName, pSignature, *pParsedSignature);
                break;
            }
            *va_arg(vlist, int *) = lua_toboolean(m_pLuaState, resultIndex);
            break;

        case 's':
            if (!lua_isstring(m_pLuaState, resultIndex))
            {
                Debug::Print( Debug::TypeError, "LuaVM::ExecuteFunctionV() - '%s %s(%s)' - invalid result '%c'\n", pClassName, pFunctionName, pSignature, *pParsedSignature);
                break;
            }
            if (returnStringCount < MAX_RETURN_STRING_COUNT)
            {
                String::Copy(m_ReturnStringCache[returnStringCount].value, lua_tostring(m_pLuaState, resultIndex), MAX_RETURN_STRING_LENGTH);
                *va_arg(vlist, const char **) = m_ReturnStringCache[returnStringCount].value;
                returnStringCount++;
            }
            else
            {
                Debug::Print( Debug::TypeError, "Returning more than %d string values from a lua function is unsupported.\n", MAX_RETURN_STRING_COUNT );
            }
            break;

        default:
            Debug::Print( Debug::TypeError, "LuaVM::ExecuteFunctionV() - '%s %s(%s)' - invalid result type '%c'\n", pClassName, pFunctionName, pSignature, *pParsedSignature);
            break;
        }

        // Remove the result from the stack      
        //lua_remove(m_pLuaState, -1);

        pParsedSignature++;
        resultIndex++;
    }
    va_end(vlist);

    // Remove the results from the stack      
    lua_pop(m_pLuaState, resultCount);
    return true;
}

void LuaVM::PushLuaContext(LuaContext *pLuaContext)
{
    m_LuaContexts.Push(pLuaContext); 
}

LuaContext *LuaVM::PopLuaContext()
{
    if (m_LuaContexts.GetSize() > 0)
    {
        m_LuaContexts.Pop();
    }

    return NULL;
}

void LuaVM::OnAssetReloaded(
    const Channel *pChannel,
    const char *pName,
    const ArgList &list
    )
{
    ResourceHandle luaResourceHandle;
    list.GetArg( 0, &luaResourceHandle );

    if ( !GetResource(luaResourceHandle, Resource)->GetType( ).IsTypeOf( Lua::StaticType() ) )
    {
        return;
    }

    ChunkData chunkData;

    if (m_Chunks.Get( luaResourceHandle.GetId( ), &chunkData) )
    {
        if (chunkData.m_RefCount > 0)
        {
            ExecuteLuaAsset(luaResourceHandle);
        }
    }
}

bool LuaVM::SetLuaObjectTableFloat(float value, LuaObject *pLuaObject, const char *fieldName, int arrayIndex /* = -1 */)
{
    return SetLuaTableFloat(value, pLuaObject->GetName(), fieldName, arrayIndex);
}

bool LuaVM::SetLuaTableFloat(float value, const char *tableName, const char *fieldName, int arrayIndex /* = -1 */)
{
    ExecuteFunction("CToLuaFloat", "ssif", tableName, fieldName, arrayIndex, value);

    return true;
}

bool LuaVM::SetLuaObjectTableInt(int value, LuaObject *pLuaObject, const char *fieldName, int arrayIndex /* = -1 */)
{
    return SetLuaTableInt(value, pLuaObject->GetName(), fieldName, arrayIndex);
}

bool LuaVM::SetLuaTableInt(int value, const char *tableName, const char *fieldName, int arrayIndex /* = -1 */)
{
    ExecuteFunction("CToLuaInt", "ssii", tableName, fieldName, arrayIndex, value);

    return true;
}

bool LuaVM::SetLuaObjectTableBool(bool value, LuaObject *pLuaObject, const char *fieldName, int arrayIndex /* = -1 */)
{
    return SetLuaTableBool(value, pLuaObject->GetName(), fieldName, arrayIndex);
}

bool LuaVM::SetLuaTableBool(bool value, const char *tableName, const char *fieldName, int arrayIndex /* = -1 */)
{
    ExecuteFunction("CToLuaBool", "ssib", tableName, fieldName, arrayIndex, value);

    return true;
}

bool LuaVM::SetLuaObjectTableString(const char *value, LuaObject *pLuaObject, const char *fieldName, int arrayIndex /* = -1 */)
{
    return SetLuaTableString(value, pLuaObject->GetName(), fieldName, arrayIndex);
}

bool LuaVM::SetLuaTableString(const char *value, const char *tableName, const char *fieldName, int arrayIndex /* = -1 */)
{
    ExecuteFunction("CToLuaString", "ssis", tableName, fieldName, arrayIndex, value);

    return true;
}
