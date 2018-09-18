#pragma once

#include "EngineGlobal.h"
#include "List.h"
#include "SystemId.h"
#include "UtilityClock.h"

class TouchObject;

class TouchWorld
{
private:
   typedef List<TouchObject *> TouchList;

private:
   struct TouchStage
   {
      Id id;

      TouchList list;
      float stage;
   };

   struct TouchStart
   {
      Clock clock;
      Vector2 touch;
   };

   struct TouchDesc
   {
      int id;
      TouchObject *pCurrentTouch;
      TouchStart touchStart;
   };

   typedef List<TouchStage*> TouchStages;
   typedef List<TouchDesc>   TouchDescs;

public:
   static TouchWorld &Instance( void );

private:
   TouchStages  m_TouchStages;
   TouchObject *m_pCurrentHover;
   TouchDescs   m_Touches;
   
public:
   void Create( void ) 
   {
      m_TouchStages.Create( );
      m_Touches.Create( );
      
      m_pCurrentHover = NULL;
   }
   
   void Destroy( void );

   void AddTouchStage(
      Id touchStageId,
      float stage
   );

   void RemoveTouchStage(
      Id touchStageId
   );

   void AddObject(
      Id touchStageId,
      TouchObject *pObject
   );

   void AddObject(
      TouchObject *pObject,
      float stage
   );

   void RemoveObject(
      TouchObject *pObject
   );

   void BeginTouch(
      int touchId,
      float x,
      float y
   );

   void Touch(
      int touchId,
      float x,
      float y
   );

   void EndTouch(
      int touchId,
      float x,
      float y
   );

   void Hover(
      float x,
      float y
   );
   
   void AddTouchStage(
      const char *pTouchStage,
      float stage
   )
   {
      AddTouchStage( Id(pTouchStage), stage );
   }

private:
   TouchStage *GetTouchStage(
      Id touchStageId
   );

   TouchStage *GetTouchStage(
      float stage
   );

   TouchObject *FindObject(
      TouchObject *pCurrentTouch,
      const Vector2 &touch
   );
   
   TouchDesc *GetTouchDesc(
      int touchId
   );
   
   void RemoveTouchDesc(
      int touchId
   );
};
