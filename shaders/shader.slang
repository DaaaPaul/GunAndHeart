struct VertexInput {
    float3 inColour;
    float2 inPosition;
};

struct VertexOutput {
    float4 outColour;
    float4 sv_position : SV_Position;
};

struct TransformationMatrices {
    float4x4 model;
    float4x4 view;
    float4x4 projection;
};
ConstantBuffer<TransformationMatrices> transforms;

[shader("vertex")]
VertexOutput vertexShader(VertexInput inputData) {
    VertexOutput output;
    output.outColour = float4(inputData.inColour, 1.0);
    output.sv_position = 
    
    mul(transforms.projection,
    mul(transforms.view, 
    mul(transforms.model, float4(inputData.inPosition, 0.0, 1.0))));

    return output;
}

[shader("fragment")]
float4 fragmentShader(VertexOutput vertexOutput) {
    return float4(vertexOutput.outColour);
}