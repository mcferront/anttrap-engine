struct VS_INPUT                   
{                                 
    float4 position   : POSITION;
};                                

struct VS_OUTPUT                  
{                                 
    float4 position : SV_POSITION;     
};                                

cbuffer cb0 : register(b0)
{
	float4x4 cb_world;
	float4x4 cb_view_proj;

    float4 cb_base_color;
    float4 cb_subsurface_color;

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

VS_OUTPUT vs_main(VS_INPUT input)    
{                                                                   
    float4 position = mul( input.position, cb_world );   
    position = mul( position, cb_view_proj ); 
    
    VS_OUTPUT output;
    output.position = position;
    
    return output;
}

struct PS_INPUT
{                    
    float4 position : SV_POSITION;
};        

float4 ps_main(PS_INPUT input) : SV_TARGET
{                                    
    return float4(1,1,1,1);
}