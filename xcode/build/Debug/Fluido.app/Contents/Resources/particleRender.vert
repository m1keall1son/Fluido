#version 330

in vec4 position;
in vec4 velocity;

uniform mat4 ciModelViewProjection;

void main()
{
    gl_Position = ciModelViewProjection * vec4(position.xyz, 1.);
}