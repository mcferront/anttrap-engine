#pragma once

#include "EngineGlobal.h"

#include "Component.h"
#include "TouchEvent.h"

class TouchComponent : public Component
{
public:
   DeclareComponentType(TouchComponent);

private:
   TouchEvent m_TouchEvent;

public:
   virtual void Create(
      Id id,
      const Vector2 &size,
      Id touchStageId
   );

   virtual void Destroy( void );
   
   virtual void AddToScene( void );

   virtual void RemoveFromScene( void );
   
   void SetClampsTouch( bool clamps ) { m_TouchEvent.SetClampsTouch(clamps);}
   
   Id GetTouchStageId( void ) const { return m_TouchEvent.GetTouchStageId( ); }
   void SetTouchStageId( Id touchStageId ) { m_TouchEvent.SetTouchStageId( touchStageId ); }
};
