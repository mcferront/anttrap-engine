struct VS_INPUT                   
{                                 
  float4 position   : POSITION;     
  float4 uv         : TEXCOORD0;
  uint4 color       : COLOR0;
};                                

struct VS_INPUT_SKIN
{
    float4 position : POSITION;
    float4 uv       : TEXCOORD0;
    uint4 color     : COLOR0;
    float4 boneIndices : BLENDINDICES;
    float4 boneWeights : BLENDWEIGHT;
};

struct VS_OUTPUT                  
{                                 
  float4 position : SV_POSITION;     
  float4 uv       : TEXCOORD0;    
  float4 color    : COLOR0;
};                                

cbuffer cb0 : register(b0)
{
	float4 color; 
	float4x4 world;
	float4x4 view;
	float4x4 projection;
	float4x4 skin[ 32 ];
}

VS_OUTPUT vs_main(VS_INPUT input)    
{                                                                   
    float4 position = mul( input.position, world );   
    position = mul( position, view ); 
    position = mul( position, projection );

    VS_OUTPUT output;
    output.position = position;
    output.uv       = input.uv;
    output.color    = input.color / 255.0;
    
    return output;
}

VS_OUTPUT vs_main_skin( VS_INPUT_SKIN input )
{
    int4 indices = int4( input.boneIndices );

    float4x4 boneMatrix = 
        mul( skin[ indices[ 0 ] ], input.boneWeights.x ) +
        mul( skin[ indices[ 1 ] ], input.boneWeights.y ) +
        mul( skin[ indices[ 2 ] ], input.boneWeights.z ) +
        mul( skin[ indices[ 3 ] ], input.boneWeights.w );
    
    float4 position = mul( input.position, boneMatrix );
                                                          
    position = mul( position, world ); 
    position = mul( position, view ); 
    position = mul( position, projection );
	
    VS_OUTPUT output;
    output.position = position;
    output.uv = input.uv;
    output.color = input.color / 255.0;

    return output;
};  

struct PS_INPUT
{                    
    float4 position : SV_POSITION;
    float4 uv       : TEXCOORD0;
    float4 color    : COLOR0;     
};        

Texture2D g_texture1 : register(t0);
Texture2D g_texture2 : register(t1);
SamplerState g_sampler1 : register(s0);
SamplerState g_sampler2 : register(s1);

float4 ps_main_vertexblend(PS_INPUT input) : SV_TARGET
{                                    
    float4 output;
   
    output = (color * (input.color * ( g_texture1.Sample( g_sampler1, input.uv.xy) * (input.color.aaaa) + g_texture2.Sample( g_sampler2, input.uv.xy ) * (1 - input.color.aaaa))));
    return output;
}

float4 ps_main_prelit(PS_INPUT input) : SV_TARGET
{                                    
    float4 output;
    output  = (color * input.color * g_texture1.Sample( g_sampler1, input.uv.xy ));
    return output;
}