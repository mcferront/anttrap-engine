#include "EnginePch.h"

#include "LuaObject.h"
#include "LuaVM.h"
#include "Component.h"

LuaObject::LuaObject(Component *pComponent, ResourceHandle luaResourceHandle)
{
    m_IsValid = false;

   Debug::Assert( Condition(IsResourceLoaded(luaResourceHandle)), "luaResourceHandle %s must be loaded", luaResourceHandle.GetId().ToString() );
   
   //m_ID = s_NextID++;
   m_Handle = luaResourceHandle;

   // Generate the class name
   char nameOnly[ MaxNameLength ];
   String::Copy( nameOnly, m_Handle.GetName(), sizeof(nameOnly) );
   
   char *pExt = strrchr(nameOnly, '.');
   if ( pExt ) *pExt = NULL;

   //int i = 0;
   //char tempString[MaxNameLength];
   //String::Copy(tempString, m_Handle.GetId().ToString(), MaxNameLength);
   //char *pName = strrchr(tempString, '/') + 1;
   //while(pName[i] != '.')
   //{
   //   i++;
   //}
   //pName[i] = '\0';
   //String::Copy(m_ClassName, pName, MaxNameLength);
   String::Copy(m_ClassName, nameOnly, sizeof(m_ClassName));

   // Generate the object name
   //String::Format(m_Name, sizeof(m_Name), "%s_%d", GetClassName(), m_ID);
   char idString[256];
   String::Copy( idString, pComponent->GetId().ToString(), sizeof(idString) );

   String::Replace( idString, '/', '_' );
   String::Replace( idString, ':', '_' );
   String::Replace( idString, '-', '_' );

   String::Format(m_Name, sizeof(m_Name), "%s_%s", GetClassName(), idString);
   Debug::Assert( Condition(strlen(m_Name) < sizeof(m_Name) - 2), "LuaObject::LuaObject() - object name is too long." );

   // Initialize the class
   if (true == LuaVM::Instance().ExecuteLuaAsset(m_Handle))
   {
       // Construct the object
       char constructorName[sizeof(m_Name) + 10];
       char constructionScript[ sizeof(constructorName) + sizeof(m_Name) + 10];

       String::Format(constructorName, sizeof(constructorName), "%s:New", GetClassName());
       Debug::Assert( Condition(strlen(m_Name) < sizeof(m_Name) - 2), "LuaObject::LuaObject() - constructor name is too long." );

       String::Format(constructionScript, sizeof(constructionScript), "%s = %s()", m_Name, constructorName);
       LuaVM::Instance().ExecuteString(constructionScript);

       LuaVM::Instance().AddChunkRef(m_Handle);

       m_IsValid = true;
    }
}

LuaObject::~LuaObject()
{
    if (true == m_IsValid)
    {
        // Destruct the object
        char destructionScript[sizeof(m_Name) + 10];

        String::Format(destructionScript, sizeof(destructionScript), "%s = nil", m_Name);
        LuaVM::Instance().ExecuteString(destructionScript);

        LuaVM::Instance().RemoveChunkRef(m_Handle);
    }

    m_Handle = NullHandle;
}
