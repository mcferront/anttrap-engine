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

// filip strugar for these conversion routines
float LINEAR_to_SRGB( float val )
{
    if( val < 0.0031308 )
        val *= float( 12.92 );
    else
        val = float( 1.055 ) * pow( abs( val ), float( 1.0 ) / float( 2.4 ) ) - float( 0.055 );
    return val;
}
float3 LINEAR_to_SRGB( float3 val )
{
    return float3( LINEAR_to_SRGB( val.x ), LINEAR_to_SRGB( val.y ), LINEAR_to_SRGB( val.z ) );
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
   output.rgb = LINEAR_to_SRGB(output.rgb);
   
   g_outTexture[ dispatchThreadId.xy ] =  output;
}




   


































