#include "EnginePch.h"

#include "DefaultRenderer.h"
#include "Viewport.h"
#include "Camera.h"
#include "RenderWorld.h"
#include "IRenderModifier.h"
#include "TaskWorld.h"
#include "MaterialObject.h"
#include "RegistryWorld.h"

DefaultRenderer::DefaultRenderer(
    const char *pPass,
    bool multithreaded,
    Id groups[],
    int numGroups
)
{
    Identifiable::Create( Id::Create( ) );

    m_pPass = StringRef( pPass );
    m_NumCustomSamplePositions = 0;

    m_Modifiers.Create( );
    m_Renderables.Create( );
    m_Lights.Create( );
    m_RendererDesc.Create( );
    m_RenderStats.Create( );
    m_Groups.Create( 1, 4 );
    m_PassHash.Create( 64, 256, RenderContext::Hash, RenderContext::Compare );

    m_RenderGroupModification = -1;
    m_Multithreaded = multithreaded;

    m_Stats = RenderStats::Empty;

    for ( int i = 0; i < numGroups; i++ )
        AddGroup( groups[i] );
}

DefaultRenderer::~DefaultRenderer( void )
{
    // delete modifiers
    {
        List<IRenderModifier *>::Enumerator eModifiers = m_Modifiers.GetEnumerator( );

        while ( eModifiers.EnumNext( ) )
            delete eModifiers.Data( );
    }

    {
        PassEnum passEnum = m_PassHash.GetEnumerator( );

        while ( passEnum.EnumNext( ) )
        {
            RenderList *pList = passEnum.Data( );

            for ( uint32 i = 0; i < pList->lists.GetSize( ); i++ )
                pList->lists.GetAt( i )->Destroy( );

            pList->lists.Destroy( );
        }
    }

    m_RenderStats.Destroy( );
    m_PassHash.Destroy( );
    m_Groups.Destroy( );
    m_RendererDesc.Destroy( );
    m_Lights.Destroy( );
    m_Renderables.Destroy( );
    m_Modifiers.Destroy( );

    StringRel( m_pPass );

    Identifiable::Destroy( );
}

void DefaultRenderer::AddModifier(
    IRenderModifier *pModifier
)
{
    m_Modifiers.Add( pModifier );
}

void DefaultRenderer::AddGroup(
    Id groupId
)
{
    m_Groups.Add( groupId );
}

void DefaultRenderer::RemoveGroup(
    Id groupId
)
{
    m_Groups.Remove( groupId.ToString( ) );
}

void DefaultRenderer::SetSamplePositions(
    Vector2 *pPositions,
    byte count
)
{
    Debug::Assert(
        Condition( count <= MaxCustomSamplePositions ),
        "Too many custom sample positions" );

    memcpy( m_CustomSamplePositions, pPositions, sizeof( Vector2 ) * count );
    m_NumCustomSamplePositions = count;
}

void DefaultRenderer::GetRenderData(
    const Viewport &viewport
)
{
    int modification = RenderWorld::Instance( ).GetRenderGroupModification( );

    if ( m_RenderGroupModification != modification )
    {
        m_RenderGroupModification = modification;

        m_Renderables.Clear( );
        m_Lights.Clear( );

        IdList::Enumerator eGroup = m_Groups.GetEnumerator( );

        while ( eGroup.EnumNext( ) )
        {
            const RenderWorld::RenderObjectHash *pHash = RenderWorld::Instance( ).GetRenderGroup( eGroup.Data( ) );
            if ( NULL == pHash ) continue;

            Enumerator<nuint, RenderObject *> eHash = pHash->GetEnumerator( );

            while ( eHash.EnumNext( ) )
            {
                if ( eHash.Data( )->GetRenderType( ) == RenderObject::Type::Light )
                    m_Lights.Add( eHash.Data( ) );
                else
                    m_Renderables.Add( eHash.Data( ) );
            }
        }
    }

    m_RendererDesc.Clear( );
    m_RendererDesc.pPass = m_pPass;
    m_RendererDesc.viewport.Copy( viewport );

    List<IRenderModifier *>::Enumerator eModifiers = m_Modifiers.GetEnumerator( );

    while ( eModifiers.EnumNext( ) )
        eModifiers.Data( )->Begin( );

    // Lights first
    {
        List<RenderObject *>::Enumerator eRenderables = m_Lights.GetEnumerator( );

        while ( eRenderables.EnumNext( ) )
            eRenderables.Data( )->GetRenderData( &m_RendererDesc );
    }

    //Pack into a shader  friendly format
    m_RendererDesc.packedLights.ambient = m_RendererDesc.ambient_light;
    m_RendererDesc.packedLights.shadowProjMatrix = m_RendererDesc.shadowProjMatrix;
    m_RendererDesc.packedLights.shadowViewMatrix = m_RendererDesc.shadowViewMatrix;

    uint32 i;

    for ( i = 0; i < m_RendererDesc.lightDescs.GetSize( ); i++ )
    {
        const LightDesc *pLightDesc = m_RendererDesc.lightDescs.GetPointer( i );

        m_RendererDesc.packedLights.light_color[i] = pLightDesc->color * pLightDesc->nits;
        m_RendererDesc.packedLights.light_dir[i] = pLightDesc->direction;
        m_RendererDesc.packedLights.light_pos_type[i] = pLightDesc->position;
        m_RendererDesc.packedLights.light_pos_type[i].w = (float) pLightDesc->cast;
        m_RendererDesc.packedLights.light_atten[i] = Vector( pLightDesc->inner, pLightDesc->outer, pLightDesc->range, 1.0f );
    }

    m_RendererDesc.packedLights.num_lights = i;


    eModifiers.Reset( );
    while ( eModifiers.EnumNext( ) )
        eModifiers.Data( )->Process( &m_RendererDesc );

    // All other objects, because they have dependencies on the lights
    {
        List<RenderObject *>::Enumerator eRenderables = m_Renderables.GetEnumerator( );

        while ( eRenderables.EnumNext( ) )
            eRenderables.Data( )->GetRenderData( &m_RendererDesc );
    }

    eModifiers.Reset( );
    while ( eModifiers.EnumNext( ) )
        eModifiers.Data( )->Process( &m_RendererDesc );
}

struct ThreadCommandList
{
    GpuDevice::CommandList *pCommandList;
    ThreadId threadId;
};

struct BatchRenderContext_ThreadParams
{
    RenderStats *pStats;
    const char *pPass;
    List<RenderObjectDesc *> *pObjects;
    const DefaultRenderer *pDebugInfo;
    const Viewport *pViewport;
    const Vector2 *pSamplePositions;
    ThreadCommandList *pThreadCommandLists;
    uint32 numSamplePositions;
};

void DefaultRenderer::Render(
    const Viewport &viewport,
    GpuDevice::CommandList *pBatchCommandList
)
{
    if ( true == m_Multithreaded )
    {
        Debug::Assert( Condition(NULL == pBatchCommandList), "Batch command lists aren't supported with multithreaded default renderer" );

        RenderMultithreaded( viewport );
    }
    else
        RenderSequentially( viewport, pBatchCommandList );
}

void DefaultRenderer::RenderSequentially(
    const Viewport &viewport,
    GpuDevice::CommandList *pBatchCommandList
)
{
    m_Stats = RenderStats::Empty;

    {
        TaskGroup group;
        Task task = TaskCreate( &group );

        BatchRenderContext_ThreadParams params;

        ThreadCommandList threadCommandList;
        threadCommandList.pCommandList = pBatchCommandList;
        
        if ( NULL == pBatchCommandList )
            threadCommandList.pCommandList = GpuDevice::Instance( ).AllocGraphicsCommandList( );

        params.pStats = &m_Stats;
        params.pPass = m_RendererDesc.pPass;
        params.pObjects = &m_RendererDesc.renderObjectDescs;
        params.pDebugInfo = this;
        params.pViewport = &viewport;
        params.pSamplePositions = m_CustomSamplePositions;
        params.numSamplePositions = m_NumCustomSamplePositions;
        params.pThreadCommandLists = &threadCommandList;

        task.pData = &params;
        task.Execute = Task_SequentialRender;

        TaskSpawn( task );
        TaskGroupWait( group );

        if ( NULL == pBatchCommandList )
            GpuDevice::Instance( ).ExecuteCommandLists( &threadCommandList.pCommandList, 1 );
    }
}

void DefaultRenderer::RenderMultithreaded(
    const Viewport &viewport
)
{
    //sort by render context
    //get command list by hw thread
    //render all items under a render context in that thread
    m_Stats = RenderStats::Empty;

    static RegistryInt objectsPerThread( StringRef( "Render/objects_per_thread" ), 128 );
    const int maxObjectsPerThread = objectsPerThread.GetValue( );

    // create a list of renderables for each render context
    int i, renderables = m_RendererDesc.renderObjectDescs.GetSize( );

    for ( i = 0; i < renderables; i++ )
    {
        RenderList *pList;

        RenderObjectDesc *pDesc = m_RendererDesc.renderObjectDescs.GetAt( i );
        if ( false == m_PassHash.Get( pDesc->renderContext, &pList ) )
        {
            pList = new RenderList; pList->lists.Create( );
            pList->pCurrent = NULL;

            m_PassHash.Add( pDesc->renderContext, pList );
        }

        if ( pList->pCurrent == NULL || pList->pCurrent->GetSize( ) == maxObjectsPerThread )
        {
            ObjectList *pObjects = new ObjectList; pObjects->Create( );
            pList->lists.Add( pObjects );
            pList->pCurrent = pObjects;
        }

        pList->pCurrent->Add( pDesc );
    }

    // for each list in renderContext hash
    // spawn a thread and have it render the associated list
    {
        ThreadCommandList threadCommandLists[TaskWorld::MaxThreads];
        GpuDevice::CommandList *pCommandLists[TaskWorld::MaxThreads];

        int numThreads = TaskWorld::Instance( ).GetNumThreads( );

        for ( int i = 0; i < numThreads; i++ )
        {
            threadCommandLists[i].pCommandList = GpuDevice::Instance( ).AllocGraphicsCommandList( );
            threadCommandLists[i].threadId = TaskWorld::Instance( ).GetThreadId( i );

            pCommandLists[i] = threadCommandLists[i].pCommandList;
        }

        TaskGroup group;

        PassEnum passEnum = m_PassHash.GetEnumerator( );

        while ( passEnum.EnumNext( ) )
        {
            RenderList *pList = passEnum.Data( );

            for ( uint32 i = 0; i < pList->lists.GetSize( ); i++ )
            {
                ObjectList *pObjects = pList->lists.GetAt( i );

                if ( pObjects->GetSize( ) > 0 )
                {
                    uint32 index = m_RenderStats.Add( RenderStats::Empty );

                    Task task = TaskCreate( &group );

                    BatchRenderContext_ThreadParams *pParams =
                        (BatchRenderContext_ThreadParams *) task.AllocTLM( sizeof( BatchRenderContext_ThreadParams ) );

                    pParams->pStats = m_RenderStats.GetPointer( index );
                    pParams->pPass = m_RendererDesc.pPass;
                    pParams->pObjects = pObjects;
                    pParams->pDebugInfo = this;
                    pParams->pViewport = &viewport;
                    pParams->pSamplePositions = m_CustomSamplePositions;
                    pParams->numSamplePositions = m_NumCustomSamplePositions;
                    pParams->pThreadCommandLists = threadCommandLists;

                    task.pData = pParams;
                    task.Execute = Task_BatchRenderContext;

                    TaskSpawn( task );
                }
            }
        }


        TaskGroupWait( group );

        GpuDevice::Instance( ).ExecuteCommandLists( pCommandLists, numThreads );
    }

    {
        PassEnum passEnum = m_PassHash.GetEnumerator( );

        int rs_index = 0;

        while ( passEnum.EnumNext( ) )
        {
            RenderList *pList = passEnum.Data( );

            for ( uint32 i = 0; i < pList->lists.GetSize( ); i++ )
            {
                m_Stats += *m_RenderStats.GetPointer( rs_index++ );
                pList->lists.GetAt( i )->Clear( );
            }

            pList->lists.Clear( );
            pList->pCurrent = NULL;
        }

        m_RenderStats.Clear( );
    }
}

void DefaultRenderer::Task_SequentialRender(
    const struct TaskExecution &task
)
{
    const BatchRenderContext_ThreadParams *pParams = (const BatchRenderContext_ThreadParams *) task.pData;
    List<RenderObjectDesc *> *pRenderObjects = pParams->pObjects;

    GpuDevice::CommandList *pCommandList = pParams->pThreadCommandLists[0].pCommandList;

    Viewport viewport;
    viewport.Copy( *pParams->pViewport );
    viewport.Set( pCommandList );

    for ( uint32 i = 0, n = pRenderObjects->GetSize( ); i < n; i++ )
    {
        RenderObjectDesc *pDesc = pRenderObjects->GetAt( i );
        pDesc->pMaterial->SetRenderContext( pDesc->renderContext, pCommandList );

        // must be set after pipeline state or it crashes
        if ( pParams->numSamplePositions > 0 )
            pCommandList->SetSamplePositions( pParams->pSamplePositions, pParams->numSamplePositions );

        RenderDesc renderDesc;
        RenderStats stats = RenderStats::Empty;

        renderDesc.pCommandList = pCommandList;
        renderDesc.pPass = pParams->pPass;
        renderDesc.pViewport = &viewport;
        renderDesc.pDesc = pDesc;
        renderDesc.pTask = &task;

        renderDesc.pDesc->renderFunc( renderDesc, &stats );
        *pParams->pStats += stats;
    }

    if ( pRenderObjects->GetSize( ) > 0 && pParams->numSamplePositions > 0 )
        pCommandList->SetSamplePositions( NULL, 0 );
}

void DefaultRenderer::Task_BatchRenderContext(
    const struct TaskExecution &task
)
{
    const BatchRenderContext_ThreadParams *pParams = (const BatchRenderContext_ThreadParams *) task.pData;
    List<RenderObjectDesc *> *pRenderObjects = pParams->pObjects;

    GpuDevice::CommandList *pCommandList = NULL;

    int numThreads = TaskWorld::Instance( ).GetNumThreads( );

    for ( int i = 0; i < numThreads; i++ )
    {
        if ( pParams->pThreadCommandLists[i].threadId == Thread::GetCurrentThreadId( ) )
        {
            pCommandList = pParams->pThreadCommandLists[i].pCommandList;
            break;
        }
    }

    pRenderObjects->GetAt( 0 )->pMaterial->SetRenderContext( pRenderObjects->GetAt( 0 )->renderContext, pCommandList );

    Viewport viewport;
    viewport.Copy( *pParams->pViewport );
    viewport.Set( pCommandList );

    if ( pParams->numSamplePositions > 0 )
        pCommandList->SetSamplePositions( pParams->pSamplePositions, pParams->numSamplePositions );

    for ( uint32 i = 0, n = pRenderObjects->GetSize( ); i < n; i++ )
    {
        RenderDesc renderDesc;
        RenderStats stats = RenderStats::Empty;

        renderDesc.pCommandList = pCommandList;
        renderDesc.pPass = pParams->pPass;
        renderDesc.pViewport = &viewport;
        renderDesc.pDesc = pRenderObjects->GetAt( i );
        renderDesc.pTask = &task;

        renderDesc.pDesc->renderFunc( renderDesc, &stats );
        *pParams->pStats += stats;
    }

    if ( pParams->numSamplePositions > 0 )
        pCommandList->SetSamplePositions( NULL, 0 );
}
