/*
#version 300 es

precision highp float;

out vec4 FragColor;

uniform sampler2D uTexture;

in vec2 vTexCoord;

void main()
{
    vec4 texColor = texture2D( uTexture, vTexCoord );
    FragColor = texColor;
}
*/

varying mediump vec2 vTexCoord;

uniform mediump sampler2D uTexture;

void main() 
{
	mediump vec4 texColor = texture2D( uTexture, vTexCoord );
	gl_FragColor = texColor;
}