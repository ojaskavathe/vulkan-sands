#version 460

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor; 

layout(location = 0) out vec3 outFragColor;

// layout(binding = 0) uniform UniformBufferObject {
//     vec2 grid_size;
// } ubo;

const vec2 grid = {4, 4};

void main() {

    vec2 pos = vec2(1, 1) / grid * 2;

    vec2 grid_pos = vec2(inPosition.x + 1, inPosition.y - 1) / grid;
    grid_pos = vec2(grid_pos.x - 1, grid_pos.y + 1);
    grid_pos = vec2(grid_pos.x + pos.x, grid_pos.y - pos.y);

    gl_Position = vec4(grid_pos, 0.0, 1.0);
    outFragColor = inColor;
}