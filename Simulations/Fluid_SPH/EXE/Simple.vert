#version 460 core

uniform mat4 projTransform;

layout(location = 0) in vec2  vPosition;

void main()
{
    vec4 vPos = vec4( vPosition.x, vPosition.y, 0.0f, 1.0f );
    gl_Position = projTransform * vPos;
}
