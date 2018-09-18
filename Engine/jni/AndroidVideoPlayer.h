#pragma once

#include "EngineGlobal.h"

#include "Channel.h"

class VideoPlayer
{
public:      
   enum Seek
   {
      SeekBegin,
      SeekCurrent,
   };

private:
    jobject     m_Object;

public:
   VideoPlayer();
   ~VideoPlayer();

   void Load( 
      const char *pPath,
      uint32 offset,
      uint32 size,
      Channel *pChannel
   );
   
   void Unload( void );
   
   void Play( void );
   void Stop( void );

   void SeekTo(
      Seek from,
      float seconds
   );
   
   void Show( void );
   void Hide( void );

   float GetTime( void );
};