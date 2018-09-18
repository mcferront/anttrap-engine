struct VS_INPUT                   
{                                 
  float4 position   : POSITION;     
};                                

struct VS_INPUT_SKIN
{
    float4 position : POSITION;
    float4 boneIndices : BLENDINDICES;
    float4 boneWeights : BLENDWEIGHT;
};

struct VS_OUTPUT                  
{                                 
  float4 position : SV_POSITION;     
};                                

cbuffer cb0 : register(b0)
{
	float4x4 viewProj;
	float4x4 world;
	float4x4 skin[ 32 ];
}

VS_OUTPUT vs_main(VS_INPUT input)    
{                                                                   
    float4 position = mul( input.position, world );   
    position = mul( position, viewProj ); 

    VS_OUTPUT output;
    output.position = position;
    
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
    position = mul( position, viewProj ); 
	
    VS_OUTPUT output;
    output.position = position;

    return output;
};  
