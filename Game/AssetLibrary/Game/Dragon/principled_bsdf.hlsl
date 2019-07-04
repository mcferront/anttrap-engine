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

float ggx_shadowing(float roughness, float v_dot_h, float v_dot_n)
{
    float a = .5 + roughness * .5;
    float a2 = a * a;
    
    if ( sign(v_dot_h) != sign(v_dot_n) )
        return 0;
    
    float sec_v = 1.0 / (v_dot_n + .00001);
    float tan2_v = (sec_v * sec_v) - 1;
    
    float y = a2 * tan2_v;
    
    return 2.0 / (1 + sqrt(1 + y));
}

float disney_gt2_modified_distribution(float roughness, float n_dot_h)
{
    if ( n_dot_h <= 0 )
        return 0;
    
    float n_dot_h_2 = n_dot_h * n_dot_h;
    
    float a2 = roughness * roughness;
    
    // using disney's gt2 but including a .6 coefficient as roughness increases
    // which keeps some specular at the half vector as it gets more rough
    // https://github.com/wdas/brdf/blob/master/src/brdfs/disney.brdf
    float b = a2 * n_dot_h_2;
    float d = lerp( b, b * .6, roughness );

    d += (1 - n_dot_h_2);
    
    return a2 / (d * d * PI);
}

float cook_torrence_brdf(float roughness, float n_dot_h, float n_dot_v, float n_dot_l, float l_dot_h, float v_dot_h)
{
    roughness = max(.001, roughness * roughness);
    float d = disney_gt2_modified_distribution(roughness, n_dot_h);
    
    float g1 = ggx_shadowing(roughness, v_dot_h, n_dot_v);
    float g2 = ggx_shadowing(roughness, l_dot_h, n_dot_l);

    float g = g1 * g2;

    return (g * d) / abs(4 * n_dot_v * n_dot_l);
}

// blender users can input [0, 1+] for specular
// disney has a modified formula to compute the fresnel approx
// https://github.com/wdas/brdf/blob/master/src/brdfs/disney.brdf
float modified_schlick_fresnel(float specular, float cu)
{
    float theta = pow( 1 - cu, 5 );
    return lerp( specular, 1, theta );
}

float schlick_fresnel(float ior, float cu)
{
    float r = (1 - ior) / (1 + ior);
    float r0 = r * r;
    float theta = pow( 1 - cu, 5 );
    
    return r0 + (1 - r0) * theta;
}

float custom_diffuse_brdf(float roughness, float n_dot_l)
{
    // the more rough the surface the more n_dot_l is distributed
    // over the valid 0-PI angle
    float distribution = max(1, 1 + roughness * roughness * 2);
    return 1 - pow(1 - n_dot_l, distribution);
}

float2 vector_to_uv(float3 v)
{
    // length of vector.xy
    float c = sqrt(v.x * v.x + v.y * v.y);

    // the angle from the look vector [-1, 1]
    float phi = acos(- v.z) / PI;

    // our scalar used to pull in or push out vector.xy
    // the bigger our look angle or smaller our vector.xy the more to push vector.xy to the ends
    float r = phi / c;
    
    float2 polarUV = (v.xy * r) * .5 + .5;

    return float2(polarUV.x, 1.0 - polarUV.y);
}

float3 environment_spec(float3 view_vector, float3 normal, float3 specColor, float n_dot_v, float roughness)
{
    roughness = roughness * roughness;
    
    float3 refl = reflect(view_vector, normal);

    n_dot_v = saturate(n_dot_v);
     
    float2 spec_diffuse = g_brdf_lut.SampleLevel( g_lut_sampler, float2(n_dot_v, roughness), 0 );
     
    // http://www.pauldebevec.com/Probes/
    // environment has unwrapped to a 2d texture
    // -look values are on the circumference of the image
    // refl.xy * scalar used to reference the unwrapped pixel

    float2 grad = lerp( 0, 0.1, roughness );
    float3 specMap = g_env_map.SampleGrad( g_sampler, vector_to_uv(refl), grad, grad );
    
    // enviroment map was not prefiltere
    // so divide by PI to bring into typical lambertian and our spec diffuse range
    specMap /= PI;

    return specMap * ( specColor * spec_diffuse.x + spec_diffuse.y );
}

float3 light_pixel(PS_INPUT input)
{
    float3 normal = normalize(input.normal);
    float3 view_vector = normalize(input.view);

    float metallic = cb_metallic;
    float roughness = cb_roughness;
    float n_dot_v = dot(normal, - view_vector);

    float fS = 0;
    float fD = 0;

    //* .08 matches disney/blender's automatic ior conversion
    float specular = lerp( cb_specular * .08, 1.0, metallic);
    
    // for each light - disabled as we just focus on IBL
    if (false)
    {
        float3 light_vector = -cb_light_dir[0].xyz;
        float3 half_vector = normalize(light_vector + (-view_vector));

        float l_dot_h = dot(light_vector, half_vector);
        float n_dot_l = dot(normal, light_vector);
        float n_dot_h = dot(normal, half_vector);
        float v_dot_h = dot(-view_vector, half_vector);

        float fresnel = modified_schlick_fresnel(specular, l_dot_h);
        float fD = custom_diffuse_brdf(roughness, n_dot_l);
        float fS = cook_torrence_brdf(roughness, n_dot_h, n_dot_v, n_dot_l, l_dot_h, v_dot_h);
        
        // to improve the specular falloff for metallic materials
        // I incorporate the diffuse falloff into the specular based on the metallic parameter
        fS = lerp( fS, fS * fD, metallic );
        fS = fS * fresnel;
        
        fD = fD / PI;
        fD *= (1 - metallic) * (1 - fresnel);

        // after all this and our slightly custom spec
        // it's possible we will have fS + fD > 1
        // so renormalize the values
        if ( fD + fS > 1 )
        {    
            float x0 = min(fD, fS);
            float x1 = max(fD, fS);
            float range = 1.0 / (x1 - x0);
            
            fD = (fD - x0) * range;
            fS = (fS - x0) * range;
        }
    }

    float3 base_color = float(1).xxx;//cb_base_color.rgb;
    float3 specColor = lerp( float3(1, 1, 1), base_color, cb_specular_tint );
    
    float3 finalDiffuse = base_color * fD;
    float3 finalSpec = fS * specColor;
    float3 finalPunctualColor = finalDiffuse + finalSpec;
    
    // IBL
    float3 iblDiffuse, iblSpec;
    {
        float2 grad = lerp( 0, 0.1, roughness );
        float3 imgDiffuse = g_diff_ibl_map.SampleGrad( g_sampler, vector_to_uv(normal), grad, grad );

        iblDiffuse = imgDiffuse / PI * (1 - metallic) * base_color;
        
        iblSpec = environment_spec( view_vector, normal, specular * specColor, n_dot_v, roughness );
    }
    
    return finalPunctualColor + iblSpec + iblDiffuse;
}

float4 ps_main(PS_INPUT input) : SV_TARGET
{          
    float3 pixel = light_pixel(input);
    
    return float4( pixel, 1 );

}

































