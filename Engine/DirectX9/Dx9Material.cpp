#include "EnginePch.h"

#include "MaterialAsset.h"
#include "TextureAsset.h"
#include "ShaderAsset.h"
#include "Renderer.h"
#include "Viewport.h"
#include "Dx9.h"
#include "LightComponent.h"
#include "RenderWorld.h"

DefineResourceType( Material, Asset, new MaterialSerializer );

//A hash table for each material
static Lock s_Lock;
Material::MaterialIdHash Material::s_MaterialHash;

void Material::Destroy( void )
{
    delete [] m_pPasses;

    //find the list and free it in the list
    PassAllocatorHash *pPassesHash;

    {
        ScopeLock lock( s_Lock );
        
        if ( false == s_MaterialHash.Remove(GetHandle().GetId(), NULL, &pPassesHash) )
            pPassesHash = NULL;
    }

    if ( NULL != pPassesHash )
    {
        {
            Enumerator<const char *, List<Pass*> *> passEnum = pPassesHash->GetEnumerator( );

            while ( passEnum.EnumNext( ) )
            {
                for ( uint32 i = 0; i < passEnum.Data()->GetSize(); i++ )
                    delete passEnum.Data()->Get(i);

                passEnum.Data()->Destroy();
                delete passEnum.Data();
            }
        }

        pPassesHash->Destroy();
        delete pPassesHash;
    }

    Asset::Destroy( );
}

void Material::Submit(
    const Material::Pass *pPass
    ) const
{
    int i;

    Shader *pShader = GetResource( pPass->shader, Shader );
    pShader->MakeActive( );

    Dx9::Instance( ).GetDevice( )->SetRenderState( D3DRS_ZENABLE, pPass->header.depthTest );
    Dx9::Instance( ).GetDevice( )->SetRenderState( D3DRS_ZFUNC, pPass->header.depthFunc );
    Dx9::Instance( ).GetDevice( )->SetRenderState( D3DRS_ZWRITEENABLE, pPass->header.depthWrite );
    Dx9::Instance( ).GetDevice( )->SetRenderState( D3DRS_CULLMODE, pPass->header.cullMode );

    Dx9::Instance( ).GetDevice( )->SetRenderState( D3DRS_COLORWRITEENABLE,
        D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE |
        D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED );

    for ( i = 0; i < pPass->header.numTextures; i++ )
    {
        if ( IsResourceLoaded( pPass->pTextures[ i ].texture ) )
            pShader->SetTexture( pPass->pTextures[ i ].pName, pPass->pTextures[ i ].texture, pPass->pTextures[ i ].header.address, pPass->pTextures[ i ].header.filter );
    }

    pShader->SetVectors( pPass->pFloat4s[ 0 ].pName, pPass->pVector4Values, pPass->header.totalFloat4s );
    pShader->SetMatrices( pPass->pMatrix4s[ 0 ].pName, pPass->pMatrixValues, pPass->header.totalMatrix4s );

    Dx9::Instance( ).GetDevice( )->SetRenderState( D3DRS_SRCBLEND, pPass->header.sourceBlend );
    Dx9::Instance( ).GetDevice( )->SetRenderState( D3DRS_DESTBLEND, pPass->header.destBlend );
    Dx9::Instance( ).GetDevice( )->SetRenderState( D3DRS_ALPHABLENDENABLE, pPass->header.blendEnable );
}

Material::Pass *Material::AllocPass( 
    const char *pName 
    ) const
{
    const Pass *pSourcePass;
    int i;

    for (i = 0; i < m_NumPasses; i++)
    {
        if (pName == m_pPasses[i].pName)
            break;
    }

    if ( i == m_NumPasses ) return NULL;

    pSourcePass = &m_pPasses[i];

    {
        ScopeLock lock(s_Lock);

        PassAllocatorHash *pPassesHash;

        if ( false == s_MaterialHash.Get(GetHandle().GetId(), &pPassesHash) )
        {
            pPassesHash = new HashTable<const char *, List<Pass*> *>( ); pPassesHash->Create();
            s_MaterialHash.Add(GetHandle().GetId(), pPassesHash);
        }

        List<Pass*> *pPasses;

        if ( false == pPassesHash->Get(pName, &pPasses) )
        {
            pPasses = new List<Pass*>( ); pPasses->Create();
            pPassesHash->Add( StringRef(pName), pPasses );
        }

        Pass *pPass;

        if ( pPasses->GetSize() > 0 )
        {
            pPass = pPasses->Get(0);
            pPasses->Remove((uint32)0);
            pSourcePass->CopyValuesTo(pPass);
        }
        else
        {
            pPass = new Pass;
            pSourcePass->CloneTo(pPass);
        }

        return pPass;
    }
}

bool Material::HasPass( 
    const char *pName 
    ) const
{
    for (int i = 0; i < m_NumPasses; i++)
    {
        if (pName == m_pPasses[i].pName)
            return true;
    }

    return false;
}

void Material::FreePass( 
    Material::Pass *pPass 
    ) const
{
    if ( NULL == pPass )
        return;

    ScopeLock lock( s_Lock );

    //find the list and free it in the list
    PassAllocatorHash *pPassesHash;
        
    if ( false == s_MaterialHash.Get(GetHandle().GetId(), &pPassesHash) )
        Debug::Assert( Condition(false), "How is this material no longer there?" );

    List<Pass*> *pPasses;

    if ( false == pPassesHash->Get(pPass->pName, &pPasses) )
        Debug::Assert( Condition(false), "How is this pass no longer there?" );

    pPasses->Add( pPass );
}

ISerializable *MaterialSerializer::Deserialize(
    Serializer *pSerializer,
    ISerializable *pSerializable
    )
{
    const uint32 Version = 1;

    struct Header
    {
        unsigned int version;
        byte numPasses;
    };

    Header header;

    if ( NULL == pSerializable ) pSerializable = new Material;

    Material *pMaterial = (Material *) pSerializable;
    pSerializer->GetInputStream( )->Read( &header, sizeof( header ), NULL );
    Debug::Assert( Condition( header.version == Version ), "Incorrect material version: %d, expecting: %d", header.version, Version );
    
    Material::Pass *pPasses;
    if ( header.numPasses > 0)
        pPasses= new Material::Pass[ header.numPasses ];
    else
        pPasses = NULL;

    for ( int i = 0; i < header.numPasses; i++ )
    {
        Material::Pass *pPass = &pPasses[ i ];

        pPass->pFloat4s = NULL;
        pPass->pMatrix4s = NULL;
        pPass->pMatrixValues = NULL;
        pPass->pVector4Values = NULL;
        pPass->pTextures = NULL;

        pSerializer->GetInputStream( )->Read( &pPass->header, sizeof( pPass->header ) );

        pPass->pName = StringPool::Deserialize( pSerializer->GetInputStream( ) );
        pPass->shader = ResourceHandle( Id::Deserialize( pSerializer->GetInputStream( ) ) );

        if ( pPass->header.totalFloat4s )
        {
            pPass->pVector4Values = new Vector[ pPass->header.totalFloat4s ];
            pPass->pFloat4s = new Material::Pass::Float4[ pPass->header.numFloat4Names ];
        }

        if ( pPass->header.totalMatrix4s )
        {
            pPass->pMatrixValues = new Matrix[ pPass->header.totalMatrix4s ];
            pPass->pMatrix4s = new Material::Pass::Matrix4[ pPass->header.numMatrix4Names ];
        }

        if ( pPass->header.numTextures )
            pPass->pTextures = new Material::Pass::Texture[ pPass->header.numTextures ];

        for ( int c = 0; c < pPass->header.numTextures; c++ )
        {
            pPass->pTextures[ c ].pName = StringPool::Deserialize( pSerializer->GetInputStream( ) );
            pPass->pTextures[ c ].texture = ResourceHandle( Id::Deserialize( pSerializer->GetInputStream( ) ) );
            pSerializer->GetInputStream( )->Read( &pPass->pTextures[ c ].header, sizeof( pPass->pTextures[ c ].header ) );
        }

        int float4ValueOffset = 0;

        for ( int c = 0; c < pPass->header.numFloat4Names; c++ )
        {
            const char *pName = StringPool::Deserialize( pSerializer->GetInputStream( ) );
            const char *pRef = StringPool::Deserialize( pSerializer->GetInputStream( ) );
            
            int amount;
            pSerializer->GetInputStream()->Read(&amount, sizeof(amount));
                        
            pPass->pFloat4s[ c ].offset = float4ValueOffset;
            float4ValueOffset += amount;

            pPass->pFloat4s[ c ].pName = pName;
            pPass->pFloat4s[ c ].pRef = pRef;
            pSerializer->GetInputStream( )->Read( &pPass->pVector4Values[pPass->pFloat4s[c].offset], sizeof(Vector) );
        }

        int matrix4ValueOffset = 0;

        for ( int c = 0; c < pPass->header.numMatrix4Names; c++ )
        {
            const char *pName = StringPool::Deserialize( pSerializer->GetInputStream( ) );
            const char *pRef = StringPool::Deserialize( pSerializer->GetInputStream( ) );

            int amount;
            pSerializer->GetInputStream()->Read(&amount, sizeof(amount));

            pPass->pMatrix4s[ c ].offset = matrix4ValueOffset;
            matrix4ValueOffset += amount;

            pPass->pMatrix4s[ c ].pName = pName;
            pPass->pMatrix4s[ c ].pRef = pRef;
            pSerializer->GetInputStream( )->Read( &pPass->pMatrixValues[pPass->pMatrix4s[c].offset], sizeof(Matrix) );
        }
    }

    pMaterial->m_NumPasses = header.numPasses;
    pMaterial->m_pPasses = pPasses;

    return pSerializable;
}

