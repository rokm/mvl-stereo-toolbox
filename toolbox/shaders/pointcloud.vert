#version 330

// Inputs
in vec4 vertex;
in vec4 color;

uniform mat4 pvm;

// Outputs
smooth out vec4 vertexColor;

// Shader
void main (void)
{
    gl_Position = pvm  * vertex;
    vertexColor = color;
}
