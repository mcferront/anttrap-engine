#define MAX_MIPS     12  //Match HiZMipLevels in RenderNodes.h

Texture2D<float> g_depth : register(t0);
Texture2D<float4> g_mat_properties : register(t1);
Texture2D<unorm float2> g_normals : register(t2);
RWTexture2D<uint2> g_output : register(u0);

cbuffer cb0 : register(b0)
{
   float4 g_params;
   float4x4 g_inv_proj;
   float4x4 g_proj;
   float4x4 g_view;
};

#define THREADS_X 8
#define THREADS_Y 8

#define MISSED_PIXEL       uint2(65000,65000)
#define DOES_NOT_REFLECT   uint2(60000,60000)
#define REFL_DAMPER   4.0

float3 to_vs(float2 pixel, float depth, float2 resolution)
{
   // Projection is flipped from UV coords
   pixel.y = resolution.y - pixel.y - 1;
   
   float2 current_projected = pixel / resolution * 2.0 - 1;
   
   float4 re_projected_pre_w_divide = float4(current_projected.x, current_projected.y, depth, 1.0 );
   float4 curr_vs = mul( re_projected_pre_w_divide, g_inv_proj );
   curr_vs /= curr_vs.w;
   
   return curr_vs.xyz;
}

uint2 ray_trace( float3 start, float3 end, float distance_3d, int iterations, float offset_bias, float2 resolution )
{
   const float2 half_res = resolution / 2.0;

   float4 proj_start = mul( float4(start, 1), g_proj );
   proj_start /= proj_start.w;
   proj_start.y = - proj_start.y;
   
   float4 proj_end = mul( float4(end, 1), g_proj );
   proj_end /= proj_end.w;
   proj_end.y = - proj_end.y;

   float3 proj_dir = normalize(proj_end.xyz - proj_start.xyz);
   
   float step = distance_3d / iterations;
   
   int2 pixel, depth_pixel;
   float depth;

   float3 position = proj_start.xyz + 1 * step * proj_dir;
   pixel = mad( position.xy, half_res, half_res );

    for ( int i = 1; i <= iterations; )
   {
      depth = position.z;
      depth_pixel = pixel;

      // continue looping through this integer pixel
      // and find the smallest depth value
      while ( i++ <= iterations )
      {
         position = proj_start.xyz + i * step * proj_dir;
         pixel = mad( position.xy, half_res, half_res );
         
         if ( pixel.x != depth_pixel.x || pixel.y != depth_pixel.y )
            break;
         else
            depth = min( depth, position.z );
      }
      
      // drill down mip levels until the test fails
      int mip;
      
      for ( mip = MAX_MIPS - 1; mip >= 0; mip-- )
      {
         float depth_test = g_depth.Load( uint3(depth_pixel >> mip, mip) );
         
         // if it's not covered by this mip, stop searching
         if ( depth + offset_bias > depth_test )
            break;
      }

      // if we drilled down through all of the mips then we have a match
      if ( mip < 0 )
         return depth_pixel;
   }
   
   return MISSED_PIXEL;
}

float3 normal_decode(float2 enc)
{
    float2 fenc = enc * 4 - 2;
    float f = dot( fenc, fenc );
    float g = sqrt( 1 - f / 4 );
    float3 n;
    n.xy = fenc * g;
    n.z = 1 - f / 2;

    return n;
}

[numthreads(THREADS_X, THREADS_Y, 1)]
void cs_ssr(uint3 dispatchThreadId : SV_DispatchThreadID)
{  
   
   const float ray_dist = 1.0;
   const int iterations = g_params.x;
   const float offset_bias = g_params.y;
   const uint flags = asuint(g_params.z);
   const int2 d_pixel = dispatchThreadId.xy;

   int2 resolution;
   g_output.GetDimensions( resolution.x, resolution.y );
   
   uint mat_width, mat_height, count;
   g_mat_properties.GetDimensions( mat_width, mat_height );
   
   const float scale = mat_width / float(resolution.x);
   
   float4 props = g_mat_properties[ d_pixel * scale ];   

   float refl = saturate(1.0 - props.x * REFL_DAMPER);

   uint2 hdr_pixel = DOES_NOT_REFLECT;

   if ( refl > 0.001 )
   {
      float3 view_space;
      {
         const float depth = g_depth[ d_pixel ];

         // view space position
         view_space = to_vs( d_pixel, depth, resolution );
      }
      
      float3 world_normal, view_normal;
      {
         // view space normal
         world_normal = normal_decode( g_normals[ d_pixel * scale ].xy );
         
         view_normal = mul( float4(world_normal, 0), g_view ).xyz;
      }
      
      // reflection vector
      float3 ray_dir, reflection;
      {
         float3 pixel = float3(d_pixel, 0);
         pixel.y = resolution.y - pixel.y - 1;
         pixel.xy = (pixel.xy + .5) / resolution * 2.0 - 1.0;
         pixel.z = 1.0; // eye pos

         ray_dir = normalize( pixel );
         reflection = reflect( ray_dir, view_normal );
      }

      float3 start = view_space;
      float3 end = view_space + reflection * ray_dist;
      
      // trace a ray
      hdr_pixel = ray_trace( start, end, ray_dist, iterations, offset_bias, resolution );
   }
   
   g_output[ d_pixel ] = hdr_pixel;
}







































