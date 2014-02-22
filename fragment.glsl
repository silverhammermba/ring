#version 150

in vec3 Color;
in vec2 Texcoord;

out vec4 outColor;

void main()
{
	vec2 dist = floor(Texcoord * vec2(4.0, 10.0));
	float alt = mod(dist.x + dist.y, 2.0);
	outColor = vec4(Color * alt, 1.0);
}
