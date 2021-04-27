#version 410 core
in vec2 TexCoords;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;

out vec4 FragColor;

void main()
{          
   vec4 tex = texture(texture_diffuse1, TexCoords);
   tex.rgb = normalize(tex.rgb);
   FragColor = tex;
}