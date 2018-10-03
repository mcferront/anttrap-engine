Texture2D<float3> g_hdr_blur  : register(t0);
RWTexture2D<float3> g_hdr : register(u0);

SamplerState g_input_sampler : register(s0);

cbuffer cb0 : register(b0)
{
   float4 g_params;
};

[numthreads(8, 8, 1)]
void cs_composite(uint3 dispatch_thread_id : SV_DispatchThreadID )   
{
   uint2 d_pixel = dispatch_thread_id.xy;
  
   uint2 resolution;
   g_hdr_blur.GetDimensions( resolution.x, resolution.y );
   
   float spec_bloom_min = g_params.x;
   float spec_bloom_gradient = g_params.y;

   float3 color = g_hdr[ d_pixel ];
   float3 spec_bloom = g_hdr_blur.SampleLevel( g_input_sampler, (d_pixel + .5) / resolution, 5 );
   
   // get the energy and normalize
   float bloom_energy = sqrt(dot(spec_bloom, spec_bloom));
   spec_bloom = spec_bloom / bloom_energy;
   
   // remove the lows and scale the curve
   bloom_energy = max(0, bloom_energy - spec_bloom_min) / spec_bloom_gradient;
   
   // restore the bloom color based on the new energy level
   spec_bloom = spec_bloom * bloom_energy;

   g_hdr[ d_pixel ] = color.rgb + spec_bloom;
}







































