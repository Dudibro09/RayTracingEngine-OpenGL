#version 460 core

layout (location = 0) in vec2 aCoordinate;

out vec2 coordinate;

void main()
{
	coordinate = aCoordinate;
	gl_Position = vec4(aCoordinate,0,1);
}