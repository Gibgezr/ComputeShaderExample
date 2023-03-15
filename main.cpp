//#define GL_GLEXT_PROTOTYPES

#include<iostream>

#include"glad/glad.h"
#include"GLFW/glfw3.h"

#include <chrono>
#include<cassert>

GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
		assert(false);
	}
	
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__) 

const unsigned int SCREEN_WIDTH = 1024;
const unsigned int SCREEN_HEIGHT = 1024;

const unsigned short OPENGL_MAJOR_VERSION = 4;
const unsigned short OPENGL_MINOR_VERSION = 6;

bool vSync = true;

GLfloat vertices[] =
{
	-1.0f, -1.0f , 0.0f, 0.0f, 0.0f,
	-1.0f,  1.0f , 0.0f, 0.0f, 1.0f,
	 1.0f,  1.0f , 0.0f, 1.0f, 1.0f,
	 1.0f, -1.0f , 0.0f, 1.0f, 0.0f,
};

GLuint indices[] =
{
	0, 2, 1,
	0, 3, 2
};


const char* screenVertexShaderSource = R"(#version 460 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uvs;
out vec2 UVs;
void main()
{
	gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);
	UVs = uvs;
})";
const char* screenFragmentShaderSource = R"(#version 460 core
out vec4 FragColor;
uniform sampler2D screen;
in vec2 UVs;
void main()
{
	FragColor = texture(screen, UVs);
})";
const char* screenComputeShaderSource = R"(#version 460 core
uniform float timer;
layout(local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D screen;

void main()
{
	vec4 pixel;
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	
	ivec2 dims = imageSize(screen);
	float x = -(float(pixel_coords.x * 2 - dims.x) / dims.x); // transforms to [-1.0, 1.0]
	float y = -(float(pixel_coords.y * 2 - dims.y) / dims.y); // transforms to [-1.0, 1.0]
	pixel.r =  (sin(timer*x/y + x*x) + 1)/2;
	pixel.g =  (cos(timer*y/x - y*y) + 1)/2;
	pixel.b =  min(1.0, max(0, tan(x*y/(x+y)/timer))) + min(1.0, max(0, tan(y*x/(x-y)/timer))); 
	pixel.a = 1.0;

	imageStore(screen, pixel_coords, pixel);
})";

void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}

int main()
{
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_MAJOR_VERSION);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_MINOR_VERSION);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL Compute Shaders", NULL, NULL);
	if (!window)
	{
		std::cout << "Failed to create the GLFW window\n";
		glfwTerminate();
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(vSync);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize OpenGL context" << std::endl;
	}
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);


	int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		// initialize debug output 
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
	else
		assert(false);


	GLuint VAO, VBO, EBO;
	glCreateVertexArrays(1, &VAO);
	glCreateBuffers(1, &VBO);
	glCreateBuffers(1, &EBO);

	glNamedBufferData(VBO, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glNamedBufferData(EBO, sizeof(indices), indices, GL_STATIC_DRAW);

	glEnableVertexArrayAttrib(VAO, 0);
	glVertexArrayAttribBinding(VAO, 0, 0);
	glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);

	glEnableVertexArrayAttrib(VAO, 1);
	glVertexArrayAttribBinding(VAO, 1, 0);
	glVertexArrayAttribFormat(VAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat));

	glVertexArrayVertexBuffer(VAO, 0, VBO, 0, 5 * sizeof(GLfloat));
	glVertexArrayElementBuffer(VAO, EBO);

	GLuint screenTex;
	glCreateTextures(GL_TEXTURE_2D, 1, &screenTex);
	glTextureParameteri(screenTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(screenTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(screenTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(screenTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureStorage2D(screenTex, 1, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT);
	glBindImageTexture(0, screenTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	GLuint screenVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(screenVertexShader, 1, &screenVertexShaderSource, NULL);
	glCompileShader(screenVertexShader);
	GLuint screenFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(screenFragmentShader, 1, &screenFragmentShaderSource, NULL);
	glCompileShader(screenFragmentShader);

	GLuint screenShaderProgram = glCreateProgram();
	glAttachShader(screenShaderProgram, screenVertexShader);
	glAttachShader(screenShaderProgram, screenFragmentShader);
	glLinkProgram(screenShaderProgram);

	glDeleteShader(screenVertexShader);
	glDeleteShader(screenFragmentShader);

	GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
	glCheckError();
	glShaderSource(computeShader, 1, &screenComputeShaderSource, NULL);
	glCheckError();
	glCompileShader(computeShader);
	glCheckError();

	GLuint computeProgram = glCreateProgram();
	glCheckError();
	glAttachShader(computeProgram, computeShader);
	glCheckError();
	glLinkProgram(computeProgram);
	glCheckError();


	int work_grp_cnt[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);
	std::cout << "Max work groups per compute shader" << 
		" x:" << work_grp_cnt[0] <<
		" y:" << work_grp_cnt[1] <<
		" z:" << work_grp_cnt[2] << "\n";

	int work_grp_size[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);
	std::cout << "Max work group sizes" <<
		" x:" << work_grp_size[0] <<
		" y:" << work_grp_size[1] <<
		" z:" << work_grp_size[2] << "\n";

	int work_grp_inv;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
	std::cout << "Max invocations count per work group: " << work_grp_inv << "\n";

	auto currentTime = std::chrono::high_resolution_clock::now();
	auto previousTime = currentTime;
	float elapsed_time = 0;
	float timer = 0.333f;
	while (!glfwWindowShouldClose(window))
	{
		currentTime = std::chrono::high_resolution_clock::now();
		
		elapsed_time = std::chrono::duration_cast<std::chrono::duration<float>> (currentTime - previousTime).count();
		previousTime = currentTime;
		if (elapsed_time > 0.15f) elapsed_time = 0.15f;
		timer += elapsed_time;
		if (timer > 6.28318530718f) timer -= 6.28318530718f;

		glUseProgram(computeProgram);
		glCheckError();
		GLint timerHandle = glGetUniformLocation(computeProgram, "timer");
		assert(timerHandle != -1);
		glUniform1f(timerHandle, timer);
		glCheckError();
		//glUniform1fv(timerHandle, 1, &timer);
		//glUniform1f(1, timer);
		//glProgramUniform1f(computeProgram, timerHandle, timer);
		glMemoryBarrier(GL_UNIFORM_BARRIER_BIT);
		glDispatchCompute(ceil(SCREEN_WIDTH / 8), ceil(SCREEN_HEIGHT / 4), 1);
	
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		glUseProgram(screenShaderProgram);

		glBindTextureUnit(0, screenTex);
		glUniform1i(glGetUniformLocation(screenShaderProgram, "screen"), 0);		
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
}