/*
#version 300 es

precision highp float;

out vec4 FragColor;

in vec3 vNormal;  
in vec3 vFragPos;  
  
uniform vec3 uLightDir; 
uniform vec3 uViewPos; 
uniform vec3 uObjectColor;

void main()
{
    vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);

    // ambient
    float ambientStrength = 0.4;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(uLightDir);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    float specularStrength = 0.6;
    vec3 viewDir = normalize(uViewPos - vFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
    vec3 specular = specularStrength * spec * lightColor;  
        
    vec3 result = (ambient + diffuse + specular) * uObjectColor;
    FragColor = vec4(result, 1.0);
}
*/

varying mediump vec3 vNormal;
varying mediump vec3 vFragPos;

uniform mediump vec3 uLightDir;
uniform mediump vec3 uViewPos;
uniform mediump vec3 uObjectColor;

void main() 
{
	mediump vec3 lightColor = vec3(1.0, 1.0, 1.0);

    // ambient
    mediump float ambientStrength = 0.65;
    mediump vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    mediump vec3 norm = normalize(vNormal);
    mediump vec3 lightDir = normalize(uLightDir);
    mediump float diff = max(dot(norm, lightDir), 0.0);
    mediump vec3 diffuse = diff * lightColor;
    
    // specular
    mediump float specularStrength = 0.6;
    mediump vec3 viewDir = normalize(uViewPos - vFragPos);
    mediump vec3 reflectDir = reflect(-lightDir, norm);  
    mediump float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
    mediump vec3 specular = specularStrength * spec * lightColor;  
        
    mediump vec3 result = (ambient + diffuse + specular) * uObjectColor;
    mediump vec4 FragColor = vec4(result, 1.0);
	gl_FragColor = FragColor;
}