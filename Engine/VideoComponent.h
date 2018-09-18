#pragma once

#include "EngineGlobal.h"
#include "Component.h"
#include "SystemId.h"
#include "ResourceWorld.h"
#include "VideoPlayer.h"

class VideoDesc;

class VideoComponent : public Component
{
public:
   DeclareComponentType(VideoComponent);

private:
   VideoDesc *m_pVideo;
   
public:
   VideoComponent( void )
   {}

   void Create(
      const char *pPath,
      Channel *pEventChannel
   );

   virtual void Destroy( void );

   void Play( void );
   void Stop( void );

   void Seek(
      VideoPlayer::Seek seek,
      float seconds
   );

   void AddToScene( void );
   void RemoveFromScene( void );

   float GetTime( void );
};

class Database;

class VideoManager
{
public:
   static VideoManager &Instance( void );

private:
   List<VideoDesc*>  m_Videos;
   uint32 m_AccessId;

   Database *m_pDatabase;
   char      m_VideoPackFile[ 256 ];

public:
   void Create( 
      const char *pVideoPackFile
   );

   void Destroy( void );

   VideoDesc *LoadVideo(
      const char *pPath,
      Channel *pChannel
   );

   void UnloadVideo(
      VideoDesc *pDesc
   );

   void PlayVideo(
      VideoDesc *pDesc
   );

   void StopVideo(
      VideoDesc *pDesc
   );

   void SeekVideo(
      VideoDesc *pDesc,
      VideoPlayer::Seek seek,
      float seconds
   );

   void ShowVideo(
      VideoDesc *pDesc
   );

   void HideVideo(
      VideoDesc *pDesc
   );

   float GetVideoTime(
      VideoDesc *pDesc
   );

private:
   void UnloadLowIds( 
      VideoDesc *pDesc
   );
   void UnloadAll   ( void );
};

class VideoDesc
{
   friend class VideoManager;
   
private:
   VideoPlayer  *m_pPlayer;
   Channel      *m_pChannel;
   float         m_CurrentTime;
   uint32        m_AccessId;
   bool          m_Visible;
   
   uint32 m_Size;
   uint32 m_Offset;

   //this is a full path, not just a name
   //so don't cap at MaxNameLength
   char   m_Path[ 256 ];
   
public:
   VideoDesc(
      const char *pPath,
      uint32 offset,
      uint32 size,
      Channel *pChannel
   );

   void Unload( void );

   void Play( void );

   void Stop( void );

   void Seek(
      VideoPlayer::Seek seek,
      float seconds
   );

   void ShowVideo( void );

   void HideVideo( void );

   float GetTime( void );

private:
   void LoadPlayer( void );
};