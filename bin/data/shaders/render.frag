#version 150

out vec4 fragColor;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 modelViewProjectionMatrix;

uniform samplerCube depthMap;
uniform float farPlane;
uniform vec3 lightPos;
uniform vec4 ambientColor;

in vec3 normalVarying;
in vec4 positionVarying;
in vec4 vColor;

// based on http://learnopengl.com/#!Advanced-Lighting/Shadows/Point-Shadows
float ShadowCalculation(vec3 fragPos)
{
    float shadow = 0.0;
    vec3 fragToLight = fragPos - lightPos;
    float closestDepth = texture(depthMap, fragToLight).r;
    if(closestDepth < 1.0){
        float currentDepth = length(fragToLight);
        closestDepth *= (farPlane + farPlane * 0.001 + (farPlane * 0.1 * (currentDepth / farPlane)));
        // 影で0,日向で1
        shadow = currentDepth > closestDepth ? 1.0 : 0.0;
    }
    return shadow;
}

void main (void) {
    //影影響係数
    float shadow = ShadowCalculation(positionVarying.xyz);
    float lambert = max(dot(normalize(vec3(vec4(lightPos, 1.0) - positionVarying)), normalVarying), 0.0);
    fragColor = vec4(vec3(lambert) * (1.0 - shadow), 1.0);
    //fragColor = vec4(vec3(lambert) * (1.0 - shadow), 1.0) + ambientColor;
}
 
