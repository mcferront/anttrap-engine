#pragma once

#include "EngineGlobal.h"

#if defined WIN32 && defined DIRECTX8
   #include "Win32VideoPlayer.h"
#elif defined MAC
   #include "MacVideoPlayer.h"
#else
   #include "Channel.h"
   #include "ResourceWorld.h"

   class VideoPlayer
   {
   public:      
      enum Seek
      {
         SeekBegin,
         SeekCurrent,
      };

   public:
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
         Seek seek,
         float seconds
      );
      
      void Show( void );
      void Hide( void );

      float GetTime( void );

   #if defined IOS
   public:
      int player;
   #endif
};
#endif
