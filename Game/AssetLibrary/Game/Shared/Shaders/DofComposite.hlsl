Texture2D<float> g_coc_map : register(t0);
Texture2D<float4> g_dof_map : register(t1);
RWTexture2D<float3> g_hdr : register(u0);

SamplerState g_input_sampler : register(s0);

[numthreads(8, 8, 1)]
void cs_composite(uint3 dispatch_thread_id : SV_DispatchThreadID )   
{
   
   int2 d_pixel = dispatch_thread_id.xy;
   
   int2 dispatch_res;
   g_hdr.GetDimensions( dispatch_res.x, dispatch_res.y );

   const float2 uv = (d_pixel + .5) / (float2) dispatch_res;
   const float2 near_uv = uv * float2(.5, 1.0);
   const float2 far_uv = near_uv + float2(.5, 0.0);
   
   float coc = g_coc_map[ d_pixel ];

   float4 dof_near = g_dof_map.SampleLevel( g_input_sampler, near_uv, 0 );
   
   float3 color = g_hdr[ d_pixel ];
   
   if ( coc > 0 )
   {
      float4 far_color = g_dof_map.SampleLevel( g_input_sampler, far_uv, 0 );
      color = lerp( color, far_color.rgb, coc );
   }

   {

      float alpha = dof_near.a * (1.0 - coc); // fade the more we're going into far blur territory
      color = lerp( color, dof_near.rgb, alpha );
   }
   
   g_hdr[ d_pixel ] = color;
}







































