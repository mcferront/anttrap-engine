Texture2D<float3> g_input : register(t0);
RWTexture2D<float3> g_output : register(u0);

[numthreads(8, 8, 1)]
void cs_float3_copy(uint3 dispatchThreadId : SV_DispatchThreadID)
{
   g_output[dispatchThreadId.xy] = g_input[dispatchThreadId.xy];
}
