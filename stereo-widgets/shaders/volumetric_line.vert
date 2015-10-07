#version 330

// Inputs
in vec4 vertex;

uniform mat4 pvm;

// Output
smooth out vec3 vertexPosition;

void main()
{
    gl_Position = pvm * vertex;
    vertexPosition = vertex.xyz;
}
