#version 330

// Inputs
smooth in vec4 vertexColor;

// Outputs
out vec4 fragmentColor;

void main (void)
{
    fragmentColor = vertexColor;
}
