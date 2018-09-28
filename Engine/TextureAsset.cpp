#include "EnginePch.h"

#include "TextureAsset.h"
#include "Log.h"

DefineResourceType( Texture, GpuBuffer, new TextureSerializer );

ISerializable *TextureSerializer::Deserialize(
    Serializer *pSerializer,
    ISerializable *pSerializable
)
{
    const uint32 Version = 1;

    struct Header
    {
        uint32 version;
        uint32 desiredWidth, desiredHeight;
        uint32 actualWidth, actualHeight;
        uint32 size, format, type, compressed;
        uint32 pixelFormat, mipLevels;
    };

    Header header;

    if ( NULL == pSerializable ) pSerializable = Instantiate( );

    Texture *pTexture = (Texture *) pSerializable;

    pSerializer->GetInputStream( )->Read( &header, sizeof( header ), NULL );
    Debug::Assert( Condition( header.version == Version ), "Incorrect texture version: %d, expecting: %d", header.version, Version );

    pSerializer->GetInputStream( )->Seek( header.mipLevels * sizeof( int ), SeekCurrent );

    DXGI_FORMAT format;

    //TODO: DX12 - real format support
    switch ( header.format )
    {
    case 21:
        format = DXGI_FORMAT_R8G8B8A8_UNORM;
        break;

    case MAKEFOURCC( 'D', 'X', 'T', '1' ):
        format = DXGI_FORMAT_BC1_UNORM_SRGB;
        break;

    case MAKEFOURCC( 'D', 'X', 'T', '2' ):
    case MAKEFOURCC( 'D', 'X', 'T', '3' ):
        format = DXGI_FORMAT_BC2_UNORM_SRGB;
        break;

    case MAKEFOURCC( 'D', 'X', 'T', '4' ):
        format = DXGI_FORMAT_BC3_UNORM_SRGB;
        break;
    case MAKEFOURCC( 'D', 'X', 'T', '5' ):
        format = DXGI_FORMAT_BC3_UNORM;
        break;

    default:
        Debug::Assert( Condition( false ), "Unsupported texture format" );
    }

    pTexture->Create( GpuBuffer::Heap::Default, GpuBuffer::State::CopyDest, GpuBuffer::Flags::None, (GpuResource::Format::Type) format, header.actualWidth, header.actualHeight, 1, -1, header.mipLevels, NULL, NULL );

#ifdef DIRECTX12
    GpuDevice::CommandList *pCommandList = NULL;
    uint32 *pNumRows = NULL;
    uint64 *pRowSizeInBytes = NULL;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT *pDestLayouts = NULL;

    pCommandList = GpuDevice::Instance( ).AllocThreadCommandList( );

    // Ask the device for the layout of the data so we know how to copy it
    uint64 requiredSize;
    pNumRows = (uint32 *) malloc( sizeof( uint32 ) * header.mipLevels );
    pRowSizeInBytes = (uint64 *) malloc( sizeof( uint64 ) * header.mipLevels );
    pDestLayouts = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT *) malloc( sizeof( D3D12_PLACED_SUBRESOURCE_FOOTPRINT ) * header.mipLevels );

    GpuDevice::Instance( ).GetDevice( )->GetCopyableFootprints( &pTexture->GetApiResource( )->GetDesc( ), 0, header.mipLevels, 0, pDestLayouts, pNumRows, pRowSizeInBytes, &requiredSize );

    GpuBuffer *pUpload = new GpuBuffer;
    pUpload->Create( GpuResource::Heap::Upload, GpuResource::State::GenericRead, GpuResource::Flags::None,
        GpuResource::Format::Unknown, (size_t) requiredSize, 1, 1, -1, 1, true, NULL, NULL );

    byte *pHead = (byte *) pUpload->Map( );

    // Copy the data to the upload buffer
    for ( uint32 i = 0; i < header.mipLevels; i++ )
    {
        uint32 c;

        uint32 numRows = pNumRows[i];
        uint64 rowSizeInBytes = pRowSizeInBytes[i];

        byte *pData = pHead + pDestLayouts[i].Offset;

        for ( c = 0; c < numRows; c++ )
        {
            pSerializer->GetInputStream( )->Read( pData, (uint32) rowSizeInBytes, NULL );
            pData += pDestLayouts[i].Footprint.RowPitch;
        }
    }

    pUpload->Unmap( );

    // copy each mip level over to the texture
    for ( uint32 i = 0; i < header.mipLevels; i++ )
    {
        D3D12_TEXTURE_COPY_LOCATION dest = { };
        dest.pResource = pTexture->GetApiResource( );
        dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dest.SubresourceIndex = i;

        // tell the GPU how to interpret this unknown buffer
        // by giving it the layout desc of the texture it contains
        D3D12_TEXTURE_COPY_LOCATION source = { };
        source.pResource = pUpload->GetApiResource( );
        source.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        source.PlacedFootprint = pDestLayouts[i];

        pCommandList->pList->CopyTextureRegion( &dest, 0, 0, 0, &source, NULL );
    }

    // Ask the GPU to transition the texture memory type from copy to a shader resource
    pTexture->TransitionTo( pCommandList, GpuResource::State::PixelShaderResource );

    GpuDevice::Instance( ).ExecuteCommandLists( &pCommandList, 1, true );

    free( pNumRows );
    free( pRowSizeInBytes );
    free( pDestLayouts );

    pUpload->Destroy( );
    delete pUpload;
#else
#error Graphics API not defined
#endif

    return pTexture;
}
