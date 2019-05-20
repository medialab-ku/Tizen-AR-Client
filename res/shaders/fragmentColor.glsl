/*
#version 300 es

precision highp float;

out vec4 FragColor;

uniform float uAlpha;

in vec3 vColor;

void main()
{
    vec4 Color = vec4(vColor, uAlpha);
    FragColor = Color;
}
*/

varying mediump vec3 vColor;

uniform mediump float uAlpha;

void main() 
{
	mediump vec4 FragColor = vec4(vColor, uAlpha);
	gl_FragColor = FragColor;
}