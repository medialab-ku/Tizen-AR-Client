/*
#version 300 es

precision highp float;

out vec4 FragColor;

uniform sampler2D uTexture;

in vec2 vTexCoord;
in vec4 vColor;

void main()
{
    vec4 texColor = texture2D( uTexture, vTexCoord );
    FragColor = texColor * vColor;
    //FragColor = texColor;
}
*/

varying mediump vec2 vTexCoord;
varying mediump vec4 vColor;

uniform mediump sampler2D uTexture;

void main() 
{
	mediump vec4 texColor = texture2D( uTexture, vTexCoord );
	gl_FragColor = texColor * vColor;
}