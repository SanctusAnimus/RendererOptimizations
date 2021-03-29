#version 430

layout(location = 0) out vec4 fragColor;

in vec3 gFacetNormal;
in vec2 gTexCoord;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_normal1;
    sampler2D texture_specular1;
    sampler2D texture_height1;
};

uniform Material material;
uniform vec3 lightPos;

void main() {
    vec3 color = texture(material.texture_diffuse1, gTexCoord).xyz; 
    vec3 N = normalize(gFacetNormal);
    vec3 L = lightPos;
    float df = abs(dot(N,L));
    color = df * color;
    fragColor = vec4(color, 1.0);
}