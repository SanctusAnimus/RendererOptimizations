#version 430

layout(triangles) in;

in vec3 tcPosition[];
in vec3 tcNormal[];
in vec2 tcTexCoord[];

out vec3 tePosition;
out vec2 teTexCoord;
out vec3 teNormal;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_normal1;
    sampler2D texture_specular1;
    sampler2D texture_height1;
};

uniform Material material;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main(){
    vec3 p0 = gl_TessCoord.x * tcPosition[0];
    vec3 p1 = gl_TessCoord.y * tcPosition[1];
    vec3 p2 = gl_TessCoord.z * tcPosition[2];
    vec3 pos = p0 + p1 + p2;

    vec3 n0 = gl_TessCoord.x * tcNormal[0];
    vec3 n1 = gl_TessCoord.y * tcNormal[1];
    vec3 n2 = gl_TessCoord.z * tcNormal[2];
    vec3 normal = normalize(n0 + n1 + n2);

    vec2 tc0 = gl_TessCoord.x * tcTexCoord[0];
    vec2 tc1 = gl_TessCoord.y * tcTexCoord[1];
    vec2 tc2 = gl_TessCoord.z * tcTexCoord[2];  
    teTexCoord = tc0 + tc1 + tc2;

    float height = texture(material.texture_height1, teTexCoord).x;
    pos += normal * (height * 0.02f);

    mat4 mvp = projection * view * model;
    mat4 mv = view * model;

    gl_Position = mvp * vec4(pos, 1);
    teNormal    = vec3(mv * vec4(normal,0.0)).xyz;
    tePosition  = vec3(mv * vec4(pos,1.0)).xyz;
}