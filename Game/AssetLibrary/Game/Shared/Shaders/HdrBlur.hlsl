RWTexture2D<float3> g_uav : register(u0);

#define THREADS_X 8
#define THREADS_Y 8

cbuffer cb0 : register(b0)
{
   float g_flags;
};

float3 blur(float2 pixel, float2 direction) 
{
   uint2 resolution;
   g_uav.GetDimensions( resolution.x, resolution.y );
  
   float3 color = 0;
   float2 off1 = float2(1.411764705882353, 1.411764705882353) * direction;
   float2 off2 = float2(3.2941176470588234, 3.2941176470588234) * direction;
   float2 off3 = float2(5.176470588235294, 5.176470588235294) * direction;

   color += g_uav.Load( pixel ) * 0.1964825501511404;
   color += g_uav.Load( (pixel + off1) ) * 0.2969069646728344;
   color += g_uav.Load( (pixel - off1) ) * 0.2969069646728344;
   color += g_uav.Load( (pixel + off2) ) * 0.09447039785044732;
   color += g_uav.Load( (pixel - off2) ) * 0.09447039785044732;
   color += g_uav.Load( (pixel + off3) ) * 0.010381362401148057;
   color += g_uav.Load( (pixel - off3) ) * 0.010381362401148057;

   return color;
}

[numthreads(THREADS_X, THREADS_Y, 1)]
void cs_blur(uint3 dispatchThreadId : SV_DispatchThreadID)
{  
   uint bit_flags = asuint(g_flags);
   
   uint2 input, output;
      
   if ( (bit_flags & 0x01) != 0 )
      g_uav[ dispatchThreadId.xy ] = blur(dispatchThreadId.xy + .5, float2(0,1));
   else
      g_uav[ dispatchThreadId.xy ] = blur(dispatchThreadId.xy + .5, float2(1,0));
}




























