#version 150

in vec3 Camera;
in vec3 Light;
in vec3 Normal;

out vec4 outColor;

void main()
{
	vec3 halfway = normalize(Camera + Light);

	vec3 light_diff = vec3(1.0, 1.0, 1.0); // diffuse color of light
	vec3 light_spec = light_diff; // specular color of light

	vec3 global_amb = vec3(0.1, 0.1, 0.1); // global ambient light

	vec3 mat_amb = vec3(0.925, 0.788, 0.525); // ambient color of material
	vec3 mat_diff = mat_amb; // diffuse color of material
	vec3 mat_spec = mat_amb; // specular color of material
	float mat_gloss = 10.0; // glossiness of material


	vec3 color = (light_spec * mat_spec) * pow(max(dot(Normal, halfway), 0), mat_gloss) + (light_diff * mat_diff) * max(dot(Normal, Light), 0) + global_amb * mat_amb;
	outColor = vec4(color, 1.0);
}
