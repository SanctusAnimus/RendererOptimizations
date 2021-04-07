#version 460 core

in vec2 teTexCoord;
out vec4 color;

uniform sampler2D image;
uniform float time;

void main(void)
{
    vec2 scrolled_coords = vec2(teTexCoord.x - time / 2.0, teTexCoord.y - time / 2.0);
    color = texture(image, scrolled_coords);
    // color = vec4(1.0);
}