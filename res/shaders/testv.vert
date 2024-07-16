#version 460

layout(location = 0) in vec2 inPosition;

layout(location = 0) out vec3 outFragColor;

layout(binding = 0) uniform UniformBufferObject {
    vec2 grid_size;
} ubo;

struct Cell {
    uint value;
};

layout(std140, binding = 1) readonly buffer StorageBufferObject {
    Cell cell_state[];
} ssbo;

void main() {
    int i = gl_InstanceIndex;
    vec2 cell_pos = vec2(i % uint(ubo.grid_size.x), floor(i / uint(ubo.grid_size.y)));

    cell_pos = cell_pos / ubo.grid_size * 2;
    // cell_pos *= ssbo.cell_state[i].value;

    vec2 grid_pos = vec2(inPosition.x + 1, inPosition.y - 1) / ubo.grid_size;
    grid_pos = vec2(grid_pos.x - 1, grid_pos.y +1);
    grid_pos = vec2(grid_pos.x + cell_pos.x, grid_pos.y - cell_pos.y);

    gl_Position = vec4(grid_pos, 0.0, 1.0);

    vec3 color;
    switch (ssbo.cell_state[i].value) {
        case 0:
            color = vec3(0);
            break;
        case 1:
            color = vec3(203, 189, 147) / 255;
            break;
        case 2:
            color = vec3(48, 92, 222) / 255;
            break;
    }
    outFragColor = color;
}
