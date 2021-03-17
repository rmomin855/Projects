#version 460 core

in layout(location = 0) vec3 modelPosition;

uniform mat4 ViewTransform;
uniform mat4 ProjTransform;

void main()
{
  gl_Position = (ProjTransform * ViewTransform) * vec4(modelPosition, 1.0f);
}