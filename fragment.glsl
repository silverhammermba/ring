#version 150

in vec3 Position;
in vec3 Normal;

out vec4 outColor;

uniform uint time;

void main()
{
	outColor = vec4(Normal, 1.0);
}
