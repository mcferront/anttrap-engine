Texture2D<float2> g_ssao : register(t0);
RWTexture2D<float2> g_output : register(u0);

#define PARAMS

// Gaussian coefficients
#define WEIGHTS 7
static const float gaussian[WEIGHTS] = 
	{ 0.111220, 0.107798, 0.098151, 0.083953, 0.067458, 0.050920, 0.036108 }; // stddev = 3.0

cbuffer cb0 : register(b0)
{
   float g_flags;
   
#ifdef PARAMS
   float EDGE_SHARPNESS;
   float SCALE;
#endif
};


#ifndef PARAMS
   #define EDGE_SHARPNESS     (1.0)
   #define SCALE  2.0
#endif

#define THREADS_X 8
#define THREADS_Y 8


float2 blur_vertical(uint2 pixel)
{
   float2 ssao = g_ssao[ pixel ];
   float sum = ssao.r;
   float z = ssao.g;

   float totalWeight = gaussian[0];
   sum *= totalWeight;

   int i;
   
   for (i = 0; i < WEIGHTS - 1; i++)
   {
      float2 ssao_tap = g_ssao[ int2(pixel.x, pixel.y - (WEIGHTS - 1) * SCALE + (i * SCALE)) ];
      float tap = ssao_tap.r;
      float tap_z = ssao_tap.g;
      
      float weight = 0.3 + gaussian[ WEIGHTS - i - 1 ];

      // range domain (the "bilateral" weight). As depth difference increases, decrease weight.
      weight *= max(0.0, 1.0 - EDGE_SHARPNESS * abs(z - tap_z));
      
      sum += tap * weight;
      totalWeight += weight;
   }
   
   for (i = 0; i < WEIGHTS - 1; i++)
   {
      float2 ssao_tap = g_ssao[ int2(pixel.x, pixel.y + (i * SCALE) + SCALE) ];
      float tap = ssao_tap.r;
      float tap_z = ssao_tap.g;
      
      float weight = 0.3 + gaussian[ i + 1 ];

      // range domain (the "bilateral" weight). As depth difference increases, decrease weight.
      weight *= max(0.0, 1.0 - EDGE_SHARPNESS * abs(z - tap_z));
      
      sum += tap * weight;
      totalWeight += weight;
   }
   
   const float epsilon = 0.0001;
   return float2(sum / (totalWeight + epsilon), z);
}

float2 blur_horizontal(uint2 pixel)
{
   float2 ssao = g_ssao[ pixel ];
   float sum = ssao.r;
   float z = ssao.g;

   float totalWeight = gaussian[0];
   sum *= totalWeight;

   int i;
   
   //10 - 4 * 3 + i * 3 = -2  : w=4
   //10 - 4 * 3 + i * 3 = 1   : w=3
   //10 - 4 * 3 + i * 3 = 4   : w=2
   //10 - 4 * 3 + i * 3 = 7   : w=1

   //10

   //10 + i * 3 + 3 = 13      : w=1
   //10 + i * 3 + 3 = 16      : w=2
   //10 + i * 3 + 3 = 19      : w=3
   //10 + i * 3 + 3 = 22      : w=4
   
   
   for (i = 0; i < WEIGHTS - 1; i++)
   {
      float2 ssao_tap = g_ssao[ int2(pixel.x - (WEIGHTS - 1) * SCALE + (i * SCALE), pixel.y) ];
      float tap = ssao_tap.r;
      float tap_z = ssao_tap.g;
      
      float weight = 0.3 + gaussian[ WEIGHTS - i - 1 ];

      // range domain (the "bilateral" weight). As depth difference increases, decrease weight.
      weight *= max(0.0, 1.0 - EDGE_SHARPNESS * abs(z - tap_z));
      
      sum += tap * weight;
      totalWeight += weight;
   }
   
   for (i = 0; i < WEIGHTS - 1; i++)
   {
      float2 ssao_tap = g_ssao[ int2(pixel.x + (i * SCALE) + SCALE, pixel.y) ];
      float tap = ssao_tap.r;
      float tap_z = ssao_tap.g;
      
      float weight = 0.3 + gaussian[ i + 1 ];

      // range domain (the "bilateral" weight). As depth difference increases, decrease weight.
      weight *= max(0.0, 1.0 - EDGE_SHARPNESS * abs(z - tap_z));
      
      sum += tap * weight;
      totalWeight += weight;
   }
   
   const float epsilon = 0.0001;
   return float2(sum / (totalWeight + epsilon), z);
}

[numthreads(THREADS_X, THREADS_Y, 1)]
void cs_blur(uint3 dispatchThreadId : SV_DispatchThreadID)
{  
   uint bit_flags = asuint(g_flags);
   
   if ( (bit_flags & 0x01) != 0 )
      g_output[ dispatchThreadId.xy ] = blur_horizontal(dispatchThreadId.xy);
   else
      g_output[ dispatchThreadId.xy ] = blur_vertical(dispatchThreadId.xy);
}









