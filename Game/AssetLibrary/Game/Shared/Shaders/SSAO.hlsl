//#define SHOW_SAMPLE_PATTERNS

Texture2D<float> g_linear_depth_0 : register(t0);
RWTexture2D<float2> g_output : register(u0);

cbuffer cb0 : register(b0)
{
   float4 g_camera_params;

   float RADIUS;
   float REF_SIZE;
   float Z_SCALE;
   float AO_INTENSITY;
   float AO_COMPRESSION;
};

#define THREADS_X 8
#define THREADS_Y 8

#define PI  3.141593
#define NUM_SAMPLES  11  // used by nvidia

groupshared float3 g_vs[THREADS_X * THREADS_Y][NUM_SAMPLES + 1];

float3 to_vs(float2 pixel, float depth, float2 resolution)
{
   const float p00 = g_camera_params.x;
   const float p11 = g_camera_params.y;
   const float p20 = g_camera_params.z;
   const float p21 = g_camera_params.w;

   const float w = resolution.x;
   const float h = resolution.y;
   
   float3 vs;
   
   vs.x = ((1.0f - p20) / p00) - (( 2.0f * (pixel.x + .5f)) / (w * p00));
   vs.y = ((1.0f + p21) / p11) - ((-2.0f * (pixel.y + .5f)) / (h * p11));
   vs.z = depth;
   
   vs.xy *= vs.z;
   
   return vs;
}

[numthreads(THREADS_X, THREADS_Y, 1)]
void cs_ssao(uint3 groupThreadId : SV_GroupThreadId, uint3 dispatchThreadId : SV_DispatchThreadID)
{  
   uint2 resolution;
   g_linear_depth_0.GetDimensions( resolution.x, resolution.y ); 

   int2 d_pixel = dispatchThreadId.xy;

   //convert to camera space x, y, z
   float depth = g_linear_depth_0[d_pixel];

   float3 vs_this_pixel = to_vs( d_pixel, depth, resolution );
   
   // calc screen space radius
   const float r = - (RADIUS * REF_SIZE) / vs_this_pixel.z;
   const float t = 6;   //turns around the circle made by the spiral
   const float o = (30 * d_pixel.x) ^ d_pixel.y + 10 * d_pixel.x * d_pixel.y; // angular offset
   
   // sample positions
   int i;

#ifdef SHOW_SAMPLE_PATTERNS
   g_output[ dispatchThreadId.xy ] = 0.0f;
#endif

   for ( i = 0; i < NUM_SAMPLES; i++ )
   {
      // put sample positions in a spiral pattern
      //float a = (1.0 / NUM_SAMPLES) * ( i - NUM_SAMPLES / 2.0 + .5 );
      float a = (1.0 / NUM_SAMPLES) * ( i + .5 );
      float h = r * a;
      float radians = 2.0 * PI * a * t + o;
      
      float2 u = float2( cos(radians), sin(radians) );
      int2 full_res_position = int2(d_pixel + h * u);

      float z = g_linear_depth_0[ full_res_position ];

#ifdef SHOW_SAMPLE_PATTERNS
   if ( dispatchThreadId.x == resolution.x / 2 && dispatchThreadId.y == resolution.y / 2 )
   {
      g_output[ full_res_position ] = 10.0f;
      g_output[ dispatchThreadId.xy ] = 10.0f;
   }
#endif

      g_vs[ groupThreadId.y * THREADS_X + groupThreadId.x ][ i ] = to_vs( full_res_position, z, resolution );
   }

#ifdef SHOW_SAMPLE_PATTERNS
   return;
#endif
   
   g_vs[ groupThreadId.y * THREADS_X + groupThreadId.x ][ NUM_SAMPLES ] = vs_this_pixel;
   
   GroupMemoryBarrierWithGroupSync( );

   const float epsilon = 0.01; // prevent divide by 0
   
   float ao = 0;
   
   //get normal
   float3 vs_dx, vs_dy;

   if ( groupThreadId.x + 1 < THREADS_X )
      vs_dx = vs_this_pixel.xyz - g_vs[ groupThreadId.y * THREADS_X + groupThreadId.x + 1 ][ NUM_SAMPLES ];
   else
      vs_dx = g_vs[ groupThreadId.y * THREADS_X + groupThreadId.x - 1 ][ NUM_SAMPLES ] - vs_this_pixel.xyz;
      
   if ( groupThreadId.y + 1 < THREADS_Y )
      vs_dy = vs_this_pixel.xyz - g_vs[ (groupThreadId.y + 1) * THREADS_X + groupThreadId.x ][ NUM_SAMPLES ];
   else
      vs_dy = g_vs[ (groupThreadId.y - 1) * THREADS_X + groupThreadId.x ][ NUM_SAMPLES ] - vs_this_pixel.xyz;
   
   float3 normal = normalize( cross(vs_dx, vs_dy) );

   // go through all the samples and calculate AO
   for ( i = 0; i < NUM_SAMPLES; i++ )
   {
      // determine view space normal at our pixel
      float3 vs_this_sample = g_vs[ groupThreadId.y * THREADS_X + groupThreadId.x ][ i ];
      
      float3 v = vs_this_sample - vs_this_pixel;
      float vn = dot(v, normal);
      float vv = dot(v, v);
   
      float value = 0;
   
      value = max( 0, vn + (vs_this_pixel.z * Z_SCALE)) / (vv + epsilon);
      
      ao += value;
   }

   ao = pow( max( 0, 1.0 - (2 * AO_INTENSITY / NUM_SAMPLES) * ao ), AO_COMPRESSION );

   g_output[dispatchThreadId.xy] = float2(1.0 - ao, vs_this_pixel.z);
}












