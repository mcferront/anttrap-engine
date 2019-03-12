Texture2D<float3> g_inTexture  : register(t0);
Texture2D<float> g_ssao  : register(t1);
StructuredBuffer<float> g_whitepoint : register(t2);
RWTexture2D<unorm float4> g_outTexture : register(u0);

cbuffer cb0 : register(b0)
{
   float4 g_params;
};

float3 Sigmoidal( float3 color )
{
   const float a = g_params.x;
   const float b = g_params.y;
   const float lm = g_whitepoint[1];
   
   const float3 pow_color_b = pow(color, b);
   
   return (pow_color_b / (pow(lm / a, b) + pow_color_b));
}

[numthreads(8, 8, 1)]
void cs_main(uint3 dispatchThreadId : SV_DispatchThreadID )   
{
   float method = g_whitepoint[0];

   uint2 d_pixel = dispatchThreadId.xy;
   
   float4 output;
   
   if ( method == 3 )
      output.rgb = g_ssao[ d_pixel ];
   else
   {
      output.rgb = g_inTexture[ d_pixel ].rgb;

      if ( method == 1 )
         output.rgb = Sigmoidal(output.rgb);
   }
   output.a = 1.0;
   g_outTexture[ dispatchThreadId.xy ] =  pow(output, 1 / 2.2);
}




   


































