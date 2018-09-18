
Texture2DMS<float3> g_opaque  : register( t0 );
Texture2DMS<unorm float2> g_normal : register( t1 );
Texture2DMS<unorm float> g_depth_even : register( t2 );
Texture2DMS<unorm float> g_depth_odd : register( t3 );
Texture2D<float> g_shadow_map : register( t4 );
Texture2DMS<float3> g_specular : register( t5 );
Texture2DMS<float4> g_properties : register( t6 );
Texture2DMS<uint> g_light_mask : register( t7 );
Texture2D<float2> g_ssao : register(t8);

RWTexture2D<float3> g_outputEven : register( u0 );
RWTexture2D<float3> g_outputOdd : register( u1 );

SamplerComparisonState g_shadow_sampler : register(s0);
SamplerState g_ssao_sampler : register(s1);

#define MaxLights 16
#define PI 3.14159265

cbuffer cb0 : register(b0)
{
   float4 g_params;
   float4 g_world_camera_view;
   float4 g_ambient_light;
   float4 g_light_dir[ MaxLights ];
   float4 g_light_pos_type[ MaxLights ];
	float4 g_light_color[ MaxLights ]; 
	float4 g_light_inner_outer_range__[ MaxLights ]; 
   float4x4 g_inv_view_proj;
   float4x4 g_shadow_view_matrix;
   float4x4 g_shadow_proj_matrix;
}	

struct LightDesc
{
    float3 color;
    
    float falloff;
    float n_dot_l;
    float n_dot_h;
    float v_dot_h;
    float l_dot_h;
};

float distribution_beckmann( float n_dot_h, float roughness )
{
    float a = roughness * roughness;
    float n_dot_h_sq = n_dot_h * n_dot_h;
    return exp((n_dot_h_sq - 1.0) / (a * n_dot_h_sq)) / (PI * a * n_dot_h_sq * n_dot_h_sq);
}

float fresnel_schlick(float f0, float c)
{
    return f0 + (1.0 - f0) * pow(1.0 - c, 5.0);
}

float geometric_shadowing_schlick_beckmann(float roughness, float c)
{
   float k = roughness * sqrt(2.0 / PI);
   
   return c / ( c * ( 1 - k ) + k );
}

float3 calculate_lighting(LightDesc light_desc, float n_dot_v, float roughness, float geometric_shadowing, float3 diff_color, float3 spec_color, float3 rough_metallic_emissive)
{
   float n_dot_l = saturate(light_desc.n_dot_l);
   float l_dot_h = saturate(light_desc.l_dot_h);
   float n_dot_h = saturate(light_desc.n_dot_h);

   float geometry_light = geometric_shadowing_schlick_beckmann( roughness, n_dot_l );
     
   float geometry = geometric_shadowing * geometry_light;
   float distribution = distribution_beckmann( n_dot_h, roughness );
   float fresnel = fresnel_schlick(rough_metallic_emissive.y, l_dot_h);

   float denominator = 4 * n_dot_l * n_dot_v;
   float brdf;
     
   if ( denominator == 0 )
      brdf = 0;
   else
      brdf = (fresnel * distribution * geometry) / denominator;
     
   float3 spec = max( 0, brdf * spec_color * n_dot_l ) * light_desc.color;
   float3 diff = (1.0 - fresnel_schlick(fresnel, n_dot_l)) * n_dot_l * light_desc.color * diff_color;
     
   return (spec + diff) * pow(1 - light_desc.falloff, 2);
}

float3 light_pixel(uint2 pixel_pos, uint light_mask, float3 world_position, float3 world_normal, float3 material_color, float3 spec_color, float3 rough_metallic_emissive)
{
   LightDesc light_desc;
   
   float3 lighting = float3(0, 0, 0);
   
   float n_dot_v = saturate( dot(world_normal, - g_world_camera_view.xyz) );
   
   for ( int i = 0; i < MaxLights; i++ )
   {
       if ((light_mask & (1 << i)) == 0)
          continue;
       
       float3 direction;
        
      if ( g_light_pos_type[i].w == 1 ) // directional light
      {
         direction = g_light_dir[i].xyz;
         light_desc.falloff = 0.0;
      }
      else if ( g_light_pos_type[i].w == 2 ) // spot light
      {
         direction = normalize(world_position.xyz - g_light_pos_type[i].xyz);

         float3 local_position = world_position.xyz - g_light_pos_type[i].xyz;
         float3 light_to_pixel = direction;

         float distance = length(local_position) + .0001;
         float intensity = 1.0 / (distance * distance);

         float inner_cos_angle = g_light_inner_outer_range__[i].x;
         float outer_cos_angle = g_light_inner_outer_range__[i].y;

         float pixel_cos_angle = dot(light_to_pixel, g_light_dir[i].xyz);
         
         float pixel_local = pixel_cos_angle - inner_cos_angle;
         float range_local = outer_cos_angle - inner_cos_angle;
         float angle_falloff = saturate(pixel_local / (range_local * range_local));
         float range_falloff = 1 - saturate(intensity);
         
         angle_falloff = 1.0 - pow(1.0 - angle_falloff, 8);
         light_desc.falloff = max(angle_falloff, range_falloff);
      }
      else
      {
         direction = normalize(world_position.xyz - g_light_pos_type[i].xyz);

         float distance = length(world_position.xyz - g_light_pos_type[i].xyz) + .0001;
         float intensity = 1.0 / (distance * distance);

         light_desc.falloff = 1 - saturate(intensity);
      }
        
      float3 h = - normalize(direction + g_world_camera_view.xyz);
            
      light_desc.n_dot_l = dot(world_normal, -direction);
      light_desc.n_dot_h = dot(world_normal, h);
      light_desc.v_dot_h = dot(-g_world_camera_view.xyz, h);
      light_desc.l_dot_h = dot(-direction, h);

      light_desc.color = g_light_color[i].rgb;
      
      float roughness = rough_metallic_emissive.x;
    
      float geometric_shadowing = geometric_shadowing_schlick_beckmann( roughness, n_dot_v );
      
      lighting += calculate_lighting( light_desc, n_dot_v, roughness, geometric_shadowing, material_color.rgb, spec_color, rough_metallic_emissive );
   }
   
   float3 lit_color = lighting + material_color.rgb * rough_metallic_emissive.z;
   float3 output = lit_color;
	
   // shadowing
   {
		int width, height;
		g_shadow_map.GetDimensions( width, height ); 
		
		const float texel_u = 1.0 / width;
		const float texel_v = 1.0 / height;
		const float bias = - 3.0;
		const float sample_count = 9 + 1;// + 1 because center sample is weighted  twice

      float4 shadow_view_pos = mul( float4(world_position, 1), g_shadow_view_matrix );
		float4 shadow_proj_pos = mul( shadow_view_pos, g_shadow_proj_matrix );
		
		shadow_proj_pos.xy /= shadow_proj_pos.w;
		shadow_proj_pos.xy = shadow_proj_pos.xy * float2(.5, -.5) + .5;
		
		float light_occlusion =       
			g_shadow_map.SampleCmpLevelZero( g_shadow_sampler, shadow_proj_pos.xy + float2(- texel_u, - texel_v), shadow_view_pos.z + bias ) +
			g_shadow_map.SampleCmpLevelZero( g_shadow_sampler, shadow_proj_pos.xy + float2(0,         - texel_v), shadow_view_pos.z + bias ) +
			g_shadow_map.SampleCmpLevelZero( g_shadow_sampler, shadow_proj_pos.xy + float2(texel_u,   - texel_v), shadow_view_pos.z + bias ) +

			g_shadow_map.SampleCmpLevelZero( g_shadow_sampler, shadow_proj_pos.xy + float2(- texel_u,   0), shadow_view_pos.z + bias ) +
			g_shadow_map.SampleCmpLevelZero( g_shadow_sampler, shadow_proj_pos.xy + float2(0,           0), shadow_view_pos.z + bias ) * 2 +
			g_shadow_map.SampleCmpLevelZero( g_shadow_sampler, shadow_proj_pos.xy + float2(texel_u,     0), shadow_view_pos.z + bias ) +

			g_shadow_map.SampleCmpLevelZero( g_shadow_sampler, shadow_proj_pos.xy + float2(- texel_u,   texel_v), shadow_view_pos.z + bias ) +
			g_shadow_map.SampleCmpLevelZero( g_shadow_sampler, shadow_proj_pos.xy + float2(0,           texel_v), shadow_view_pos.z + bias ) +
			g_shadow_map.SampleCmpLevelZero( g_shadow_sampler, shadow_proj_pos.xy + float2(texel_u,     texel_v), shadow_view_pos.z + bias );
		
      light_occlusion = light_occlusion / sample_count;
      light_occlusion = max( light_occlusion, .75 );
      
		output.rgb *= (light_occlusion * light_occlusion);     
	}

   // ssao
   float ssao;
   {
      uint width, height;
      g_ssao.GetDimensions( width, height );
      
      ssao = 1.0 - g_ssao.SampleLevel( g_ssao_sampler, (pixel_pos + .5f) / float2(width, height), 0 ).r;
   }
   
	output.rgb += material_color.rgb * g_ambient_light.rgb * ssao;
	
	return output;
}

float3 derive_world_position(float2 pixel, float depth, float width, float height)
{
   pixel.y = height - pixel.y - 1;
   
   float2 current_projected = float2(
      pixel.x / width * 2.0 - 1.0, 
      pixel.y / height * 2.0 - 1.0 );
      
   float4 curr_ws = float4( current_projected.x, current_projected.y, depth, 1.0f );

   curr_ws = mul( curr_ws, g_inv_view_proj );
   curr_ws /= curr_ws.w;
   
   return curr_ws.xyz;  
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

[numthreads(8, 8, 1)]
void cs_main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
   float3 normal[2];
   float3 opaque[2];
   float3 specular[2];
   float4 rough_metallic_emissive_lit[2];
   uint light_mask[2];
   float depth[2];
   
   int bit_fields = asuint(g_params.x);
   bool is_even_frame = bit_fields & 0x01;
   
   float width, samples, height;
   g_opaque.GetDimensions(width, height, samples);
   
   opaque[0] = g_opaque.Load( dispatchThreadId.xy, 0 );
   opaque[1] = g_opaque.Load( dispatchThreadId.xy, 1 );

   normal[0] = normal_decode( g_normal.Load( dispatchThreadId.xy, 0 ).xy );
   normal[1] = normal_decode( g_normal.Load( dispatchThreadId.xy, 1 ).xy );

   specular[0] = g_specular.Load( dispatchThreadId.xy, 0 );
   specular[1] = g_specular.Load( dispatchThreadId.xy, 1 );

   rough_metallic_emissive_lit[0] = g_properties.Load( dispatchThreadId.xy, 0 );
   rough_metallic_emissive_lit[1] = g_properties.Load( dispatchThreadId.xy, 1 );

   light_mask[0] = g_light_mask.Load( dispatchThreadId.xy, 0 );
   light_mask[1] = g_light_mask.Load( dispatchThreadId.xy, 1 );
   
   if ( is_even_frame )
   {
      depth[0] = g_depth_even.Load( dispatchThreadId.xy, 0 );
      depth[1] = g_depth_even.Load( dispatchThreadId.xy, 1 );
   }
   else
   {
      depth[0] = g_depth_odd.Load( dispatchThreadId.xy, 0 );
      depth[1] = g_depth_odd.Load( dispatchThreadId.xy, 1 );
   }
   
   float3 color[2];
   
   for ( int i = 0; i < 2; i++ )
   {
      float3 world_position;
      
      if ( rough_metallic_emissive_lit[i].a == 1 )
      {
         world_position = derive_world_position(dispatchThreadId.xy * 2 + int2(1 - i, 1 - i), depth[i], width * 2, height * 2);
         color[i] = light_pixel(dispatchThreadId.xy, light_mask[i], world_position, normal[i], opaque[i], specular[i], rough_metallic_emissive_lit[i].xyz);
      }
      else
         color[i] = opaque[i];
   }
   
   if ( is_even_frame )
   {
      g_outputEven[ dispatchThreadId.xy * int2(2,1) + int2(0,0) ] = color[0];
      g_outputEven[ dispatchThreadId.xy * int2(2,1) + int2(1,0) ] = color[1];
   }
   else
   {
      g_outputOdd[ dispatchThreadId.xy * int2(2,1) + int2(0,0) ] = color[0];
      g_outputOdd[ dispatchThreadId.xy * int2(2,1) + int2(1,0) ] = color[1];
   }
}
















