Texture2D<float4> g_input : register(t0);
RWTexture2D<float4> g_output : register(u0);

SamplerState g_input_sampler : register(s0);

#define THREADS_X 8
#define THREADS_Y 4

cbuffer cb0 : register(b0)
{
   float4 g_params;
};


float4 blur(float2 pixel, uint2 resolution, float kernel_size) 
{
   const int max_bokeh_samples = 21;
   static const float2 kernel[max_bokeh_samples] = {
      float2(0.500000, 0.000000),
      float2(0.311745, 0.390916),
      float2(-0.111261, 0.487464),
      float2(-0.450484, 0.216942),
      float2(-0.450484, -0.216942),
      float2(-0.111260, -0.487464),
      float2(0.311745, -0.390916),
      float2(1.000000, 0.000000),
      float2(0.900969, 0.433884),
      float2(0.623490, 0.781832),
      float2(0.222521, 0.974928),
      float2(-0.222521, 0.974928),
      float2(-0.623490, 0.781831),
      float2(-0.900969, 0.433884),
      float2(-1.000000, -0.000000),
      float2(-0.900969, -0.433884),
      float2(-0.623490, -0.781832),
      float2(-0.222521, -0.974928),
      float2(0.222521, -0.974928),
      float2(0.623490, -0.781831),
      float2(0.900969, -0.433884),
   };

   const int total_colors = max_bokeh_samples + 1;

   float dist = kernel_size;
   
   float2 texel_size = 1.0f / resolution;
   float2 uv = pixel / resolution;
   
   
   float4 color[ total_colors ];
   
   int i;
 
   color[ 0 ] = g_input.SampleLevel( g_input_sampler, uv, 0 );

   int start = 1;
   
   for ( i = 0; i < max_bokeh_samples; i++ )
   {
      float2 sample_uv = uv + (texel_size * kernel[i] * dist);
      color[ start++ ] = g_input.SampleLevel( g_input_sampler, sample_uv, 0 );
   }

 
   // only use pixels which should have some blur (alpha > 0)
   // this prevents smeering of focal pixels in the foreground blur
   float4 blur_color = color[ 0 ];

   int valid_count = 1;
   
   for ( i = 1; i < total_colors; i++ )
   {
      if ( color[ i ].a > 0 )
      {
         blur_color += color[i];
         valid_count++;
      }
   }

   blur_color *= (1.0f / valid_count);

   return blur_color;
}

float4 blur_gauss(float2 pixel, uint2 resolution, float2 direction, float kernel)
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

   float2 off1 = float2(1.411764705882353, 1.411764705882353) * direction * dist;
   float2 off2 = float2(3.2941176470588234, 3.2941176470588234) * direction * dist;
   float2 off3 = float2(5.176470588235294, 5.176470588235294) * direction * dist;

   
   color[ 1 ] = g_input.SampleLevel( g_input_sampler, (pixel + off1) / resolution, 0 );
   color[ 2 ] = g_input.SampleLevel( g_input_sampler, (pixel - off1) / resolution, 0 );
   color[ 3 ] = g_input.SampleLevel( g_input_sampler, (pixel + off2) / resolution, 0 );
   color[ 4 ] = g_input.SampleLevel( g_input_sampler, (pixel - off2) / resolution, 0 );
   color[ 5 ] = g_input.SampleLevel( g_input_sampler, (pixel + off3) / resolution, 0 );
   color[ 6 ] = g_input.SampleLevel( g_input_sampler, (pixel - off3) / resolution, 0 );

   // replace 0 alpha colors with color at sample 0 (prevents bringing in unwanted focal colors)
   float3 avg_color = 0;

   int i;
   
   for ( i = 0; i < total_colors; i++ )
      avg_color += color[ i ].a > 0 ? color[ i ].rgb : color[ 0 ].rgb;

   avg_color = avg_color / total_colors;

   // gauss blur colors, but anytime there is a 0 alpha instead use the avg_color
   float4 blur_color = 0;
   
   for ( i = 0; i < total_colors; i++ )
      blur_color += (color[ i ].a > 0 ? color[ i ] : float4(avg_color, color[i].a)) * weights[ i ];

   return blur_color;
}

[numthreads(THREADS_X, THREADS_Y, 1)]
void cs_blur(uint3 group_thread_id : SV_GroupThreadID, uint3 dispatch_thread_id : SV_DispatchThreadID)
{  
   const int type = g_params.x;
   
   const float near_kernel = g_params.y;
   
   uint2 resolution;
   g_output.GetDimensions( resolution.x, resolution.y );
   
   const float2 pixel = dispatch_thread_id.xy + .5;
   
   const float2 blur_directions[3] = 
   {
     float2(0, 0),
     float2(1, 0),
     float2(0, 1),
   };

   if ( type == 0 )
      g_output[ pixel ] = blur(pixel, resolution, near_kernel);
   else
      g_output[ pixel ] = blur_gauss(pixel, resolution, blur_directions[type], 1);
}




























