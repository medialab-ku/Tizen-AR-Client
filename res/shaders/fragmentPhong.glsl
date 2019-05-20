/*
#version 300 es

precision highp float;

out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec3 uLightPos;
uniform vec3 uViewPos;
uniform vec3 uLightColor;

in vec3 vNormal;
in vec3 vFragPos;
in vec2 vTexCoord;

void main()
{
    // ambient
    float ambientStrength = 0.25;
    vec3 ambient = ambientStrength * uLightColor;
    
    // diffuse
    vec3 norm = normalize(vNormal);
    // directional light
    vec3 lightDir = normalize(uLightPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(uViewPos - vFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * uLightColor;  
        
    vec4 texColor = texture2D( uTexture, vTexCoord );    
    vec4 result = vec4((ambient + diffuse + specular), 1.0) * texColor;
    
    FragColor = result;
}
*/

varying mediump vec3 vNormal;
varying mediump vec3 vFragPos;
varying mediump vec2 vTexCoord;

uniform mediump sampler2D uTexture;
uniform mediump vec3 uLightPos;
uniform mediump vec3 uViewPos;
uniform mediump vec3 uLightColor;

void main() 
{
	// ambient
    mediump float ambientStrength = 0.65;
    mediump vec3 ambient = ambientStrength * uLightColor;
    
    // diffuse
    mediump vec3 norm = normalize(vNormal);
    // directional light
    mediump vec3 lightDir = normalize(uLightPos);
    mediump float diff = max(dot(norm, lightDir), 0.0);
    mediump vec3 diffuse = diff * uLightColor;
    
    // specular
    mediump float specularStrength = 0.5;
    mediump vec3 viewDir = normalize(uViewPos - vFragPos);
    mediump vec3 reflectDir = reflect(-lightDir, norm);  
    mediump float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    mediump vec3 specular = specularStrength * spec * uLightColor;  
        
    mediump vec4 texColor = texture2D( uTexture, vTexCoord );    
    mediump vec4 result = vec4((ambient + diffuse + specular), 1.0) * texColor;
    
    gl_FragColor = result;
}