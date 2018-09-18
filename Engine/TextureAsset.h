#pragma once

#include "EngineGlobal.h"
#include "Asset.h"
#include "Serializer.h"

#ifdef OPENGL
#include "GlShader.h"

class SerializableTexture;

class Texture : public Asset
{
friend class TextureSerializer;
private:
   struct Header
   {
      uint32 version;
      uint32 desiredWidth, desiredHeight;
      uint32 actualWidth, actualHeight;
      uint32 size, format, type, compressed;
      uint32 pixelFormat, mipLevels;
   };

private:
   void  *m_pData;
   int   *m_pMipSizes;
   Header m_Header;
   GLuint m_glTexture;
   GLuint m_glType;
   int    m_ActualWidth;
   int    m_ActualHeight;
   int    m_DesiredWidth;
   int    m_DesiredHeight;
   int    m_MipLevels;
   bool   m_Owner;
   bool   m_NeedsReload;

public:
   virtual void Create(
      Id id
   )
   {
      Asset::Create( id );
      
      m_Owner     = true;
      m_glTexture = 0;
      m_glType    = GL_TEXTURE_2D;

      m_ActualWidth   = 0;
      m_ActualHeight  = 0;
      m_DesiredWidth  = 0;
      m_DesiredHeight = 0;
      m_NeedsReload   = false;
   }

   virtual void Destroy( void )
   {
      if ( m_glTexture && m_Owner )
      {
         glDeleteTextures( 1, &m_glTexture );
         m_glTexture = 0;
      }

      free( m_pData );
      free( m_pMipSizes );

      Asset::Destroy( );
   }

   GLuint GetTexture( void )        const { return m_glTexture; }
   GLuint GetTextureType   ( void ) const { return m_glType; }
   
   int GetDesiredWidth ( void ) const { return m_DesiredWidth; }
   int GetDesiredHeight( void ) const { return m_DesiredHeight; }

   int GetActualWidth ( void ) const { return m_ActualWidth; }
   int GetActualHeight( void ) const { return m_ActualHeight; }

   bool HasMipMaps( void ) const { return m_MipLevels > 1; }

   DeclareResourceType(Texture);

   //this is only here so we can get textures
   //from the ios ui / objective-c
   void Override(
      GLuint glTexture,
      int    width,
      int    height
   )
   {
      glDeleteTextures( 1, &m_glTexture );

      m_glTexture = glTexture;
      m_DesiredWidth  = width;
      m_DesiredHeight = height;
      m_ActualWidth   = width;
      m_ActualHeight  = height;
   }

   void SetTextureType(GLuint type)
   {
      m_glType = type;
   }

   void SetTexture( 
      GLuint glTexture,
      int actualWidth,
      int actualHeight,
      int desiredWidth,
      int desiredHeight,
      bool iOwnTexture 
   )
   {
      if ( m_glTexture && m_Owner )
      {
         glDeleteTextures( 1, &m_glTexture );
         m_glTexture = 0;
      }

      m_Owner     = iOwnTexture;
      m_glTexture = glTexture;
   
      m_ActualWidth   = actualWidth;
      m_ActualHeight  = actualHeight;
      m_DesiredWidth  = desiredWidth;
      m_DesiredHeight = desiredHeight;
   }

   void Reload( void );
};

class TextureSerializer : public ISerializer
{
public:
    virtual bool Serialize(
        Serializer *pSerializer,
        const ISerializable *pSerializable
        ) { return false; }

    virtual ISerializable *Deserialize(
        Serializer *pSerializer,
        ISerializable *pSerializable
        );

    virtual ISerializable *Instantiate() const { return new Texture; }

    virtual const SerializableType &GetSerializableType( void ) const { return Texture::StaticSerializableType( ); }

    virtual uint32 GetVersion( void ) const { return 2; }
};

#elif defined DIRECTX9

class Texture : public Asset
{
friend class TextureSerializer;

private:
   IDirect3DTexture9 *m_pTexture;

   int    m_ActualWidth;
   int    m_ActualHeight;
   int    m_DesiredWidth;
   int    m_DesiredHeight;
   bool   m_HasMips;
   bool   m_Owner;

public:
   virtual void Create(  )
   {
      m_Owner    = true;
      m_pTexture = 0;

      m_ActualWidth   = 0;
      m_ActualHeight  = 0;
      m_DesiredWidth  = 0;
      m_DesiredHeight = 0;
   }

   virtual void Destroy( void )
   {
      //release regardless if we are the owner or not
      //because d3d textures are ref counted
      if ( m_pTexture )
         m_pTexture->Release( );

      m_pTexture = NULL;
      
      Asset::Destroy( );
   }

   IDirect3DTexture9 *GetTexture( void ) const { return m_pTexture; }

   int GetDesiredWidth ( void ) const { return m_DesiredWidth; }
   int GetDesiredHeight( void ) const { return m_DesiredHeight; }

   int GetActualWidth ( void ) const { return m_ActualWidth; }
   int GetActualHeight( void ) const { return m_ActualHeight; }

   bool HasMipMaps( void ) const { return m_HasMips; }

   DeclareResourceType(Texture);

   void SetTexture( 
      IDirect3DTexture9 *pTexture,
      int actualWidth,
      int actualHeight,
      int desiredWidth,
      int desiredHeight,
      bool iOwnTexture 
   )
   {
      //release regardless if we are the owner or not
      //because d3d textures are ref counted
      if ( m_pTexture )
      {
         m_pTexture->Release( );
      }

      m_Owner = iOwnTexture;
      
      m_pTexture = pTexture;

      if ( m_pTexture )
      {
         m_pTexture->AddRef( );
      }

      m_ActualWidth   = actualWidth;
      m_ActualHeight  = actualHeight;
      m_DesiredWidth  = desiredWidth;
      m_DesiredHeight = desiredHeight;
   }
};

class TextureSerializer : public ISerializer
{
public:
    virtual bool Serialize(
        Serializer *pSerializer,
        const ISerializable *pSerializable
        ) { return false; }

    virtual ISerializable *Deserialize(
        Serializer *pSerializer,
        ISerializable *pSerializable
        );

    virtual ISerializable *Instantiate() const { return new Texture; }

    virtual const SerializableType &GetSerializableType( void ) const { return Texture::StaticSerializableType( ); }

    virtual uint32 GetVersion( void ) const { return 2; }
};

#elif defined DIRECTX12
    #include "Dx12Texture.h"
#else
   #error Graphics API Undefined
#endif

typedef Texture ImageBuffer;
