#import "MacObjCVideoPlayer.h"

#include "VideoPlayer.h"
#include "Material.h"
#include "Texture.h"

void VideoSprite::PreRender(
   RenderDescList *pList
)
{
   Sprite::PreRender( pList );   

   m_pPlayer->PrepareImage( );   
}

void VideoPlayer::Load(
   const char *pPath,
   uint32 offset,
   uint32 size,
   Channel *pChannel
)
{
   player = (int) [ObjCVideoPlayer alloc];   
   [((ObjCVideoPlayer*)player) load:pPath offset:offset size:size movieEnd:&VideoPlayer::MovieComplete param:(nuint)this];

   
   //build our display sprite, texture
   //i would prefer the sprite wasn't a memeber of this class and this class
   //simply drew to a texture - but we're mimicing iOS behaviour and that renders
   //directly the the back buffer
   ResourceHandle videoTexture( SystemId::Create() );      
   ResourceHandle videoMaterial( SystemId::Create() );
      
   m_pMaterial = new Material;
   m_pMaterial->Create( videoMaterial.GetSystemId(), videoTexture, Vector(1,1,1,1), false, false, false, false, true );
   m_pMaterial->UseVideoShader( true );
   m_pMaterial->AddToScene( );

   m_pTexture = new Texture;
   m_pTexture->Create( videoTexture.GetSystemId() );
   m_pTexture->SetTexture( 0, 1024, 768, 1024, 768, false );
   m_pTexture->AddToScene( );
      
   m_Sprite.Create( Math::IdentityTransform, videoMaterial, Vector2(1024, 768), SystemId("Video") );
   m_Sprite.SetPlayer( this );
   
   m_pChannel = pChannel;
}

void VideoPlayer::Unload( void )
{
   m_Sprite.RemoveFromScene( );
   m_Sprite.Destroy( );
   
   m_pMaterial->RemoveFromScene( );
   m_pMaterial->Destroy( );
   
   m_pTexture->RemoveFromScene( );
   m_pTexture->Destroy( );
   
   delete m_pMaterial;
   delete m_pTexture;
   
   m_pMaterial = NULL;
   m_pTexture  = NULL;
   m_pChannel  = NULL;
   
   [((ObjCVideoPlayer*)player) unload];
   [((ObjCVideoPlayer*)player) release];
}

void VideoPlayer::Play( void )
{
   [((ObjCVideoPlayer*)player) play];
}

void VideoPlayer::Stop( void )
{
   [((ObjCVideoPlayer*)player) stop];
}

float VideoPlayer::GetTime( void )
{
   return [((ObjCVideoPlayer*)player) getTime];
}

void VideoPlayer::SeekTo( 
   VideoPlayer::Seek from,
   float seconds
)
{
   if ( from == VideoPlayer::SeekCurrent )
   {
      float currentTime = [((ObjCVideoPlayer*)player) getTime];
      seconds += currentTime;
   }

   [((ObjCVideoPlayer*)player) seek:seconds];
}

void VideoPlayer::Show( void )
{
   m_Sprite.AddToScene( );
}

void VideoPlayer::Hide( void )
{
   m_Sprite.RemoveFromScene( );
}

void VideoPlayer::PrepareImage( void )
{
   GLuint image, target;
   [((ObjCVideoPlayer*)player) getImage:&target texture:&image];

   m_pTexture->SetTexture( image, 
                           m_pTexture->GetActualWidth( ),  m_pTexture->GetActualHeight( ), 
                           m_pTexture->GetDesiredWidth( ), m_pTexture->GetDesiredHeight( ), false );

   m_pTexture->SetTextureType( target );
}

void VideoPlayer::OnMovieComplete( void )
{
   m_pChannel->QueueEvent( "Finished", ArgList() );
}

void VideoPlayer::MovieComplete( 
   nuint param
)
{
   ((VideoPlayer *) param)->OnMovieComplete( );
}

