Texture2D<float> g_coc_map : register(t0);
Texture2D<float4> g_dof_map : register(t1);
Texture2D<float3> g_hdr_in : register(t2);
RWTexture2D<float3> g_hdr_out : register(u0);

SamplerState g_input_sampler : register(s0);

cbuffer cb0 : register(b0)
{
   float4 g_params;
};

[numthreads(8, 8, 1)]
void cs_composite(uint3 dispatch_thread_id : SV_DispatchThreadID )   
{
   const float focal_start = g_params.x;
   const float focal_end = g_params.y;
   const float near_pow = g_params.z;
   const float coc_blend = g_params.w;
   
   int2 d_pixel = dispatch_thread_id.xy;
   
   int2 dispatch_res;
   g_hdr_out.GetDimensions( dispatch_res.x, dispatch_res.y );

   const float2 uv = (d_pixel + .5) / (float2) dispatch_res;
   const float2 near_uv = uv * float2(.5, 1.0);
   const float2 far_uv = near_uv + float2(.5, 0.0);
   
   float coc = g_coc_map[ d_pixel ];

   float4 dof_near = g_dof_map.SampleLevel( g_input_sampler, near_uv, 0 );
   
   float3 color;
   
   if ( coc > 0 )
   {
      float4 far_color = g_dof_map.SampleLevel( g_input_sampler, far_uv, 0 );
      color = far_color.rgb;
      
      // lerp based on coc if a soft blend is needed
      // but that will cause some haloing
      if ( coc_blend > 0 )
      {
         color = g_hdr_in[ d_pixel ];
         color = lerp( color, far_color.rgb, min(coc * coc_blend, 1) );
      }
   }
   else
      color = g_hdr_in[ d_pixel ];

   {

      // if there is any far coc, don't ramp our near alpha because it could cause
      // focal pixels bleeding and 'haloing' around the far blur
      float alpha = coc == 0 ? 1.0 - pow( (1.0 - dof_near.a), near_pow ) : dof_near.a;
      
      alpha = alpha * (1.0 - coc); // fade the more we're going into far blur territory
      color = lerp( color, dof_near.rgb, alpha );
   }
   
   g_hdr_out[ d_pixel ] = color;
}







































