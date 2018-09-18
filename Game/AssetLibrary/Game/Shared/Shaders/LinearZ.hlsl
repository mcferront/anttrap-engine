Texture2D<unorm float> g_depth : register(t0);
RWTexture2D<float> g_output : register(u0);

cbuffer cb0 : register(b0)
{
   float4 g_params;
};

[numthreads(8, 8, 1)]
void cs_depth_to_linear(uint3 dispatchThreadId : SV_DispatchThreadID)
{  
   float depth = g_depth[ dispatchThreadId.xy ];
   
   const float row_2_z = g_params.x;
   const float row_3_z = g_params.y;
   const float row_2_w = g_params.z;
   const float row_3_w = g_params.w;

   // linear depth from near_clip to far_clip
   g_output[dispatchThreadId.xy] = (depth * row_2_z + row_3_z) / (depth * row_2_w + row_3_w);
}

