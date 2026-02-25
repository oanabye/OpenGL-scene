#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

uniform int isSkyDome;
uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

// Lighting uniforms
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform vec3 lightPos2;
uniform vec3 lightColor2;

// Point lights
uniform int pointLightsEnabled;
uniform vec3 pointLightPos1;
uniform vec3 pointLightColor1;
uniform float pointLightConstant1;
uniform float pointLightLinear1;
uniform float pointLightQuadratic1;

uniform vec3 pointLightPos2;
uniform vec3 pointLightColor2;
uniform float pointLightConstant2;
uniform float pointLightLinear2;
uniform float pointLightQuadratic2;

// Fog
uniform int pointFogEnabled;
uniform float fogDensity;
uniform vec3 fogColor;

uniform mat4 lightSpaceTrMatrix;

out vec4 fColor;

float computeShadow() {
    // Your shadow calculation here
    vec4 fragPosLightSpace = lightSpaceTrMatrix * vec4(fPosition, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    if(projCoords.z > 1.0)
        return 0.0;
    
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    
    return shadow;
}

void main() {
    // CRITICAL: Check skydome flag FIRST
    if (isSkyDome == 1) {
        // For skybox: just sample texture, no lighting
        fColor = texture(diffuseTexture, fTexCoords);
        return;  // Exit immediately - don't do any lighting calculations
    }
    
    // === REST OF YOUR SCENE LIGHTING CODE ===
    // This only runs when isSkyDome == 0
    
    vec4 colorFromTexture = texture(diffuseTexture, fTexCoords);
    if(colorFromTexture.a < 0.1)
        discard;
    
    vec3 normal = normalize(fNormal);
    vec3 viewDir = normalize(viewPos - fPosition);
    
    // Ambient
    vec3 ambient = 0.2 * lightColor;
    
    // Diffuse - Light 1
    vec3 lightDir = normalize(lightPos - fPosition);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular - Light 1
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = spec * lightColor * 0.5;
    
    // Light 2
    vec3 lightDir2 = normalize(lightPos2 - fPosition);
    float diff2 = max(dot(normal, lightDir2), 0.0);
    vec3 diffuse2 = diff2 * lightColor2;
    
    vec3 halfwayDir2 = normalize(lightDir2 + viewDir);
    float spec2 = pow(max(dot(normal, halfwayDir2), 0.0), 32.0);
    vec3 specular2 = spec2 * lightColor2 * 0.3;
    
    // Point lights
    vec3 pointLight = vec3(0.0);
    if (pointLightsEnabled == 1) {
        // Point light 1
        vec3 pl1Dir = normalize(pointLightPos1 - fPosition);
        float pl1Distance = length(pointLightPos1 - fPosition);
        float pl1Attenuation = 1.0 / (pointLightConstant1 + pointLightLinear1 * pl1Distance + 
                                      pointLightQuadratic1 * (pl1Distance * pl1Distance));
        float pl1Diff = max(dot(normal, pl1Dir), 0.0);
        pointLight += pl1Diff * pointLightColor1 * pl1Attenuation;
        
        // Point light 2
        vec3 pl2Dir = normalize(pointLightPos2 - fPosition);
        float pl2Distance = length(pointLightPos2 - fPosition);
        float pl2Attenuation = 1.0 / (pointLightConstant2 + pointLightLinear2 * pl2Distance + 
                                      pointLightQuadratic2 * (pl2Distance * pl2Distance));
        float pl2Diff = max(dot(normal, pl2Dir), 0.0);
        pointLight += pl2Diff * pointLightColor2 * pl2Attenuation;
    }
    
    // Shadow
    float shadow = computeShadow();
    
    // Combine lighting
    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular + diffuse2 + specular2) + pointLight;
    vec3 color = lighting * colorFromTexture.rgb;
    
    // Fog
    if (pointFogEnabled == 1) {
        float distance = length(viewPos - fPosition);
        float fogFactor = exp(-fogDensity * distance);
        fogFactor = clamp(fogFactor, 0.0, 1.0);
        color = mix(fogColor, color, fogFactor);
    }
    
    fColor = vec4(color, colorFromTexture.a);
}