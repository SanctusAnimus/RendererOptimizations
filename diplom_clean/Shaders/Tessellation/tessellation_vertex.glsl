#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

// uniform vec3 lightPos;
// uniform vec3 viewPos;

out vec3 vPosition;
out vec3 vNormal;
out vec2 vTexCoord;

void main()
{                                                                                  
    vPosition = aPos.xyz;
    vNormal = aNormal.xyz;
    vTexCoord = aTexCoords;
}