#include "Encodings.hlsl"

//#define MOTION_VECTORS
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
   };
  
#endif

cbuffer cb0 : register(b0)
{
	float4 g_material_color;               //0
   float4 g_specular_color;               //16
   float4 g_rough_metallic_emissive_lit;  //32
	float4 g_resscale_lightmask;           //48
   float4x4 g_vp;
	float4x4 g_world;
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

   return output;
}

VS_OUTPUT vs_main_skin( VS_INPUT_SKIN input )
{
	float4x4 boneMatrix = compute_bone_matrices( int4( input.boneIndices ), input.boneWeights, g_skin );
	
	float4 skin_position = mul( input.position, boneMatrix );
	float4 world_position = mul( skin_position, g_world );
	float4 proj_position = mul( world_position, g_vp );
	
	float3 model_normal = mul( float4(input.normal, 0), boneMatrix ).xyz;
    
	VS_OUTPUT output;
   output.position = proj_position;
	output.uv = input.uv;
   output.color = input.color / 255.0;
   output.normal = model_normal;
   output.binormal = input.binormal;
   output.tangent  = input.tangent;
   
   return output;
};  


Texture2D g_diff : register(t0);
Texture2D g_spec : register(t1);
Texture2D g_norm : register(t2);

SamplerState g_diff_sampler : register(s0);
SamplerState g_spec_sampler : register(s1);
SamplerState g_norm_sampler : register(s2);

struct PS_OUT
{
	float3 color : SV_TARGET0;
	unorm float2 normal : SV_TARGET1;
	float3 specular : SV_TARGET2;
   float4 properties : SV_TARGET3;
   uint light_mask : SV_TARGET4;
};

float4 sample( Texture2D tex, SamplerState samplerState, float2 uv, float2 tdx, float2 tdy )
{
   return tex.SampleGrad( samplerState, uv, tdx, tdy );
}

PS_OUT ps_main(PS_INPUT input)
{
   float2 tdx = ddx_fine( input.uv.xy ) * g_resscale_lightmask.x;
   float2 tdy = ddy_fine( input.uv.xy ) * g_resscale_lightmask.y;
   
   float4 material_color = g_material_color * input.color * sample( g_diff, g_diff_sampler, input.uv.xy, tdx, tdy );
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
	ps_out.color = material_color.rgb;
	ps_out.normal = normal_encode(world_normal);
   ps_out.specular = spec_color.rgb;
   ps_out.properties = g_rough_metallic_emissive_lit;
   ps_out.light_mask = asuint(g_resscale_lightmask.z);
   
   return ps_out;
}
