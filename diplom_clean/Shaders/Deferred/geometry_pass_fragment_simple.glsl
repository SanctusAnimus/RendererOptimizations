#version 460 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in mat3 TBN;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_specular1;
uniform mat4 normal_model;
uniform vec3 lightColor;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    // also store the per-fragment normals into the gbuffer
    // gNormal = normalize(Normal);
    vec3 normal_sampled = texture(texture_normal1, TexCoords).rgb;
    gNormal = normalize(TBN * (normal_sampled * 2.0 - 1.0));
    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = lightColor * 3;
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(texture_specular1, TexCoords).r;
}