#pragma once

#include "EngineGlobal.h"
#include "StringPool.h"

class Channel;

struct ResourceType 
{ 
friend class ResourceWorld;

private:
   const char *m_pType;

   const ResourceType *m_pBaseType;

public:
   ResourceType( void )
   {
      m_pType = NULL;
      m_pBaseType = NULL;
   }

   bool operator == (const ResourceType &rhs) const
   {
      return 0 == memcmp( this, &rhs, sizeof(rhs) );
   }

   const char *ToString( ) const { return m_pType; }

   const ResourceType *GetBaseType( void ) const
   { 
      return m_pBaseType;
   }

   Channel *GetChannel( void ) const;

   bool IsTypeOf(const ResourceType &type) const
   { 
     if ( *this == type ) return true; 
     if ( NULL != m_pBaseType ) return m_pBaseType->IsTypeOf(type); 
     
     return false; 
   }

private:
   ResourceType(
      const char *pType,
      const ResourceType *pBaseType
   )
   {
      m_pType = StringRef(pType);
      m_pBaseType = pBaseType;
   }
};
