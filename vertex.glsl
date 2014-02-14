#version 150

in vec2 position;

uniform mat4 trans;

void main()
{
	gl_Position = trans * vec4(position, 0.0, 1.0);
}
