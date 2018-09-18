#pragma once

#include "EngineGlobal.h"
#include "ShaderAsset.h"
#include "Renderer.h"

struct LightDesc;

class Material : public Asset
{
    friend class MaterialSerializer;
public:
    class Pass
    {
        friend class Material;
        friend class MaterialSerializer;

    public:
        int GetMatrixMacroIndex(
            const char *pMacro
            ) const
        {
            int i;

            for ( i = 0; i < header.numMatrix4Names; i++ )
            {
                if ( StringRefEqual(pMatrix4s[ i ].pRef, pMacro) )
                    break;
            }

            return i < header.numMatrix4Names ? i : -1;
        }

        void SetMacro(
            const char *pMacro,
            const Matrix *pMatrices,
            int numMatrices
            )
        {
            for ( int i = 0; i < header.numMatrix4Names; i++ )
            {
                if ( StringRefEqual(pMatrix4s[ i ].pRef, pMacro) )
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
            memcpy( &pMatrixValues[pMatrix4s[index].offset], pMatrices, sizeof(Matrix) * numMatrices );
        }

        int GetVectorMacroIndex(
            const char *pMacro
            ) const
        {
            int i;

            for ( i = 0; i < header.numFloat4Names; i++ )
            {
                if ( StringRefEqual(pFloat4s[ i ].pRef, pMacro) )
                    break;
            }

            return i < header.numFloat4Names ? i : -1;
        }

        void SetMacro(
            const char *pMacro,
            const Vector *pVectors,
            int numVectors
            )
        {
            for ( int i = 0; i < header.numFloat4Names; i++ )
            {
                if ( StringRefEqual(pFloat4s[ i ].pRef, pMacro) )
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
            memcpy( &pVector4Values[pFloat4s[index].offset], pVectors, sizeof(Vector) * numVectors );
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
        void CloneTo( Pass *pPass ) const
        {
            pPass->header = header;
            pPass->pName = StringRef( pName );
            pPass->shader = shader;

            pPass->pFloat4s = NULL;
            pPass->pMatrix4s = NULL;
            pPass->pMatrixValues = NULL;
            pPass->pVector4Values = NULL;
            pPass->pTextures = NULL;

            pPass->pVector4Values = new Vector[ pPass->header.totalFloat4s ];
            pPass->pFloat4s = new Material::Pass::Float4[ pPass->header.numFloat4Names ];

            pPass->pMatrixValues = new Matrix[ pPass->header.totalMatrix4s ];
            pPass->pMatrix4s = new Material::Pass::Matrix4[ pPass->header.numMatrix4Names ];

            pPass->pTextures = new Material::Pass::Texture[ pPass->header.numTextures ];

            for ( int c = 0; c < pPass->header.numTextures; c++ )
            {
                pPass->pTextures[ c ].pName = StringRef( pTextures[ c ].pName );
                pPass->pTextures[ c ].texture = pTextures[ c ].texture;
                pPass->pTextures[ c ].header = pTextures[ c ].header;
            }

            for ( int c = 0; c < pPass->header.numFloat4Names; c++ )
            {
                pPass->pFloat4s[ c ].pName = StringRef( pFloat4s[ c ].pName );
                pPass->pFloat4s[ c ].pRef = StringRef( pFloat4s[ c ].pRef );
                pPass->pFloat4s[ c ].offset = pFloat4s[ c ].offset;
            }

            for ( int c = 0; c < pPass->header.numMatrix4Names; c++ )
            {
                pPass->pMatrix4s[ c ].pName = StringRef( pMatrix4s[ c ].pName );
                pPass->pMatrix4s[ c ].pRef = StringRef( pMatrix4s[ c ].pRef );
                pPass->pMatrix4s[ c ].offset = pMatrix4s[ c ].offset;
            }

            memcpy( pPass->pMatrixValues, pMatrixValues, sizeof(Matrix) * pPass->header.totalMatrix4s );
            memcpy( pPass->pVector4Values, pVector4Values, sizeof(Vector) * pPass->header.totalFloat4s );
        }

        void CopyValuesTo( Pass *pPass ) const
        {
            for ( int c = 0; c < pPass->header.numTextures; c++ )
                pPass->pTextures[ c ].texture = pTextures[ c ].texture;

            memcpy( pPass->pMatrixValues, pMatrixValues, sizeof(Matrix) * pPass->header.totalMatrix4s );
            memcpy( pPass->pVector4Values, pVector4Values, sizeof(Vector) * pPass->header.totalFloat4s );
        }

        ~Pass( void )
        {
            StringRel( pName );

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
            delete[ ] pVector4Values;
            delete[ ] pMatrix4s;
            delete[ ] pMatrixValues;
            delete[ ] pTextures;
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
                D3DTEXTUREADDRESS address;
                D3DTEXTUREFILTERTYPE filter;
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

        Header header;
        const char *pName;
        ResourceHandle shader;

        Float4 *pFloat4s;
        Texture *pTextures;
        Matrix4 *pMatrix4s;

        Vector *pVector4Values;
        Matrix *pMatrixValues;
    };

public:
    DeclareResourceType( Material );

    typedef HashTable<const char *, List<Material::Pass*> *> PassAllocatorHash;
    typedef HashTable<Id, PassAllocatorHash*> MaterialIdHash;
    static MaterialIdHash s_MaterialHash;

private:
    Pass    *m_pPasses;
    int      m_NumPasses;

public:
    void Destroy( void );

    void Submit(
        const Material::Pass *pPass
        ) const;

    bool HasPass(
        const char *pPass
        ) const;

    Pass *AllocPass(
        const char *pName
        ) const;

    void FreePass(
        Pass *pPass
        ) const;
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
};
