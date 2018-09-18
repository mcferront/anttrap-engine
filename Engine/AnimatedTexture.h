#pragma once

#include "EngineGlobal.h"
#include "ResourceWorld.h"
#include "TextureAsset.h"

class WrappedTexture;

class AnimatedTexture
{
private:
   ResourceHandle  m_FrameMap;
   WrappedTexture *m_pTexture;

   float  m_CurrentTime;

public:
   void Create(
      ResourceHandle frameMap,
      WrappedTexture *pTexture
   );

   void Destroy( void );

   void Update( 
      float deltaSeconds
   );

   void GetDeltaTransform(
      float startTime,
      float endTime,
      Transform *pDeltaTransform
   ) const;

   void SetLocalTime( 
      float localTime 
   ) 
   { m_CurrentTime = localTime; }

   bool WillLoop( 
      float deltaSeconds 
   ) const;

   float GetDuration ( void ) const;
   float GetLocalTime( void ) const { return m_CurrentTime; }

   ResourceHandle GetFrameMap( void ) const { return m_FrameMap; }
};


class WrappedTexture : public Texture
{
public:
   virtual void Destroy( void )
   {
      Resource::Destroy( );
   }  

   void Copy(
      Texture *pTexture
   )
   {
#ifdef DIRECTX12
       //TODO: DX12
#else
       SetTexture( pTexture->GetTexture( ), 
           pTexture->GetActualWidth( ), 
           pTexture->GetActualHeight( ), 
           pTexture->GetDesiredWidth( ), 
           pTexture->GetDesiredHeight( ), 
           false );
#endif
   }
};
