//#define NDF

//  http://renderwonk.com/publications/s2010-shading-course/gotanda/course_note_practical_implementation_at_triace.pdf
//  http://simonstechblog.blogspot.com/2011/12/microfacet-brdf.html
//  http://www.cs.virginia.edu/~jdl/bib/appearance/analytic%20models/schlick94b.pdf
//  http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html

struct VS_INPUT
{                                 
   float4 position   : POSITION;     
   float3 normal     : NORMAL;
   float3 binormal   : BINORMAL;
   float3 tangent    : TANGENT;
   float4 uv         : TEXCOORD0;
   uint4 color       : COLOR0;
};                                

struct VS_INPUT_SKIN
{
   float4 position : POSITION;
   float3 normal   : NORMAL;
   float3 binormal : BINORMAL;
   float3 tangent  : TANGENT;
   float4 uv       : TEXCOORD0;
   uint4 color     : COLOR0;
   float4 boneIndices : BLENDINDICES;
   float4 boneWeights : BLENDWEIGHT;
};

struct VS_OUTPUT                  
{                                 
   float4 position : SV_POSITION;     
   float3 normal   : NORMAL;
   float3 binormal : BINORMAL;
   float3 tangent  : TANGENT;
   float4 uv       : TEXCOORD0;    
   float4 color    : COLOR0;
   float3 world_camera_view : TEXCOORD1;
   float4 world_position : TEXCOORD2;
};                                

#if SHADE_PER_SAMPLE == 1
   struct PS_INPUT
   {                    
      sample float4 position : SV_POSITION;
      sample float3 normal   : NORMAL;
      sample float3 binormal : BINORMAL;
      sample float3 tangent  : TANGENT;
      sample float4 uv       : TEXCOORD0;
      sample float4 color    : COLOR0;     
      sample float3 world_camera_view : TEXCOORD1;
      sample float4 world_position : TEXCOORD2;
   };
#else
   struct PS_INPUT
   {                    
      float4 position : SV_POSITION;
      float3 normal   : NORMAL;
      float3 binormal : BINORMAL;
      float3 tangent  : TANGENT;
      float4 uv       : TEXCOORD0;
      float4 color    : COLOR0;     
      float3 world_camera_view : TEXCOORD1;
      float4 world_position : TEXCOORD2;
   };
  
#endif


#define MaxLights 16
#define PI 3.14159265

cbuffer cb0 : register(b0)
{
	float4 g_material_color;   //0
   float4 g_specular_color;   //16
   float4 g_rough_metallic_emissive__; //32
   float4 g_ambient_light; //48
   float4 g_light_dir[ MaxLights ];    //80
   float4 g_light_pos_type[ MaxLights ];
	float4 g_light_color[ MaxLights ]; 
	float4 g_light_inner_outer_range__[ MaxLights ]; 
   float4 g_resscale_lightmask;
	float4x4 g_shadow_proj_matrix;
	float4x4 g_shadow_view_matrix; 
   float4x4 g_vp;
   float4x4 g_pwvp;
	float4x4 g_world;
	float4x4 g_camera_world;
	float4x4 g_skin[ 32 ];
}	

float4x4 compute_bone_matrices( int4 indices, float4 weights, float4x4 skin[32] )
{
    return 
        mul( skin[ indices[ 0 ] ], weights.x ) +
        mul( skin[ indices[ 1 ] ], weights.y ) +
        mul( skin[ indices[ 2 ] ], weights.z ) +
        mul( skin[ indices[ 3 ] ], weights.w );		
}

VS_OUTPUT vs_main(VS_INPUT input)    
{                                                                   
   float4 world_position = mul( input.position, g_world );   
   float4 proj_position = mul( world_position, g_vp ); 
	
   VS_OUTPUT output;
   output.position = proj_position;
   output.uv       = input.uv;
   output.color    = input.color / 255.0;
   output.normal   = input.normal;
   output.binormal = input.binormal;
   output.tangent  = input.tangent;
   output.world_position = world_position;
   output.world_camera_view = g_camera_world[2].xyz;

   return output;
}

VS_OUTPUT vs_main_skin( VS_INPUT_SKIN input )
{
	float4x4 boneMatrix = compute_bone_matrices( int4( input.boneIndices ), input.boneWeights, g_skin );
	
	float4 skin_position = mul( input.position, boneMatrix );
	float4 world_position = mul( skin_position, g_world );
	float4 proj_position = mul( world_position, g_vp );
	
	float4 prev_proj_position = mul( skin_position, g_pwvp );

	float3 model_normal = mul( float4(input.normal, 0), boneMatrix ).xyz;
    
	VS_OUTPUT output;
   output.position = proj_position;
	output.uv = input.uv;
   output.color = input.color / 255.0;
   output.normal = model_normal;
   output.binormal = input.binormal;
   output.tangent  = input.tangent;
   output.world_position = world_position;
   output.world_camera_view = g_camera_world[2].xyz;
 
   return output;
};  


Texture2D g_diff : register(t0);
Texture2D g_spec : register(t1);
Texture2D g_norm : register(t2);
Texture2D g_shadowMap : register(t3);
Texture2D g_ssao : register(t4);
Texture2D g_ssr : register(t5);
Texture2D g_texture2 : register(t6); //for island's other material

SamplerState g_diff_sampler : register(s0);
SamplerState g_spec_sampler : register(s1);
SamplerState g_norm_sampler : register(s2);
SamplerComparisonState g_shadowSampler : register(s3);
SamplerState g_ssao_sampler : register(s4);
SamplerState g_ssr_sampler : register(s5);
SamplerState g_sampler3 : register(s6);	//for island's other material

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

float3 calculate_lighting(LightDesc light_desc, float n_dot_v, float roughness, float geometric_shadowing, float3 diff_color, float3 spec_color)
{
   float n_dot_l = saturate(light_desc.n_dot_l);
   float l_dot_h = saturate(light_desc.l_dot_h);
   float n_dot_h = saturate(light_desc.n_dot_h);

   float geometry_light = geometric_shadowing_schlick_beckmann( roughness, n_dot_l );
     
   float geometry = geometric_shadowing * geometry_light;
   float distribution = distribution_beckmann( n_dot_h, roughness );
   float fresnel = fresnel_schlick(g_rough_metallic_emissive__.y, l_dot_h);

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

float4 light_pixel(PS_INPUT input, float3 world_normal, float4 material_color, float3 spec_color)
{
   LightDesc light_desc;
   
   float3 lighting = float3(0, 0, 0);
   
   float n_dot_v = saturate( dot(world_normal, - input.world_camera_view) );

#ifdef NDF
   float3x3 model_to_tang = transpose(float3x3( input.tangent, input.binormal, input.normal ));
#endif
   
   uint light_mask = asuint(g_resscale_lightmask.z);

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
         direction = normalize(input.world_position.xyz - g_light_pos_type[i].xyz);

         float3 local_position = input.world_position.xyz - g_light_pos_type[i].xyz;
         float3 light_to_pixel = direction;

         float distance = length(local_position) + .0001;
         float intensity = 1.0 / (distance * distance);

         float inner_cos_angle = g_light_inner_outer_range__[i].x;
         float outer_cos_angle = g_light_inner_outer_range__[i].y;

         float pixel_cos_angle = dot(light_to_pixel, g_light_dir[i].xyz);
         
         float pixel_local = pixel_cos_angle - inner_cos_angle;
         float range_local = outer_cos_angle - inner_cos_angle;
         float angle_falloff = saturate(pixel_local / (range_local));
         float range_falloff = 1 - saturate(intensity);
         
         angle_falloff = 1.0 - pow(1.0 - angle_falloff, 8);
         light_desc.falloff = max(angle_falloff, range_falloff);
      }
      else
      {
         direction = normalize(input.world_position.xyz - g_light_pos_type[i].xyz);

         float distance = length(input.world_position.xyz - g_light_pos_type[i].xyz) + .0001;
         float intensity = 1.0 / (distance * distance);
         light_desc.falloff = 1 - saturate(intensity);
      }
        
      float3 h = - normalize(direction + input.world_camera_view);
            
      light_desc.n_dot_l = dot(world_normal, -direction);
      light_desc.n_dot_h = dot(world_normal, h);
      light_desc.v_dot_h = dot(-input.world_camera_view, h);
      light_desc.l_dot_h = dot(-direction, h);

      light_desc.color = g_light_color[i].rgb;
      
      float roughness = g_rough_metallic_emissive__.x;
      
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
      
      lighting += calculate_lighting( light_desc, n_dot_v, roughness, geometric_shadowing, material_color.rgb, spec_color );
   }
   
   float3 lit_color = lighting + material_color.rgb * g_rough_metallic_emissive__.z;
   float4 output = float4(lit_color, material_color.a);
	
   // shadowing
   {
		float4 shadow_view_pos = mul( input.world_position, g_shadow_view_matrix );
		float4 shadow_proj_pos = mul( shadow_view_pos, g_shadow_proj_matrix );
		
		shadow_proj_pos.xy /= shadow_proj_pos.w;
		shadow_proj_pos.xy = shadow_proj_pos.xy * float2(.5, -.5) + .5;
		
		// todo: PCF should be hardcoded
		int width, height;
		g_shadowMap.GetDimensions( width, height ); 
		
		float texel_u = 1.0 / width;
		float texel_v = 1.0 / height;
		float bias = - 3.0;
		
      const float sample_count = 9 + 1;// + 1 because center sample is weighted  twice

		float light_occlusion = 
			g_shadowMap.SampleCmpLevelZero( g_shadowSampler, shadow_proj_pos.xy + float2(- texel_u, - texel_v), shadow_view_pos.z + bias ) +
			g_shadowMap.SampleCmpLevelZero( g_shadowSampler, shadow_proj_pos.xy + float2(0,         - texel_v), shadow_view_pos.z + bias ) +
			g_shadowMap.SampleCmpLevelZero( g_shadowSampler, shadow_proj_pos.xy + float2(texel_u,   - texel_v), shadow_view_pos.z + bias ) +

			g_shadowMap.SampleCmpLevelZero( g_shadowSampler, shadow_proj_pos.xy + float2(- texel_u,   0), shadow_view_pos.z + bias ) +
			g_shadowMap.SampleCmpLevelZero( g_shadowSampler, shadow_proj_pos.xy + float2(0,           0), shadow_view_pos.z + bias ) * 2 +
			g_shadowMap.SampleCmpLevelZero( g_shadowSampler, shadow_proj_pos.xy + float2(texel_u,     0), shadow_view_pos.z + bias ) +

			g_shadowMap.SampleCmpLevelZero( g_shadowSampler, shadow_proj_pos.xy + float2(- texel_u,   texel_v), shadow_view_pos.z + bias ) +
			g_shadowMap.SampleCmpLevelZero( g_shadowSampler, shadow_proj_pos.xy + float2(0,           texel_v), shadow_view_pos.z + bias ) +
			g_shadowMap.SampleCmpLevelZero( g_shadowSampler, shadow_proj_pos.xy + float2(texel_u,     texel_v), shadow_view_pos.z + bias );
		
      light_occlusion = light_occlusion / sample_count;
      light_occlusion = max( light_occlusion, .75 );
      
      output.rgb *= (light_occlusion * light_occlusion);     
	}
	
   // ssao
   float ssao;
   {
      uint width, height;
      g_ssao.GetDimensions( width, height );
      
      ssao = 1.0 - g_ssao.Sample( g_ssao_sampler, (input.position.xy + .5f) / float2(width, height) ).r;
   }

	output.rgb += material_color.rgb * g_ambient_light.rgb * ssao;

	return output;
}

struct PS_OUT
{
	float4 color : SV_TARGET0;
};

float4 sample( Texture2D tex, SamplerState samplerState, float2 uv, float2 tdx, float2 tdy )
{
   return tex.SampleGrad( samplerState, uv, tdx, tdy );
}

PS_OUT ps_main_lit(PS_INPUT input)
{
   float2 tdx = ddx_fine( input.uv.xy ) * g_resscale_lightmask.x;
   float2 tdy = ddy_fine( input.uv.xy ) * g_resscale_lightmask.y;
   
   float4 diff_color = sample( g_diff, g_diff_sampler, input.uv.xy, tdx, tdy );
   
   float4 material_color = g_material_color;
   material_color.rgb *= input.color * diff_color;
   
   float3 spec_color = sample( g_spec, g_spec_sampler, input.uv.xy, tdx, tdy ).rgb * g_specular_color.rgb;
	float3 tang_normal = sample( g_norm, g_norm_sampler, input.uv.xy, tdx, tdy ).xyz;
   
   float3 world_normal;

   if ( dot(tang_normal, tang_normal) == 0 )
      world_normal = normalize( mul(float4(input.normal, 0), g_world).xyz );
   else
   {
      tang_normal = normalize(tang_normal * 2 - 1);
      float3x3 tang_to_model = float3x3( input.tangent, input.binormal, input.normal );
      world_normal = normalize( mul( float4(mul(tang_normal, tang_to_model), 0), g_world ).xyz );
   }
   
	PS_OUT ps_out;
	ps_out.color = light_pixel(input, world_normal, material_color, spec_color);
   ps_out.color.rgb *= material_color.a; //force premultiplied alpha
   
   return ps_out;
}

PS_OUT ps_main_lit_vertexblend(PS_INPUT input)
{
   float2 tdx = ddx_fine( input.uv.xy ) * g_resscale_lightmask.x;
   float2 tdy = ddy_fine( input.uv.xy ) * g_resscale_lightmask.y;

   float4 diff1 = sample( g_diff, g_diff_sampler, input.uv.xy, tdx, tdy );
   float4 diff2 = sample( g_texture2, g_sampler3, input.uv.xy, tdx, tdy );
   
   float4 material_color = g_material_color;
   material_color.rgb *= (input.color * (diff1 * input.color.aaaa + diff2 * (1 - input.color.aaaa)));

	float3 spec_color = sample( g_spec, g_spec_sampler, input.uv.xy, tdx, tdy ).rgb * g_specular_color.rgb;
   float3 tang_normal = sample( g_norm, g_norm_sampler, input.uv.xy, tdx, tdy ).xyz;
   
   float3 world_normal;

   if ( dot(tang_normal, tang_normal) == 0 )
      world_normal = normalize( mul(float4(input.normal, 0), g_world).xyz );
   else
   {
      tang_normal = normalize(tang_normal * 2 - 1);
      float3x3 tang_to_model = float3x3( input.tangent, input.binormal, input.normal ); 
      world_normal = normalize( mul( float4(mul(tang_normal, tang_to_model), 0), g_world ).xyz );
   }
   
   PS_OUT ps_out;
	ps_out.color = light_pixel(input, world_normal, material_color, spec_color);
   ps_out.color.rgb *= material_color.a; //force premultiplied alpha
   
   return ps_out;
}