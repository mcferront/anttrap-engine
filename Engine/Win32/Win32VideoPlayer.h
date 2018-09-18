#pragma once

#include "EngineGlobal.h"
#include "Channel.h"
#include "ResourceWorld.h"
#include "Sprite.h"

struct IGraphBuilder;
struct IMediaControl;
struct IMediaSeeking;
struct IMediaEvent;
struct IBaseFilter;

class Texture;
class Material;
class VideoSurface;

class VideoPlayer
{
public:      
   enum Seek
   {
      SeekBegin,
      SeekCurrent,
   };

private:
   Sprite         m_Sprite;
   IGraphBuilder *m_pGraph;
   IMediaControl *m_pMediaControl;
   IMediaSeeking *m_pMediaSeeking;
   IMediaEventEx *m_pMediaEvent;
   IBaseFilter   *m_pVMR;
   VideoSurface  *m_pVideoSurface;
   Material      *m_pMaterial;
   Channel       *m_pChannel;
   sint64         m_LastRecordedFrame;
   char           m_VideoFile[ MAX_PATH ];

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

   void DoEvents( void );

private:
   void LoadVMR  ( void );
   void UnloadVMR( void );

   void OnDeviceLost(
      const Channel *pSender,
      const char *pName,
      const ArgList &list
   );

   void OnDeviceRestored(
      const Channel *pSender,
      const char *pName,
      const ArgList &list
   );

public:
   static HWND s_MainWindow;
   static char s_VideoPath[ MAX_PATH ];
};
