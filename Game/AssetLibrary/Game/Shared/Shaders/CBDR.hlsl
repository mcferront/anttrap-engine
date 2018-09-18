Texture2D<float3> g_srt0  : register( t0 );
Texture2D<float3> g_srt1  : register( t1 );
Texture2DMS<float4> g_ctb0  : register( t2 );
Texture2DMS<float4> g_ctb1  : register( t3 );
Texture2DMS<unorm float> g_depth_0  : register( t4 );
Texture2DMS<unorm float> g_depth_1  : register( t5 );
RWTexture2D<float3> g_output : register( u0 );

cbuffer cb0 : register(b0)
{
   float4 g_params;
   float4x4 g_currInvViewProj;
   float4x4 g_prevViewProj;
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

float2 deriveMotionVectors(float2 pixel, float currDepth, float2 res)
{
   // no depth buffer information
   if ( currDepth <= 0.0 )
      return int2(0, 0);
   
   // Projection is flipped from UV coords
   pixel.y = res.y - pixel.y - 1;
   
   float2 current_projected = pixel / res * 2.0 - 1;
   
   float4 re_projected_pre_w_divide = float4(current_projected.x, current_projected.y, currDepth, 1.0 );
   float4 curr_ws = mul( re_projected_pre_w_divide, g_currInvViewProj );
   curr_ws /= curr_ws.w;

   float4 curr_ws_to_prev_projection = mul( curr_ws, g_prevViewProj );
   curr_ws_to_prev_projection = curr_ws_to_prev_projection / curr_ws_to_prev_projection.w;
   
   float2 delta = current_projected - curr_ws_to_prev_projection.xy;
            
   // reverse motion vector - 
   // we want to point back to where we came from
   delta = - delta;

   // reverse Y's motion because the pixel top-to-bottom
   // is reversed for the uv look up
   delta.y = - delta.y;

   return delta;
}
   
void readFromQuadrant(int2 pixel, int quadrant, out float3 opaque, out float4 transparent)
{
   if ( 0 == quadrant )
   {
      opaque = g_srt0[ pixel * int2(2,1) + int2(1, 0) ];
      transparent = g_ctb0.Load( pixel, 1 );
   }
   else if ( 1 == quadrant )
   {
      opaque = g_srt1[ pixel * int2(2,1) + int2(3, 0) ];
      transparent = g_ctb1.Load( pixel + int2(1, 0), 1);
   }
   else if ( 2 == quadrant )
   {
      opaque = g_srt1[ pixel * int2(2,1) + int2(0, 0) ];
      transparent = g_ctb1.Load( pixel, 0);
   }
   else
   {
      opaque = g_srt0[ pixel * int2(2,1) + int2(0, 0) ];
      transparent = g_ctb0.Load( pixel, 0);
   }
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

float projectedDepthToLinear( float linearZTransform, float depth )
{
   return 1.0 / (linearZTransform * depth + 1.0);
}

#define DEBUG_RENDER
[numthreads(8, 8, 1)]
void cs_main(uint3 dispatchThreadId : SV_DispatchThreadID)
{  
   const float min_motion = 0.01;
   const int flags = asuint(g_params.x);
   const float tolerance = g_params.y;
   const float linearZTransform = g_params.z;
   const bool is_even_frame = flags & 0x10;

#ifdef DEBUG_RENDER
   const bool render_motion_vectors = (flags & 0x01) != 0;
   const bool render_missing_pixels = (flags & 0x02) != 0;
   const bool render_qtr_motion_pixels = (flags & 0x04) != 0;
   const bool render_alternate_pixels = (flags & 0x08) != 0;
#endif

   uint2 full_res;
   g_output.GetDimensions( full_res.x, full_res.y ); 

   const uint2 qtr_res = full_res * .5;
   const uint2 qtr_res_pixel = dispatchThreadId.xy;

   const uint2 pixels[4] = 
   {
      dispatchThreadId.xy * 2 + int2(0,0),
      dispatchThreadId.xy * 2 + int2(1,0),
      dispatchThreadId.xy * 2 + int2(0,1),
      dispatchThreadId.xy * 2 + int2(1,1),
   };

   //quadrants are:
   //  0,1
   //  2,3
   int frame_quadrants[2];

   float depth;
   int even_motion_scalar = 1;
   int odd_motion_scalar = 1;
   
   if ( is_even_frame )
   {
      even_motion_scalar = 0;
      frame_quadrants[0] = 0;
      frame_quadrants[1] = 3;
      
   }
   else
   {
      odd_motion_scalar = 0;
      frame_quadrants[0] = 1;
      frame_quadrants[1] = 2;

   }
   
   for ( int i = 0; i < 4; i++ )
   {
      const uint2 full_res_pixel = pixels[i];

      float3 opaque;
      float4 ctb;

      // if the pixel we are writing to is in a quadrant which matches our latest CB frame
      // then read it directly and we're done
      if ( frame_quadrants[0] == i || frame_quadrants[1] == i )
         readFromQuadrant( qtr_res_pixel, i, opaque, ctb );
      else
      {
         const int quadrant = i;
         
         float depth = readDepthFromQuadrant(qtr_res_pixel, quadrant);
         float2 f_motion = deriveMotionVectors(full_res_pixel + .5, depth, full_res);
         float2 screen_motion = (f_motion * full_res / 2);
         float2 abs_screen_motion = abs(screen_motion);


         // If the motion is sub pixel, ignore it
         if ( abs_screen_motion.x < min_motion )
            screen_motion.x = abs_screen_motion.x = f_motion.x = 0;
         if ( abs_screen_motion.y < min_motion )
            screen_motion.y = abs_screen_motion.y = f_motion.y = 0;

      #ifdef DEBUG_RENDER
            if ( render_motion_vectors && (f_motion.x || f_motion.y))
            {
               g_output[ full_res_pixel ] = abs(float3(int2(f_motion * 100), 0));
               continue;
            }
      #endif
      
         bool missing_pixel = false;
      
         int2 cardinal_offsets[4];
         int cardinal_quadrants[2];
         // If there is movement, check the discrepency between the depth of the quadrant we are in
         // from our previous frame, and the interpolated (up,down,left,right) depth of the quadrant
         // from the current frame (that quadrant will not have rendered, which is why we interpolate)
         if ( abs_screen_motion.x > 0 || abs_screen_motion.y > 0 )
         {
            float other_depth[4];

            getCardinalOffsets( quadrant, cardinal_offsets, cardinal_quadrants );

            other_depth[Up] = readDepthFromQuadrant( qtr_res_pixel + cardinal_offsets[Up], cardinal_quadrants[0] );
            other_depth[Down] = readDepthFromQuadrant( qtr_res_pixel + cardinal_offsets[Down], cardinal_quadrants[0] );
            other_depth[Left] = readDepthFromQuadrant( qtr_res_pixel + cardinal_offsets[Left], cardinal_quadrants[1] );
            other_depth[Right] = readDepthFromQuadrant( qtr_res_pixel + cardinal_offsets[Right], cardinal_quadrants[1] );

            float other_depth_avg = 0;

            for ( int c = 0; c < 4; c++ )
               other_depth_avg += projectedDepthToLinear( linearZTransform, other_depth[i] );

            other_depth_avg *= .25;

            // if the discrepency is too large assume the pixel we need to 
            // fetch from the previous buffer is missing
            float linear_depth = projectedDepthToLinear( linearZTransform, depth );
            
            float diff = linear_depth - other_depth_avg;
            missing_pixel = diff * diff  >= tolerance * tolerance;

         #ifdef DEBUG_RENDER
            if ( render_missing_pixels && missing_pixel )
               {
                  g_output[ full_res_pixel ] = float3(1, 0, 0) * 10;
                  continue;
               }
         #endif
         }
         
         // If we've determined the we need is missing, then 
         // extrapolate the missing color by blending the 
         // current frame's up, down, left, right pixels
         if ( missing_pixel )
         {
            float3 color[4];
            float4 transp[4];

            readFromQuadrant( qtr_res_pixel + cardinal_offsets[Up], cardinal_quadrants[0], color[Up], transp[Up] );
            readFromQuadrant( qtr_res_pixel + cardinal_offsets[Down], cardinal_quadrants[0], color[Down], transp[Down]  );
            readFromQuadrant( qtr_res_pixel + cardinal_offsets[Left], cardinal_quadrants[1], color[Left], transp[Left]  );
            readFromQuadrant( qtr_res_pixel + cardinal_offsets[Right], cardinal_quadrants[1], color[Right], transp[Right]  );
         
            float2 w;
            w.x = colorDiffBlend( color[0].rgb, color[1].rgb );
            w.y = colorDiffBlend( color[2].rgb, color[3].rgb );
            float3 c = (0.5f*((color[0].xyz) + (color[1].xyz)) * w.x + 0.5f*((color[2].xyz) + (color[3].xyz)) * w.y) / (w.x + w.y + 0.001f);

            opaque = c;
            // TODO: better blend of the ctb samples
            ctb = (transp[0] + transp[1] + transp[2] + transp[3]) * .25;
         }
         else
         {
            //-1,1 quarter res pixel motion
            int2 d_motion = int2(f_motion * qtr_res / 2.0);
            int2 even_motion = d_motion * even_motion_scalar;
            int2 odd_motion = d_motion * odd_motion_scalar;
                  
            // otherwise we need to look up the quadrant from the previous frame
            // where was the sample and what quadrant does it fall in when motion vectors are applied...
            uint2 motion_to_quadrant = uint2(full_res_pixel + even_motion + odd_motion);
            uint quadrant_needed = (motion_to_quadrant.x & 0x1) + (motion_to_quadrant.y & 0x1) * 2;

         #ifdef DEBUG_RENDER
            if ( render_qtr_motion_pixels && (odd_motion.x || even_motion.x || odd_motion.y || even_motion.y) )
            {
               g_output[ full_res_pixel ] = float3(0, 1, 0) * 10;
               continue;
            }
            if ( render_alternate_pixels && quadrant_needed != quadrant )
            {
               g_output[ full_res_pixel ] = float3(0, 0, 1) * 10;
               continue;
            }
         #endif

            // if it falls on this frame's quadrant then remove the motion
            // because we're looking up the most recent data
            if ( frame_quadrants[0] == quadrant_needed || frame_quadrants[1] == quadrant_needed )
               odd_motion = even_motion = 0;

            readFromQuadrant( qtr_res_pixel + odd_motion + even_motion, quadrant_needed, opaque, ctb );
         }
      }
      g_output[ full_res_pixel ] = (ctb.rgb) + opaque.rgb * (1 - ctb.a);
   }
}
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 