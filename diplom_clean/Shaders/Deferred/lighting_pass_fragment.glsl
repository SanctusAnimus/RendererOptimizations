#version 460 core

in vec2 TexCoords;

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

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform int light_count;
uniform float intensity;
uniform float bloom_threshold;
uniform float ambient;
uniform vec3 viewPos;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec4 AlbedoSpec = texture(gAlbedoSpec, TexCoords);
    vec3 Diffuse = AlbedoSpec.rgb;
    float Specular = AlbedoSpec.a;
    
    // then calculate lighting as usual
    vec3 lighting  = Diffuse * ambient;
    vec3 viewDir  = normalize(viewPos - FragPos);

    for(int i = 0; i < light_count; ++i)
    {
        Light light = lights[i];
        // calculate distance between light source and current fragment
        vec3 light_position = light.Position.rgb;
        float distance = length(light_position - FragPos);
        if(distance < light.Radius * 1.3)
        {
            vec3 light_color = light.Color.rgb;
            // diffuse
            vec3 lightDir = normalize(light_position - FragPos);
            vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * light_color;
            // specular
            vec3 halfwayDir = normalize(lightDir + viewDir);  
            float spec = pow(max(dot(Normal, halfwayDir), 0.0), 32.0);
            vec3 specular = light_color * spec * Specular;
            // attenuation
            float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distance * distance);
            attenuation = attenuation * intensity;
            diffuse *= attenuation;
            specular *= attenuation;
            lighting += diffuse + specular;
        }
    }    
    FragColor = vec4(lighting, 1.0);

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > bloom_threshold)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}