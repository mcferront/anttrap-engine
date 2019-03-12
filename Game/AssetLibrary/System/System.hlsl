struct VS_INPUT                   
{                                 
  float4 position : POSITION;     
  float2 uv       : TEXCOORD0;    
  uint4 color     : COLOR0;       
};                                

struct VS_OUTPUT                  
{                                 
  float4 position : SV_POSITION;
  float2 uv       : TEXCOORD0;    
  float4 color    : COLOR0;
};                                

cbuffer cb : register(b0)
{
	float4x4 world; 
	float4x4 vp;
	float4 color; 
};

VS_OUTPUT vs_main(VS_INPUT input)    
{                                                                   
    float4 position = mul( input.position, world );   
    position = mul( position, vp ); 

    VS_OUTPUT output;
    output.position = position;
    output.uv       = input.uv;
    output.color    = input.color / 255.0;
    return output;
}

struct PS_INPUT_VERTEXCOLOR
{                    
	float4 position : SV_POSITION;
	float2 uv       : TEXCOORD0;
    float4 color    : COLOR0;     
};        

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);


float4 ps_main_vertexcolor(PS_INPUT_VERTEXCOLOR input) : SV_TARGET
{                                    
    float4 output;
    output = input.color * color;
    return output;
}

float4 ps_main_prelit(PS_INPUT_VERTEXCOLOR input) : SV_TARGET
{                                    
    float4 output;
    output = (color * input.color * g_texture.Sample( g_sampler, input.uv.xy ));
    return output;
}