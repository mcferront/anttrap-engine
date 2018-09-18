struct VS_INPUT
{
    float4 position : POSITION;
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
    float4 view_pos : TEXCOORD0;
};                                

cbuffer cb0 : register(b0)
{
   float4 cameraParams;
	float4x4 world;
	float4x4 view;
	float4x4 projection;
	float4x4 skin[ 32 ];
}

VS_OUTPUT vs_main(VS_INPUT input)    
{                                                                   
    float4 position = mul( input.position, world );   

    float4 view_position = mul( position, view );
    float4 proj_position = mul( view_position, projection );

    VS_OUTPUT output;
    output.position = proj_position;
    output.view_pos = view_position;
    
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
    
    float4 view_position = mul( position, view );
    float4 proj_position = mul( view_position, projection );
	
    VS_OUTPUT output;
    output.position = proj_position;
    output.view_pos = view_position;
    
    return output;
};  

struct PS_INPUT
{                    
    float4 position : SV_POSITION;
    float4 view_pos : TEXCOORD0;
};

float4 ps_main(PS_INPUT input) : SV_TARGET
{                                    
   float4 output;
   
   float nearPlane = cameraParams.x;
   float farPlane = cameraParams.y;
   
   //output.r = (input.view_pos.z - nearPlane) / (farPlane - nearPlane);
   output.r = input.view_pos.z;
   return output;
}























