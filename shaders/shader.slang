struct VertexInput {
    float3 inColour;
    float2 inPosition;
};

struct VertexOutput {
    float4 outColour;
    float4 sv_position : SV_Position;
};

[shader("vertex")]
VertexOutput vertexShader(VertexInput inputData) {
    VertexOutput output;
    output.outColour = float4(inputData.inColour, 1.0);
    output.sv_position = float4(inputData.inPosition, 0.0, 1.0);

    return output;
}

[shader("fragment")]
float4 fragmentShader(VertexOutput vertexOutput) {
    return float4(vertexOutput.outColour);
}