#version 330

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 24) out;

// Inputs; four since we are using GL_LINE_STRIP_ADJACENCY
smooth in vec3 vertexPosition[];

uniform mat4 pvm;
uniform float radius;

// Scratch space for the eight corners of the prismoid
vec4 corners[8];

void emitFace (int a, int b, int c, int d)
{
    gl_Position = corners[a]; EmitVertex();
    gl_Position = corners[b]; EmitVertex();
    gl_Position = corners[c]; EmitVertex();
    gl_Position = corners[d]; EmitVertex();
    EndPrimitive();
}

vec3 safeNormalize (vec3 vector)
{
    float len = length(vector);
    if (len != 0.0) {
        return vector / len;
    } else {
        return vector;
    }
}

void main ()
{
    // Compute object-space plane normals:
    vec3 p0 = vertexPosition[0];
    vec3 p1 = vertexPosition[1];
    vec3 p2 = vertexPosition[2];
    vec3 p3 = vertexPosition[3];

    vec3 n0 = safeNormalize(p1-p0);
    vec3 n1 = safeNormalize(p2-p1);
    vec3 n2 = safeNormalize(p3-p2);

    vec3 u = safeNormalize(n0 + n1);
    vec3 v = safeNormalize(n1 + n2);

    // Declare scratch variables for basis vectors:
    vec3 i, j, k;

    // Compute face 1 of 2:
    j = u;
    i = vec3(0, 0, 1);
    if (dot(i, j) == 1) {
        i = vec3(0, 1, 0);
    }
    k = cross(i, j);
    i = cross(k, j);

    i = radius*safeNormalize(i); k = radius*safeNormalize(k);
    corners[0] = pvm * vec4(p1 + i + k, 1);
    corners[1] = pvm * vec4(p1 + i - k, 1);
    corners[2] = pvm * vec4(p1 - i - k, 1);
    corners[3] = pvm * vec4(p1 - i + k, 1);

    // Compute face 2 of 2:
    j = v;
    i = vec3(0, 0, 1);
    if (dot(i, j) == 1) {
        i = vec3(0, 1, 0);
    }
    k = cross(i, j);
    i = cross(k, j);

    i = radius*safeNormalize(i); k = radius*safeNormalize(k);
    corners[4] = pvm * vec4(p2 + i + k, 1);
    corners[5] = pvm * vec4(p2 + i - k, 1);
    corners[6] = pvm * vec4(p2 - i - k, 1);
    corners[7] = pvm * vec4(p2 - i + k, 1);


    // Emit the faces of the cuboid:
    emitFace(0,1,3,2);
    emitFace(5,4,6,7);
    emitFace(4,5,0,1);
    emitFace(3,2,7,6);
    emitFace(0,3,4,7);
    emitFace(2,1,6,5);
}
