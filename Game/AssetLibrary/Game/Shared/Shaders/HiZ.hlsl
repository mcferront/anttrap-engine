Texture2D<float> g_input : register(t0);
RWTexture2D<float> g_output : register(u0);

[numthreads(8, 8, 1)]
void cs_hiz(uint3 dispatchThreadId : SV_DispatchThreadID)
{  
   uint width, height;
   g_output.GetDimensions(width, height);
   
   if ( dispatchThreadId.x < width )
   {
      const uint2 topLeft = dispatchThreadId.xy * 2;
      const uint2 topRight = topLeft + uint2(1, 0);
      const uint2 bottomLeft = topLeft + uint2(0, 1);
      const uint2 bottomRight = topLeft + uint2(1, 1);
      
      float depths[ 4 ];
      
      depths[ 0 ] = g_input[ topLeft ];
      depths[ 1 ] = g_input[ topRight ];
      depths[ 2 ] = g_input[ bottomLeft ];
      depths[ 3 ] = g_input[ bottomRight ];
      
      g_output[dispatchThreadId.xy] = max( max(depths[0], depths[1]), max(depths[2], depths[3]) );
   }
}