#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using std::cerr;
using std::endl;

// read entire contents of file into string
std::string read_file(const std::string& filename)
{
	std::string content;

	std::ifstream in(filename);
	if (!in)
		throw std::runtime_error("Failed to read file: " + filename);

	in.seekg(0, std::ios::end);
	content.resize(in.tellg());
	in.seekg(0, std::ios::beg);
	in.read(&content[0], content.size());
	in.close();

	return content;
}

// load file contents, compile shader, report errors
void load_shader(GLuint shader, const std::string& filename)
{
	std::string source = read_file(filename);
	const char* ptr = source.c_str();
	glShaderSource(shader, 1, (const GLchar**)&ptr, nullptr);
	glCompileShader(shader);
	// error check
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	GLint log_length;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
	if (log_length > 0)
	{
		char buffer[log_length];
		glGetShaderInfoLog(shader, log_length, nullptr, buffer);
		cerr << buffer;
	}
	if (status == GL_FALSE)
		throw std::runtime_error("Failed to compile shader: " + filename);
}

// construct view matrix a la FPS positioning
glm::mat4 cam_mat(float heading, float pitch, const glm::vec3& pos)
{
	// TODO move to gfx card
	float ct = cosf(heading);
	float st = sinf(heading);
	float cp = cosf(pitch);
	float sp = sinf(pitch);
	glm::mat4 view;
	view[0] = glm::vec4(ct, -sp * st, cp * st, 0);
	view[1] = glm::vec4(0, cp, sp, 0);
	view[2] = glm::vec4(-st, -sp * ct, cp * ct, 0);
	view[3] = glm::vec4(-ct * pos.x + st * pos.z, sp * st * pos.x - cp * pos.y + sp * ct * pos.z, -cp * st * pos.x - sp * pos.y - cp * ct * pos.z, 1);
	return view;
}

int main(int argc, char** argv)
{
	// start SDL
	SDL_Init(SDL_INIT_VIDEO);

	// specify non-deprecated OpenGL context
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// window dimensions
	unsigned int width = 1920;
	unsigned int height = 1080;

	// create window
	SDL_Window* window = SDL_CreateWindow(
		"Hide",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width,
		height,
		SDL_WINDOW_OPENGL
	);

	if (window == nullptr)
	{
		cerr << "SDL_CreateWindow failed: " << SDL_GetError() << endl;
		return 1;
	}

	// relative mouse motion for FPS controls
	if (SDL_SetRelativeMouseMode(SDL_TRUE) != 0)
	{
		cerr << "SDL_SetRelativeMouseMode failed: " << SDL_GetError() << endl;
	}

	// create context
	SDL_GLContext context = SDL_GL_CreateContext(window);

	if (context == nullptr)
	{
		cerr << "SDL_GL_CreateContext failed: " << SDL_GetError() << endl;
		return 1;
	}

	// start GLEW
	glewExperimental = GL_TRUE;
	GLenum error = glewInit();
	if (error != GLEW_OK)
	{
		cerr << "glewInit failed: " << glewGetErrorString(error) << endl;
		return 1;
	}

	// rainbow room!
	float vertices[] = {
	//  position          color          texture
		// front
		-5.f,  0.f, -5.f, 1.f, 0.f, 0.f, 0.0, 0.0,
		-5.f, 10.f, -5.f, 1.f, 0.f, 0.f, 0.0, 1.0,
		 5.f, 10.f, -5.f, 1.f, 0.f, 0.f, 1.0, 1.0,

		-5.f,  0.f, -5.f, 1.f, 0.f, 0.f, 0.0, 0.0,
		 5.f, 10.f, -5.f, 1.f, 0.f, 0.f, 1.0, 1.0,
		 5.f,  0.f, -5.f, 1.f, 0.f, 0.f, 1.0, 0.0,
		// left
		-5.f,  0.f,  5.f, 1.f, 0.f, 1.f, 0.0, 0.0,
		-5.f, 10.f,  5.f, 1.f, 0.f, 1.f, 0.0, 1.0,
		-5.f, 10.f, -5.f, 1.f, 0.f, 1.f, 1.0, 1.0,

		-5.f,  0.f,  5.f, 1.f, 0.f, 1.f, 0.0, 0.0,
		-5.f, 10.f, -5.f, 1.f, 0.f, 1.f, 1.0, 1.0,
		-5.f,  0.f, -5.f, 1.f, 0.f, 1.f, 1.0, 0.0,
		// right
		 5.f,  0.f, -5.f, 0.f, 1.f, 0.f, 0.0, 0.0,
		 5.f, 10.f, -5.f, 0.f, 1.f, 0.f, 0.0, 1.0,
		 5.f, 10.f,  5.f, 0.f, 1.f, 0.f, 1.0, 1.0,

		 5.f,  0.f, -5.f, 0.f, 1.f, 0.f, 0.0, 0.0,
		 5.f, 10.f,  5.f, 0.f, 1.f, 0.f, 1.0, 1.0,
		 5.f,  0.f,  5.f, 0.f, 1.f, 0.f, 1.0, 0.0,
		// back
		 5.f,  0.f,  5.f, 0.f, 0.f, 1.f, 0.0, 0.0,
		 5.f, 10.f,  5.f, 0.f, 0.f, 1.f, 0.0, 1.0,
		-5.f, 10.f,  5.f, 0.f, 0.f, 1.f, 1.0, 1.0,

		 5.f,  0.f,  5.f, 0.f, 0.f, 1.f, 0.0, 0.0,
		-5.f, 10.f,  5.f, 0.f, 0.f, 1.f, 1.0, 1.0,
		-5.f,  0.f,  5.f, 0.f, 0.f, 1.f, 1.0, 0.0,
		// top
		 5.f, 10.f,  5.f, 0.f, 0.f, 0.f, 1.0, 1.0,
		 5.f, 10.f, -5.f, 0.f, 0.f, 0.f, 1.0, 0.0,
		-5.f, 10.f, -5.f, 0.f, 0.f, 0.f, 0.0, 0.0,

		 5.f, 10.f,  5.f, 0.f, 0.f, 0.f, 1.0, 1.0,
		-5.f, 10.f, -5.f, 0.f, 0.f, 0.f, 0.0, 0.0,
		-5.f, 10.f,  5.f, 0.f, 0.f, 0.f, 0.0, 1.0,
		// bottom
		-5.f,  0.f, -5.f, 1.f, 1.f, 1.f, 0.0, 1.0,
		 5.f,  0.f, -5.f, 1.f, 1.f, 1.f, 1.0, 1.0,
		 5.f,  0.f,  5.f, 1.f, 1.f, 1.f, 1.0, 0.0,

		-5.f,  0.f, -5.f, 1.f, 1.f, 1.f, 0.0, 1.0,
		 5.f,  0.f,  5.f, 1.f, 1.f, 1.f, 1.0, 0.0,
		-5.f,  0.f,  5.f, 1.f, 1.f, 1.f, 0.0, 0.0,
	};

	// create vertex buffer
	GLuint vbo;
	glGenBuffers(1, &vbo);

	// set active
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// load data
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// compile shaders
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	load_shader(vertex_shader, "vertex.glsl");

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	load_shader(fragment_shader, "fragment.glsl");

	// create program
	GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	glLinkProgram(program);
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	GLint log_length;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
	if (log_length > 0)
	{
		char buffer[log_length];
		glGetProgramInfoLog(program, log_length, nullptr, buffer);
		cerr << buffer;
	}
	if (status == GL_FALSE)
	{
		cerr << "Failed to link program\n";
		return 1;
	}

	glUseProgram(program);

	// create vertex array and set active
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// describe position attributes
	GLint pos_attrib = glGetAttribLocation(program, "position");
	glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
	glEnableVertexAttribArray(pos_attrib);

	// describe color attributes
	GLint col_attrib = glGetAttribLocation(program, "color");
	glVertexAttribPointer(col_attrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(col_attrib);

	// describe texture attributes
	GLint tex_attrib = glGetAttribLocation(program, "texcoord");
	glVertexAttribPointer(tex_attrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(tex_attrib);

	// uniforms for transformations
	GLint model_u = glGetUniformLocation(program, "model");
	GLint view_u = glGetUniformLocation(program, "view");
	GLint proj_u = glGetUniformLocation(program, "proj");

	// model transform (currently does nothing)
	glm::mat4 model;
	model = glm::rotate(model, 0.f, glm::vec3(0.f, 0.f, 1.f));

	glUniformMatrix4fv(model_u, 1, GL_FALSE, glm::value_ptr(model));

	// view transform stuff
	glm::vec3 camera_pos(0.f, 3.f, 0.f);
	float heading = 0.f;
	float pitch = 0.f;

	glm::mat4 view = cam_mat(heading, pitch, camera_pos);

	glUniformMatrix4fv(view_u, 1, GL_FALSE, glm::value_ptr(view));

	// projection matrix
	glm::mat4 proj = glm::perspective((float)M_PI_2, (float)width / (float)height, 1.f, 1024.f);
	glUniformMatrix4fv(proj_u, 1, GL_FALSE, glm::value_ptr(proj));

	glEnable(GL_DEPTH_TEST);

	// transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	unsigned int last_time = SDL_GetTicks();
	unsigned int now;
	unsigned int frame_time;

	const Uint8* state = SDL_GetKeyboardState(nullptr);

	// main loop
	SDL_Event event;
	bool running = true;
	while (running)
	{
		// update timers
		now = SDL_GetTicks();
		frame_time = now - last_time;
		last_time = now;

		bool update_view = false;
		glm::vec3 move;

		// event handling
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				running = false;
			else if (event.type == SDL_MOUSEMOTION)
			{
				// mouse look
				update_view = true;

				heading += event.motion.xrel / -90.f;
				pitch += event.motion.yrel / 90.f;

				pitch = std::max(-(float)M_PI_2, std::min(pitch, (float)M_PI_2));
			}
		}

		// keyboard movement
		bool moved = false;
		if (state[SDL_SCANCODE_W])
		{
			move.z -= 1.f;
			moved =  true;
		}
		if (state[SDL_SCANCODE_S])
		{
			move.z += 1.f;
			moved =  true;
		}
		if (state[SDL_SCANCODE_A])
		{
			move.x -= 1.f;
			moved =  true;
		}
		if (state[SDL_SCANCODE_D])
		{
			move.x += 1.f;
			moved =  true;
		}

		if (moved)
		{
			update_view = true;

			move = glm::normalize(move) * (float)frame_time / 200.f;

			camera_pos.x += cosf(heading) * move.x + sinf(heading) * move.z;
			camera_pos.z += cosf(heading) * move.z - sinf(heading) * move.x;
		}

		// update view matrix if anything changed
		if (update_view)
		{
			glm::mat4 view = cam_mat(heading, pitch, camera_pos);
			glUniformMatrix4fv(view_u, 1, GL_FALSE, glm::value_ptr(view));
		}

		// draw
		glClearColor(0.2f, 0.2f, 0.2f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		SDL_GL_SwapWindow(window);
	}

	// clean up
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);

	// stop SDL
	SDL_Quit();

	return 0;
}
