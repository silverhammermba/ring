#version 150

in vec3 position;
in vec3 normal;

out vec3 Camera; // direction to camera
out vec3 Light; // direction to light source
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform vec3 light;

void main()
{
	Camera = normalize(vec3(0) - (view * model * vec4(position, 1.0)).xyz);
	Light = normalize(light - (model * vec4(position, 1.0)).xyz);
	Normal = normal;

	gl_Position = proj * view * model * vec4(position, 1.0);
}
