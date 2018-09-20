#include "Encodings.hlsl"

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
   float4 g_rough_metallic_emissive_; //32
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
Texture2D g_shadow_map : register(t3);
Texture2D g_ssao : register(t4);
Texture2D g_ssr : register(t5);
Texture2D g_texture2 : register(t6); //for island's other material

SamplerState g_diff_sampler : register(s0);
SamplerState g_spec_sampler : register(s1);
SamplerState g_norm_sampler : register(s2);
SamplerComparisonState g_shadow_sampler : register(s3);
SamplerState g_ssao_sampler : register(s4);
SamplerState g_ssr_sampler : register(s5);
SamplerState g_sampler3 : register(s6);	//for island's other material

#include "Lighting.hlsl"

struct PS_OUT
{
	float4 color : SV_TARGET0;
   
   #ifndef TRANSPARENCY
      unorm float4 matProperties : SV_TARGET1;
      unorm float2 normals : SV_TARGET2;
   #endif
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
   material_color.rgb *= input.color.rgb * diff_color.rgb;
   
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
   
   uint light_mask = asuint(g_resscale_lightmask.z);

	PS_OUT ps_out;
	ps_out.color = light_pixel(input.position.xy, light_mask, input.world_position.xyz, world_normal, 
                     material_color, spec_color, g_rough_metallic_emissive_.xyz, input.world_camera_view);
   ps_out.color.rgb *= material_color.a; //force premultiplied alpha

   #ifndef TRANSPARENCY
      ps_out.matProperties = g_rough_metallic_emissive_;
      ps_out.normals = normal_encode( world_normal );
   #endif
   
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
   
   uint light_mask = asuint(g_resscale_lightmask.z);

   PS_OUT ps_out;
	ps_out.color = light_pixel(input.position.xy, light_mask, input.world_position.xyz, world_normal, 
                     material_color, spec_color, g_rough_metallic_emissive_.xyz, input.world_camera_view);
   ps_out.color.rgb *= material_color.a; //force premultiplied alpha
   
   #ifndef TRANSPARENCY
      ps_out.matProperties = g_rough_metallic_emissive_;
      ps_out.normals = normal_encode( world_normal );
   #endif
   
   return ps_out;
}