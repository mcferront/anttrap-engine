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

float3 light_pixel(PS_INPUT input)
{
    float3 normal = normalize(input.normal);
    float3 light_vector = -cb_light_dir[0].xyz;
    float3 view_vector = -normalize(input.view);
    float3 half_vector = normalize(light_vector + view_vector);

    float l_dot_h = dot(light_vector, half_vector);
    float n_dot_l = dot(normal, light_vector);
    float n_dot_v = dot(normal, view_vector);
    float n_dot_h = dot(normal, half_vector);
    float v_dot_h = dot(view_vector, half_vector);

    float metallic = cb_metallic;
    float roughness = cb_roughness;
    
    //* .08 matches disney/blender's automatic ior conversion
    float specular = lerp( cb_specular * .08, 1.0, metallic);

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

    float3 specColor = lerp( float3(1, 1, 1), cb_base_color.rgb, cb_specular_tint );
    
    float3 finalDiffuse = cb_base_color.rgb * fD;
    float3 finalSpec = fS * specColor;
    
    return (finalDiffuse + finalSpec);
}

float4 ps_main(PS_INPUT input) : SV_TARGET
{          
    float3 color = light_pixel(input);
    return float4( color, 1 );

}


/*


float ggx_distribution(float roughness, float n_dot_h)
{
    if (n_dot_h <= 0) return 0.0;
    
    float a2 = roughness * roughness;

    float n_dot_h_2 = n_dot_h * n_dot_h;
    float n_dot_h_4 = n_dot_h_2 * n_dot_h_2;

    float sec_v = 1.0 / n_dot_h;
    float tan2_v = (sec_v * sec_v) - 1;

    float d = (a2 + tan2_v);
    
    return a2 / (PI * n_dot_h_4 * d * d);
}
float disney_diffuse_brdf(float roughness, float n_dot_l, float n_dot_v, float l_dot_h)
{
    //https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
    float fL = pow(1 - n_dot_l, 5.0); // disney uses 5 (same as schilck approx) - but blender matches if I do ~1.8
    float fV = pow(1 - n_dot_v, 5.0); // disney uses 5 (same as schilck approx) - but blender matches if I do ~1.8
    float fd90 = .5 + 2 * roughness * l_dot_h * l_dot_h;
    float rfL = lerp( 1, fd90, fL );//(1 + (fd90 - 1) * fL);
    float rfV = lerp( 1, fd90, fV );//(1 + (fd90 - 1) * fV);
    
    float fd = rfL * rfV;
    
    return fd;
}

float4 ps_main(PS_INPUT input) : SV_TARGET
{          
    return float4(1,1,1,1);
}
    
float ggx_distribution(float n_dot_h)
{
    float sec_m = 1.0 / n_dot_h;
    float tan2_m = (sec_m * sec_m) - 1;

    float a = .5 + cb_roughness * .5; //disney scale
    float a2 = a * a;
    
    float x = n_dot_h > 0 ? 1 : 0;

    float numerator = a2 * x;
    float denominator = PI * pow(n_dot_h, 4) * pow(a2 + tan2_m, 2);
    
    return numerator / denominator;  
}
 
*/



































