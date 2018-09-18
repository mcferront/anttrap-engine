Texture2D<uint2> g_ssr : register(t0);
Texture2D<float4> g_mat_properties : register(t1);
Texture2D<float3> g_hdr : register(t2);
Texture2D<float> g_z : register(t3);
RWTexture2D<float3> g_scene : register(u0);

SamplerState g_input_sampler : register(s0);

cbuffer cb0 : register(b0)
{
   float4 g_params;
};

#define THREADS_X 8
#define THREADS_Y 8

#define MISSED_PIXEL       uint2(65000,65000)
#define DOES_NOT_REFLECT   uint2(60000,60000)
#define REFL_DAMPER        4.0
#define MAX_MIPS           4  //must be <= to HdrMipLevels in RenderNodes.h

[numthreads(THREADS_X, THREADS_Y, 1)]
void cs_ssr_composite(uint3 groupThreadId : SV_GroupThreadId, uint3 dispatchThreadId : SV_DispatchThreadID)
{     
   const uint flags = asuint(g_params.x);
   const bool show_mip = flags & 0x01;
   const bool force_mip = flags & 0x02;
   const bool show_z = flags & 0x04;
   const int forced_mip_level = g_params.y;

   uint2 resolution;
   g_hdr.GetDimensions( resolution.x, resolution.y );
   
   if ( show_mip )
   {
      float2 uv = ((dispatchThreadId.xy) + .5) / resolution;
      float3 color = g_hdr.SampleLevel( g_input_sampler, uv, forced_mip_level );
      g_scene[ dispatchThreadId.xy ] = color;
   }
   else if ( show_z )
   {
      float3 color = g_z.Load( uint3(dispatchThreadId.xy >> forced_mip_level, forced_mip_level) ).rrr * 10.0;
      g_scene[ dispatchThreadId.xy ] = color;
   }
   else
   {
      uint2 ssr_index = g_ssr[ dispatchThreadId.xy ];
      
      if ( ssr_index.x != DOES_NOT_REFLECT.x )
      {
         uint width, height;
         g_ssr.GetDimensions( width, height );

         float3 scene = g_hdr.Load( uint3(dispatchThreadId.xy, 0) );

         uint mat_width, mat_height, count;
         g_mat_properties.GetDimensions( mat_width, mat_height );
            
         float scale = mat_width / float(width);
         
         float4 props = g_mat_properties[ dispatchThreadId.xy * scale ];   
         
         
         // TODO: spend some time on a non linear, smarter fade
         const float edge = .15;
         const float roughness = saturate(1.0 - props.x * REFL_DAMPER);
         
         int mip = int((MAX_MIPS - 1) * roughness);
         
         if ( force_mip )
            mip = forced_mip_level;
         
         float3 ssr = g_hdr.SampleLevel( g_input_sampler, (ssr_index + .5) / resolution, mip );

         float2 half_res = float2( width / 2, height / 2 );
         float2 pos = float2( ssr_index.x - half_res.x, ssr_index.y - half_res.y );
         
         float dist = 1.0 - max( abs(pos.x) / half_res.x, abs(pos.y) / half_res.y );
         float fade = dist > edge ? 1.0f : dist / edge;
         
         float refl = roughness * fade;
         g_scene[ dispatchThreadId.xy ] = scene + ssr * refl;
      }
   }
}




