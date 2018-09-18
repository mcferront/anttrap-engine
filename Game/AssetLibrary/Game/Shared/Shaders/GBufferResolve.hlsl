
Texture2D<float3> g_opaque  : register( t0 );
Texture2D<unorm float2> g_normal : register( t1 );
Texture2D<unorm float> g_depth : register( t2 );
Texture2D<float> g_shadow_map : register( t3 );
Texture2D<float3> g_specular : register( t4 );
Texture2D<float4> g_properties : register( t5 );
Texture2D<uint> g_light_mask : register( t6 );
Texture2D<float2> g_ssao : register(t7);
RWTexture2D<float3> g_output : register( u0 );

SamplerComparisonState g_shadow_sampler : register(s0);
SamplerState g_ssao_sampler : register(s1);


#define MaxLights 16
#define PI 3.14159265

cbuffer cb0 : register(b0)
{
   float4 g_world_camera_view;   //0
   float4 g_ambient_light;       //16
   float4 g_light_dir[ MaxLights ];    //48
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
   float3 normal;
   float3 opaque;
   float3 specular;
   float4 properties;
   uint light_mask;
   
   float depth, width, height;
   g_opaque.GetDimensions(width, height);
   
   opaque = g_opaque[ dispatchThreadId.xy ];
   normal = normal_decode( g_normal[ dispatchThreadId.xy ].xy );
   depth = g_depth[ dispatchThreadId.xy ];
   specular = g_specular[ dispatchThreadId.xy ];
   properties = g_properties[ dispatchThreadId.xy ];
   light_mask = g_light_mask[ dispatchThreadId.xy ];
   
   float4 rough_metallic_emissive_lit = properties;
   
   float3 color, world_position;
   
   if ( rough_metallic_emissive_lit.a == 1 )
   {
      world_position = derive_world_position(dispatchThreadId.xy, depth, width, height);
      color = light_pixel(dispatchThreadId.xy, light_mask, world_position, normal, opaque, specular, rough_metallic_emissive_lit.xyz);
   }
   else
      color = opaque;
                             
   g_output[ dispatchThreadId.xy ] = color;
}
















