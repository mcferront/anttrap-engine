
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
   float4x4 g_inv_view_proj;
   float4x4 g_shadow_view_matrix;
   float4x4 g_shadow_proj_matrix;

   float4 g_world_camera_view;   //0
   float4 g_ambient_light;       //16
   float4 g_light_dir[ MaxLights ];    //48
   float4 g_light_pos_type[ MaxLights ];
	float4 g_light_color[ MaxLights ]; 
	float4 g_light_inner_outer_range__[ MaxLights ]; 
}	

#include "Lighting.hlsl"

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
      color = light_pixel(dispatchThreadId.xy, light_mask, world_position, normal, float4(opaque, 1), specular, rough_metallic_emissive_lit.xyz, g_world_camera_view);
   }
   else
      color = opaque;
                             
   g_output[ dispatchThreadId.xy ] = color;
}
















