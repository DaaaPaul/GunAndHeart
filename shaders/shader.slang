static float2 positions[3] = float2[](
	float2(0.0, -0.5),
	float2(0.5, 0.5),
	float2(-0.5, 0.5)
);

static float4 colours[3] = float4[](
	float4(1.0, 0.0, 0.0, 1.0),
	float4(0.0, 1.0, 0.0, 1.0),
	float4(0.0, 0.0, 1.0, 1.0)
);

struct VertexOutput {
    float4 colour;
    float2 sv_position : SV_Position;
};

[shader("vertex")]
VertexOutput vertexShader(uint vertexID: SV_VertexID) {
    VertexOutput output;
    output.colour = colours[vertexID];
    output.sv_position = positions[vertexID];

    return output;
}

[shader("fragment")]
float4 fragmentShader(VertexOutput vertexOutput) {
    return float4(vertexOutput.colour);
}