#pragma comment(linker, "/NODEFAULTLIB:MSVCRT")

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include <cmath>
#include <cfenv>
using namespace std;

#define ERROR_INIT_GLFW 1
#define ERROR_INIT_GLEW 2

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm/gtx/transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// element values
#define FIRE 0.5f
#define WATER 1.0f
#define OTHER 0.0F

// VARIABLES
GLFWwindow*		window;
int				windowWidth = 640;
int				windowHeight = 480;
bool			running = true;
glm::mat4		proj_matrix;
float           aspect = (float)windowWidth / (float)windowHeight;
float			fovy = 45.0f;
bool			keyStatus[1024];
GLfloat			deltaTime = 0.0f;
GLfloat			lastTime = 0.0f;

// FPS camera variables
GLfloat			yaw = -90.0f;	// init pointing to inside
GLfloat			pitch = 0.0f;	// start centered
GLfloat			lastX = (GLfloat)windowWidth / 2.0f;	// start middle screen
GLfloat			lastY = (GLfloat)windowHeight / 2.0f;	// start middle screen
bool			firstMouse = true;

typedef struct cam_s {
	glm::vec3		eye = glm::vec3(0.0f, 0.0f, 5.0f);
	glm::vec3		target = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3		up = glm::vec3(0.0f, 1.0f, 0.0f);
} Camera;
Camera cam;

// OBJ Variables
typedef struct obj_model_s {
	std::vector <glm::vec3> vertices;
	std::vector <glm::vec2> uvs;
	std::vector <glm::vec3> normals;
	GLuint*		texture;
	GLuint      program;
	GLuint      vao;
	GLuint      buffer[2];
	GLint       mv_location;
	GLint       proj_location;
	GLint		tex_location;
} ObjModel;

ObjModel mountainModel, pondModel, cloudModel, boatModel, fireModel, moonModel;

glm::vec3		mountainScale = glm::vec3(0.2f, 0.2f, 0.2f);
glm::vec3		mountainAngle = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3		*mountainPositions;
glm::vec3		*mountainRotations;

glm::vec3		pondScale = glm::vec3(0.2f, 0.2f, 0.2f);;
glm::vec3		pondAngle = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3		*pondPositions;
glm::vec3		*pondRotations;

glm::vec3		*cloudScales;
glm::vec3		cloudAngle = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3		cloudRef = glm::vec3(0.0f, 1.5f, 0.0f);
glm::vec3		*cloudPositions;
glm::vec3		*cloudRotations;
int cloudCount = 9;

glm::vec3		boatScale = glm::vec3(0.1f, 0.1f, 0.1f);
glm::vec3		boatAngle = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3		*boatPositions;
glm::vec3		*boatRotations;

glm::vec3		fireScale = glm::vec3(0.12f, 0.12f, 0.12f);
glm::vec3		fireAngle = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3		*firePositions;
glm::vec3		*fireRotations;

glm::vec3		moonScale = glm::vec3(0.12f, 0.12f, 0.12f);
glm::vec3		moonAngle = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3		*moonPositions;
glm::vec3		*moonRotations;

// Light
glm::vec3		*lightPositions;	// fire and moon
glm::vec3		fireLightPos = glm::vec3(0.35f, 0.15f, 0.0f);
glm::vec3		moonLightPos = glm::vec3(-0.05f, 1.7f, 1.95f);
glm::vec3		*ia;
GLfloat			ka = 1.0f;
glm::vec3		*id;
GLfloat			kd = 1.0f;
glm::vec3		is = glm::vec3(1.0f, 1.0f, 1.0f);
GLfloat			ks = 0.01f;

void errorCallbackGLFW(int error, const char* description);
void hintsGLFW();
void endProgram();
void render(GLfloat currentTime);
void update(GLfloat currentTime);
void setupRender();
void startup();
void onResizeCallback(GLFWwindow* window, int w, int h);
void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void onMouseMoveCallback(GLFWwindow* window, double x, double y);
void onMouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset);
void debugGL();
static void APIENTRY openGLDebugCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const GLvoid* userParam);
string readShader(string name);
void checkErrorShader(GLuint shader);
bool readObj(const char* filepath, ObjModel *obj);
bool readTexture(string filepath, GLuint textureName);

int main()
{
	if (!glfwInit()) {							// Checking for GLFW
		cout << "Could not initialise GLFW...";
		return ERROR_INIT_GLEW;
	}

	glfwSetErrorCallback(errorCallbackGLFW);	// Setup a function to catch and display all GLFW errors.

	hintsGLFW();								// Setup glfw with various hints.		

												// Start a window using GLFW
	string title = "My OpenGL Application";

	// Fullscreen
	const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	windowWidth = mode->width; windowHeight = mode->height;
	window = glfwCreateWindow(windowWidth, windowHeight, title.c_str(), NULL, NULL);
	if (!window) {								// Window or OpenGL context creation failed
		cout << "Could not initialise GLFW...";
		endProgram();
		return ERROR_INIT_GLFW;
	}

	glfwMakeContextCurrent(window);				// making the OpenGL context current

												// Start GLEW (note: always initialise GLEW after creating your window context.)
	glewExperimental = GL_TRUE;					// hack: catching them all - forcing newest debug callback (glDebugMessageCallback)
	GLenum errGLEW = glewInit();
	if (GLEW_OK != errGLEW) {					// Problems starting GLEW?
		cout << "Could not initialise GLEW...";
		endProgram();
		return ERROR_INIT_GLEW;
	}

	debugGL();									// Setup callback to catch openGL errors.	

												// Setup all the message loop callbacks.
	glfwSetWindowSizeCallback(window, onResizeCallback);		// Set callback for resize
	glfwSetKeyCallback(window, onKeyCallback);					// Set Callback for keys
	glfwSetMouseButtonCallback(window, onMouseButtonCallback);	// Set callback for mouse click
	glfwSetCursorPosCallback(window, onMouseMoveCallback);		// Set callback for mouse move
	glfwSetScrollCallback(window, onMouseWheelCallback);		// Set callback for mouse wheel.
																//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);	// Set mouse cursor. Fullscreen
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	// Set mouse cursor FPS.

	setupRender();								// setup some render variables.
	startup();									// Setup all necessary information for startup (aka. load texture, shaders, models, etc).

	do {										// run until the window is closed
		GLfloat currentTime = (GLfloat)glfwGetTime();		// retrieve timelapse
		deltaTime = currentTime - lastTime;		// Calculate delta time
		lastTime = currentTime;					// Save for next frame calculations.
		glfwPollEvents();						// poll callbacks
		update(currentTime);					// update (physics, animation, structures, etc)
		render(currentTime);					// call render function.

		glfwSwapBuffers(window);				// swap buffers (avoid flickering and tearing)

		running &= (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE);	// exit if escape key pressed
		running &= (glfwWindowShouldClose(window) != GL_TRUE);
	} while (running);

	endProgram();			// Close and clean everything up...

	cout << "\nPress any key to continue...\n";
	cin.ignore(); cin.get(); // delay closing console to read debugging errors.

	return 0;
}

void errorCallbackGLFW(int error, const char* description) {
	cout << "Error GLFW: " << description << "\n";
}

void hintsGLFW() {
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);			// Create context in debug mode - for debug message callback
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
}

void endProgram() {
	glfwMakeContextCurrent(window);		// destroys window handler
	glfwTerminate();	// destroys all windows and releases resources.

	// tidy heap memory
	delete[] mountainModel.texture;
	delete[] pondModel.texture;
	delete[] boatModel.texture;
	
	delete[] mountainPositions;
	delete[] mountainRotations;
	delete[] pondPositions;
	delete[] pondRotations;
	delete[] boatPositions;
	delete[] boatRotations;
	delete[] cloudPositions;
	delete[] cloudRotations;
	delete[] cloudScales;
	delete[] moonPositions;
	delete[] moonRotations;
	delete[] lightPositions;
}

void setupRender() {
	glfwSwapInterval(1);	// Ony render when synced (V SYNC)

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 0);
	glfwWindowHint(GLFW_STEREO, GL_FALSE);
}

void startup() {
	// Load main object model and shaders

	//--------------Mountain Model---------------------
	mountainModel.program = glCreateProgram();

	string vs_text = readShader("vs_main.glsl"); static const char* vs_source = vs_text.c_str();
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vs_source, NULL);
	glCompileShader(vs);
	checkErrorShader(vs);
	glAttachShader(mountainModel.program, vs);

	string fs_text = readShader("fs_main.glsl"); static const char* fs_source = fs_text.c_str();
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fs_source, NULL);
	glCompileShader(fs);
	checkErrorShader(fs);
	glAttachShader(mountainModel.program, fs);

	glLinkProgram(mountainModel.program);
	glUseProgram(mountainModel.program);

	readObj("mountain_dent.obj", &mountainModel);

	glCreateBuffers(3, mountainModel.buffer);		// Create a new buffer

	// Store the vertices
	glNamedBufferStorage(mountainModel.buffer[0], mountainModel.vertices.size() * sizeof(glm::vec3), &mountainModel.vertices[0], GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, mountainModel.buffer[0]);	// Bind Buffer

	// Store the texture coordinates
	glNamedBufferStorage(mountainModel.buffer[1], mountainModel.uvs.size() * sizeof(glm::vec2), &mountainModel.uvs[0], GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, mountainModel.buffer[1]);	// Bind Buffer

	// Store the normal Vectors
	glNamedBufferStorage(mountainModel.buffer[2], mountainModel.normals.size() * sizeof(glm::vec3), &mountainModel.normals[0], GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, mountainModel.buffer[3]);	// Bind Buffer

	glCreateVertexArrays(1, &mountainModel.vao);		// Create Vertex Array Object

	// Bind vertex position buffer to the vao and format
	glVertexArrayVertexBuffer(mountainModel.vao, 0, mountainModel.buffer[0], 0, sizeof(GLfloat) * 3);
	glVertexArrayAttribFormat(mountainModel.vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(mountainModel.vao, 0);	// Enable Vertex Array Attribute

	// Bind texture coordinate buffer to the vao and format
	glVertexArrayVertexBuffer(mountainModel.vao, 1, mountainModel.buffer[1], 0, sizeof(GLfloat) * 2);
	glVertexArrayAttribFormat(mountainModel.vao, 1, 2, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(mountainModel.vao, 1);	// Enable Vertex Array Attribute

	// Bind the normals buffer to the vao and format
	glVertexArrayVertexBuffer(mountainModel.vao, 2, mountainModel.buffer[2], 0, sizeof(GLfloat) * 3);
	glVertexArrayAttribFormat(mountainModel.vao, 2, 3, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(mountainModel.vao, 2);	// Enable Vertex Array Attribute

	glBindVertexArray(mountainModel.vao);				// Bind VertexArrayObject

	mountainModel.mv_location = glGetUniformLocation(mountainModel.program, "mv_matrix");
	mountainModel.proj_location = glGetUniformLocation(mountainModel.program, "proj_matrix");
	mountainModel.tex_location = glGetUniformLocation(mountainModel.program, "tex");

	//--------------Pond Model---------------------------
	pondModel.program = glCreateProgram();

	glAttachShader(pondModel.program, vs);

	glAttachShader(pondModel.program, fs);

	glLinkProgram(pondModel.program);
	glUseProgram(pondModel.program);

	readObj("water.obj", &pondModel);

	glCreateBuffers(3, pondModel.buffer);		// Create a new buffer
													// Store the vertices
	glNamedBufferStorage(pondModel.buffer[0], pondModel.vertices.size() * sizeof(glm::vec3), &pondModel.vertices[0], GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, pondModel.buffer[0]);	// Bind Buffer

															// Store the texture coordinates
	glNamedBufferStorage(pondModel.buffer[1], pondModel.uvs.size() * sizeof(glm::vec2), &pondModel.uvs[0], GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, pondModel.buffer[1]);	// Bind Buffer

															// Store the normal Vectors
	glNamedBufferStorage(pondModel.buffer[2], pondModel.normals.size() * sizeof(glm::vec3), &pondModel.normals[0], GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, pondModel.buffer[3]);	// Bind Buffer

	glCreateVertexArrays(1, &pondModel.vao);		// Create Vertex Array Object

														// Bind vertex position buffer to the vao and format
	glVertexArrayVertexBuffer(pondModel.vao, 0, pondModel.buffer[0], 0, sizeof(GLfloat) * 3);
	glVertexArrayAttribFormat(pondModel.vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(pondModel.vao, 0);	// Enable Vertex Array Attribute

														// Bind texture coordinate buffer to the vao and format
	glVertexArrayVertexBuffer(pondModel.vao, 1, pondModel.buffer[1], 0, sizeof(GLfloat) * 2);
	glVertexArrayAttribFormat(pondModel.vao, 1, 2, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(pondModel.vao, 1);	// Enable Vertex Array Attribute

														// Bind the normals buffer to the vao and format
	glVertexArrayVertexBuffer(pondModel.vao, 2, pondModel.buffer[2], 0, sizeof(GLfloat) * 3);
	glVertexArrayAttribFormat(pondModel.vao, 2, 3, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(pondModel.vao, 2);	// Enable Vertex Array Attribute

	glBindVertexArray(pondModel.vao);				// Bind VertexArrayObject

	pondModel.mv_location = glGetUniformLocation(pondModel.program, "mv_matrix");
	pondModel.proj_location = glGetUniformLocation(pondModel.program, "proj_matrix");
	pondModel.tex_location = glGetUniformLocation(pondModel.program, "tex");

	//--------------Boat Model---------------------------
	boatModel.program = glCreateProgram();

	glAttachShader(boatModel.program, vs);

	glAttachShader(boatModel.program, fs);

	glLinkProgram(boatModel.program);
	glUseProgram(boatModel.program);

	readObj("boat.obj", &boatModel);

	glCreateBuffers(3, boatModel.buffer);		// Create a new buffer
												// Store the vertices
	glNamedBufferStorage(boatModel.buffer[0], boatModel.vertices.size() * sizeof(glm::vec3), &boatModel.vertices[0], GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, boatModel.buffer[0]);	// Bind Buffer

														// Store the texture coordinates
	glNamedBufferStorage(boatModel.buffer[1], boatModel.uvs.size() * sizeof(glm::vec2), &boatModel.uvs[0], GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, boatModel.buffer[1]);	// Bind Buffer

														// Store the normal Vectors
	glNamedBufferStorage(boatModel.buffer[2], boatModel.normals.size() * sizeof(glm::vec3), &boatModel.normals[0], GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, boatModel.buffer[3]);	// Bind Buffer

	glCreateVertexArrays(1, &boatModel.vao);		// Create Vertex Array Object

													// Bind vertex position buffer to the vao and format
	glVertexArrayVertexBuffer(boatModel.vao, 0, boatModel.buffer[0], 0, sizeof(GLfloat) * 3);
	glVertexArrayAttribFormat(boatModel.vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(boatModel.vao, 0);	// Enable Vertex Array Attribute

													// Bind texture coordinate buffer to the vao and format
	glVertexArrayVertexBuffer(boatModel.vao, 1, boatModel.buffer[1], 0, sizeof(GLfloat) * 2);
	glVertexArrayAttribFormat(boatModel.vao, 1, 2, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(boatModel.vao, 1);	// Enable Vertex Array Attribute

													// Bind the normals buffer to the vao and format
	glVertexArrayVertexBuffer(boatModel.vao, 2, boatModel.buffer[2], 0, sizeof(GLfloat) * 3);
	glVertexArrayAttribFormat(boatModel.vao, 2, 3, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(boatModel.vao, 2);	// Enable Vertex Array Attribute

	glBindVertexArray(boatModel.vao);				// Bind VertexArrayObject

	boatModel.mv_location = glGetUniformLocation(boatModel.program, "mv_matrix");
	boatModel.proj_location = glGetUniformLocation(boatModel.program, "proj_matrix");
	boatModel.tex_location = glGetUniformLocation(boatModel.program, "tex");

	//--------------fire Model--------------------------
	fireModel.program = glCreateProgram();

	glAttachShader(fireModel.program, vs);

	glAttachShader(fireModel.program, fs);

	glLinkProgram(fireModel.program);

	readObj("fire.obj", &fireModel);

	glCreateBuffers(3, fireModel.buffer);		// Create a new buffer

	// Store the vertices
	glNamedBufferStorage(fireModel.buffer[0], fireModel.vertices.size() * sizeof(glm::vec3), &fireModel.vertices[0], GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, fireModel.buffer[0]);	// Bind Buffer

	// Store the texture coordinates
	glNamedBufferStorage(fireModel.buffer[1], fireModel.uvs.size() * sizeof(glm::vec2), &fireModel.uvs[0], GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, fireModel.buffer[1]);	// Bind Buffer

	// Store the normal Vectors
	glNamedBufferStorage(fireModel.buffer[2], fireModel.normals.size() * sizeof(glm::vec3), &fireModel.normals[0], GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, fireModel.buffer[3]);	// Bind Buffer

	glCreateVertexArrays(1, &fireModel.vao);		// Create Vertex Array Object

	// Bind vertex position buffer to the vao and format
	glVertexArrayVertexBuffer(fireModel.vao, 0, fireModel.buffer[0], 0, sizeof(GLfloat) * 3);
	glVertexArrayAttribFormat(fireModel.vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(fireModel.vao, 0);	// Enable Vertex Array Attribute

	// Bind texture coordinate buffer to the vao and format
	glVertexArrayVertexBuffer(fireModel.vao, 1, fireModel.buffer[1], 0, sizeof(GLfloat) * 2);
	glVertexArrayAttribFormat(fireModel.vao, 1, 2, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(fireModel.vao, 1);	// Enable Vertex Array Attribute

	// Bind the normals buffer to the vao and format
	glVertexArrayVertexBuffer(fireModel.vao, 2, fireModel.buffer[2], 0, sizeof(GLfloat) * 3);
	glVertexArrayAttribFormat(fireModel.vao, 2, 3, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(fireModel.vao, 2);	// Enable Vertex Array Attribute

	glBindVertexArray(fireModel.vao);				// Bind VertexArrayObject

	fireModel.mv_location = glGetUniformLocation(fireModel.program, "mv_matrix");
	fireModel.proj_location = glGetUniformLocation(fireModel.program, "proj_matrix");
	fireModel.tex_location = glGetUniformLocation(fireModel.program, "tex");

	//--------------moon Model--------------------------
	moonModel.program = glCreateProgram();

	glAttachShader(moonModel.program, vs);

	glAttachShader(moonModel.program, fs);

	glLinkProgram(moonModel.program);

	readObj("moon.obj", &moonModel);

	glCreateBuffers(3, moonModel.buffer);		// Create a new buffer

	// Store the vertices
	glNamedBufferStorage(moonModel.buffer[0], moonModel.vertices.size() * sizeof(glm::vec3), &moonModel.vertices[0], GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, moonModel.buffer[0]);	// Bind Buffer

	// Store the texture coordinates
	glNamedBufferStorage(moonModel.buffer[1], moonModel.uvs.size() * sizeof(glm::vec2), &moonModel.uvs[0], GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, moonModel.buffer[1]);	// Bind Buffer

	// Store the normal Vectors
	glNamedBufferStorage(moonModel.buffer[2], moonModel.normals.size() * sizeof(glm::vec3), &moonModel.normals[0], GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, moonModel.buffer[3]);	// Bind Buffer

	glCreateVertexArrays(1, &moonModel.vao);		// Create Vertex Array Object

	// Bind vertex position buffer to the vao and format
	glVertexArrayVertexBuffer(moonModel.vao, 0, moonModel.buffer[0], 0, sizeof(GLfloat) * 3);
	glVertexArrayAttribFormat(moonModel.vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(moonModel.vao, 0);	// Enable Vertex Array Attribute

	// Bind texture coordinate buffer to the vao and format
	glVertexArrayVertexBuffer(moonModel.vao, 1, moonModel.buffer[1], 0, sizeof(GLfloat) * 2);
	glVertexArrayAttribFormat(moonModel.vao, 1, 2, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(moonModel.vao, 1);	// Enable Vertex Array Attribute

	// Bind the normals buffer to the vao and format
	glVertexArrayVertexBuffer(moonModel.vao, 2, moonModel.buffer[2], 0, sizeof(GLfloat) * 3);
	glVertexArrayAttribFormat(moonModel.vao, 2, 3, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(moonModel.vao, 2);	// Enable Vertex Array Attribute

	glBindVertexArray(moonModel.vao);				// Bind VertexArrayObject

	moonModel.mv_location = glGetUniformLocation(moonModel.program, "mv_matrix");
	moonModel.proj_location = glGetUniformLocation(moonModel.program, "proj_matrix");
	moonModel.tex_location = glGetUniformLocation(moonModel.program, "tex");


	// --------------Cloud Model---------------------
	cloudModel.program = glCreateProgram();

	vs_text = readShader("vs_cloud.glsl"); vs_source = vs_text.c_str();
	vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vs_source, NULL);
	glCompileShader(vs);
	checkErrorShader(vs);
	glAttachShader(cloudModel.program, vs);

	glAttachShader(cloudModel.program, fs);


	glLinkProgram(cloudModel.program);
	glUseProgram(cloudModel.program);

	readObj("cloud.obj", &cloudModel);

	glCreateBuffers(3, cloudModel.buffer);		// Create a new buffer

	// Store the vertices
	glNamedBufferStorage(cloudModel.buffer[0], cloudModel.vertices.size() * sizeof(glm::vec3), &cloudModel.vertices[0], GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, cloudModel.buffer[0]);	// Bind Buffer

	// Store the texture coordinates
	glNamedBufferStorage(cloudModel.buffer[1], cloudModel.uvs.size() * sizeof(glm::vec2), &cloudModel.uvs[0], GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, cloudModel.buffer[1]);	// Bind Buffer

	// Store the normal Vectors
	glNamedBufferStorage(cloudModel.buffer[2], cloudModel.normals.size() * sizeof(glm::vec3), &cloudModel.normals[0], GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, cloudModel.buffer[3]);	// Bind Buffer

	glCreateVertexArrays(1, &cloudModel.vao);		// Create Vertex Array Object

	// Bind vertex position buffer to the vao and format
	glVertexArrayVertexBuffer(cloudModel.vao, 0, cloudModel.buffer[0], 0, sizeof(GLfloat) * 3);
	glVertexArrayAttribFormat(cloudModel.vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(cloudModel.vao, 0);	// Enable Vertex Array Attribute

	// Bind texture coordinate buffer to the vao and format
	glVertexArrayVertexBuffer(cloudModel.vao, 1, cloudModel.buffer[1], 0, sizeof(GLfloat) * 2);
	glVertexArrayAttribFormat(cloudModel.vao, 1, 2, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(cloudModel.vao, 1);	// Enable Vertex Array Attribute

	// Bind the normals buffer to the vao and format
	glVertexArrayVertexBuffer(cloudModel.vao, 2, cloudModel.buffer[2], 0, sizeof(GLfloat) * 3);
	glVertexArrayAttribFormat(cloudModel.vao, 2, 3, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(cloudModel.vao, 2);	// Enable Vertex Array Attribute

	glBindVertexArray(cloudModel.vao);				// Bind VertexArrayObject

	cloudModel.mv_location = glGetUniformLocation(cloudModel.program, "mv_matrix");
	cloudModel.proj_location = glGetUniformLocation(cloudModel.program, "proj_matrix");
	cloudModel.tex_location = glGetUniformLocation(cloudModel.program, "tex");

	//-----------setting model positions---------------
	mountainPositions = new glm::vec3(0.0f, 0.0f, 0.0f);
	mountainRotations = new glm::vec3(0.0f, 0.0f, 0.0f);

	pondPositions = new glm::vec3(0.0f, 0.0f, 0.0f);
	pondRotations = new glm::vec3(0.0f, 0.0f, 0.0f);

	boatPositions = new glm::vec3(-0.4f, 0.02f, -1.25f);
	boatRotations = new glm::vec3(0.0f, -0.5f, 0.0f);

	firePositions = new glm::vec3(0.35, 0.05, 0.0f);;
	fireRotations = new glm::vec3(0.0f, 0.0f, 0.0f);
	
	moonPositions = new glm::vec3(moonLightPos.x, moonLightPos.y, moonLightPos.z);
	moonRotations = new glm::vec3(0.0f, 0.0f, 0.0f);

	cloudPositions = new glm::vec3[cloudCount];
	cloudPositions[0] = glm::vec3(-6.5f, 1.5f, -6.5f);
	cloudPositions[1] = glm::vec3(8.0f, -1.0f, -8.0f);
	cloudPositions[2] = glm::vec3(4.5f, 1.5f, 9.0f);
	cloudPositions[3] = glm::vec3(-1.0f, 2.0f, -1.5f);
	cloudPositions[4] = glm::vec3(7.0f, 1.5f, -3.0f);
	cloudPositions[5] = glm::vec3(3.0f, -1.0f, 7.5f);
	cloudPositions[6] = glm::vec3(-3.5f, -1.5f, 7.0f);
	cloudPositions[7] = glm::vec3(7.0f, 2.0f, -4.5f);
	cloudPositions[8] = glm::vec3(-5.0f, -1.5f, 5.0f);

	cloudScales = new glm::vec3[3];
	cloudScales[0] = glm::vec3(0.2f, 0.2f, 0.2f);
	cloudScales[1] = glm::vec3(0.12f, 0.12f, 0.12f);
	cloudScales[2] = glm::vec3(0.08f, 0.08f, 0.08f);

	lightPositions = new glm::vec3[2];
	lightPositions[0] = fireLightPos;
	lightPositions[1] = moonLightPos;

	id = new glm::vec3[2];
	id[0] = glm::vec3(0.93f, 0.75f, 0.32f);
	id[1] = glm::vec3(0.975f, 0.975f, 0.975f);

	ia = new glm::vec3[2];
	ia[0] = glm::vec3(0.8f, 0.8f, 0.8f);
	ia[1] = glm::vec3(0.3f, 0.3f, 0.3f);

	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Calculate proj_matrix for the first time.
	aspect = (float)windowWidth / (float)windowHeight;
	proj_matrix = glm::perspective(glm::radians(fovy), aspect, 0.1f, 1000.0f);
}

void update(GLfloat currentTime) {
	// calculate movement
	GLfloat cameraSpeed = 1.0f * deltaTime;
	if (keyStatus[GLFW_KEY_W]) cam.eye += cameraSpeed * cam.target;
	if (keyStatus[GLFW_KEY_S]) cam.eye -= cameraSpeed * cam.target;
	if (keyStatus[GLFW_KEY_A]) cam.eye -= glm::normalize(glm::cross(cam.target, cam.up)) * cameraSpeed;
	if (keyStatus[GLFW_KEY_D]) cam.eye += glm::normalize(glm::cross(cam.target, cam.up)) * cameraSpeed;					
}

void send_to_fs(ObjModel * obj, Camera cam, float shininess, float is_element) {
	glUseProgram(obj->program);
	glBindVertexArray(obj->vao);
	glUniformMatrix4fv(obj->proj_location, 1, GL_FALSE, &proj_matrix[0][0]);

	for (int l = 0; l < 2; l++) {
		glUniform4f(glGetUniformLocation(obj->program,
			("lights[" + to_string(l) + "].lightPosition").c_str()),
			lightPositions[l].x, lightPositions[l].y, lightPositions[l].z, 1.0f);
		glUniform4f(glGetUniformLocation(obj->program,
			("lights[" + to_string(l) + "].id").c_str()),
			id[l].r, id[l].g, id[l].b, 1.0f);

		glUniform4f(glGetUniformLocation(obj->program,
			("lights[" + to_string(l) + "].ia").c_str()),
			ia[l].r, ia[l].g, ia[l].b, 1.0f);
	}

	glUniform4f(glGetUniformLocation(obj->program, "viewPosition"), cam.eye.x, cam.eye.y, cam.eye.z, 1.0);
	glUniform4f(glGetUniformLocation(obj->program, "lightSpotDir"), 0.0f, 1.0f, 0.0f, 0.0f);
	glUniform1f(glGetUniformLocation(obj->program, "lightSpotCutOff"), glm::cos(glm::radians(75.0f)));
	glUniform1f(glGetUniformLocation(obj->program, "lightSpotOuterCutOff"), glm::cos(glm::radians(85.0f)));
	glUniform1f(glGetUniformLocation(obj->program, "ka"), ka);
	glUniform1f(glGetUniformLocation(obj->program, "kd"), 1.0f);
	glUniform4f(glGetUniformLocation(obj->program, "is"), is.r, is.g, is.b, 1.0);
	glUniform1f(glGetUniformLocation(obj->program, "ks"), 1.0f);
	glUniform1f(glGetUniformLocation(obj->program, "shininess"), shininess);
	glUniform1f(glGetUniformLocation(obj->program, "is_element"), is_element);	// variable to test if object is fire or water
	glUniform1f(glGetUniformLocation(obj->program, "lightConstant"), 1.0f);
	glUniform1f(glGetUniformLocation(obj->program, "lightLinear"), 0.7f);
	glUniform1f(glGetUniformLocation(obj->program, "lightQuadratic"), 1.8f);

	// Bind textures and samplers
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, obj->texture[0]);
	glUniform1i(obj->tex_location, 0);
}

void send_to_vs_and_draw(ObjModel* obj, glm::mat4 modelMatrix, glm::mat4 viewMatrix) {
	glUniformMatrix4fv(glGetUniformLocation(obj->program, "model_matrix"), 1, GL_FALSE, &modelMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(obj->program, "view_matrix"), 1, GL_FALSE, &viewMatrix[0][0]);

	glDrawArrays(GL_TRIANGLES, 0, obj->vertices.size());
}

void draw_instances(ObjModel* obj, glm::vec3 reference, glm::mat4 viewMatrix, int inst_count, glm::vec3 angles, GLfloat* positions, const GLfloat* scales, float shininess) {
	glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), reference);
	GLint angle_location = glGetUniformLocation(obj->program, "angles");
	GLint positions_loc = glGetUniformLocation(obj->program, "locations");
	GLint scales_loc = glGetUniformLocation(obj->program, "scales");

	for (int l = 0; l < 2; l++) {
		glUniform4f(glGetUniformLocation(obj->program,
			("lights[" + to_string(l) + "].lightPosition").c_str()),
			lightPositions[l].x, lightPositions[l].y, lightPositions[l].z, 1.0f);
		glUniform4f(glGetUniformLocation(obj->program,
			("lights[" + to_string(l) + "].id").c_str()),
			id[l].r, id[l].g, id[l].b, 1.0f);

		glUniform4f(glGetUniformLocation(obj->program,
			("lights[" + to_string(l) + "].ia").c_str()),
			ia[l].r, ia[l].g, ia[l].b, 1.0f);
	}

	glUniform4f(glGetUniformLocation(obj->program, "viewPosition"), cam.eye.x, cam.eye.y, cam.eye.z, 1.0);
	glUniform4f(glGetUniformLocation(obj->program, "lightSpotDir"), 0.0f, 1.0f, 0.0f, 0.0f);
	glUniform1f(glGetUniformLocation(obj->program, "lightSpotCutOff"), glm::cos(glm::radians(85.0f)));
	glUniform1f(glGetUniformLocation(obj->program, "lightSpotOuterCutOff"), glm::cos(glm::radians(90.0f)));
	glUniform1f(glGetUniformLocation(obj->program, "ka"), ka);
	glUniform1f(glGetUniformLocation(obj->program, "kd"), 1.0f);
	glUniform4f(glGetUniformLocation(obj->program, "is"), is.r, is.g, is.b, 1.0);
	glUniform1f(glGetUniformLocation(obj->program, "ks"), 1.0f);
	glUniform1f(glGetUniformLocation(obj->program, "shininess"), shininess);
	glUniform1f(glGetUniformLocation(obj->program, "lightConstant"), 1.0f);
	glUniform1f(glGetUniformLocation(obj->program, "lightLinear"), 0.7f);
	glUniform1f(glGetUniformLocation(obj->program, "lightQuadratic"), 1.8f);
	
	// rotations calculated in shader
	glUniform3f(angle_location, angles.x, angles.y, angles.z);
	glUniform3fv(positions_loc, inst_count, positions);
	glUniform3fv(scales_loc, 3, scales);
	glUniformMatrix4fv(obj->mv_location, 1, GL_FALSE, &modelMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(obj->program, "model_matrix"), 1, GL_FALSE, &modelMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(obj->program, "view_matrix"), 1, GL_FALSE, &viewMatrix[0][0]);
	glDrawArraysInstanced(GL_TRIANGLES, 0, obj->vertices.size(), inst_count);

}

void render(GLfloat currentTime) {
	glViewport(0, 0, windowWidth, windowHeight);

	// Clear colour buffer
	glm::vec4 backgroundColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f); glClearBufferfv(GL_COLOR, 0, &backgroundColor[0]);

	// Clear Deep buffer
	static const GLfloat one = 1.0f; glClearBufferfv(GL_DEPTH, 0, &one);

	// Enable blend
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// fire light flicker
	float flicker_speed = 10.0f;
	float flicker = sin(currentTime * flicker_speed);
	flicker = ((0.8f - 0.75f) * ((flicker + 1) / (2))) + 0.75f;
	ia[0].x = flicker;
	ia[0].y = flicker;
	ia[0].z = flicker;

	// camera
	glm::mat4 viewMatrix = glm::lookAt(cam.eye, cam.target + cam.eye , cam.up);

	// ----------draw main model------------
	send_to_fs(&mountainModel, cam, 5.0f, OTHER);

	glm::mat4 mountainMatrix = glm::translate(glm::mat4(1.0f), mountainPositions[0]);
	mountainMatrix = glm::rotate(mountainMatrix, mountainAngle.x + mountainRotations[0].x, glm::vec3(1.0f, 0.0f, 0.0f));
	mountainMatrix = glm::rotate(mountainMatrix, mountainAngle.y + mountainRotations[0].y, glm::vec3(0.0f, 1.0f, 0.0f));
	mountainMatrix = glm::rotate(mountainMatrix, mountainAngle.z + mountainRotations[0].z, glm::vec3(0.0f, 0.0f, 1.0f));
	mountainMatrix = glm::scale(mountainMatrix, mountainScale);
	glm::mat4 mv_matrix = viewMatrix * mountainMatrix;
	
	send_to_vs_and_draw(&mountainModel, mountainMatrix, viewMatrix);

	//------------draw pond------------
	send_to_fs(&pondModel, cam, 32.0f, WATER);

	glm::mat4 pondMatrix = glm::translate(glm::mat4(1.0f), pondPositions[0]);
	pondMatrix = glm::rotate(pondMatrix, pondAngle.x + pondRotations[0].x, glm::vec3(1.0f, 0.0f, 0.0f));
	pondMatrix = glm::rotate(pondMatrix, pondAngle.y + pondRotations[0].y, glm::vec3(0.0f, 1.0f, 0.0f));
	pondMatrix = glm::rotate(pondMatrix, pondAngle.z + pondRotations[0].z, glm::vec3(0.0f, 0.0f, 1.0f));
	pondMatrix = glm::scale(pondMatrix, pondScale);
	mv_matrix = viewMatrix * pondMatrix;

	send_to_vs_and_draw(&pondModel, pondMatrix, viewMatrix);

	// ----------draw clouds-----------
	send_to_fs(&cloudModel, cam, 5.0f, OTHER);

	draw_instances(&cloudModel, cloudRef, viewMatrix, cloudCount, cloudAngle, 
		(GLfloat*)cloudPositions, (GLfloat*)cloudScales, 5.0f);

	//------------draw boat------------
	send_to_fs(&boatModel, cam, 15.0f, OTHER);

	glm::mat4 boatMatrix = glm::translate(glm::mat4(1.0f), boatPositions[0]);
	
	GLfloat radius = 0.1f;
	GLfloat boatFloat = sin(currentTime) * radius;
	
	boatMatrix = glm::rotate(boatMatrix, boatAngle.x + boatFloat + boatRotations[0].x, glm::vec3(1.0f, 0.0f, 0.0f));
	boatMatrix = glm::rotate(boatMatrix, boatAngle.y + boatRotations[0].y, glm::vec3(0.0f, 1.0f, 0.0f));
	boatMatrix = glm::rotate(boatMatrix, boatAngle.z + boatRotations[0].z, glm::vec3(0.0f, 0.0f, 1.0f));
	boatMatrix = glm::scale(boatMatrix, boatScale);
	mv_matrix = viewMatrix * boatMatrix;
	
	send_to_vs_and_draw(&boatModel, boatMatrix, viewMatrix);

	//------------draw fire------------
	send_to_fs(&fireModel, cam, 32.0f, FIRE);

	glm::mat4 fireMatrix = glm::translate(glm::mat4(1.0f), firePositions[0]);
	fireMatrix = glm::rotate(fireMatrix, fireAngle.x + fireRotations[0].x, glm::vec3(1.0f, 0.0f, 0.0f));
	fireMatrix = glm::rotate(fireMatrix, fireAngle.y + fireRotations[0].y, glm::vec3(0.0f, 1.0f, 0.0f));
	fireMatrix = glm::rotate(fireMatrix, fireAngle.z + fireRotations[0].z, glm::vec3(0.0f, 0.0f, 1.0f));
	fireMatrix = glm::scale(fireMatrix, fireScale);
	mv_matrix = viewMatrix * fireMatrix;

	send_to_vs_and_draw(&fireModel, fireMatrix, viewMatrix);

	//------------draw moon------------
	send_to_fs(&moonModel, cam, 10.0f, OTHER);

	glm::mat4 moonMatrix = glm::translate(glm::mat4(1.0f), moonPositions[0]);
	moonMatrix = glm::rotate(moonMatrix, moonAngle.x + moonRotations[0].x, glm::vec3(1.0f, 0.0f, 0.0f));
	moonMatrix = glm::rotate(moonMatrix, moonAngle.y + currentTime + moonRotations[0].y, glm::vec3(0.0f, 1.0f, 0.0f));
	moonMatrix = glm::rotate(moonMatrix, moonAngle.z + moonRotations[0].z, glm::vec3(0.0f, 0.0f, 1.0f));
	moonMatrix = glm::scale(moonMatrix, moonScale);
	mv_matrix = viewMatrix * moonMatrix;

	send_to_vs_and_draw(&moonModel, moonMatrix, viewMatrix);
}

void onResizeCallback(GLFWwindow* window, int w, int h) {
	windowWidth = w;
	windowHeight = h;

	aspect = (float)w / (float)h;
	proj_matrix = glm::perspective(glm::radians(fovy), aspect, 0.1f, 1000.0f);
}

void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) keyStatus[key] = true;
	else if (action == GLFW_RELEASE) keyStatus[key] = false;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {

}

void onMouseMoveCallback(GLFWwindow* window, double x, double y) {
	int mouseX = static_cast<int>(x);
	int mouseY = static_cast<int>(y);

	if (firstMouse) {
		lastX = (GLfloat)mouseX; lastY = (GLfloat)mouseY; firstMouse = false;
	}

	GLfloat xoffset = mouseX - lastX;
	GLfloat yoffset = lastY - mouseY; // Reversed
	lastX = (GLfloat)mouseX; lastY = (GLfloat)mouseY;

	GLfloat sensitivity = 0.05f;
	xoffset *= sensitivity; yoffset *= sensitivity;

	yaw += xoffset; pitch += yoffset;

	// check for pitch out of bounds otherwise screen gets flipped
	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	cam.target = glm::normalize(front);

}

static void onMouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset) {
	int yoffsetInt = static_cast<int>(yoffset);

	fovy += yoffsetInt;
	if (fovy >= 1.0f && fovy <= 45.0f) fovy -= (GLfloat)yoffset;
	if (fovy <= 1.0f) fovy = 1.0f;
	if (fovy >= 45.0f) fovy = 45.0f;
	proj_matrix = glm::perspective(glm::radians(fovy), aspect, 0.1f, 1000.0f);
}

void debugGL() {
	//Output some debugging information
	cout << "VENDOR: " << (char *)glGetString(GL_VENDOR) << endl;
	cout << "VERSION: " << (char *)glGetString(GL_VERSION) << endl;
	cout << "RENDERER: " << (char *)glGetString(GL_RENDERER) << endl;

	// Enable Opengl Debug
	//glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback((GLDEBUGPROC)openGLDebugCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
}

static void APIENTRY openGLDebugCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const GLvoid* userParam) {

	cout << "---------------------opengl-callback------------" << endl;
	cout << "Message: " << message << endl;
	cout << "type: ";
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		cout << "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		cout << "DEPRECATED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		cout << "UNDEFINED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		cout << "PORTABILITY";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		cout << "PERFORMANCE";
		break;
	case GL_DEBUG_TYPE_OTHER:
		cout << "OTHER";
		break;
	}
	cout << " --- ";

	cout << "id: " << id << " --- ";
	cout << "severity: ";
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW:
		cout << "LOW";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		cout << "MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		cout << "HIGH";
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		cout << "NOTIFICATION";
	}
	cout << endl;
	cout << "-----------------------------------------" << endl;
}

string readShader(string name) {
	string vs_text;
	std::ifstream vs_file(name);

	string vs_line;
	if (vs_file.is_open()) {

		while (getline(vs_file, vs_line)) {
			vs_text += vs_line;
			vs_text += '\n';
		}
		vs_file.close();
	}
	return vs_text;
}

void  checkErrorShader(GLuint shader) {
	// Get log lenght
	GLint maxLength;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

	// Init a string for it
	std::vector<GLchar> errorLog(maxLength);

	if (maxLength > 1) {
		// Get the log file
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

		cout << "--------------Shader compilation error-------------\n";
		cout << errorLog.data();
	}

}

bool readObj(const char* filepath, ObjModel *obj) {
	cout << "Loading model " << filepath << "\n";

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<string> materials, textures;
	std::vector<glm::vec3> tmp_vertices;
	std::vector<glm::vec2> tmp_uvs;
	std::vector<glm::vec3> tmp_normals;

	FILE * file = fopen(filepath, "r");
	if (file == NULL) {
		cout << "\nError while opening " << filepath << "\n";
		return false;
	}

	// read the file until the end:
	while (1) {
		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // end of file, exit loop
		else {
			if (strcmp(lineHeader, "v") == 0) { // vertices
				glm::vec3 vertex;
				fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				tmp_vertices.push_back(vertex);
			}
			else if (strcmp(lineHeader, "vt") == 0) { // texture coordinate of one vertex
				glm::vec2 uv;
				fscanf(file, "%f %f\n", &uv.x, &uv.y);
				tmp_uvs.push_back(uv);
			}
			else if (strcmp(lineHeader, "vn") == 0) { // normal of a vertex
				glm::vec3 normal;
				fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
				tmp_normals.push_back(normal);
			}
			else if (strcmp(lineHeader, "f") == 0) {
				unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
				int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
				if (matches != 9) {
					cout << "\nFile can't be read by parser\n";
					return false;
				}
				for (int i = 0; i < 3; i++) {
					vertexIndices.push_back(vertexIndex[i]);
					uvIndices.push_back(uvIndex[i]);
					normalIndices.push_back(normalIndex[i]);
				}
			}
			else if (strcmp(lineHeader, "mtllib") == 0) {
				char mat[32];
				fscanf(file, "%s", mat);
				materials.push_back(mat);
			}
		}
	}
	// for each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {
		obj->vertices.push_back(tmp_vertices[vertexIndices[i] - 1]);
		obj->normals.push_back(tmp_normals[normalIndices[i] - 1]);
		obj->uvs.push_back(tmp_uvs[uvIndices[i] - 1]);
	}
	fclose(file);

	// load materials
	for (unsigned int i = 0; i < materials.size(); i++) {
		FILE * file = fopen(materials[i].c_str(), "r");
		if (file == NULL) {
			cout << "\nImpossible to open the mtl file " << materials[i] << "\n";
			return false;
		}

		while (1) {
			char lineHeader[128];
			int res = fscanf(file, "%s", lineHeader);
			if (res == EOF)
				break; // end of file, exit loop
			else {
				if (strcmp(lineHeader, "map_Kd") == 0) {
					char texfile[32];
					fscanf(file, "%s", texfile);
					textures.push_back(texfile);
				}
			}
		}
		fclose(file);
	}

	obj->texture = new GLuint[textures.size()];
	glCreateTextures(GL_TEXTURE_2D, textures.size(), obj->texture);

	for (unsigned int i = 0; i < textures.size(); i++) {
		readTexture(textures[i], obj->texture[i]);
	}

	cout << "\nDone loading " << filepath << "\n";
	return true;
}

bool readTexture(string filepath, GLuint textureName) {
	// Flip images as OpenGL expects 0.0 coordinates on the y-axis to be at the bottom of the image.
	stbi_set_flip_vertically_on_load(true);

	// Load image Information.
	int iWidth, iHeight, iChannels;
	unsigned char *iData = stbi_load(filepath.c_str(), &iWidth, &iHeight, &iChannels, 0);
	if (iData == NULL) {
		cout << "\nError loading texture " << filepath << "\n";
		return false;
	}

	// Load and create a texture 
	glBindTexture(GL_TEXTURE_2D, textureName); // All upcoming operations now have effect on this texture object

	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, iWidth, iHeight);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, iWidth, iHeight, GL_RGBA, GL_UNSIGNED_BYTE, iData);


	// This only works for 2D Textures...
	// Set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);// Unbind texture when done, so we won't accidentily mess up our texture.

	stbi_image_free(iData);
	cout << "\nDone loading " << filepath << " texture\n";
	return true;
}