#version 150

in vec3 Position;
in vec3 Normal;

out vec4 outColor;

uniform uint time;

void main()
{
	outColor = vec4(vec3(clamp(dot(Normal, vec3(1, 1, 1)), 0.0, 1.0)), 1.0);
}
