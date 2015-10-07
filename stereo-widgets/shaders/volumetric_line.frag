#version 330

// Inputs
uniform vec4 color;

// Outputs
out vec4 finalFragmentColor;

// Shader
void main (void)
{
    finalFragmentColor = color;
}
