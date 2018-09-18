Texture2D<float> g_input : register(t0);
RWTexture2D<float> g_output : register(u0);

[numthreads(8, 8, 1)]
void cs_float_copy(uint3 dispatchThreadId : SV_DispatchThreadID)
{
   g_output[dispatchThreadId.xy] = g_input[dispatchThreadId.xy];
}
