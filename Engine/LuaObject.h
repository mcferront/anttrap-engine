#pragma once

#include "EngineGlobal.h"
#include "Resource.h"

class LuaContext;
class Component;

class LuaObject
{
friend class LuaVM;

protected:
   //static unsigned int s_NextID;
   //unsigned int m_ID;
   char m_Name[256];
   char m_ClassName[MaxNameLength + 1];
   bool m_IsValid;
   ResourceHandle m_Handle;

public:
   LuaObject(Component *pComponent, ResourceHandle luaResourceHandle);
   ~LuaObject();

   const bool IsValid() const { return m_IsValid; }
   const char *GetName() { return m_Name; }
   const char *GetClassName() { return m_ClassName; }
   ResourceHandle GetHandle() { return m_Handle; }
};
