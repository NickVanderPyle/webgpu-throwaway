struct VertexInput {
    @location(0) position: vec3<f32>,
    @location(1) color: vec3<f32>,
    @location(2) modelMatrix0: vec4<f32>,   // First column of the matrix
    @location(3) modelMatrix1: vec4<f32>,   // Second column of the matrix
    @location(4) modelMatrix2: vec4<f32>,   // Third column of the matrix
    @location(5) modelMatrix3: vec4<f32>,   // Fourth column of the matrix
};

struct VertexOutput {
	@builtin(position) position: vec4<f32>,
	@location(0) color: vec3<f32>,
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    let modelMatrix = mat4x4<f32>(
        in.modelMatrix0,
        in.modelMatrix1,
        in.modelMatrix2,
        in.modelMatrix3
    );
    var out: VertexOutput;
    out.position = modelMatrix * vec4<f32>(in.position, 1.0);

    out.color = in.color;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {
    return vec4<f32>(in.color, 1.0);
}
