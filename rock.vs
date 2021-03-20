#version 430

layout (location = 0) in vec2 position;

uniform float layer;
uniform float height;

out vec3 varPosition;

void main(void) { 
    gl_Position = vec4(position, 0.0, 1.0);
    // Add height to move the generated 3D Texture when moving up or down
    varPosition = vec3(position.x, position.y + height, layer * 2 - 1);
}