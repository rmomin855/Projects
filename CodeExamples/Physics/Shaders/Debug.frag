#version 460 core

out vec3 fragColour;
uniform vec3 colour;

void main()
{
  fragColour = colour;
}