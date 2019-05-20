/*
#version 300 es

in vec2 aPosition;
in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
    vTexCoord = aTexCoord;
    gl_Position = vec4(aPosition.x, aPosition.y, 1.0, 1.0);
}
*/

attribute mediump vec2 aPosition;
attribute mediump vec2 aTexCoord;

varying mediump vec2 vTexCoord;

void main()
{
	vTexCoord = aTexCoord;
    gl_Position = vec4(aPosition.x, aPosition.y, 1.0, 1.0);
}

