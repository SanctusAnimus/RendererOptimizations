#version 430

layout(vertices = 4) out;

in vec3 vPosition[];
in vec3 vNormal[];
in vec2 vTexCoord[];
out vec3 tcPosition[];
out vec3 tcNormal[];
out vec2 tcTexCoord[];

uniform float level;

void main(){

    float inTess  = level / 2.0;
    float outTess = level;

    tcPosition[gl_InvocationID] = vPosition[gl_InvocationID];
    tcNormal[gl_InvocationID]   = vNormal[gl_InvocationID];
    tcTexCoord[gl_InvocationID] = vTexCoord[gl_InvocationID];

    if(gl_InvocationID == 0) {
        gl_TessLevelInner[0] = inTess;
        gl_TessLevelInner[1] = inTess;

        gl_TessLevelOuter[0] = outTess;
        gl_TessLevelOuter[1] = outTess;
        gl_TessLevelOuter[2] = outTess;
        gl_TessLevelOuter[3] = outTess;
    }
}