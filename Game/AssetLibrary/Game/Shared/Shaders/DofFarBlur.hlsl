Texture2D<float4> g_input : register(t0);
Texture2D<float> g_coc : register(t1);
RWTexture2D<float4> g_output : register(u0);

SamplerState g_input_sampler : register(s0);

#define THREADS_X 8
#define THREADS_Y 4

cbuffer cb0 : register(b0)
{
   float4 g_params;
};

float4 blur(float coc, float2 pixel, uint2 resolution, float kernel_size) 
{
   static const int max_bokeh_samples = 69;
   static const float2 kernel[max_bokeh_samples] = {
      float2(0.250000, 0.000000),
      float2(0.176777, 0.176777),
      float2(-0.000000, 0.250000),
      float2(-0.176777, 0.176777),
      float2(-0.250000, -0.000000),
      float2(-0.176777, -0.176777),
      float2(0.000000, -0.250000),
      float2(0.176777, -0.176777),
      float2(0.500000, 0.000000),
      float2(0.450484, 0.216942),
      float2(0.311745, 0.390916),
      float2(0.111260, 0.487464),
      float2(-0.111261, 0.487464),
      float2(-0.311745, 0.390916),
      float2(-0.450484, 0.216942),
      float2(-0.500000, -0.000000),
      float2(-0.450484, -0.216942),
      float2(-0.311745, -0.390916),
      float2(-0.111260, -0.487464),
      float2(0.111261, -0.487464),
      float2(0.311745, -0.390916),
      float2(0.450484, -0.216942),
      float2(0.750000, 0.000000),
      float2(0.713292, 0.231763),
      float2(0.606763, 0.440839),
      float2(0.440839, 0.606763),
      float2(0.231763, 0.713292),
      float2(-0.000000, 0.750000),
      float2(-0.231763, 0.713292),
      float2(-0.440839, 0.606763),
      float2(-0.606763, 0.440839),
      float2(-0.713292, 0.231763),
      float2(-0.750000, -0.000000),
      float2(-0.713292, -0.231763),
      float2(-0.606763, -0.440839),
      float2(-0.440839, -0.606763),
      float2(-0.231763, -0.713292),
      float2(0.000000, -0.750000),
      float2(0.231763, -0.713292),
      float2(0.440839, -0.606763),
      float2(0.606763, -0.440839),
      float2(0.713292, -0.231763),
      float2(1.000000, 0.000000),
      float2(0.973045, 0.230616),
      float2(0.893633, 0.448799),
      float2(0.766044, 0.642788),
      float2(0.597159, 0.802123),
      float2(0.396080, 0.918216),
      float2(0.173648, 0.984808),
      float2(-0.058145, 0.998308),
      float2(-0.286803, 0.957990),
      float2(-0.500000, 0.866025),
      float2(-0.686242, 0.727374),
      float2(-0.835488, 0.549509),
      float2(-0.939693, 0.342020),
      float2(-0.993238, 0.116093),
      float2(-0.993238, -0.116093),
      float2(-0.939693, -0.342020),
      float2(-0.835488, -0.549509),
      float2(-0.686242, -0.727374),
      float2(-0.500000, -0.866025),
      float2(-0.286803, -0.957990),
      float2(-0.058145, -0.998308),
      float2(0.173649, -0.984808),
      float2(0.396080, -0.918216),
      float2(0.597159, -0.802123),
      float2(0.766045, -0.642787),
      float2(0.893633, -0.448799),
      float2(0.973045, -0.230615),
   };
      
   const int total_colors = max_bokeh_samples + 1;

   float dist = kernel_size;
   
   float2 texel_size = 1.0f / resolution;
   float2 uv = pixel / resolution;
   
   
   float4 color[ total_colors ];
   
   int i;
 
   color[ 0 ] = g_input.SampleLevel( g_input_sampler, uv, 0 );
   
   if ( coc == 0 )
      return color[ 0 ];

   int start = 1;
   
   for ( i = 0; i < max_bokeh_samples; i++ )
   {
      float2 sample_uv = uv + (texel_size * kernel[i] * dist);
      color[ start++ ] = g_input.SampleLevel( g_input_sampler, sample_uv, 0 );
   }

   // only use pixels which are a definite blur (alpha == 1)
   // this prevents haloing when bilinear samples take in a blur and non blur pixel
   float4 blur_color = color[ 0 ];

    float valid_count = 2;

    // a little extra wait for our center color
    // helps prevents holes in the middle
    blur_color *= valid_count;
   
   for ( i = 1; i < total_colors; i++ )
   {
      blur_color += (color[i] * color[i].a);
      valid_count += color[i].a;
   }

   blur_color /= valid_count;

   // this pixel has blur
    // mark it as such (with the alpha) so the gauss blur passes
    // will make sure to incorporate it
   blur_color.a = 1;
  
   return blur_color;
}

float4 blur_gauss(float coc, float2 pixel, uint2 resolution, float2 direction, float kernel)
{
   const int total_colors = 7;
   
   float4 color[ total_colors ];
   const float weights[ total_colors ] =
   {
      0.1964825501511404,
      0.2969069646728344,
      0.2969069646728344,
      0.09447039785044732,
      0.09447039785044732,
      0.010381362401148057,
      0.010381362401148057,
   };
   
   float dist = kernel;
   
   color[ 0 ] = g_input.SampleLevel( g_input_sampler, pixel / resolution, 0 );
   
   if ( color[ 0 ].a != 1 )
      return color[ 0 ];

    dist *= coc;
   float2 off1 = float2(1.411764705882353, 1.411764705882353) * direction * dist;
   float2 off2 = float2(3.2941176470588234, 3.2941176470588234) * direction * dist;
   float2 off3 = float2(5.176470588235294, 5.176470588235294) * direction * dist;

   
   color[ 1 ] = g_input.SampleLevel( g_input_sampler, (pixel + off1) / resolution, 0 );
   color[ 2 ] = g_input.SampleLevel( g_input_sampler, (pixel - off1) / resolution, 0 );
   color[ 3 ] = g_input.SampleLevel( g_input_sampler, (pixel + off2) / resolution, 0 );
   color[ 4 ] = g_input.SampleLevel( g_input_sampler, (pixel - off2) / resolution, 0 );
   color[ 5 ] = g_input.SampleLevel( g_input_sampler, (pixel + off3) / resolution, 0 );
   color[ 6 ] = g_input.SampleLevel( g_input_sampler, (pixel - off3) / resolution, 0 );

   // replace colors with alpha != 1 with color at sample 0 
   // (prevents haloing of unwanted focal colors)
   float3 avg_color = 0;

   int i;
   
   for ( i = 0; i < total_colors; i++ )
      avg_color += color[ i ].a == 1 ? color[ i ].rgb : color[ 0 ].rgb;

   avg_color = avg_color / total_colors;

   float4 blur_color = 0;
   
   for ( i = 0; i < total_colors; i++ )
      blur_color += (color[ i ].a == 1 ? color[ i ] : float4(avg_color, color[i].a)) * weights[ i ];

   // this pixel has blur
    // mark it as such (with the alpha) so the gauss blur passes
    // will make sure to incorporate it
   blur_color.a = 1;

   return blur_color;
}

[numthreads(THREADS_X, THREADS_Y, 1)]
void cs_blur(uint3 group_thread_id : SV_GroupThreadID, uint3 dispatch_thread_id : SV_DispatchThreadID)
{  
   const int type = g_params.x;
   
   const float far_kernel = g_params.z;
   
   uint2 resolution;
   g_output.GetDimensions( resolution.x, resolution.y );

   const float2 pixel = dispatch_thread_id.xy + .5;
   
   float2 dof_pixel = pixel + float2( resolution.x * .5, 0);
   
   float2 coc_uv = pixel / (resolution * float2(.5, 1));
   
   const float coc = g_coc.SampleLevel( g_input_sampler, coc_uv, 0 );
   
   const float2 blur_directions[] = 
   {
      float2(1, 0),
      float2(0, 1),
   };

   // bokeh blur
   if ( type == 0 )
      g_output[ dof_pixel ] = blur(coc, dof_pixel, resolution, far_kernel);
   else //gaussian out bokeh
      g_output[ dof_pixel ] = blur_gauss(coc, dof_pixel, resolution, blur_directions[type - 1], 1);
}

   


























