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
uniform vec3 viewPos;

out vec4 FragColor;

void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec4 AlbedoSpec = texture(gAlbedoSpec, TexCoords);
    vec3 Diffuse = AlbedoSpec.rgb;
    float Specular = AlbedoSpec.a;
    
    // then calculate lighting as usual
    vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component
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
            diffuse *= attenuation;
            specular *= attenuation;
            lighting += diffuse + specular;
        }
    }    
    FragColor = vec4(lighting, 1.0);
}