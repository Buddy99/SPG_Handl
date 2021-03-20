#version 430

layout (points) in; // in: Points (pointcloud from the vertexshader)
layout (triangle_strip, max_vertices = 12) out; // out: Triangle Strips, max. 4 triangles with 3 vertices => max 12 vertices out

uniform mat4 proj;
uniform mat4 view;

in vec3 varTexture[];
in int mcCase[];    // Marching Cubes Case, 0-255
out vec3 varTextureG;

// First Lookup-Table 
// e.g.: (0.0, 0.0, 0.0) means no triangles are generated
vec3 vectors[13] = {
    vec3(0.0, 0.0, 0.0), vec3(0.5, 1.0, 1.0), vec3(0.0, 0.5, 1.0), 
    vec3(0.0, 1.0, 0.5), vec3(1.0, 0.5, 1.0), vec3(1.0, 1.0, 0.5),
    vec3(0.5, 1.0, 0.0), vec3(1.0, 0.5, 0.0), vec3(0.0, 0.5, 0.0), 
    vec3(0.0, 0.0, 0.5), vec3(0.5, 0.0, 1.0), vec3(1.0, 0.0, 0.5),
    vec3(0.5, 0.0, 0.0)};

// Second Lookup-Table
// 256 (possible Marching Cube Cases) * 12 (12 points produce 4 triangles) = 3072
// e.g.: three zeros in a row means no triangles
int table[3072] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  4,  1,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  3,  5,
    2,  5,  2,  4,  0,  0,  0,  0,  0,  0,  5,  6,  7,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  5,  6,  7,  3,  1,  2,  0,  0,  0,  0,  0,  0,  7,  4,  6,  4,
    6,  1,  0,  0,  0,  0,  0,  0,  3,  6,  2,  7,  6,  2,  7,  2,  4,  0,  0,
    0,  3,  8,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  1,  8,  1,  8,  2,
    0,  0,  0,  0,  0,  0,  6,  3,  8,  1,  5,  4,  0,  0,  0,  0,  0,  0,  6,
    5,  8,  4,  5,  8,  4,  8,  2,  0,  0,  0,  5,  3,  7,  3,  7,  8,  0,  0,
    0,  0,  0,  0,  5,  1,  7,  2,  1,  7,  2,  7,  8,  0,  0,  0,  1,  3,  4,
    8,  3,  4,  8,  4,  7,  0,  0,  0,  8,  7,  2,  4,  7,  2,  0,  0,  0,  0,
    0,  0,  9,  2,  10, 0,  0,  0,  0,  0,  0,  0,  0,  0,  9,  3,  10, 3,  10,
    1,  0,  0,  0,  0,  0,  0,  4,  1,  5,  2,  10, 9,  0,  0,  0,  0,  0,  0,
    4,  10, 5,  9,  10, 5,  9,  5,  3,  0,  0,  0,  9,  2,  10, 5,  7,  6,  0,
    0,  0,  0,  0,  0,  9,  3,  10, 3,  10, 1,  7,  6,  5,  0,  0,  0,  7,  4,
    6,  4,  6,  1,  9,  10, 2,  0,  0,  0,  4,  10, 7,  3,  10, 7,  3,  10, 9,
    3,  6,  7,  9,  2,  10, 3,  8,  6,  0,  0,  0,  0,  0,  0,  9,  8,  10, 6,
    8,  10, 6,  10, 1,  0,  0,  0,  6,  8,  3,  10, 2,  9,  1,  4,  5,  0,  0,
    0,  4,  5,  6,  4,  10, 6,  8,  10, 6,  8,  10, 9,  5,  3,  7,  3,  7,  8,
    10, 2,  9,  0,  0,  0,  9,  7,  1,  9,  10, 1,  5,  7,  1,  9,  7,  8,  9,
    10, 8,  4,  10, 8,  4,  10, 7,  3,  2,  1,  9,  10, 8,  4,  10, 8,  4,  8,
    7,  0,  0,  0,  10, 4,  11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  10, 4,  11,
    1,  2,  3,  0,  0,  0,  0,  0,  0,  5,  11, 1,  11, 1,  10, 0,  0,  0,  0,
    0,  0,  10, 2,  11, 3,  2,  11, 3,  11, 5,  0,  0,  0,  7,  5,  6,  4,  11,
    10, 0,  0,  0,  0,  0,  0,  3,  2,  1,  11, 4,  10, 5,  7,  6,  0,  0,  0,
    7,  11, 6,  10, 11, 6,  10, 6,  1,  0,  0,  0,  7,  6,  3,  7,  11, 3,  2,
    11, 3,  2,  11, 10, 10, 4,  11, 6,  8,  3,  0,  0,  0,  0,  0,  0,  6,  1,
    8,  1,  8,  2,  11, 4,  10, 0,  0,  0,  10, 1,  11, 1,  11, 5,  8,  3,  6,
    0,  0,  0,  10, 8,  5,  10, 11, 5,  6,  8,  5,  10, 8,  2,  8,  7,  3,  7,
    3,  5,  10, 11, 4,  0,  0,  0,  10, 11, 2,  7,  11, 2,  7,  11, 8,  1,  4,
    5,  7,  11, 8,  1,  11, 8,  1,  11, 10, 1,  3,  8,  10, 11, 2,  7,  11, 2,
    7,  2,  8,  0,  0,  0,  11, 9,  4,  9,  4,  2,  0,  0,  0,  0,  0,  0,  1,
    4,  3,  11, 4,  3,  11, 3,  9,  0,  0,  0,  2,  1,  9,  5,  1,  9,  5,  9,
    11, 0,  0,  0,  3,  5,  9,  11, 5,  9,  0,  0,  0,  0,  0,  0,  2,  4,  9,
    4,  9,  11, 6,  5,  7,  0,  0,  0,  7,  6,  11, 3,  6,  11, 3,  6,  9,  4,
    5,  1,  7,  1,  9,  7,  11, 9,  2,  1,  9,  7,  1,  6,  7,  6,  11, 3,  6,
    11, 3,  11, 9,  0,  0,  0,  11, 9,  4,  9,  4,  2,  6,  8,  3,  0,  0,  0,
    1,  4,  6,  9,  4,  6,  9,  4,  11, 9,  8,  6,  6,  8,  5,  9,  8,  5,  9,
    8,  11, 1,  3,  2,  6,  8,  5,  9,  8,  5,  9,  5,  11, 0,  0,  0,  7,  11,
    9,  7,  9,  9,  5,  4,  2,  5,  2,  2,  9,  11, 8,  11, 8,  7,  1,  4,  5,
    0,  0,  0,  7,  8,  11, 8,  11, 9,  1,  3,  2,  0,  0,  0,  9,  11, 8,  11,
    8,  7,  0,  0,  0,  0,  0,  0,  11, 7,  12, 0,  0,  0,  0,  0,  0,  0,  0,
    0,  11, 7,  12, 3,  2,  1,  0,  0,  0,  0,  0,  0,  11, 7,  12, 5,  4,  1,
    0,  0,  0,  0,  0,  0,  3,  5,  2,  5,  2,  4,  12, 7,  11, 0,  0,  0,  11,
    5,  12, 5,  12, 6,  0,  0,  0,  0,  0,  0,  11, 5,  12, 5,  12, 6,  2,  1,
    3,  0,  0,  0,  11, 4,  12, 1,  4,  12, 1,  12, 6,  0,  0,  0,  11, 2,  6,
    11, 12, 6,  3,  2,  6,  11, 2,  4,  7,  12, 11, 8,  6,  3,  0,  0,  0,  0,
    0,  0,  2,  8,  1,  8,  1,  6,  11, 12, 7,  0,  0,  0,  1,  4,  5,  12, 7,
    11, 6,  8,  3,  0,  0,  0,  11, 12, 4,  8,  12, 4,  8,  12, 2,  5,  7,  6,
    8,  12, 3,  11, 12, 3,  11, 3,  5,  0,  0,  0,  8,  12, 2,  5,  12, 2,  5,
    12, 11, 5,  1,  2,  8,  3,  1,  8,  12, 1,  4,  12, 1,  4,  12, 11, 11, 12,
    4,  8,  12, 4,  8,  4,  2,  0,  0,  0,  12, 11, 7,  10, 9,  2,  0,  0,  0,
    0,  0,  0,  1,  10, 3,  10, 3,  9,  7,  11, 12, 0,  0,  0,  12, 7,  11, 1,
    4,  5,  10, 2,  9,  0,  0,  0,  12, 7,  9,  5,  7,  9,  5,  7,  3,  10, 11,
    4,  6,  12, 5,  12, 5,  11, 2,  9,  10, 0,  0,  0,  12, 9,  3,  12, 3,  3,
    11, 10, 1,  11, 1,  1,  11, 4,  12, 1,  4,  12, 1,  4,  6,  9,  10, 2,  3,
    9,  6,  9,  6,  12, 4,  10, 11, 0,  0,  0,  3,  6,  8,  11, 12, 7,  9,  10,
    2,  0,  0,  0,  7,  11, 6,  10, 11, 6,  10, 11, 1,  8,  12, 9,  12, 8,  9,
    6,  7,  5,  1,  2,  3,  6,  7,  5,  9,  8,  12, 5,  7,  6,  11, 4,  10, 0,
    0,  0,  8,  12, 3,  11, 12, 3,  11, 12, 5,  2,  9,  10, 5,  11, 1,  11, 1,
    10, 8,  12, 9,  0,  0,  0,  8,  12, 9,  4,  10, 11, 2,  1,  3,  0,  0,  0,
    11, 10, 4,  9,  12, 8,  0,  0,  0,  0,  0,  0,  12, 10, 7,  10, 7,  4,  0,
    0,  0,  0,  0,  0,  12, 10, 7,  10, 7,  4,  3,  2,  1,  0,  0,  0,  5,  7,
    1,  12, 7,  1,  12, 1,  10, 0,  0,  0,  12, 7,  10, 3,  7,  10, 3,  7,  5,
    3,  2,  10, 4,  5,  10, 6,  5,  10, 6,  10, 12, 0,  0,  0,  4,  5,  10, 6,
    5,  10, 6,  5,  12, 2,  1,  3,  12, 6,  10, 1,  6,  10, 0,  0,  0,  0,  0,
    0,  3,  2,  6,  10, 2,  6,  10, 6,  12, 0,  0,  0,  4,  7,  10, 7,  10, 12,
    3,  6,  8,  0,  0,  0,  6,  8,  2,  6,  2,  2,  7,  12, 10, 7,  10, 10, 5,
    7,  1,  12, 7,  1,  12, 7,  10, 3,  6,  8,  10, 12, 2,  12, 2,  8,  5,  7,
    6,  0,  0,  0,  4,  12, 3,  4,  5,  3,  8,  12, 3,  4,  12, 10, 8,  2,  12,
    2,  12, 10, 5,  1,  4,  0,  0,  0,  8,  3,  12, 1,  3,  12, 1,  12, 10, 0,
    0,  0,  10, 12, 2,  12, 2,  8,  0,  0,  0,  0,  0,  0,  12, 9,  7,  2,  9,
    7,  2,  7,  4,  0,  0,  0,  12, 4,  3,  12, 9,  3,  1,  4,  3,  12, 4,  7,
    2,  1,  5,  2,  9,  5,  7,  9,  5,  7,  9,  12, 12, 7,  9,  5,  7,  9,  5,
    9,  3,  0,  0,  0,  2,  9,  4,  6,  9,  4,  6,  9,  12, 6,  5,  4,  12, 6,
    9,  6,  9,  3,  4,  5,  1,  0,  0,  0,  2,  9,  1,  12, 9,  1,  12, 1,  6,
    0,  0,  0,  3,  9,  6,  9,  6,  12, 0,  0,  0,  0,  0,  0,  12, 9,  7,  2,
    9,  7,  2,  9,  4,  6,  8,  3,  1,  6,  4,  6,  4,  7,  9,  8,  12, 0,  0,
    0,  5,  7,  6,  9,  8,  12, 3,  2,  1,  0,  0,  0,  12, 8,  9,  6,  7,  5,
    0,  0,  0,  0,  0,  0,  4,  2,  5,  2,  5,  3,  12, 9,  8,  0,  0,  0,  12,
    8,  9,  1,  4,  5,  0,  0,  0,  0,  0,  0,  8,  9,  12, 2,  3,  1,  0,  0,
    0,  0,  0,  0,  12, 8,  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12, 8,  9,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  8,  9,  12, 2,  3,  1,  0,  0,  0,  0,
    0,  0,  12, 8,  9,  1,  4,  5,  0,  0,  0,  0,  0,  0,  4,  2,  5,  2,  5,
    3,  12, 9,  8,  0,  0,  0,  12, 8,  9,  6,  7,  5,  0,  0,  0,  0,  0,  0,
    5,  7,  6,  9,  8,  12, 3,  2,  1,  0,  0,  0,  1,  6,  4,  6,  4,  7,  9,
    8,  12, 0,  0,  0,  12, 9,  7,  2,  9,  7,  2,  9,  4,  6,  8,  3,  3,  9,
    6,  9,  6,  12, 0,  0,  0,  0,  0,  0,  2,  9,  1,  12, 9,  1,  12, 1,  6,
    0,  0,  0,  12, 6,  9,  6,  9,  3,  4,  5,  1,  0,  0,  0,  2,  9,  4,  6,
    9,  4,  6,  9,  12, 6,  5,  4,  12, 7,  9,  5,  7,  9,  5,  9,  3,  0,  0,
    0,  2,  1,  5,  2,  9,  5,  7,  9,  5,  7,  9,  12, 12, 4,  3,  12, 9,  3,
    1,  4,  3,  12, 4,  7,  12, 9,  7,  2,  9,  7,  2,  7,  4,  0,  0,  0,  10,
    12, 2,  12, 2,  8,  0,  0,  0,  0,  0,  0,  8,  3,  12, 1,  3,  12, 1,  12,
    10, 0,  0,  0,  8,  2,  12, 2,  12, 10, 5,  1,  4,  0,  0,  0,  4,  12, 3,
    4,  5,  3,  8,  12, 3,  4,  12, 10, 10, 12, 2,  12, 2,  8,  5,  7,  6,  0,
    0,  0,  5,  7,  1,  12, 7,  1,  12, 7,  10, 3,  6,  8,  6,  8,  2,  6,  2,
    2,  7,  12, 10, 7,  10, 10, 4,  7,  10, 7,  10, 12, 3,  6,  8,  0,  0,  0,
    3,  2,  6,  10, 2,  6,  10, 6,  12, 0,  0,  0,  12, 6,  10, 1,  6,  10, 0,
    0,  0,  0,  0,  0,  4,  5,  10, 6,  5,  10, 6,  5,  12, 2,  1,  3,  4,  5,
    10, 6,  5,  10, 6,  10, 12, 0,  0,  0,  12, 7,  10, 3,  7,  10, 3,  7,  5,
    3,  2,  10, 5,  7,  1,  12, 7,  1,  12, 1,  10, 0,  0,  0,  12, 10, 7,  10,
    7,  4,  3,  2,  1,  0,  0,  0,  12, 10, 7,  10, 7,  4,  0,  0,  0,  0,  0,
    0,  11, 10, 4,  9,  12, 8,  0,  0,  0,  0,  0,  0,  8,  12, 9,  4,  10, 11,
    2,  1,  3,  0,  0,  0,  5,  11, 1,  11, 1,  10, 8,  12, 9,  0,  0,  0,  8,
    12, 3,  11, 12, 3,  11, 12, 5,  2,  9,  10, 9,  8,  12, 5,  7,  6,  11, 4,
    10, 0,  0,  0,  12, 8,  9,  6,  7,  5,  1,  2,  3,  6,  7,  5,  7,  11, 6,
    10, 11, 6,  10, 11, 1,  8,  12, 9,  3,  6,  8,  11, 12, 7,  9,  10, 2,  0,
    0,  0,  3,  9,  6,  9,  6,  12, 4,  10, 11, 0,  0,  0,  11, 4,  12, 1,  4,
    12, 1,  4,  6,  9,  10, 2,  12, 9,  3,  12, 3,  3,  11, 10, 1,  11, 1,  1,
    6,  12, 5,  12, 5,  11, 2,  9,  10, 0,  0,  0,  12, 7,  9,  5,  7,  9,  5,
    7,  3,  10, 11, 4,  12, 7,  11, 1,  4,  5,  10, 2,  9,  0,  0,  0,  1,  10,
    3,  10, 3,  9,  7,  11, 12, 0,  0,  0,  12, 11, 7,  10, 9,  2,  0,  0,  0,
    0,  0,  0,  11, 12, 4,  8,  12, 4,  8,  4,  2,  0,  0,  0,  8,  3,  1,  8,
    12, 1,  4,  12, 1,  4,  12, 11, 8,  12, 2,  5,  12, 2,  5,  12, 11, 5,  1,
    2,  8,  12, 3,  11, 12, 3,  11, 3,  5,  0,  0,  0,  11, 12, 4,  8,  12, 4,
    8,  12, 2,  5,  7,  6,  1,  4,  5,  12, 7,  11, 6,  8,  3,  0,  0,  0,  2,
    8,  1,  8,  1,  6,  11, 12, 7,  0,  0,  0,  7,  12, 11, 8,  6,  3,  0,  0,
    0,  0,  0,  0,  11, 2,  6,  11, 12, 6,  3,  2,  6,  11, 2,  4,  11, 4,  12,
    1,  4,  12, 1,  12, 6,  0,  0,  0,  11, 5,  12, 5,  12, 6,  2,  1,  3,  0,
    0,  0,  11, 5,  12, 5,  12, 6,  0,  0,  0,  0,  0,  0,  3,  5,  2,  5,  2,
    4,  12, 7,  11, 0,  0,  0,  11, 7,  12, 5,  4,  1,  0,  0,  0,  0,  0,  0,
    11, 7,  12, 3,  2,  1,  0,  0,  0,  0,  0,  0,  11, 7,  12, 0,  0,  0,  0,
    0,  0,  0,  0,  0,  9,  11, 8,  11, 8,  7,  0,  0,  0,  0,  0,  0,  7,  8,
    11, 8,  11, 9,  1,  3,  2,  0,  0,  0,  9,  11, 8,  11, 8,  7,  1,  4,  5,
    0,  0,  0,  7,  11, 9,  7,  9,  9,  5,  4,  2,  5,  2,  2,  6,  8,  5,  9,
    8,  5,  9,  5,  11, 0,  0,  0,  6,  8,  5,  9,  8,  5,  9,  8,  11, 1,  3,
    2,  1,  4,  6,  9,  4,  6,  9,  4,  11, 9,  8,  6,  11, 9,  4,  9,  4,  2,
    6,  8,  3,  0,  0,  0,  7,  6,  11, 3,  6,  11, 3,  11, 9,  0,  0,  0,  7,
    1,  9,  7,  11, 9,  2,  1,  9,  7,  1,  6,  7,  6,  11, 3,  6,  11, 3,  6,
    9,  4,  5,  1,  2,  4,  9,  4,  9,  11, 6,  5,  7,  0,  0,  0,  3,  5,  9,
    11, 5,  9,  0,  0,  0,  0,  0,  0,  2,  1,  9,  5,  1,  9,  5,  9,  11, 0,
    0,  0,  1,  4,  3,  11, 4,  3,  11, 3,  9,  0,  0,  0,  11, 9,  4,  9,  4,
    2,  0,  0,  0,  0,  0,  0,  10, 11, 2,  7,  11, 2,  7,  2,  8,  0,  0,  0,
    7,  11, 8,  1,  11, 8,  1,  11, 10, 1,  3,  8,  10, 11, 2,  7,  11, 2,  7,
    11, 8,  1,  4,  5,  8,  7,  3,  7,  3,  5,  10, 11, 4,  0,  0,  0,  10, 8,
    5,  10, 11, 5,  6,  8,  5,  10, 8,  2,  10, 1,  11, 1,  11, 5,  8,  3,  6,
    0,  0,  0,  6,  1,  8,  1,  8,  2,  11, 4,  10, 0,  0,  0,  10, 4,  11, 6,
    8,  3,  0,  0,  0,  0,  0,  0,  7,  6,  3,  7,  11, 3,  2,  11, 3,  2,  11,
    10, 7,  11, 6,  10, 11, 6,  10, 6,  1,  0,  0,  0,  3,  2,  1,  11, 4,  10,
    5,  7,  6,  0,  0,  0,  7,  5,  6,  4,  11, 10, 0,  0,  0,  0,  0,  0,  10,
    2,  11, 3,  2,  11, 3,  11, 5,  0,  0,  0,  5,  11, 1,  11, 1,  10, 0,  0,
    0,  0,  0,  0,  10, 4,  11, 1,  2,  3,  0,  0,  0,  0,  0,  0,  10, 4,  11,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  9,  10, 8,  4,  10, 8,  4,  8,  7,  0,
    0,  0,  9,  10, 8,  4,  10, 8,  4,  10, 7,  3,  2,  1,  9,  7,  1,  9,  10,
    1,  5,  7,  1,  9,  7,  8,  5,  3,  7,  3,  7,  8,  10, 2,  9,  0,  0,  0,
    4,  5,  6,  4,  10, 6,  8,  10, 6,  8,  10, 9,  6,  8,  3,  10, 2,  9,  1,
    4,  5,  0,  0,  0,  9,  8,  10, 6,  8,  10, 6,  10, 1,  0,  0,  0,  9,  2,
    10, 3,  8,  6,  0,  0,  0,  0,  0,  0,  4,  10, 7,  3,  10, 7,  3,  10, 9,
    3,  6,  7,  7,  4,  6,  4,  6,  1,  9,  10, 2,  0,  0,  0,  9,  3,  10, 3,
    10, 1,  7,  6,  5,  0,  0,  0,  9,  2,  10, 5,  7,  6,  0,  0,  0,  0,  0,
    0,  4,  10, 5,  9,  10, 5,  9,  5,  3,  0,  0,  0,  4,  1,  5,  2,  10, 9,
    0,  0,  0,  0,  0,  0,  9,  3,  10, 3,  10, 1,  0,  0,  0,  0,  0,  0,  9,
    2,  10, 0,  0,  0,  0,  0,  0,  0,  0,  0,  8,  7,  2,  4,  7,  2,  0,  0,
    0,  0,  0,  0,  1,  3,  4,  8,  3,  4,  8,  4,  7,  0,  0,  0,  5,  1,  7,
    2,  1,  7,  2,  7,  8,  0,  0,  0,  5,  3,  7,  3,  7,  8,  0,  0,  0,  0,
    0,  0,  6,  5,  8,  4,  5,  8,  4,  8,  2,  0,  0,  0,  6,  3,  8,  1,  5,
    4,  0,  0,  0,  0,  0,  0,  6,  1,  8,  1,  8,  2,  0,  0,  0,  0,  0,  0,
    3,  8,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  3,  6,  2,  7,  6,  2,  7,
    2,  4,  0,  0,  0,  7,  4,  6,  4,  6,  1,  0,  0,  0,  0,  0,  0,  5,  6,
    7,  3,  1,  2,  0,  0,  0,  0,  0,  0,  5,  6,  7,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  3,  5,  2,  5,  2,  4,  0,  0,  0,  0,  0,  0,  4,  1,  5,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};

// Marching Cubes algorithm generates (between 0 and 4) triangles from the points of the vertexshader
void main(void)
{
    vec4 base = gl_in[0].gl_Position;
    int index = mcCase[0] * 12;
    
    // Use Marching Cubes Case from the Vertexshader as Index for the Lookup-Tables
    for(int i = 0; i < 12; i += 3)  // Executed 4 times (max. 4 triangles)
    {
        for(int j = 0; j < 3; j++)  // Executed 3 times (each triangle contains 3 points)
        {
            gl_Position = proj * view * (base + vec4(vectors[table[index + i + j]], 0.0));
            varTextureG = varTexture[0];
            EmitVertex();   // Vertex is reads
        }
        EndPrimitive(); // Triangle is ready
    }
    // Depending on the Marching Cubes case, up to 4 triangles will be produced
}