#version 410 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int isSkyDome;

out vec3 fPosition;
out vec3 fNormal;
out vec2 fTexCoords;

void main() {
    if (isSkyDome == 1) {
        // Pentru skydome: normalizează poziția (creează sferă unitară)
        // apoi lasă matricea model să o scaleze corect
        vec3 normalizedPos = normalize(vPosition);
        vec4 pos = projection * view * model * vec4(normalizedPos, 1.0);
        gl_Position = pos.xyww;
        
        // Set varyings pentru fragment shader
        fTexCoords = vTexCoords;
        fPosition = (model * vec4(normalizedPos, 1.0)).xyz;
        fNormal = normalizedPos;  // Normal-ul e doar direcția pentru sferă
    } else {
        // Normal rendering - calculează totul normal
        fPosition = (model * vec4(vPosition, 1.0)).xyz;
        fNormal = mat3(transpose(inverse(model))) * vNormal;
        fTexCoords = vTexCoords;
        
        gl_Position = projection * view * model * vec4(vPosition, 1.0);
    }
}