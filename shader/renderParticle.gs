#version 430

uniform mat4 proj;
uniform mat4 view;

uniform vec3 quad1, quad2;

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

in vec3 colorPass[];
in float lifeTimePass[];
in float sizePass[];
in int typePass[];

out vec2 texCoord;
out vec4 colorPart;

void main()
{
    // Create quads from a point
    vec3 posOld = gl_in[0].gl_Position.xyz;
    float size = sizePass[0];
    mat4 vp = proj * view;
    
    colorPart = vec4(colorPass[0], lifeTimePass[0]);
       
    // Emit 4 vertices (since a quad has 4 vertices)
    vec3 pos = posOld+(-quad1-quad2)*size;
    texCoord = vec2(0.0, 0.0);
    gl_Position = vp*vec4(pos, 1.0);
    EmitVertex();
    
    pos = posOld+(-quad1+quad2)*size;
    texCoord = vec2(0.0, 1.0);
    gl_Position = vp*vec4(pos, 1.0);
    EmitVertex();
    
    pos = posOld+(quad1-quad2)*size;
    texCoord = vec2(1.0, 0.0);
    gl_Position = vp*vec4(pos, 1.0);
    EmitVertex();
    
    pos = posOld+(quad1+quad2)*size;
    texCoord = vec2(1.0, 1.0);
    gl_Position = vp*vec4(pos, 1.0);
    EmitVertex();
      
    EndPrimitive();
}