#pragma once

#include "HashTable.h"
#include "ThreadLocks.h"
#include "Resource.h"
#include "Stack.h"
#include "ResourceMaps.h"
#include "Threads.h"

class LuaObject;
class LuaEventHandler;
class LuaMethod;
class LuaGetProperty;
class LuaSetProperty;

struct lua_State;

typedef int (*LUA_FUNC)(lua_State *pState);

class Component;

class LuaContext
{
private:
    ResourceHandle m_ParentResource;
    Component *m_pParentComponent;

public:
    LuaContext(Component *pComponent, ResourceHandle parentHandle) { m_pParentComponent = pComponent; m_ParentResource = parentHandle; }
    LuaContext() { m_pParentComponent = NULL; m_ParentResource = NullHandle; }
    ~LuaContext() { m_ParentResource = NullHandle; }

    void SetParentResource(ResourceHandle parentHandle) { m_ParentResource = parentHandle; }
    ResourceHandle GetParentResource() { return m_ParentResource; }
    Component *GetParentComponent() { return m_pParentComponent; }
};

class LuaVM
{
private:
    friend class GarbageCollectionThread;

    class GarbageCollectionThread : public Thread
    {
    public:
        void Create()
        {
        }

        virtual void OnThreadRun( void );
    };

    GarbageCollectionThread m_Thread;
    void OnThreadRun( void );

public:
    void Create();
    void Destroy();

    bool ExecuteLuaAsset(ResourceHandle luaResourceHandle);
    bool ExecuteFunction(const char *pFunctionName, const char *pSignature, ...);
    bool ExecuteFunctionV(const char *pFunctionName, const char *pSignature, va_list vlist);
    bool ExecuteLuaObject(LuaObject *pLuaObject, const char *pFunctionName, bool mustExist, const char *pSignature, ...);
    bool ExecuteLuaObjectV(LuaObject *pLuaObject, const char *pFunctionName, bool mustExist, const char *pSignature, va_list vaList);
    bool ExecuteObjectFunction(const char *pClassName, const char *pObjectName, const char *pFunctionName, bool mustExist, const char *pSignature, ...);
    bool ExecuteObjectFunctionV(const char *pClassName, const char *pObjectName, const char *pFunctionName, bool mustExist, const char *pSignature, va_list vlist);
    bool ExecuteString(const char *pString);
    bool PushLuaObjectEvent(LuaObject *pLuaObject, const char *pScriptMethod, const Channel *pSender, const char *pEvent, const ArgList &list);
    bool PushLuaObjectEvent(const char *pClassName, const char *pObjectName, const char *pFunctionName, const Channel *pSender, const char *pEvent, const ArgList &list);
    void RegisterFunction(const char *pLuaFuncName, LUA_FUNC func);

    int GetInt(const char *pVarName);
    char ** GetString(const char *pVarName);

    static LuaVM &Instance();

    bool LoadChunk(ResourceHandle luaResourceHandle, const char *pData, int size);
    bool UnloadChunk(ResourceHandle luaResourceHandle);
    bool IsChunkLoaded(ResourceHandle luaResourceHandle);
    void AddChunkRef(ResourceHandle luaResourceHandle);
    void RemoveChunkRef(ResourceHandle luaResourceHandle);

    const lua_State *GetLuaState() { return m_pLuaState; }
    void ForceFullGarbageCollect();
    void GarbageCollect();

    const char *GetCurrentFunctionName() { return m_pCurrentFunctionName; }

    void PushLuaContext(LuaContext *pLuaContext);
    LuaContext *PopLuaContext();
    LuaContext *GetLuaContext() { return m_LuaContexts.GetSize() > 0 ? m_LuaContexts.Top() : NULL; }

    // Push/Pop/Get the LuaObject
    void PushLuaObject(LuaObject *pLuaObject)
    { 
        m_LuaObjects.Push(pLuaObject);
    }

    void PopLuaObject()
    {
        if (m_LuaObjects.GetSize() > 0)
        {
            m_LuaObjects.Pop();
        }
    }

    LuaObject *GetLuaObject() 
    {
        return m_LuaObjects.GetSize() > 0 ? m_LuaObjects.Top() : NULL;
    }

    bool SetLuaObjectTableFloat(float value, LuaObject *pLuaObject, const char *fieldName, int arrayIndex = -1);
    bool SetLuaObjectTableInt(int value, LuaObject *pLuaObject, const char *fieldName, int arrayIndex = -1);
    bool SetLuaObjectTableBool(bool value, LuaObject *pLuaObject, const char *fieldName, int arrayIndex = -1);
    bool SetLuaObjectTableString(const char *value, LuaObject *pLuaObject, const char *fieldName, int arrayIndex = -1);

    bool SetLuaTableFloat(float value, const char *tableName, const char *fieldName, int arrayIndex = -1);
    bool SetLuaTableInt(int value, const char *tableName, const char *fieldName, int arrayIndex = -1);
    bool SetLuaTableBool(bool value, const char *tableName, const char *fieldName, int arrayIndex = -1);
    bool SetLuaTableString(const char *value, const char *tableName, const char *fieldName, int arrayIndex = -1);

    void OnAssetReloaded(
        const Channel *pChannel,
        const char *pName,
        const ArgList &list
        );

    //it's ok that lists are saved by pointer
    //because they will be pushed and popped within the scope
    //of the same function
    //if I've missed something, we could serialize the arg list
    //like we do for queued events and networking
    void PushArgList( 
        const ArgList &list 
        )
    {
        m_ArgListStack.Push( &list );
    }

    void PopArgList( void )
    {
        m_ArgListStack.Pop( );
    }

    const ArgList *GetArgList( void )
    {
        return m_ArgListStack.Top( );
    }

protected:
    struct ChunkData
    {
        int m_RefCount;

        ChunkData()
        {
            m_RefCount = 0;
        }
    };

    // Return Value string cacheing
    static const int MAX_RETURN_STRING_LENGTH = 64;
    static const int MAX_RETURN_STRING_COUNT = 8;

    struct ReturnString
    {
        char value[MAX_RETURN_STRING_LENGTH];
    };

    ReturnString m_ReturnStringCache[MAX_RETURN_STRING_COUNT];

private:
    LuaVM();
    virtual ~LuaVM();

    lua_State* m_pLuaState;
    static LuaVM *s_Instance;

    HashTable<Id, ChunkData>  m_Chunks;
    Stack<LuaContext *> m_LuaContexts;
    Stack<LuaObject *>  m_LuaObjects;
    Stack<const ArgList*>     m_ArgListStack;
    Lock       m_Lock;


    const char *m_pCurrentFunctionName;
};







