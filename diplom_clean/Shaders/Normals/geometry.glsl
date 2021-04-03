#version 460 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

const float MAGNITUDE = 0.1;
  
uniform mat4 projection;

void GenerateLine()
{
    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;
    vec4 p2 = gl_in[2].gl_Position;

    vec3 normal = normalize(cross(vec3(p1 - p0), vec3(p2 - p0)));
    vec4 midpoint = (p0 + p1 + p2) / 3.0;

    gl_Position = projection * midpoint;
    EmitVertex();
    gl_Position = projection * (midpoint + vec4(normal, 0.0) * MAGNITUDE);
    EmitVertex();
    EndPrimitive();
}

void main()
{
    GenerateLine(); // first vertex normal
}  