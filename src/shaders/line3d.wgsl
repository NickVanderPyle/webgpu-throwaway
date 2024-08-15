struct VertexInput {
	@location(0) position: vec3<f32>,
	@location(1) color: vec3<f32>,
};

struct VertexOutput {
	@builtin(position) position: vec4<f32>,
	@location(0) color: vec3<f32>,
};

struct Uniforms {
    modelMatrix: mat4x4<f32>,
    viewMatrix: mat4x4<f32>,
    projectionMatrix: mat4x4<f32>,
    time: f32,
};

@group(0) @binding(0) var<uniform> uUniforms: Uniforms;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    var position = vec4<f32>(in.position, 1.0);
    out.position = uUniforms.projectionMatrix * uUniforms.viewMatrix * uUniforms.modelMatrix * position;

    out.color = in.color;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {
    return vec4<f32>(in.color, 1.0);
}