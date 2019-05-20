/*
#version 300 es

in vec3 aPosition;
in vec2 aTexCoord;
in vec4 aColor;

uniform mat4 uMvpMatrix;
uniform vec3 uSize;
uniform mat3 uNormalMatrix;
uniform mat4 uModelMatrix;
uniform mat4 uModelView;
uniform mat4 uViewMatrix;

out vec2 vTexCoord;
out vec4 vColor;

void main()
{   
    vec4 vertexPosition = vec4(aPosition, 1.0);
    vertexPosition.xyz *= uSize;

    vTexCoord = aTexCoord;
    vColor = aColor;

    gl_Position = uMvpMatrix * vertexPosition;
}
*/

attribute mediump vec3 aPosition;
attribute mediump vec2 aTexCoord;
attribute mediump vec4 aColor;

uniform mediump mat4 uMvpMatrix;
uniform mediump vec3 uSize;

varying mediump vec2 vTexCoord;
varying mediump vec4 vColor;

void main() 
{
	mediump vec4 vertexPosition = vec4(aPosition, 1.0);
	vertexPosition.xyz *= uSize;
	
	vTexCoord = aTexCoord;
	vColor = aColor;
	
	gl_Position = uMvpMatrix * vertexPosition;           
}