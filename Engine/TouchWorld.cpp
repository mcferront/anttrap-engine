#include "EnginePch.h"

#include "TouchWorld.h"
#include "TouchObject.h"

TouchWorld &TouchWorld::Instance( void )
{
   static TouchWorld s_instance;
   return s_instance;
}

void TouchWorld::Destroy( void )
{
   uint32 i, count = m_TouchStages.GetSize( );

   for ( i = 0; i < count; i++ )
   {
      m_TouchStages.GetAt( i )->list.Destroy( );
      delete m_TouchStages.GetAt( i );
   }

   m_TouchStages.Destroy( );
   m_Touches.Destroy( );
}

void TouchWorld::AddTouchStage(
   Id touchStageId,
   float stage
)
{
   TouchStage *pStage = new TouchStage;

   pStage->list.Create( );
   pStage->stage = stage;
   pStage->id = touchStageId;

   uint32 i, count = m_TouchStages.GetSize( );

   for ( i = 0; i < count; i++ )
   {
      TouchStage *pSortedStage = m_TouchStages.GetAt(i);

      if ( stage < pSortedStage->stage )
      {
         m_TouchStages.Insert( pStage, i );
         break;
      }
   }

   if ( i == count )
   {
      m_TouchStages.Add( pStage );
   }
}

void TouchWorld::RemoveTouchStage(
   Id touchStageId
)
{
   uint32 i, count = m_TouchStages.GetSize( );

   TouchStage *pStage;
   
   for ( i = 0; i < count; i++ )
   {
      pStage = m_TouchStages.GetAt(i);

      if ( touchStageId == pStage->id )
      {
         m_TouchStages.RemoveAt( i );
         break;
      }
   }

   if ( i < count )
   {
      pStage->list.Destroy( );
      delete pStage;
   }
}

void TouchWorld::AddObject(
   Id touchStageId,
   TouchObject *pObject
)
{
   TouchStage *pTouchStage = GetTouchStage( touchStageId ); 
   Debug::Assert( Condition(NULL != pTouchStage), "TouchStage %s cannot be found", touchStageId.ToString( ) ); 
   
   if ( List<TouchObject*>::InvalidIndex == pTouchStage->list.GetIndex(pObject) )
   {
      pTouchStage->list.AddUnique( pObject );
   }
}

void TouchWorld::AddObject(
   TouchObject *pObject,
   float stage
)
{
   TouchStage *pTouchStage = GetTouchStage( stage );
   Debug::Assert( Condition(NULL != pTouchStage), "TouchStage %.4f cannot be found", stage ); 

   if ( List<TouchObject*>::InvalidIndex == pTouchStage->list.GetIndex(pObject) )
   {
      pTouchStage->list.AddUnique( pObject );
   }
}

void TouchWorld::RemoveObject(
   TouchObject *pObject
)
{
   uint32 i, count = m_TouchStages.GetSize( );

   for ( i = 0; i < count; i++ )
   {
      TouchStage *pStage = m_TouchStages.GetAt(i);
      if ( pStage->list.Contains(pObject) )
      {
         pStage->list.RemoveSorted( pObject );
         break;
      }
   }

   for ( i = 0; i < m_Touches.GetSize(); i++ )
   {
      TouchDesc *pDesc = m_Touches.GetPointer( i );
      
      if ( pObject == pDesc->pCurrentTouch )
      {
         pDesc->pCurrentTouch = NULL;
      }
   }

   if ( pObject == m_pCurrentHover )
   {
      m_pCurrentHover = NULL;
   }
}

void TouchWorld::BeginTouch(
   int touchId,
   float x,
   float y
)
{
   Vector2 touch = Vector2( x, y );
   
   if ( NULL != m_pCurrentHover )
   {
      m_pCurrentHover->EndHover( touch );
   }

   TouchDesc *pDesc = GetTouchDesc(touchId);

   pDesc->id = touchId;
   pDesc->pCurrentTouch = FindObject( NULL, touch );

   if ( pDesc->pCurrentTouch )
   {
      pDesc->pCurrentTouch->BeginTouch( touch );

      pDesc->touchStart.touch = touch;
      pDesc->touchStart.clock.Start( );
   }
}

void TouchWorld::Touch(
   int touchId,
   float x,
   float y
)
{
   Vector2 touch = Vector2( x, y );

   TouchDesc *pDesc = GetTouchDesc(touchId);

   if ( NULL  == pDesc->pCurrentTouch ||
        false == pDesc->pCurrentTouch->ClampsTouch() )
   {
       TouchObject *pTouchObject = FindObject( pDesc->pCurrentTouch, touch );
       
       if ( pDesc->pCurrentTouch && pTouchObject != pDesc->pCurrentTouch )
       {
          //resetting the touch start position
          //because if they've swiped to another object
          //a swipe for the previous one is no longer applicable
          pDesc->touchStart.touch = touch;
          pDesc->touchStart.clock.Start( );
          pDesc->pCurrentTouch->CancelTouch( touch );
       }

       pDesc->pCurrentTouch = pTouchObject;
   }

   if ( pDesc->pCurrentTouch )
   {
      pDesc->pCurrentTouch->Touch( touch );
   }
}

void TouchWorld::EndTouch(
   int touchId,
   float x,
   float y
)
{
   Vector2 touch = Vector2( x, y );
 
   TouchDesc *pDesc = GetTouchDesc(touchId);
     
   TouchObject *pTouchObject = FindObject( pDesc->pCurrentTouch, touch );

   if ( pDesc->pCurrentTouch && pTouchObject == pDesc->pCurrentTouch )
   {
      Vector2 delta = touch - pDesc->touchStart.touch;
      float time = pDesc->touchStart.clock.TestSample( );

      float distance = Math::Magnitude(delta);
      float velocity = distance / time;

      if ( velocity > 500 )
      {
         pDesc->pCurrentTouch->CancelTouch( touch );
         pDesc->pCurrentTouch->Swipe( delta );
      }
      else
      {
         //if it wasn't a swipe then complete
         //the touch commands
         pDesc->pCurrentTouch->EndTouch( touch );
      }
   }
   else if ( pDesc->pCurrentTouch )
   {
      pDesc->pCurrentTouch->CancelTouch( touch );
   }

   pDesc->pCurrentTouch = NULL;
   RemoveTouchDesc( touchId );
}  

void TouchWorld::Hover(
   float x,
   float y
)
{
   Vector2 touch = Vector2( x, y );
   
   TouchObject *pHover = FindObject( NULL, touch );

   if ( m_pCurrentHover != pHover )
   {
      if ( NULL != m_pCurrentHover ) 
      {
         m_pCurrentHover->EndHover( touch );
      }

      if ( NULL != pHover )
      {
         pHover->BeginHover( touch );
         m_pCurrentHover = pHover;
      }
   }
   else if ( NULL != m_pCurrentHover )
   {
      m_pCurrentHover->Hover( touch );
   }
}

TouchWorld::TouchStage *TouchWorld::GetTouchStage(
   Id touchStageId
)
{
   uint32 i, count = m_TouchStages.GetSize( );

   for ( i = 0; i < count; i++ )
   {
      TouchStage *pStage = m_TouchStages.GetAt(i);

      if ( touchStageId == pStage->id )
      {
         return pStage;
      }
   }

   return NULL;
}

TouchWorld::TouchStage *TouchWorld::GetTouchStage(
   float stage
)
{
   uint32 i, count = m_TouchStages.GetSize( );

   for ( i = 0; i < count; i++ )
   {
      TouchStage *pStage = m_TouchStages.GetAt(i);
      
      if ( pStage->stage == stage )
      {
         return pStage;
      }
   }

   return NULL;
}

TouchObject *TouchWorld::FindObject(
   TouchObject *pCurrentTouch,
   const Vector2 &touch
)
{
   if ( NULL != pCurrentTouch )
   {
      const float scale = 1.50f;

      //scale up the precision of the object
      //we are currently touching - make the ui
      //feel more responsive because fingers tend to slide
      //as they are lifting up
      if ( true == pCurrentTouch->InRange(touch.x, touch.y, scale) )
      {
         return pCurrentTouch;
      }
   }

   int i, count = m_TouchStages.GetSize( );

   //go backwards through the stages
   //because the last thing added
   //should be the first one to check for touch events
   for ( i = count - 1; i >= 0; i-- )
   {
      TouchStage *pStage = m_TouchStages.GetAt(i);
      
      int c, listSize = pStage->list.GetSize( );

      for ( c = listSize - 1; c >= 0; c-- )
      {
         if ( true == pStage->list.GetAt(c)->InRange(touch.x, touch.y, 1.0f) )
         {
            return pStage->list.GetAt( c );
         }
      }
   }

   return NULL;
}

TouchWorld::TouchDesc *TouchWorld::GetTouchDesc(
   int touchId
)
{
   uint32 i;
   
   for ( i = 0; i < m_Touches.GetSize(); i++ )
   {
      TouchDesc *pDesc = m_Touches.GetPointer( i );
      if ( pDesc->id == touchId ) return pDesc;
   }
   
   TouchDesc desc;
   desc.id = touchId;
   desc.touchStart.touch = Math::ZeroVector2( );
   desc.pCurrentTouch = NULL;

   m_Touches.Add(desc);
   
   return m_Touches.GetPointer( i );
}

void TouchWorld::RemoveTouchDesc(
   int touchId
)
{
   uint32 i;
   
   for ( i = 0; i < m_Touches.GetSize(); i++ )
   {
      TouchDesc *pDesc = m_Touches.GetPointer( i );
      if ( pDesc->id == touchId )
      {
         m_Touches.RemoveAt( i );
         break;
      }
   }
}
