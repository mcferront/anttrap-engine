#include "GamePch.h"
#include "RenderNodes.h"
#include "ConvertToRenderer.h"
#include "ComputeNode.h"
#include "MaterialObject.h"
#include "Node.h"
#include "CameraComponent.h"
#include "RegistryAsset.h"
#include "RenderTree.h"
#include "MipGenNode.h"
#include "DefaultRenderer.h"
#include "SearchHierarchy.h"
#include "GpuBuffer.h"

void SSAOProc(
    ComputeMaterialObject *pCompute,
    RenderContext renderContext,
    const void *pUserData
)
{
    static const char *pParams = StringRef( "$PARAMS" );
    static const char *pMainCam = StringRef( "MainCamera" );
    static const char *pProjParams = StringRef( "$PROJ_PARAMS" );
    static const char *pSSAO = StringRef( "SSAO" );
    static const char *pSSAOHBlur = StringRef( "SSAO_H_Blur" );
    static const char *pSSAOVBlur = StringRef( "SSAO_V_Blur" );
    static const char *pLinearZ = StringRef( "LinearZ" );

    ComputeMaterialObject::Pass *pPass = pCompute->GetPass( renderContext );

    ResourceHandle hCamera = ResourceHandle::FromAlias( pMainCam );
    Node *pNode = GetResource( hCamera, Node );

    Camera *pCamera = pNode->GetComponent<CameraComponent>( )->GetCamera( );

    if ( pPass->GetData( )->GetName( ) == pSSAO )
    {
        static RegistryInt hdr_width( "Render/hdr_width" );

        static const char *pParams1 = StringRef( "$PARAMS1" );

        static RegistryFloat radius( StringRef( "SSAO/world_radius" ), 4.0f );
        static RegistryFloat zScale( StringRef( "SSAO/z_scale" ), 0.00f );
        static RegistryFloat intensity( StringRef( "SSAO/ao_intensity" ), 2.0f );
        static RegistryFloat compression( StringRef( "SSAO/ao_compression" ), 1.0f );

        float ref_size_value = hdr_width.GetValue( ) / pCamera->GetFovX( );

        Vector params( radius.GetValue( ), ref_size_value, zScale.GetValue( ), intensity.GetValue( ) );
        pPass->GetData( )->SetMacro( pParams, &params, 1 );

        params = Vector( compression.GetValue( ), 0, 0, 0 );
        pPass->GetData( )->SetMacro( pParams1, &params, 1 );

        Vector2 jitter;
        Matrix projMatrix;
        pCamera->GetJitter( &jitter );
        {
            pCamera->SetJitter( Math::ZeroVector2( ) );

            pCamera->GetReverseDepthProjection( &projMatrix );

            Vector row[ 3 ];
            projMatrix.GetRow( 0, &row[ 0 ] );
            projMatrix.GetRow( 1, &row[ 1 ] );
            projMatrix.GetRow( 2, &row[ 2 ] );

            Vector params( row[ 0 ].x, row[ 1 ].y, row[ 2 ].x, row[ 2 ].y );
            pPass->GetData( )->SetMacro( pProjParams, &params, 1 );

            pCamera->SetJitter( jitter );
        }
    }
    else if ( pPass->GetData( )->GetName( ) == pSSAOHBlur )
    {
        static RegistryFloat edgeSharpness( StringRef( "SSAO/edge_sharpness" ), 1.0f );
        static RegistryFloat blurScale( StringRef( "SSAO/blur_scale" ), 2.0f );

        uint32 bitFlags = 0;
        bitFlags |= 0x01; //horizontal

        Vector params( *(float *) (uint32 *) &bitFlags, edgeSharpness.GetValue( ), blurScale.GetValue( ), 0 );
        pPass->GetData( )->SetMacro( pParams, &params, 1 );
    }
    else if ( pPass->GetData( )->GetName( ) == pSSAOVBlur )
    {
        static RegistryFloat edgeSharpness( StringRef( "SSAO/edge_sharpness" ), 1.0f );
        static RegistryFloat blurScale( StringRef( "SSAO/blur_scale" ), 2.0f );

        uint32 bitFlags = 0;
        bitFlags |= 0x00; //vertical

        Vector params( *(float *) (uint32 *) &bitFlags, edgeSharpness.GetValue( ), blurScale.GetValue( ), 0 );
        pPass->GetData( )->SetMacro( pParams, &params, 1 );
    }
    else if ( pPass->GetData( )->GetName( ) == pLinearZ )
    {
        Matrix vpMatrix, invVpMatrix;
        Vector2 jitter;

        Vector row_2, row_3;

        pCamera->GetJitter( &jitter );
        {
            pCamera->SetJitter( Math::ZeroVector2( ) );

            Transform view;
            pCamera->GetReverseDepthProjection( &vpMatrix );
            Math::Invert( &invVpMatrix, vpMatrix );

            invVpMatrix.GetRow( 2, &row_2 );
            invVpMatrix.GetRow( 3, &row_3 );

            pCamera->SetJitter( jitter );
        }

        Vector params( row_2.z, row_3.z, row_2.w, row_3.w );
        pPass->GetData( )->SetMacro( pParams, &params, 1 );
    }
}

void SSRProc(
    ComputeMaterialObject *pCompute,
    RenderContext renderContext,
    const void *pUserData
)
{
    static const char *pParams = StringRef( "$PARAMS" );
    static const char *pInvProj = StringRef( "$INV_PROJ" );
    static const char *pProj = StringRef( "$PROJ" );
    static const char *pView = StringRef( "$VIEW" );
    static const char *pMainCam = StringRef( "MainCamera" );
    static const char *pSSRPass = StringRef( "SSR" );
    static const char *pCompositePass = StringRef( "Composite" );

    ComputeMaterialObject::Pass *pPass = pCompute->GetPass( renderContext );

    if ( pPass->GetData( )->GetName( ) == pSSRPass )
    {
        static RegistryInt ssr_iterations = RegistryInt( "SSR/iterations" );
        static RegistryFloat ssr_offset_bias = RegistryFloat( "SSR/offset_bias", 0.0001f );

        ResourceHandle hCamera = ResourceHandle::FromAlias( pMainCam );
        Node *pNode = GetResource( hCamera, Node );
        Camera *pCamera = pNode->GetComponent<CameraComponent>( )->GetCamera( );

        GpuBuffer *pImage = GetResource( ResourceHandle( "PropertiesRT" ), GpuBuffer );

        uint32 flags = 0;
        Vector params( (float) ssr_iterations.GetValue( ), ssr_offset_bias.GetValue( ), *(float *) &flags, 0.0f );
        pPass->GetData( )->SetMacro( pParams, &params, 1 );


        Vector2 jitter;
        pCamera->GetJitter( &jitter );
        {
            Transform view;
            Matrix projMatrix, invProjMatrix;

            pCamera->SetJitter( Math::ZeroVector2( ) );

            pCamera->GetReverseDepthProjection( &projMatrix );
            pPass->GetData( )->SetMacro( pProj, &projMatrix, 1 );

            Math::Invert( &invProjMatrix, projMatrix );
            pPass->GetData( )->SetMacro( pInvProj, &invProjMatrix, 1 );

            pCamera->GetViewTransform( &view );
            pPass->GetData( )->SetMacro( pView, &view.ToMatrix( true ), 1 );

            pCamera->SetJitter( jitter );
        }
    }
    else if ( pPass->GetData( )->GetName( ) == pCompositePass )
    {
        static RegistryBool ssr_composite_show_mip = RegistryBool( "SSR/composite_show_mip", false );
        static RegistryBool ssr_composite_force_mip = RegistryBool( "SSR/composite_force_mip", false );
        static RegistryBool ssr_composite_show_z = RegistryBool( "SSR/composite_show_z", false );
        static RegistryInt ssr_composite_mip = RegistryInt( "SSR/composite_mip", 0 );

        GpuBuffer *pImage = GetResource( ResourceHandle( "PropertiesRT" ), GpuBuffer );

        uint32 flags = 0;

        flags |= ssr_composite_show_mip.GetValue( ) ? 0x01 : 0;
        flags |= ssr_composite_force_mip.GetValue( ) ? 0x02 : 0;
        flags |= ssr_composite_show_z.GetValue( ) ? 0x04 : 0;

        Vector params( *(float *) &flags, (float) ssr_composite_mip.GetValue( ), 0.0f, 0.0f );
        pPass->GetData( )->SetMacro( pParams, &params, 1 );
    }
}

void HdrBlurProc(
    ComputeMaterialObject *pCompute,
    RenderContext renderContext,
    const void *pUserData
)
{
    static const char *pParams = StringRef( "$PARAMS" );
    static const char *pCreateMip = StringRef( "HdrCreateMip" );
    static const char *pBlur = StringRef( "HdrBlur" );

    static const char *pSource = StringRef( "$SOURCE" );
    static const char *pDest = StringRef( "$DEST" );
    static const char *pGroupSizeTarget = StringRef( "GroupSizeTarget" );

    ComputeMaterialObject::Pass *pPass = pCompute->GetPass( renderContext );

    if ( pCreateMip == pPass->GetData( )->GetName( ) )
    {
        int mip = (int) pUserData;

        pPass->GetData( )->SetMacro( pSource, ResourceHandle( g_pHdrMipLevels[ mip - 1 ] ) );
        pPass->GetData( )->SetMacro( pDest, ResourceHandle( g_pHdrMipLevels[ mip ] ) );
        pPass->GetData( )->SetMacro( pGroupSizeTarget, ResourceHandle( g_pHdrMipLevels[ mip ] ) );
    }
    else if ( pBlur == pPass->GetData( )->GetName( ) )
    {
        static int direction;

        int flags = 0;
        flags |= direction == 0;

        Vector params( *(float *) &flags, 0.0f, 0.0f, 0.0f );
        pPass->GetData( )->SetMacro( pParams, &params, 1 );

        ResourceHandle rh( (const char *) pUserData );

        pPass->GetData( )->SetMacro( pSource, rh );
        pPass->GetData( )->SetMacro( pDest, rh );
        pPass->GetData( )->SetMacro( pGroupSizeTarget, rh );

        direction = direction ^ 1;
    }
}

void SpecBloomProc(
    ComputeMaterialObject *pCompute,
    RenderContext renderContext,
    const void *pUserData
)
{
    static const char *pParams = StringRef( "$PARAMS" );

    ComputeMaterialObject::Pass *pPass = pCompute->GetPass( renderContext );

    static RegistryFloat bloom_min( "SpecBloom/min", 0.05f );
    static RegistryFloat bloom_grad( "SpecBloom/gradient", 4.0f );

    Vector params( bloom_min.GetValue( ), bloom_grad.GetValue(), 0 );
    pPass->GetData( )->SetMacro( pParams, &params, 1 );
}

void DofProc(
    ComputeMaterialObject *pCompute,
    RenderContext renderContext,
    const void *pUserData
)
{
    static const char *pParams = StringRef( "$PARAMS" );
    static const char *pSplitPlanes = StringRef( "DofSplitPlanes" );
    static const char *pDofBlur = StringRef( "DofBlur" );
    static const char *pDofGauss = StringRef( "DofGauss" );
    static const char *pDofComposite = StringRef( "DofComposite" );

    static const char *pSource = StringRef( "$SOURCE" );
    static const char *pDest = StringRef( "$DEST" );

    static RegistryFloat near_kernel( "dof.near_kernel", 5.0f );
    static RegistryFloat far_kernel( "dof.far_kernel", 15.0f );
    static RegistryFloat focal_plane_start( "dof.focal_plane_start", 40.0f );
    static RegistryFloat focal_plane_end( "dof.focal_plane_end", 150.0f );
    static RegistryFloat dof_near_pow( "dof.near_pow", 4.0f );
    static RegistryFloat coc_near_blend( "dof.coc_blend", 0.0f );

    ComputeMaterialObject::Pass *pPass = pCompute->GetPass( renderContext );

    if ( pSplitPlanes == pPass->GetData( )->GetName( ) )
    {
        Vector dof( focal_plane_start.GetValue( ), focal_plane_end.GetValue( ) );
        pPass->GetData( )->SetMacro( pParams, &dof, 1 );
    }
    else if ( pDofBlur == pPass->GetData( )->GetName( ) )
    {
        static int type;

        ResourceHandle source, dest;

        if ( ( type & 0x1 ) == 0 )
        {
            source = ResourceHandle( "DofBuffer" );
            dest = ResourceHandle( "DofBlurredBuffer" );
        }
        else
        {
            source = ResourceHandle( "DofBlurredBuffer" );
            dest = ResourceHandle( "DofBuffer" );
        }

        pPass->GetData( )->SetMacro( pSource, source );
        pPass->GetData( )->SetMacro( pDest, dest );

        Vector params( (float) type, near_kernel.GetValue( ), far_kernel.GetValue( ), 0.0f );
        pPass->GetData( )->SetMacro( pParams, &params, 1 );

        type = ( type + 1 ) % DofBlurs;
    }
    else if ( pDofGauss == pPass->GetData( )->GetName( ) )
    {
        static int type;

        ResourceHandle source, dest;

        if ( ( type & 0x1 ) == 0 )
        {
            source = ResourceHandle( "DofBuffer" );
            dest = ResourceHandle( "DofBlurredBuffer" );
        }
        else
        {
            source = ResourceHandle( "DofBlurredBuffer" );
            dest = ResourceHandle( "DofBuffer" );
        }

        pPass->GetData( )->SetMacro( pSource, source );
        pPass->GetData( )->SetMacro( pDest, dest );

        Vector params( (float) type, near_kernel.GetValue( ), far_kernel.GetValue( ), 0.0f );
        pPass->GetData( )->SetMacro( pParams, &params, 1 );

        type = ( type + 1 ) % DofBlurs;
    }
    else if ( pDofComposite == pPass->GetData( )->GetName( ) )
    {
        Vector dof( focal_plane_start.GetValue( ), focal_plane_end.GetValue( ), dof_near_pow.GetValue( ), coc_near_blend.GetValue() );
        pPass->GetData( )->SetMacro( pParams, &dof, 1 );
    }
}

void HiZMipProc(
    ComputeMaterialObject *pCompute,
    RenderContext renderContext,
    const void *pUserData
)
{
    static const char *pParams = StringRef( "$PARAMS" );
    static const char *pHiZMipCopy = StringRef( "HiZMipCopy" );

    ComputeMaterialObject::Pass *pPass = pCompute->GetPass( renderContext );

    if ( pPass->GetData( )->GetName( ) == pHiZMipCopy )
    {
    }
    else
    {

        static const char *pSource = StringRef( "$SOURCE" );
        static const char *pDest = StringRef( "$DEST" );
        static const char *pGroupDiv = StringRef( "GroupDiv" );
        static const char *pGroupSizeTarget = StringRef( "GroupSizeTarget" );

        int mip = (int) pUserData;

        pPass->GetData( )->SetMacro( pSource, ResourceHandle( g_pHiZMipLevels[ mip - 1 ] ) );
        pPass->GetData( )->SetMacro( pDest, ResourceHandle( g_pHiZMipLevels[ mip ] ) );
        pPass->GetData( )->SetMacro( pGroupSizeTarget, ResourceHandle( g_pHiZMipLevels[ mip ] ) );

        if ( mip == ( HiZMipLevels - 3 ) )
            pPass->GetData( )->SetMacro( pGroupDiv, &Vector( 4, 4 ), 1 );
        else if ( mip == ( HiZMipLevels - 2 ) )
            pPass->GetData( )->SetMacro( pGroupDiv, &Vector( 2, 2 ), 1 );
        else if ( mip == ( HiZMipLevels - 1 ) )
            pPass->GetData( )->SetMacro( pGroupDiv, &Vector( 1, 1 ), 1 );
        else
            pPass->GetData( )->SetMacro( pGroupDiv, &Vector( 8, 8 ), 1 );
    }
}

void CreateDof(
    List<GpuTimer*> *pTimers,
    DofDesc *pDesc
)
{
    pDesc->pTimerNode = new GpuTimerNode( "DofBatch" );
    pTimers->Add( pDesc->pTimerNode->GetTimer( ) );

    // hdr to near / far mip + downres
    ResourceHandle computeMaterials[ ] = { ResourceHandle( "792C762E-B552-46FD-A58A-663B30866760" ), NullHandle };
    pDesc->pSplitPlanes = new ComputeNode( "DofSplitPlanes", computeMaterials, false, DofProc );
    pTimers->Add( pDesc->pSplitPlanes->AddGpuTimer( "DofSplitPlanes" ) );

    char timer[ 256 ];

    for ( int i = 0; i < DofBlurs; i++ )
    {
        pDesc->pBlurs[i] = new ComputeNode( "DofBlur", computeMaterials, false, DofProc );
        pDesc->pGauss[i] = new ComputeNode( "DofGauss", computeMaterials, false, DofProc );

        String::Format( timer, sizeof( timer ), "DofBlur_%d", i );
        pTimers->Add( pDesc->pBlurs[i]->AddGpuTimer( timer ) );

        String::Format( timer, sizeof( timer ), "DofGauss_%d", i );
        pTimers->Add( pDesc->pGauss[i]->AddGpuTimer( timer ) );

        if ( ( i & 0x1 ) == 0 )
            pDesc->pBufferBarriers[i] = new BarrierRenderer( ResourceHandle( "DofBlurredBuffer" ), GpuBuffer::Barrier::Uav );
        else
            pDesc->pBufferBarriers[i] = new BarrierRenderer( ResourceHandle( "DofBuffer" ), GpuBuffer::Barrier::Uav );
    }

    {
        pDesc->pComposite = new ComputeNode( "DofComposite", computeMaterials, false, DofProc );
        pTimers->Add( pDesc->pComposite->AddGpuTimer( "DofComposite" ) );

        pDesc->pSplitPlanesBarrier = new BarrierRenderer( ResourceHandle( "DofBuffer" ), GpuBuffer::Barrier::Uav );
        pDesc->pCocBarrier = new BarrierRenderer( ResourceHandle( "CocBuffer" ), GpuBuffer::Barrier::Uav );
        pDesc->pMainHdrTargetBarrier = new BarrierRenderer( ResourceHandle( "MainHDRTarget" ), GpuBuffer::Barrier::Uav );
    }
}

void AddDof(
    RenderTree *pTree,
    const DofDesc &desc
)
{
    pTree->BeginBatch( );
    {
        pTree->AddNode( desc.pTimerNode );

        pTree->AddNode( desc.pSplitPlanes );              // split near/far + coc
        pTree->AddNode( desc.pSplitPlanesBarrier );       
        // wait on color
        pTree->AddNode( desc.pCocBarrier );            // wait on coc

        for ( int i = 0; i < DofBlurs; i++ )
        {
            pTree->AddNode( desc.pBlurs[ i ] );            // blur color horiz
            pTree->AddNode( desc.pGauss[ i ] );            // blur color horiz
            pTree->AddNode( desc.pBufferBarriers[ i ] );         // wait on color
        }

        pTree->AddNode( desc.pComposite );             // composite to mainhdr
        pTree->AddNode( desc.pMainHdrTargetBarrier );

        pTree->AddNode( desc.pTimerNode );
    }
    pTree->EndBatch( );
}

void CreateSsr(
    List<GpuTimer*> *pTimers,
    SsrDesc *pDesc
)
{

    ResourceHandle ssrMaterial = ResourceHandle( "47FD531C-1E5C-4EF4-9297-61DF0D27B24D" );
    ResourceHandle computeMaterials[ ] = { ssrMaterial, NullHandle };

    pDesc->pSSR = new ComputeNode( "SSR", computeMaterials, false, SSRProc );
    pTimers->Add( pDesc->pSSR->AddGpuTimer( "SSR" ) );

    pDesc->pComposite = new ComputeNode( "Composite", computeMaterials, false, SSRProc );
    pTimers->Add( pDesc->pComposite->AddGpuTimer( "SSRComposite" ) );
}

void AddSsr(
    RenderTree *pTree,
    const SsrDesc &desc
)
{
    pTree->AddNode( desc.pSSR );
    pTree->AddNode( desc.pComposite );
}

void CreateSpecBloom(
    List<GpuTimer*> *pTimers,
    SpecBloomDesc *pDesc
)
{
    //spec bloom
    ResourceHandle computeMaterials[ ] = { ResourceHandle( "54F3CB05-E552-478C-AACF-FAE8182A4403" ), NullHandle };

    pDesc->pComposite = new ComputeNode( "Composite", computeMaterials, false, SpecBloomProc );
    pTimers->Add( pDesc->pComposite->AddGpuTimer( "SpecBloom Composite" ) );

    pDesc->pMainHdrBarrier1 = new BarrierRenderer( ResourceHandle( "MainHDRTarget" ), GpuBuffer::Barrier::Uav );
    pDesc->pMainHdrBarrier2 = new BarrierRenderer( ResourceHandle( "MainHDRTarget" ), GpuBuffer::Barrier::Uav );
}

void AddSpecBloom(
    RenderTree *pTree,
    const SpecBloomDesc &desc
)
{
    pTree->AddNode( desc.pMainHdrBarrier1 );
    pTree->AddNode( desc.pComposite );
    pTree->AddNode( desc.pMainHdrBarrier2 );
}

void CreateHdrMips(
    List<GpuTimer*> *pTimers,
    HdrMipDesc *pDesc
)
{
    ResourceHandle computeMaterials[ ] =
    {
        ResourceHandle( "1F22D6C8-56B2-4C90-BA87-5345427AA0FC" ), //hdr mip material
        NullHandle
    };

    // barriers and mips
    ResourceHandle mipLevels[ HdrMipLevels ];

    for ( int i = 0; i < HdrMipLevels; i++ )
    {
        mipLevels[ i ] = ResourceHandle( g_pHdrMipLevels[ i ] );
        pDesc->hdrLevels[ i ].pMipBarrier = new BarrierRenderer( mipLevels[ i ], GpuBuffer::Barrier::Uav );
    }

    pDesc->pHdrMipBuffer = new MipGenNode( ResourceHandle( "HdrMipBuffer" ), mipLevels, HdrMipLevels );

    // hdr to top mip
    pDesc->hdrLevels[0].pDownRes = new ComputeNode( "HdrCopy", computeMaterials );
    pTimers->Add( pDesc->hdrLevels[0].pDownRes->AddGpuTimer( "HdrCopy" ) );

    char timer[ 256 ];
    for ( int m = 1; m < HdrMipLevels; m++ )
    {
        // blur to downres
        pDesc->hdrLevels[ m ].pDownRes = new ComputeNode( "HdrCreateMip", computeMaterials, false, HdrBlurProc, (void *) m );

        String::Format( timer, sizeof(timer), "HdrCreateMip_%d", m );
        pTimers->Add( pDesc->hdrLevels[ m ].pDownRes->AddGpuTimer( "HdrCreateMip" ) );

        for ( int i = 0; i < HdrBlurIterations; i++ )
        {
            pDesc->hdrLevels[ m ].pBlurIterations[ i ] = new ComputeNode( "HdrBlur", computeMaterials, false, HdrBlurProc, (const char *) g_pHdrMipLevels[ m ] );
            pDesc->hdrLevels[ m ].pBlurBarriers[ i ] = new BarrierRenderer( mipLevels[ i ], GpuBuffer::Barrier::Uav );

            String::Format( timer, sizeof(timer), "HdrBlur_%d_%d", m, i );
            pTimers->Add( pDesc->hdrLevels[ m ].pBlurIterations[ i ]->AddGpuTimer( timer ) );
        }
    }
}

void AddHdrMips(
    RenderTree *pTree,
    const HdrMipDesc &desc
)
{
    pTree->AddNode( desc.hdrLevels[ 0 ].pDownRes );
    pTree->AddNode( desc.hdrLevels[ 0 ].pMipBarrier );

    for ( int m = 1; m < HdrMipLevels; m++ )
    {
        pTree->AddNode( desc.hdrLevels[ m ].pDownRes );
        pTree->AddNode( desc.hdrLevels[ m ].pMipBarrier );

        for ( int i = 0; i < HdrBlurIterations; i++ )
        {
            pTree->AddNode( desc.hdrLevels[ m ].pBlurIterations[ i ] );
            pTree->AddNode( desc.hdrLevels[ m ].pBlurBarriers[ i ] );
        }
    }

    pTree->AddNode( desc.pHdrMipBuffer );
}

void CreateHiZMips(
    HiZMipDesc *pDesc
)
{
    ResourceHandle mipLevels[ HiZMipLevels ];

    for ( int i = 0; i < HiZMipLevels; i++ )
    {
        ResourceHandle computeMaterials[ ] =
        {
            ResourceHandle( "47FD531C-1E5C-4EF4-9297-61DF0D27B24D" ), //ssr material
            NullHandle
        };

        mipLevels[ i ] = ResourceHandle( g_pHiZMipLevels[ i ] );

        pDesc->pHiZMipBarriers[ i ] = new BarrierRenderer( ResourceHandle( g_pHiZMipLevels[ i ] ), GpuBuffer::Barrier::Uav );

        const char *pPass;

        if ( i == 0 )
            pPass = "HiZMipCopy";
        else
            pPass = "HiZMip";

        pDesc->pHiZMips[ i ] = new ComputeNode( pPass, computeMaterials, false, HiZMipProc, (const void *) i );
    }

    pDesc->pHiZBuffer = new MipGenNode( ResourceHandle( "HiZBuffer" ), mipLevels, HiZMipLevels );
}

void AddHiZMips(
    RenderTree *pTree,
    const HiZMipDesc &desc
)
{
    for ( int i = 0; i < HiZMipLevels; i++ )
    {
        pTree->AddNode( desc.pHiZMips[ i ] );
        pTree->AddNode( desc.pHiZMipBarriers[ i ] );
    }

    pTree->AddNode( desc.pHiZBuffer );
}

void CreateSSAO(
    SSAODesc *pDesc
)
{
    // SSAO
    ResourceHandle ssaoMaterial( "11574890-6025-46E4-A2C5-87F37DFABA7F" );

    ResourceHandle computeMaterials[ ] =
    {
        ssaoMaterial,
        NullHandle,
    };

    const char *pPasses[ ] =
    {
        "SSAO",
        "SSAO_H_Blur",
        "SSAO_V_Blur",
    };

    // Create SSAO map
    pDesc->pBarriers[0] = new BarrierRenderer( ResourceHandle( "SSAOMap" ), GpuBuffer::Barrier::Uav );
    pDesc->pSsaos[0] = new ComputeNode( pPasses[ 0 ], computeMaterials, false, SSAOProc );


    // Blur
    pDesc->pBarriers[1] = new BarrierRenderer( ResourceHandle( "SSAOBlur" ), GpuBuffer::Barrier::Uav );
    pDesc->pSsaos[1] = new ComputeNode( pPasses[ 1 ], computeMaterials, false, SSAOProc );

    // Blur
    pDesc->pBarriers[2] = new ConvertToRenderer( ResourceHandle( "SSAOFinal" ), GpuBuffer::State::ShaderResource );
    pDesc->pSsaos[2] = new ComputeNode( pPasses[ 2 ], computeMaterials, false, SSAOProc );
}

void AddLinearZ( 
    RenderTree *pTree
    )
{

    ResourceHandle ssaoMaterial( "11574890-6025-46E4-A2C5-87F37DFABA7F" );

    ResourceHandle computeMaterials[ ] =
    {
        ssaoMaterial,
        NullHandle,
    };

    pTree->AddNode( new ComputeNode( "LinearZ", computeMaterials, false, SSAOProc ) );
    pTree->AddNode( new BarrierRenderer( ResourceHandle( "LinearZ" ), GpuBuffer::Barrier::Uav ) );
}

void AddSSAO(
    RenderTree *pTree,
    const SSAODesc &desc
)
{
    for ( int i = 0; i < _countof( desc.pSsaos ); i++ )
    {
        pTree->AddNode( desc.pSsaos[ i ] );
        pTree->AddNode( desc.pBarriers[ i ] );
    }
}

void CreateShadowMap(
    ResourceHandle shadowMapTarget,
    ShadowMapDesc *pDesc
)
{
    // convert shadowmap to texture
    pDesc->pShadowMapToTexture = new ConvertToRenderer( shadowMapTarget, GpuBuffer::State::ShaderResource );
    
    // Shadowmap Render Node
    {
        DefaultRenderer *pRenderer;

        Id groups[] = { GeometryGroup, DebugGraphicsGroup };
        pRenderer = new DefaultRenderer( "Shadow", true, groups, _countof(groups) );

        IRenderModifier *pModifier = new FrustumCullRenderModifier;
        pRenderer->AddModifier( pModifier );

        pDesc->pShadowMapRenderer = pRenderer;
    }
}
void AddShadowMap(
    RenderTree *pTree,
    const ShadowMapDesc &desc
)
{
    pTree->AddNode( desc.pShadowMapRenderer );
    pTree->AddNode( desc.pShadowMapToTexture );
}

//   // bokeh rasterizer
////if ( false )   
//{
//      m_BokehRenderTree.Create( Id::Id("BokehRenderTree") );
//
//      RenderWorld::Instance( ).AddRenderGroup( "DofBokeh" );
//
//      GpuBuffer *pIndirectArgs = new GpuBuffer;
//      ResourceHandle indirectArgs = ResourceHandle( "DofBokehArgs" );
//      indirectArgs.Bind( NULL, pIndirectArgs );
//
//      const int MaxQuads = 2073600;
//      
//      const int bokeh_args_size = MaxQuads * sizeof(int) * 2; //(pos:int, color:int) 
//      const int arg_buffer_offset = bokeh_args_size;
//
//      struct IndirectCommand
//      {
//         D3D12_DRAW_INDEXED_ARGUMENTS drawArguments;
//      };
//
//      pIndirectArgs->CreateAsStructuredBuffer( bokeh_args_size + sizeof(IndirectCommand) + 4, false );
//      pIndirectArgs->AddToScene( );
//      pIndirectArgs->Bind( );
//
//      //--> hand a desc struct to this 
//      // which includes vertex and index buffers
//
//      VertexBuffer *pVertexBuffer = new VertexBuffer;
//      pVertexBuffer->Create( );
//
//      unsigned short indices[] = { 0, 1, 2, 2, 3, 0 };
//      pVertexBuffer->SetIndices( indices, sizeof(indices), StreamDecl::UShort, 5 );
//
//#ifdef DIRECTX12
//      D3D12_INDIRECT_ARGUMENT_DESC args[1] = {};
//      {
//         args[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
//      }
//#else
//#error "Graphics API Not Defined"
//#endif
//
//      // command sig
//      ExecuteIndirect::CommandSignatureDesc sigDesc = {};
//      sigDesc.desc.ByteStride = sizeof(IndirectCommand);
//      sigDesc.desc.pArgumentDescs = args;
//      sigDesc.desc.NumArgumentDescs = _countof(args);
//
//
//      m_pDofIndirect = new ExecuteIndirect;
//      m_pDofIndirect->Create( 
//         "DofBokeh", 
//         ResourceHandle( "9870B07B-2CE1-4150-AB4A-1FC6E3AC6BF8"),
//         ResourceHandle( "DofBokehArgs" ),
//         arg_buffer_offset,
//         sigDesc,
//         pVertexBuffer
//         ); //bokeh material
//
//      m_pDofIndirect->AddToScene( );
//
//      Id render_bokeh_id = Id::Create( );
//      {
//         DefaultRenderer *pRenderer;
//
//         pRenderer = new DefaultRenderer;
//         pRenderer->Create( render_bokeh_id, "DofBokeh", false );
//         pRenderer->AddGroup( "DofBokeh" );
//         m_Renderers.Add( pRenderer );
//      }
//
//      Id finalize_dof_indirect = Id::Create( );
//      {
//         ComputeNode *pComputeNode;
//
//         ResourceHandle computeMaterials[] = 
//         {
//            ResourceHandle( "792C762E-B552-46FD-A58A-663B30866760" ), //dof material
//            NullHandle,
//         };
//
//         pComputeNode = new ComputeNode;
//         pComputeNode->Create( finalize_dof_indirect, "DofClearIndirect", computeMaterials );
//         m_Renderers.Add( pComputeNode );
//      };
//
//      // convert hdr to texture
//      Id dofMipToSRV = Id::Create( );
//      {
//         ConvertToRenderer *pRenderer;
//
//         pRenderer = new ConvertToRenderer;
//         pRenderer->Create( dofMipToSRV, ResourceHandle("DofMip2"), GpuBuffer::State::ShaderResource );
//         m_Renderers.Add( pRenderer );
//      }
//
//      m_BokehRenderTree.AddNode( finalize_dof_indirect );
//      m_BokehRenderTree.AddNode( render_bokeh_id );
//      m_BokehRenderTree.AddNode( dofMipToSRV );
//
//      m_BokehViewport.Create( g_scene_buffer_width >> 2, g_scene_buffer_height >> 2, NullHandle, &ResourceHandle("DofMip2"), 1, NullHandle );
//      //m_BokehViewport.Create( g_scene_buffer_width, g_scene_buffer_height, NullHandle, &ResourceHandle("MainHDRTarget"), 1, NullHandle );
//      m_BokehViewport.SetClearFlags( Viewport::ClearFlagsNone );
//      m_BokehViewport.SetClearColor( ClearColor );
//
//      RenderWorld::Instance( ).AddViewport( m_BokehViewport, &m_BokehRenderTree );
//      RenderWorld::Instance( ).BindViewportToWindow( m_WindowId, m_BokehViewport.GetId( ), RenderOrder_Bokeh );
//   }

//VertexBuffer *pBuffer = m_pDofIndirect->GetVertexBuffer( );
//pBuffer->Destroy( );
//delete pBuffer;

//m_pDofIndirect->Destroy( );
//delete m_pDofIndirect;

