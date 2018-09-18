//#define VISUALIZE

Texture2D<float3> g_inTexture  : register( t0 );
RWStructuredBuffer<float> g_luminance : register( u0 );

#ifdef VISUALIZE
   RWTexture2D<unorm float4> g_outTexture : register( u1 );
#endif

#define MAX_RESOLUTION 3840
#define THREADS_X   32
#define THREADS_Y   32
#define GROUP_SIZE      (THREADS_X * THREADS_Y)
#define MAX_GROUPS_WIDE (MAX_RESOLUTION / THREADS_X)

groupshared float g_group_luminance[THREADS_X * THREADS_Y];

[numthreads(THREADS_X, THREADS_Y, 1)]
void cs_group_avg_luminance(uint groupThreadIndex : SV_GroupIndex, uint3 groupId : SV_GroupID, uint3 dispatchThreadId : SV_DispatchThreadID )   
{
   float3 color = g_inTexture[ dispatchThreadId.xy ].rgb;
   
   int width, height;
   g_inTexture.GetDimensions( width, height );
   
   uint2 tile = groupId.xy;
   
   int tiles_across = width / THREADS_X;
   
   #ifdef VISUALIZE
      float v = g_luminance[tile.y * tiles_across + tile.x];
      g_outTexture[ dispatchThreadId.xy ] = float4(v, v, v, 1);
   #endif
   
   const float eps = .00001;
   const float lum = color.r * 0.27 + color.g * 0.67 + color.b * 0.06;
   g_group_luminance[groupThreadIndex] = lum > 0 ? log( lum ) : 0;

   GroupMemoryBarrierWithGroupSync( );

   // The first thread of each group computes average luminance for that group
   if ( groupThreadIndex == 0 )
   {
      float acc_luminance = 0;
      
      for (int i = 0; i < GROUP_SIZE; i++ )
         acc_luminance += g_group_luminance[i];
      
      g_luminance[tile.y * tiles_across + tile.x] = acc_luminance / GROUP_SIZE;
   }
}
