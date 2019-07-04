#define PI 3.14159265358979323846

struct VS_INPUT                   
{                                 
   float4 position   : POSITION;     
   float3 normal     : NORMAL;
   float3 view       : TEXCOORD0;
   //float3 binormal   : BINORMAL;
   //float3 tangent    : TANGENT;
   //float4 uv         : TEXCOORD0;
   //uint4 color       : COLOR0;
};                                

struct PS_INPUT
{                    
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float3 view : TEXCOORD0;
};   

cbuffer cb0 : register(b0)
{
	float4x4 cb_world;
	float4x4 cb_view_proj;
	float4x4 cb_camera_world;

    float4 cb_base_color;
    float4 cb_subsurface_color;

    float4 cb_light_dir[1];

    float3 cb_subsurface_radius;
    float3 cb_normal;
    float3 cb_clearcoat_normal;
    float3 cb_tangent;

    float cb_subsurface;
    float cb_metallic;
    float cb_specular;
    float cb_specular_tint;
    float cb_roughness;
    float cb_anisotropic;
    float cb_anisotropic_rotation;
    float cb_sheen;
    float cb_sheen_tint;
    float cb_clearcoat;
    float cb_clearcoat_roughness;
    float cb_ior;
    float cb_transmission;
    float cb_transmission_roughness;
}

PS_INPUT vs_main(VS_INPUT input)    
{                                                                   
    float4 world_pos = mul( input.position, cb_world );   
    float4 position = mul( world_pos, cb_view_proj ); 
    
    PS_INPUT output;
    output.position = position;
    output.normal = mul( float4(input.normal, 0), cb_world ).xyz;
    output.view = normalize(world_pos.xyz - cb_camera_world[3].xyz);

    return output;
}

Texture2D<float3> g_env_map : register(t0);
SamplerState g_sampler : register(s0);

Texture2D<float3> g_diff_ibl_map : register(t1);
SamplerState g_placeholder : register(s1);

Texture2D<float2> g_brdf_lut : register(t2);
SamplerState g_lut_sampler : register(s2);

float2 vector_to_uv(float3 v)
{
    // length of vector.xy
    float c = sqrt(v.x * v.x + v.y * v.y);

    // the angle from the look vector [-1, 1]
    float phi = acos(-v.z) / PI;

    // our scalar used to pull in or push out vector.xy
    // the bigger our look angle or smaller our vector.xy the more to push vector.xy to the ends
    float r = phi / c;
    
    float2 polarUV = (v.xy * r) * .5 + .5;

    return float2(polarUV.x, 1.0 - polarUV.y);
}

float4 ps_main(PS_INPUT input) : SV_TARGET
{          
    float3 view_vector = normalize(input.view);

    float3 diffuse = g_env_map.SampleLevel( g_sampler, vector_to_uv(view_vector), 2 );
    
    return float4( diffuse / PI, 1 );
}

































