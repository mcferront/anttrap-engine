Texture2DMS<float3> g_hdr_0 : register(t0);
Texture2DMS<float3> g_hdr_1: register(t1);
Texture2DMS<unorm float> g_depth_0 : register(t2);
Texture2DMS<unorm float> g_depth_1 : register(t3);
RWTexture2D<float3> g_output : register(u0);

cbuffer cb0 : register(b0)
{
   float4 g_params;
   float4 g_linear_z;
   float4x4 g_currViewProj;
   float4x4 g_prevInvViewProj;
};

#define Up     0
#define Down   1
#define Left   2
#define Right  3

float colorDiffBlend(float3 a, float3 b) 
{
   float3 differential = a - b;
   float len = sqrt(dot(differential, differential));
   return 1.f / (len + 0.00001f);
}

// convert projected depth into projected pixel position 
// for frame N-1
uint2 previousPixelPos( float2 pixel, float currDepth, float2 res )
{
   // no depth buffer information
   if ( currDepth <= 0.0 )
      return pixel;

   uint2 old_pixel = floor(pixel);

   // Projection is flipped from UV coords
   pixel.y = res.y - pixel.y - 1;

   float2 projected = pixel / res * 2.0 - 1;

   float4 re_projected_pre_w_divide = float4( projected .x, projected .y, currDepth, 1.0 );
   float4 ws = mul( re_projected_pre_w_divide, g_prevInvViewProj );
   ws /= ws.w;

   float4 ws_to_curr_projection = mul( ws, g_currViewProj );
   ws_to_curr_projection = ws_to_curr_projection / ws_to_curr_projection.w;

   float2 curr = ws_to_curr_projection.xy * (res / 2) + (res / 2);
   curr.y = res.y - curr.y - 1;

   uint2 new_pixel = floor(curr);
   int2 delta = new_pixel - old_pixel;

   return old_pixel - delta;
}

float3 readFromQuadrant(int2 pixel, int quadrant)
{
   if ( 0 == quadrant )
      return g_hdr_0.Load( pixel, 1 );
   else if ( 1 == quadrant )
      return g_hdr_1.Load( pixel + int2(1, 0), 1);
   else if ( 2 == quadrant )
      return g_hdr_1.Load( pixel, 0);
   else
      return g_hdr_0.Load( pixel, 0);
}

float readDepthFromQuadrant(int2 pixel, int quadrant)
   {
   if ( 0 == quadrant )
      return g_depth_0.Load( pixel, 1 );
   else if ( 1 == quadrant )
      return g_depth_1.Load( pixel + int2(1, 0), 1);
   else if ( 2 == quadrant )
      return g_depth_1.Load( pixel, 0);
   else //( 3 == quadrant )
      return g_depth_0.Load( pixel, 0);
}

// Simple tonemap to invtonemap color blend
// should be replaced by customized solution
float3 hdrColorBlend( float3 a, float3 b, float3 c, float3 d )
{
   // Reinhard 
   float3 t_a = a / (a + 1);
   float3 t_b = b / (b + 1);
   float3 t_c = c / (c + 1);
   float3 t_d = d / (d + 1);

   float3 color = (t_a + t_b + t_c + t_d) * .25f;
   
   // back to hdr
   return - color / (color - 1);
}

float3 colorFromCardinalOffsets( uint2 qtr_res_pixel, int2 offsets[ 4 ], int quadrants[ 2 ] )
{
   float3 color[ 4 ];

   float2 w;

   color[ Up ] = readFromQuadrant( qtr_res_pixel + offsets[ Up ], quadrants[ 0 ] );
   color[ Down ] = readFromQuadrant( qtr_res_pixel + offsets[ Down ], quadrants[ 0 ] );
   color[ Left ] = readFromQuadrant( qtr_res_pixel + offsets[ Left ], quadrants[ 1 ] );
   color[ Right ] = readFromQuadrant( qtr_res_pixel + offsets[ Right ], quadrants[ 1 ] );

   return hdrColorBlend(color[Up].rgb, color[Down].rgb, color[Left].rgb, color[Right].rgb);   
}

void getCardinalOffsets( int quadrant, out int2 offsets[4], out int quadrants[2] )
{
   if ( quadrant == 0 )
   {
      offsets[Up] = - int2(0, 1);
      offsets[Down] = 0;
      offsets[Left] = - int2(1, 0);
      offsets[Right] = 0;

      quadrants[0] = 2;
      quadrants[1] = 1;
   }
   else if ( quadrant == 1 )
   {
      offsets[Up] = - int2(0, 1);
      offsets[Down] = 0;
      offsets[Left] = 0;
      offsets[Right] = + int2(1, 0);

      quadrants[0] = 3;
      quadrants[1] = 0;
   }
   else if ( quadrant == 2 )
   {
      offsets[Up] = 0;
      offsets[Down] = + int2(0, 1);
      offsets[Left] = - int2(1, 0);
      offsets[Right] = 0;

      quadrants[0] = 0;
      quadrants[1] = 3;
   }
   else // ( quadrant == 3 )
   {
      offsets[Up] = 0;
      offsets[Down] = + int2(0, 1);
      offsets[Left] = 0;
      offsets[Right] = + int2(1, 0);

      quadrants[0] = 1;
      quadrants[1] = 2;
   }
}

float projectedDepthToLinear( float depth )
{
   return (depth * g_linear_z.x + g_linear_z.y) / (depth * g_linear_z.z + g_linear_z.w);
}

float3 resolve_cb( uint2 dispatchThreadId )
{
   uint2 full_res;
   g_output.GetDimensions( full_res.x, full_res.y );

   uint flags = asuint(g_params.x);
   
#define DEBUG_RENDER

#ifdef DEBUG_RENDER
   const bool render_motion_vectors = ( flags & 0x01 ) != 0;
   const bool render_missing_pixels = ( flags & 0x02 ) != 0;
   const bool render_qtr_motion_pixels = ( flags & 0x04 ) != 0;
   const bool render_obstructed_pixels = ( flags & 0x08 ) != 0;
   const bool render_checker_pattern_odd = ( flags & 0x40 ) != 0;
   const bool render_checker_pattern_even = ( flags & 0x80 ) != 0;
#endif

   const uint frame_offset = (flags & 0x10) ? 0 : 1;
   const bool check_pixel_occlusion = ( flags & 0x20 ) != 0;

   const uint2 qtr_res = full_res * .5;
   const uint2 full_res_pixel = dispatchThreadId.xy;
   const uint2 qtr_res_pixel = floor(dispatchThreadId.xy * .5);
   const uint quadrant = ( dispatchThreadId.x & 0x1 ) + ( dispatchThreadId.y & 0x1 ) * 2;
   const float tolerance = g_params.y;
   
   const uint frame_lookup[ 2 ][ 2 ] = 
   {
      { 0, 3 },
      { 1, 2 } 
   };

   uint frame_quadrants[ 2 ];
   frame_quadrants[ 0 ] = frame_lookup[ frame_offset ][ 0 ];
   frame_quadrants[ 1 ] = frame_lookup[ frame_offset ][ 1 ];

#ifdef DEBUG_RENDER
   if ( render_checker_pattern_odd + render_checker_pattern_even > 0 )
   {
      if ( ( render_checker_pattern_even && ( quadrant == 0 || quadrant == 3 ) ) ||
         ( render_checker_pattern_odd && ( quadrant == 1 || quadrant == 2 ) ) )
         return readFromQuadrant( qtr_res_pixel, quadrant );
      else
         return float3( 0, 0, 0 );
   }
#endif

   // if the pixel we are writing to is in a MSAA quadrant which matches our latest CB frame
   // then read it directly and we're done
   if ( frame_quadrants[ 0 ] == quadrant || frame_quadrants[ 1 ] == quadrant )
      return readFromQuadrant( qtr_res_pixel, quadrant );
   else
   {
      // We need to read from Frame N-1

      int2 cardinal_offsets[ 4 ];
      int cardinal_quadrants[ 2 ];

      // Get the locations of the pixels in Frame N which surround
      // our current pixel location
      getCardinalOffsets( quadrant, cardinal_offsets, cardinal_quadrants );

      // What is the depth at this pixel which was written to by Frame N-1
      float depth = readDepthFromQuadrant( qtr_res_pixel, quadrant );

      // Project that through the matrices and get the screen space position
      // this pixel was rendered in Frame N-1
      uint2 prev_pixel_pos = previousPixelPos( full_res_pixel + .5f, depth, full_res );

      int2 pixel_delta = floor((full_res_pixel + .5f) - prev_pixel_pos);
      int2 qtr_res_pixel_delta = pixel_delta * .5f;

      int2 prev_qtr_res_pixel = floor(prev_pixel_pos * .5f);
      
      // Which MSAA quadrant was this pixel in when it was shaded in Frame N-1
      uint quadrant_needed = ( prev_pixel_pos.x & 0x1 ) + ( prev_pixel_pos.y & 0x1 ) * 2;

#ifdef DEBUG_RENDER
      if ( render_motion_vectors && ( pixel_delta.x || pixel_delta.y )  )
         return float3( 1, 0, 0 );

      if ( render_qtr_motion_pixels && ( qtr_res_pixel_delta.x || qtr_res_pixel_delta.y ) )
         return float3( 0, 1, 0 );
#endif

      bool missing_pixel = false;

      // if it falls on this frame (Frame N)'s quadrant then the shading information is missing
      // so extrapolate the color from the texels around us
      if ( frame_quadrants[ 0 ] == quadrant_needed || frame_quadrants[ 1 ] == quadrant_needed )
         missing_pixel = true;
      else if ( qtr_res_pixel_delta.x || qtr_res_pixel_delta.y )
      {
         // Otherwise we might have the shading information,
         // Now we check to see if it's obstructed

         // If the user doesn't want to check for obstruction we just assume it's obstructed
         // and this pixel will be an extrapolation of the Frame N pixels around it
         // This generally saves on perf and isn't noticeable because the pixels are in motion anyway
         if ( false == check_pixel_occlusion )
            missing_pixel = true;
         else
         {
            // Fetch the interpolated (up or down, left or right) depth at this location in Frame N
            float4 current_depth = 0;
            const int count = 4;

            current_depth.x = readDepthFromQuadrant( qtr_res_pixel + cardinal_offsets[ Left ], cardinal_quadrants[ 1 ] );
            current_depth.y = readDepthFromQuadrant( qtr_res_pixel + cardinal_offsets[ Right ], cardinal_quadrants[ 1 ] );

            current_depth.z = readDepthFromQuadrant( qtr_res_pixel + cardinal_offsets[ Down ], cardinal_quadrants[ 0 ] );
            current_depth.w = readDepthFromQuadrant( qtr_res_pixel + cardinal_offsets[ Up ], cardinal_quadrants[ 0 ] );

            float current_depth_avg = (projectedDepthToLinear( current_depth.x ) + 
                                       projectedDepthToLinear( current_depth.y ) + 
                                       projectedDepthToLinear( current_depth.z ) +
                                       projectedDepthToLinear( current_depth.w )) * .25f;

            // reach across the frame N-1 and grab the depth of the pixel we want
            // then compare it to Frame N's depth at this pixel to see if it's within range
            float prev_depth = readDepthFromQuadrant( prev_qtr_res_pixel, quadrant_needed );
            prev_depth = projectedDepthToLinear( prev_depth );

            // if the discrepancy is too large assume the pixel we need to 
            // fetch from the previous buffer is missing
            float diff = prev_depth - current_depth_avg;
            missing_pixel = abs(diff) >= tolerance;

         #ifdef DEBUG_RENDER
               if ( render_obstructed_pixels && missing_pixel )
                  return float3( 1, 1, 0 );
         #endif
         }
      }
      
#ifdef DEBUG_RENDER
      if ( render_missing_pixels && missing_pixel )
         return float3( 1, 0, 0 );
#endif

      // If we've determined the pixel (i.e. shading information) is missing,
      // then extrapolate the missing color by blending the 
      // current frame's up, down, left, right pixels
      if ( missing_pixel == true )
         return colorFromCardinalOffsets( qtr_res_pixel, cardinal_offsets, cardinal_quadrants );
      else
         return readFromQuadrant( prev_qtr_res_pixel, quadrant_needed );
   }
}

[numthreads(8, 8, 1)]
void cs_main( uint3 dispatchThreadId : SV_DispatchThreadID )
{
   g_output[ dispatchThreadId.xy ] = resolve_cb( dispatchThreadId.xy );
}

