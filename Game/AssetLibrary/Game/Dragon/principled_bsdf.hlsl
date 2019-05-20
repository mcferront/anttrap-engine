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
/*
float ggx_distribution(PS_INPUT input)
{
    
}

float ggx_shadowing(PS_INPUT input)
{
    
}

float cook_torrence_brdf(PS_INPUT input)
{
    float n_dot_v = dot(input.normal, input.view_vector);
    float n_dot_l = dot(input.normal, cb_light_dir[0]);
    
    float g = ggx_distribution(input);
    float d = ggx_shadowing(input);

    return (g * d) / (4 * saturate(n_dot_v) * saturate(n_dot_l));
}

float schlick_fresnel(PS_INPUT input, float3 half_vector)
{
    float3 half_vector = normalize(cb_light_dir[0] + input.view_vector);
    
    float h_dot_v = dot(half_vector, input.view_vector);

    float r0 = pow((1 - cb_ior) / (1 + cb_ior), 2);
    float theta = pow( 1 - saturate(h_dot_v), 5 );
    
    return r0 + (1 - r0) * theta;
}

float lambertian_diffuse_brdf(PS_INPUT input)
{
    return saturate( dot(cb_light_dir[0], input.normal) ); 
}

float3 light_pixel()
{
    float fresnel = schlick_fresnel(input);

    float diffuse = lambertian_diffuse_brdf(input);
    float fss = cook_torrence_brdf(input);
        
    return input.base_color * ((1 - fresnel) * diffuse + fresnel * fss);
}

float4 ps_main(PS_INPUT input) : SV_TARGET
{          
    float3 color = light_pixel(input);
    
    return color;
}
*/
float4 ps_main(PS_INPUT input) : SV_TARGET
{          
    return float4(1,1,1,1);
}
