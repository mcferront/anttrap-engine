#define MaxLights 16
#define PI 3.14159265

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

float3 calculate_lighting(LightDesc light_desc, float n_dot_v, float roughness, float geometric_shadowing, float metallic, float3 diff_color, float3 spec_color)
{
   float n_dot_l = saturate(light_desc.n_dot_l);
   float l_dot_h = saturate(light_desc.l_dot_h);
   float n_dot_h = saturate(light_desc.n_dot_h);

   float geometry_light = geometric_shadowing_schlick_beckmann( roughness, n_dot_l );
     
   float geometry = geometric_shadowing * geometry_light;
   float distribution = distribution_beckmann( n_dot_h, roughness );
   float fresnel = fresnel_schlick(metallic, l_dot_h);

   float denominator = 4 * n_dot_l * n_dot_v;
   float brdf;
     
   if ( denominator == 0 )
      brdf = 0;
   else
      brdf = (fresnel * distribution * geometry) / denominator;
     
   float3 spec = max( 0, brdf * spec_color * n_dot_l ) * light_desc.color;
   float3 diff = (1.0 - fresnel_schlick(fresnel, n_dot_l)) * n_dot_l * light_desc.color * diff_color;
     
   return (spec + diff) * (1 - light_desc.falloff);
}

float4 light_pixel(uint2 pixel_pos, uint light_mask, float3 world_position, float3 world_normal, 
                     float4 material_color, float3 spec_color, float3 rough_metallic_emissive, float3 world_camera_view)
{
   LightDesc light_desc;
   
   float3 lighting = float3(0, 0, 0);
   
   float n_dot_v = saturate( dot(world_normal, - world_camera_view) );

#ifdef NDF
   float3x3 model_to_tang = transpose(float3x3( input.tangent, input.binormal, input.normal ));
#endif
   
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
         float angle_falloff = saturate(pixel_local / range_local);
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
        
      float3 h = - normalize(direction + world_camera_view);
            
      light_desc.n_dot_l = dot(world_normal, -direction);
      light_desc.n_dot_h = dot(world_normal, h);
      light_desc.v_dot_h = dot(-world_camera_view, h);
      light_desc.l_dot_h = dot(-direction, h);

      light_desc.color = g_light_color[i].rgb;
      
      float roughness = rough_metallic_emissive.x;
      
   #ifdef NDF
      {
         // Compute plane-plane half vector (hpp)
         float3 hppWS = h / dot(h, world_normal);
         float2 hpp = mul( mul(hppWS, transpose(g_world)), model_to_tang ).xy;
         
         // Use ddx/ddy, thanks to quad shading!
         float2x2 dhduv = float2x2(ddx_fine(hpp) , ddy_fine(hpp) );

         // Compute filtering rectangular region
         float2 rectFp = min((abs(dhduv[0]) + abs(dhduv[1])) * 0.5f, 0.7f);

         // Covariance matrix of pixel filter's Gaussian (remapped in roughness units)
         float2 covMx = rectFp * rectFp * 2.f;
         roughness = sqrt(roughness * roughness + covMx); // Beckmann proxy convolution (for GGX)
      }
   #endif
      
      float geometric_shadowing = geometric_shadowing_schlick_beckmann( roughness, n_dot_v );
      
      lighting += calculate_lighting( light_desc, n_dot_v, roughness, geometric_shadowing, rough_metallic_emissive.y, material_color.rgb, spec_color );
   }
   
   float3 lit_color = lighting + material_color.rgb * rough_metallic_emissive.z;
   float4 output = float4(lit_color, material_color.a);
	
   // shadowing
   {
		float4 shadow_view_pos = mul( float4(world_position, 1), g_shadow_view_matrix );
		float4 shadow_proj_pos = mul( shadow_view_pos, g_shadow_proj_matrix );
		
		shadow_proj_pos.xy /= shadow_proj_pos.w;
		shadow_proj_pos.xy = shadow_proj_pos.xy * float2(.5, -.5) + .5;
		
		// todo: PCF should be hardcoded
		int width, height;
		g_shadow_map.GetDimensions( width, height ); 
		
		float texel_u = 1.0 / width;
		float texel_v = 1.0 / height;
		float bias = - 3.0;
		
      const float sample_count = 9 + 1;// + 1 because center sample is weighted  twice

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
