Texture2D<float3> g_hdr_0 : register(t0);
Texture2D<float3> g_hdr_1: register(t1);
RWTexture2D<float3> g_output : register(u0);

SamplerState g_sampler_0 : register(s0);
SamplerState g_sampler_1 : register(s1);

[numthreads(8, 8, 1)]
void cs_main(uint3 dispatchThreadId : SV_DispatchThreadID)
{                                    
   uint2 res;
   g_output.GetDimensions( res.x, res.y );
   
   float2 uv = dispatchThreadId.xy / (float2) res;
   
	float3 color;

   color  = g_hdr_0.SampleLevel( g_sampler_0, uv, 0 );
	color += g_hdr_1.SampleLevel( g_sampler_1, uv, 0 );

	g_output[ dispatchThreadId.xy ] = color * .5f;
}

