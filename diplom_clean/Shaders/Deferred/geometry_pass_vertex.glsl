#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;  

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 normal_model;

void main()
{
    // vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0)); 
    TexCoords = aTexCoords;
    
    Normal = mat3(normal_model) * aNormal;

    vec3 T = normalize(vec3(model * vec4(aTangent,   0.0)));
    vec3 B = normalize(vec3(model * vec4(aBitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(aNormal,    0.0)));
    TBN = mat3(T, B, N);

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}