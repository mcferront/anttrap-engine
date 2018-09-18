Texture2D<float4> g_input : register(t0);
RWTexture2D<float4> g_output : register(u0);

SamplerState g_input_sampler : register(s0);


[numthreads(8, 8, 1)]
void cs_mip(uint3 dispatchThreadId : SV_DispatchThreadID)
{
   uint2 resolution;
   g_output.GetDimensions(resolution.x, resolution.y);

   g_output[dispatchThreadId.xy] = g_input.SampleLevel(g_input_sampler, (dispatchThreadId.xy + .5) / resolution, 0);
}
