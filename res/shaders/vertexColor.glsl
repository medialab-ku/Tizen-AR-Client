/*
#version 300 es

in vec3 aPosition;
in vec3 aColor;

uniform mat4 uMvpMatrix;
uniform vec3 uSize;

out vec3 vColor;

void main()
{
    vColor = aColor;
    vec4 vertexPosition = vec4(aPosition, 1.0);
    vertexPosition.xyz *= uSize;
    gl_Position = uMvpMatrix * vertexPosition;
}
*/

attribute mediump vec3 aPosition;
attribute mediump vec3 aColor;

uniform mediump mat4 uMvpMatrix;
uniform mediump vec3 uSize;

varying mediump vec3 vColor;

void main() 
{
	mediump vec4 vertexPosition = vec4(aPosition, 1.0);
	vertexPosition.xyz *= uSize;
	vColor = aColor;
	
	mediump mat4 landscape = mat4( vec4(0, -1, 0, 0),
	                               vec4(1,  0, 0, 0),
	                               vec4(0,  0, 1, 0),
	                               vec4(0,  0, 0, 1));
	
	gl_Position = landscape * uMvpMatrix * vertexPosition;
}