#pragma once

#include "EngineGlobal.h"
#include "HashTable.h"
#include "SystemId.h"
#include "StringPool.h"
#include "Resource.h"

class Channel;

class InputMap
{
private:
   const char *m_pMapping;
   mutable bool m_NewPress;
   mutable bool m_NewRelease;

public:
   InputMap( void ) {}

   InputMap(
      const char *pMapping
   );

   ~InputMap( void )
   {
   }

   bool IsPressed( void ) const;
   bool IsReleased( void ) const;

   bool IsNewPress( void ) const;
   bool IsNewRelease( void ) const;

   void SetValue( float v );
   
   void Press  ( void );
   void Release( void );

   float GetValue( void ) const;
};

class InputSystem
{
public:
   static InputSystem &Instance( void );

private:
   HashTable<const char *, float> m_States;
   ResourceHandle m_Channel;

public:
   void Create ( void );
   void Destroy( void );

   float GetValue(
      const char *pMapping
   );

   void SetValue(
      const char *pMapping,
      float state
   );

   void KeyDown(
      int keyCode,
      const char *pModifiers
   );

   void KeyUp(
      int keyCode,
      const char *pModifiers
   );

   Channel *GetChannel( void ) { return m_Channel.GetChannel(); }
};
