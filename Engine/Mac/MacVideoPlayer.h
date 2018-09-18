#pragma once

#include "EngineGlobal.h"
#include "Channel.h"
#include "ResourceWorld.h"
#include "Sprite.h"

class VideoPlayer;
class Material;
class Texture;

class VideoSprite : public Sprite
{   
private:
   VideoPlayer *m_pPlayer;
   
public:
   void SetPlayer( 
      VideoPlayer *pPlayer
   )
   {
      m_pPlayer = pPlayer;
   }
   
   virtual void PreRender(
      RenderDescList *pList
   );
};

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
   
   void PrepareImage( void );
   
private:
   VideoSprite    m_Sprite;
   Texture       *m_pTexture;
   Material      *m_pMaterial;
   Channel       *m_pChannel;
   
public:
   int player;
   
private:
   void OnMovieComplete( void );
   
private:
   static void MovieComplete( 
      nuint param
   );
};
