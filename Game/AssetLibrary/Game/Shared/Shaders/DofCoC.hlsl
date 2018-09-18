// coc can' be low res, or the far blend looks chunky
// 

// DOF details
// DofCoc.hlsl creates near/far planes
//    near/far blur planes are a single low res texture [width, height/2]: [0-width/2) near plane, [width/2-width) far plane
//    hdr render target is separated into near and far blur planes
//    pixels which should be excluded from near or far targets are copied with a 0 alpha
// DofBlur.hlsl
//    bokeh blurs the far plane (alpha aware blur)
//    gauss blurs the near plane (alpha aware blur)
// DofComposite.hlsl
//    composites results back to the main hdr target

// coc is high res to prevent aliasing
//    coc.xy used for blur amount
// g_dof is a high bit format, required because alpha is used for invalid pixel tracking
// would be faster if we did it post tonemapping


Texture2D<float3> g_hdr : register(t0);
Texture2D<float> g_linear_z : register(t1);
RWTexture2D<float4> g_dof : register(u0);
RWTexture2D<float> g_coc : register(u1);

SamplerState g_sampler : register(s0);

cbuffer cb0 : register(b0)
{
   float4 g_dof_params;
};

#define THREADS_X    8
#define THREADS_Y    8

float2 compute_coc( float focal_start, float focal_end, float depth )
{
   float2 coc;
   
   const float eps = .0000001;
   
   coc.x = max( 0.0, 1.0 - depth / (focal_start + eps) );    // bigger blur farther away from start plane
   coc.y = max( 0.0, (depth - focal_end) / (depth + eps));
   
   coc.y = 1 - pow( 1 - coc.y, 2 );

   return coc;
}

[numthreads(THREADS_X, THREADS_Y, 1)]
void cs_coc(uint3 group_thread_id : SV_GroupThreadId, uint3 dispatch_thread_id : SV_DispatchThreadID)
{
   const float focal_start = g_dof_params.x;
   const float focal_end = g_dof_params.y;
   const uint2 d_pixel = dispatch_thread_id.xy;
   
   int2 dispatch_res;
   g_hdr.GetDimensions( dispatch_res.x, dispatch_res.y );
   
   int2 linear_z_res;
   g_linear_z.GetDimensions( linear_z_res.x, linear_z_res.y );
   
   float2 scale = linear_z_res / float2(dispatch_res);

   float depth = g_linear_z[ d_pixel * scale ];
   float2 coc = compute_coc( focal_start, focal_end, depth );
   
   // write to full resolution coc
   g_coc[ d_pixel ] = coc.y;

   // write to quarter res color buffers
   if ( (d_pixel.x & 0x1) == 0 && (d_pixel.y & 0x1) == 0 )
   {
      const float2 uv = (d_pixel + .5) / (float2) dispatch_res;
      const float3 color = g_hdr.SampleLevel(g_sampler, uv, 0);

      int2 dof_res;
      g_dof.GetDimensions( dof_res.x, dof_res.y );
      
      float2 dof_pixel = d_pixel * .5;
      
      // near coc
      g_dof[ dof_pixel ] = float4( color, coc.x > 0 ? 1 : 0 );
      
      // far coc
      g_dof[ dof_pixel + int2(dof_res.x * .5, 1) ] = float4( color, coc.y > 0 ? 1 : 0 );
   }

   g_coc[ d_pixel ] = coc.y;
}



















































