#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 vsFragPos[];
in vec2 vsTexCoords[];
in vec3 vsNormal[];
in mat3 vsTBN[];

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out mat3 TBN;

uniform float time;

vec4 explode(vec4 position, vec3 normal)
{
    float magnitude = 2.0;
    vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude; 
    return position + vec4(direction, 0.0);
} 

vec3 GetNormal()
{
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return normalize(cross(a, b));
}  

void main() {    
    vec3 normal = GetNormal();

    gl_Position = explode(gl_in[0].gl_Position, normal);
    TexCoords = vsTexCoords[0];
    FragPos = vsFragPos[0];
    Normal = vsNormal[0];
    TBN = vsTBN[0];
    EmitVertex();
    gl_Position = explode(gl_in[1].gl_Position, normal);
    TexCoords = vsTexCoords[1];
    FragPos = vsFragPos[1];
    Normal = vsNormal[1];
    TBN = vsTBN[1];
    EmitVertex();
    gl_Position = explode(gl_in[2].gl_Position, normal);
    TexCoords = vsTexCoords[2];
    FragPos = vsFragPos[2];
    Normal = vsNormal[2];
    TBN = vsTBN[2];
    EmitVertex();
    EndPrimitive();
}  