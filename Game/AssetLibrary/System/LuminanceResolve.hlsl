cbuffer cb : register(b0)
{
	float4 tonemap; //method, targetExposure, filter
   float4 dimensions; //x, y
};

StructuredBuffer<float> g_luminance : register( t0 );
RWStructuredBuffer<float> g_whitepoint : register( u0 );

#define MAX_RESOLUTION 3840
#define THREADS_X   32
#define THREADS_Y   32
#define GROUP_SIZE      (THREADS_X * THREADS_Y)
#define MAX_GROUPS_WIDE (MAX_RESOLUTION / THREADS_X)

groupshared float g_group_luminance[THREADS_X * THREADS_Y];

[numthreads(MAX_GROUPS_WIDE, 1, 1)]
void cs_avg_luminance(uint3 groupThreadId : SV_GroupThreadID)   
{                     
   float acc_luminance = 0;

   const uint tiles_across = dimensions.x / THREADS_X;
   const uint rows = dimensions.y / THREADS_Y;
   
   const uint row = groupThreadId.x;
   
   // ignore out of bounds (any resolution lower than MAX_RESOLUTION)
   if ( row < rows )
   {  
      // Each thread reads a new row of thread group luminances
      int start = row * tiles_across;
      
      for (uint i = 0; i < tiles_across; i++ )
         acc_luminance += g_luminance[start + i];

      g_group_luminance[row] = acc_luminance / tiles_across;
   }
   
   GroupMemoryBarrierWithGroupSync( );

   // Accumulate average luminance for all of the groups
   if ( row == 0 )
   {
      acc_luminance = 0;
      
      for (uint i = 0; i < rows; i++)
         acc_luminance += g_group_luminance[i];
      
      float avgLuminance = exp(acc_luminance / rows);
      
      float method = tonemap.x;
      float targetExposure = tonemap.y;
      float k = tonemap.z;
      
      float exposure = avgLuminance / targetExposure;
      
      const float prev_white = g_whitepoint[1];
      float white = (prev_white * (1 - k)) + (exposure * k);
      
      g_whitepoint[0] = method;
      g_whitepoint[1] = white;
   }
}
