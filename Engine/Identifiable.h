#pragma once

#include "EngineGlobal.h"
#include "SystemId.h"

class Identifiable
{
private:
   Id m_Id;

public:
   virtual ~Identifiable( void ) {}

   virtual void Create( Id id )
   {
      m_Id = id;
   }

   virtual void Destroy( void ) {}

   void SetId( Id id ) { m_Id = id; }
   Id GetId( void ) const { return m_Id; }
};
