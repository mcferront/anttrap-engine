#pragma once

#include "EngineGlobal.h"
#include "Asset.h"
#include "Dx12.h"
#include "Renderer.h"

struct VertexElementDesc
{
   D3D12_PRIMITIVE_TOPOLOGY_TYPE topology;
   D3D12_INPUT_ELEMENT_DESC *pElementDescs;
   uint32 numElementDescs;
};

class ComputeMaterial
{
   friend class ComputeMaterialObject;
   friend class MaterialSerializer;
   friend class Material;

private:
   ~ComputeMaterial( void );

public:
   class PassData
   { 
      friend class ComputeMaterial;
      friend class ComputeMaterialObject;
      friend class MaterialSerializer;

   public:
      PassData( void )
      {
      }

      const char *GetName( void ) const { return pName; }

      void SetMacro(
         const char *pMacro,
         const Vector *pVectors,
         int numVectors
      )
      {
         for ( int i = 0; i < header.numFloat4Names; i++ )
         {
            if ( StringRefEqual( pFloat4s[ i ].pRef, pMacro ) )
            {
               SetMacro( i, pVectors, numVectors );
               break;
            }
         }

         static const char *pGroupDiv = StringRef("GroupDiv");

         if ( pMacro == pGroupDiv && numVectors == 1 )
         {
            header.div.x = (int) pVectors[0].x;
            header.div.y = (int) pVectors[0].y;
         }
      }

      void SetMacro(
         const char *pMacro,
         const Matrix *pMatrices,
         int numMatrices
      )
      {
         for ( int i = 0; i < header.numMatrix4Names; i++ )
         {
            if ( StringRefEqual( pMatrix4s[ i ].pRef, pMacro ) )
            {
               SetMacro( i, pMatrices, numMatrices );
               break;
            }
         }
      }

      void SetMacro(
         const char *pMacro,
         ResourceHandle rh
      )
      {
         for ( int i = 0; i < header.numBuffers; i++ )
         {
            if ( StringRefEqual(pBuffers[i].pName, pMacro) )
            {
               pBuffers[i].buffer = rh;
               break;
            }
         }

         static const char *pGroupRef = StringRef("GroupSizeTarget");

         if ( pMacro == pGroupRef )
            groupSizeTarget = rh;
      }

      void SetMacro(
         int index,
         const Vector *pVectors,
         int numVectors
      )
      {
         Debug::Assert( Condition( index >= 0 && index < header.numFloat4Names ), "Invalid Index" );
         memcpy( constantBuffer.pData + pFloat4s[ index ].offset, pVectors, sizeof( Vector ) * numVectors );
      }

      void SetMacro(
         int index,
         const Matrix *pMatrices,
         int numMatrices
      )
      {
         Debug::Assert( Condition( index >= 0 && index < header.numMatrix4Names ), "Invalid Index" );
         
         Matrix *pPassMatrices = (Matrix *) (constantBuffer.pData + pMatrix4s[ index ].offset);

         for ( int i = 0; i < numMatrices; i++ )
            Math::Transpose( &pPassMatrices[ i ], pMatrices[ i ] );
      }

   private:
      ~PassData( void )
      {
         shader = NullHandle;
         groupSizeTarget = NullHandle;

         for ( int c = 0; c < header.numBuffers; c++ )
         {
            pBuffers[ c ].buffer = NullHandle;
            StringRel( pBuffers[ c ].pName );
         }

         for ( int c = 0; c < header.numFloat4Names; c++ )
         {
            StringRel( pFloat4s[ c ].pName );
            StringRel( pFloat4s[ c ].pRef );
         }

         for ( int c = 0; c < header.numMatrix4Names; c++ )
         {
            StringRel( pMatrix4s[ c ].pName );
            StringRel( pMatrix4s[ c ].pRef );
         }

         delete[ ] pFloat4s;
         delete[ ] pMatrix4s;
         delete[ ] pBuffers;

         free( constantBuffer.pData );

         GpuDevice::Instance( ).FreeViewHandles( &viewHandles );
      }

      void CloneTo( 
         PassData *pPassData 
      ) const;

      int GetVectorMacroIndex(
         const char *pMacro
      ) const
      {
         int i;

         for ( i = 0; i < header.numFloat4Names; i++ )
         {
            if ( StringRefEqual( pFloat4s[ i ].pRef, pMacro ) )
               return i;
         }

         return -1;
      }

      int GetMatrixMacroIndex(
         const char *pMacro
      ) const
      {
         int i;

         for ( i = 0; i < header.numMatrix4Names; i++ )
         {
            if ( StringRefEqual( pMatrix4s[ i ].pRef, pMacro ) )
               return i;
         }

         return -1;
      }

      struct Header
      {
         struct _group
         {
            byte x;
            byte y;
            byte z;
         };

         struct _div
         {
            byte x;
            byte y;
         };

         byte dyn_group;

         union
         {
            _group group;
            _div div;
         };

         byte numFloat4Names;
         byte totalFloat4s;
         byte numMatrix4Names;
         byte totalMatrix4s;
         byte numBuffers;
      };

      struct Buffer
      {
         enum Type
         {
            SRV,
            UAV,
         };

         struct Header
         {
            byte hasSampler;
            byte address;
            byte filter;
            byte type;
         };

         const char *pName;
         Header header;
         ResourceHandle buffer;
      };

      struct Float4
      {
         const char *pName;
         const char *pRef;
         int offset;
      };

      struct Matrix4
      {
         const char *pName;
         const char *pRef;
         int offset;
      };

   private:
      Header header;
      ResourceHandle shader;
      ResourceHandle groupSizeTarget;
      
      D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;

      const char *pName;
      Buffer *pBuffers;
      Float4 *pFloat4s;
      Matrix4 *pMatrix4s;

      GpuDevice::ConstantBuffer constantBuffer;
      GpuDevice::ViewHandle viewHandles;
   };

private:
   PassData *m_pPassDatas;
   int m_NumPasses;
};

class GraphicsMaterial
{
   friend class GraphicsMaterialObject;
   friend class MaterialSerializer;
   friend class Material;

private:
   ~GraphicsMaterial( void );

public:
   class PassData
   {
      friend class GraphicsMaterial;
      friend class GraphicsMaterialObject;
      friend class MaterialSerializer;

   public:
      PassData( void ) 
      {
      }

      int GetMatrixMacroIndex(
         const char *pMacro
      ) const
      {
         int i;

         for ( i = 0; i < header.numMatrix4Names; i++ )
         {
            if ( StringRefEqual( pMatrix4s[ i ].pRef, pMacro ) )
               return i;
         }

         return -1;
      }

      void SetMacro(
         const char *pMacro,
         const Matrix *pMatrices,
         int numMatrices
      )
      {
         for ( int i = 0; i < header.numMatrix4Names; i++ )
         {
            if ( StringRefEqual( pMatrix4s[ i ].pRef, pMacro ) )
            {
               SetMacro( i, pMatrices, numMatrices );
               break;
            }
         }
      }

      void SetMacro(
         int index,
         const Matrix *pMatrices,
         int numMatrices
      )
      {
         Debug::Assert( Condition( index >= 0 && index < header.numMatrix4Names ), "Invalid Index" );

         Matrix *pPassMatrices = (Matrix *) (constantBuffer.pData + pMatrix4s[ index ].offset);

         for ( int i = 0; i < numMatrices; i++ )
            Math::Transpose( &pPassMatrices[ i ], pMatrices[ i ] );
      }

      int GetVectorMacroIndex(
         const char *pMacro
      ) const
      {
         int i;

         for ( i = 0; i < header.numFloat4Names; i++ )
         {
            if ( StringRefEqual( pFloat4s[ i ].pRef, pMacro ) )
               return i;
         }

         return -1;
      }

      void SetMacro(
         const char *pMacro,
         const Vector *pVectors,
         int numVectors
      )
      {
         for ( int i = 0; i < header.numFloat4Names; i++ )
         {
            if ( StringRefEqual( pFloat4s[ i ].pRef, pMacro ) )
            {
               SetMacro( i, pVectors, numVectors );
               break;
            }
         }
      }

      void SetMacro(
         int index,
         const Vector *pVectors,
         int numVectors
      )
      {
         Debug::Assert( Condition( index >= 0 && index < header.numFloat4Names ), "Invalid Index" );
         memcpy( constantBuffer.pData + pFloat4s[ index ].offset, pVectors, sizeof( Vector ) * numVectors );
      }

      ResourceHandle GetTexture(
         int index
      ) const
      {
         if ( index < header.numTextures )
            return pTextures[ 0 ].texture;
         else
            return NullHandle;
      }

   private:
      void CloneTo( 
         PassData *pPassData 
      ) const;

      ~PassData( void )
      {
         shader = NullHandle;

         for ( int c = 0; c < header.numTextures; c++ )
         {
            pTextures[ c ].texture = NullHandle;
            StringRel( pTextures[ c ].pName );
         }

         for ( int c = 0; c < header.numFloat4Names; c++ )
         {
            StringRel( pFloat4s[ c ].pName );
            StringRel( pFloat4s[ c ].pRef );
         }

         for ( int c = 0; c < header.numMatrix4Names; c++ )
         {
            StringRel( pMatrix4s[ c ].pName );
            StringRel( pMatrix4s[ c ].pRef );
         }

         delete[ ] pFloat4s;
         delete[ ] pMatrix4s;
         delete[ ] pTextures;

         free( constantBuffer.pData );

         GpuDevice::Instance( ).FreeViewHandles( &viewHandles );
      }

      struct Header
      {
         byte numFloat4Names;
         byte totalFloat4s;
         byte numMatrix4Names;
         byte totalMatrix4s;
         byte numTextures;
         byte depthTest;
         byte depthWrite;
         byte depthFunc;
         byte cullMode;
         byte sourceBlend;
         byte destBlend;
         byte blendEnable;
      };

      struct Texture
      {
         struct Header
         {
            int address;
            int filter;
         };

         const char *pName;
         Header header;
         ResourceHandle texture;
      };

      struct Float4
      {
         const char *pName;
         const char *pRef;
         int offset;
      };

      struct Matrix4
      {
         const char *pName;
         const char *pRef;
         int offset;
      };

      GpuDevice::ConstantBuffer constantBuffer;

      Header header;
      ResourceHandle shader;

      Float4 *pFloat4s;
      Texture *pTextures;
      Matrix4 *pMatrix4s;
      GpuDevice::ViewHandle viewHandles;

      D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;

      const char *pName;
   };

private:
   PassData *m_pPassDatas;
   int m_NumPasses;

public:
   static bool CreateConstantBuffer(
      GpuDevice::ConstantBuffer *pBuffer,
      uint32 sizeInBytes
   );
};

class Material : public Asset
{
   friend class MaterialSerializer;

public:
   DeclareResourceType( Material );

public:
   GraphicsMaterial *GetGraphicsMaterial( void ) const { return m_pGraphicsMaterial; }
   ComputeMaterial *GetComputeMaterial( void ) const { return m_pComputeMaterial; }

   void Destroy( void );

private:
   GraphicsMaterial *m_pGraphicsMaterial;
   ComputeMaterial *m_pComputeMaterial;
};


class MaterialSerializer : public ISerializer
{
public:
   virtual bool Serialize(
      Serializer *pSerializer,
      const ISerializable *pSerializable
      )
   {
      return false;
   }

   virtual ISerializable *Deserialize(
      Serializer *pSerializer,
      ISerializable *pSerializable
      );

   virtual ISerializable *Instantiate( ) const { return new Material; }

   virtual const SerializableType &GetSerializableType( void ) const { return Material::StaticSerializableType( ); }

   virtual uint32 GetVersion( void ) const { return 1; }

private:
   ISerializable *DeserializeGraphicsMaterial(
      Serializer *pSerializer,
      ISerializable *pSerializable
   );

   ISerializable *DeserializeComputeMaterial(
      Serializer *pSerializer,
      ISerializable *pSerializable
   );
};
