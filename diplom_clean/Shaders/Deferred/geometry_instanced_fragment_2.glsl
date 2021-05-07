#version 460 core

struct Light {
    vec4 Position;
    vec4 Color;
    
    float Linear;
    float Quadratic;
    float Radius;
};

layout (std430, binding=2) buffer shader_data
{
    Light lights [];
};

in vec2 TexCoords;
in vec3 FragPos;
in vec4 FragColor;
in vec3 Normal;
in mat3 TBN;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_specular1;
uniform int has_render_color;

uniform int light_count;
uniform float intensity;
uniform float bloom_threshold;
uniform float ambient;
uniform vec3 viewPos;

layout (location = 0) out vec4 fsFragColor;
layout (location = 1) out vec4 fsBrightColor;


void main()
{    
    // store the fragment position vector in the first gbuffer texture

    vec3 Diffuse;

    if (has_render_color == 1) {
        Diffuse = FragColor.rgb * 3;
    }
    else {
        Diffuse = texture(texture_diffuse1,TexCoords).rgb;
    }
    float Specular = texture(texture_specular1, TexCoords).r;
    
    vec3 normal_sampled = texture(texture_normal1, TexCoords).rgb;
    vec3 gNormal = normalize(TBN * (normal_sampled * 2.0 - 1.0));

    // then calculate lighting as usual
    vec3 lighting  = Diffuse * ambient;
    vec3 viewDir  = normalize(viewPos - FragPos);

    for(int i = 0; i < light_count; ++i)
    {
        Light light = lights[i];
        // calculate distance between light source and current fragment
        vec3 light_position = light.Position.rgb;
        float distance = length(light_position - FragPos);

        vec3 light_color = light.Color.rgb;
        // diffuse
        vec3 lightDir = normalize(light_position - FragPos);
        vec3 diffuse = max(dot(gNormal, lightDir), 0.0) * Diffuse * light_color;
        // specular
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        float spec = pow(max(dot(gNormal, halfwayDir), 0.0), 32.0);
        vec3 specular = light_color * spec * Specular;
        // attenuation
        float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distance * distance);
        attenuation = attenuation * intensity;
        diffuse *= attenuation;
        specular *= attenuation;
        lighting += diffuse + specular;

    }    
    fsFragColor = vec4(lighting, 1.0);

    float brightness = dot(fsFragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > bloom_threshold)
        fsBrightColor = vec4(fsFragColor.rgb, 1.0);
    else
        fsBrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}