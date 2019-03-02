// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

#include <lua.hpp>
#include <string>

#include "amodel.hpp"

std::string getString(lua_State* state, std::string variableName) {
	lua_getglobal(state, variableName.c_str());
    if(lua_isstring(state, -1)) {
        std::string value = lua_tostring(state, -1);
        lua_pop(state, 1);
        return value;
    }
    return "\0";
}

GLuint generateShader(std::string shaderText, GLuint shaderType) {
	GLuint shader = glCreateShader(shaderType);
	const char *s_str = shaderText.c_str();
    glShaderSource(shader, 1, &s_str, NULL);
    glCompileShader(shader);
	GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(success == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(shader, maxLength, NULL, &errorLog[0]);
        std::string errorMessage(begin(errorLog), end(errorLog));
        glDeleteShader(shader);

		printf("%shader\n", errorMessage.c_str());
    }
	return shader;
}

bool displayFramebuffers = false;
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
		displayFramebuffers = !displayFramebuffers;
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}

int main(void)
{
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	int width = 800;
	int height = 600;
	window = glfwCreateWindow(width, height, "Tutorial 01", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, keyCallback);

	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.02f, 0.04f, 0.25f, 0.0f);

	lua_State *lua = luaL_newstate();
	luaL_openlibs(lua);
	luaL_loadfile(lua, "config.lua");
    if (lua_pcall(lua, 0, 0, 0) != 0) {
        printf("Error: %shader", lua_tostring(lua, -1));
        return -1;
    }

    GLuint vs = generateShader(getString(lua, "vertexShader"), GL_VERTEX_SHADER);
	GLuint fs = generateShader(getString(lua, "fragmentShader"), GL_FRAGMENT_SHADER);
	GLuint ss = generateShader(getString(lua, "shadowFragmentShader"), GL_FRAGMENT_SHADER);
	GLuint cs = generateShader(getString(lua, "castShader"), GL_FRAGMENT_SHADER);
	GLuint os = generateShader(getString(lua, "colorShader"), GL_FRAGMENT_SHADER);

    GLuint shaderProgramme = glCreateProgram();
    glAttachShader(shaderProgramme, fs);
    glAttachShader(shaderProgramme, vs);
    glLinkProgram(shaderProgramme);

	GLuint shadowProgramme = glCreateProgram();
    glAttachShader(shadowProgramme, ss);
    glAttachShader(shadowProgramme, vs);
    glLinkProgram(shadowProgramme);

	GLuint castProgramme = glCreateProgram();
    glAttachShader(castProgramme, cs);
    glAttachShader(castProgramme, vs);
    glLinkProgram(castProgramme);

	GLuint colorProgramme = glCreateProgram();
    glAttachShader(colorProgramme, os);
    glAttachShader(colorProgramme, vs);
    glLinkProgram(colorProgramme);

	GLuint fbo1;
	GLuint framebufferTexture1;

	glGenFramebuffers(1, &fbo1);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo1);
	glGenTextures(1, &framebufferTexture1);

	glBindTexture(GL_TEXTURE_2D, framebufferTexture1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture1, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
	{
        printf("Error creating FrameBuffer!\n");
		return -1;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLuint fbo2;
	GLuint framebufferTexture2;

	glGenFramebuffers(1, &fbo2);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
	glGenTextures(1, &framebufferTexture2);

	glBindTexture(GL_TEXTURE_2D, framebufferTexture2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture2, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
	{
        printf("Error creating FrameBuffer!\n");
		return -1;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLint mvpMatrixUniform = glGetUniformLocation(shaderProgramme, "mvpMatrix");
	GLint colorUniform = glGetUniformLocation(colorProgramme, "color");
	GLuint offsetUniform = glGetUniformLocation(castProgramme, "offsetSpace");
	float accum = 0.0f;

	AModel plane(getString(lua, "plane"));
	std::vector<AMesh>* planeMeshes = plane.getMeshes();
	AModel monkey(getString(lua, "monkey"));

	glm::vec3 cameraPos = glm::vec3(0.0f, -10.0f, 0.0f);  
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
	glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f); 
	glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
	glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);
	glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

	float fov = 45.0f;
	glm::mat4 projection = glm::perspective(glm::radians(fov), (float) width / (float) height, 0.1f, 100.0f);
	glm::mat4 modelMonkey = glm::mat4(1.0f);

	glm::mat4 nearViewProjection = projection * view;
	cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
	cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	cameraDirection = glm::normalize(cameraPos - cameraTarget);
	up = glm::vec3(0.0f, 1.0f, 0.0f); 
	cameraRight = glm::normalize(glm::cross(up, cameraDirection));
	cameraUp = glm::cross(cameraDirection, cameraRight);
	view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
	glm::mat4 farViewProject = projection * view;

	glm::mat4 modelPlane = glm::mat4(1.0f);
	modelPlane = glm::scale(modelPlane, glm::vec3(4.0f, 4.0f, 1.0f));
	modelPlane = glm::rotate(modelPlane, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	double currentTime = glfwGetTime();
	double finishFrameTime = 0.0;
	float deltaTime = 0.0f;
	char title[128];
	float angle = 90.0f;

	AMesh* monkeyMesh = &monkey.getMeshes()->at(0);

	glEnable(GL_SCISSOR_TEST);
	glActiveTexture(GL_TEXTURE0);
	glActiveTexture(GL_TEXTURE1);
	do{
		modelMonkey = glm::rotate(modelMonkey, glm::radians(10.0f) * deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
		glActiveTexture(GL_TEXTURE0);

		glBindFramebuffer(GL_FRAMEBUFFER, fbo1);
		glViewport(0, 0, width, height);
		glScissor(0, 0, width,height);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glUseProgram(shadowProgramme);
		glUniformMatrix4fv (glGetUniformLocation(shadowProgramme, "mvpMatrix"), 1, GL_FALSE, glm::value_ptr(farViewProject * modelMonkey));
        glBindTexture(GL_TEXTURE_2D, monkeyMesh->textures[0].id);
		glUniform1i(glGetUniformLocation(shadowProgramme, "texture_diffuse1"), 0); 
		monkeyMesh->Draw(shadowProgramme);
		glBindTexture(GL_TEXTURE_2D, 0);

		if(displayFramebuffers) 
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, width, height);
			glScissor(0, 0, width,height);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			glUseProgram(shadowProgramme);
			glUniformMatrix4fv (glGetUniformLocation(shadowProgramme, "mvpMatrix"), 1, GL_FALSE, glm::value_ptr(farViewProject * modelMonkey));
			glBindTexture(GL_TEXTURE_2D, monkeyMesh->textures[0].id);
			glUniform1i(glGetUniformLocation(shadowProgramme, "texture_diffuse1"), 0); 
			monkeyMesh->Draw(shadowProgramme);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
		glViewport(0, 0, width, height);
		glScissor(0, 0, width,height);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(colorProgramme);
		glUniformMatrix4fv(glGetUniformLocation(colorProgramme, "mvpMatrix"), 1, GL_FALSE, glm::value_ptr(farViewProject * modelMonkey));
		glUniform4f(glGetUniformLocation(colorProgramme, "color"), 0.55f, 0.05f, 0.015f, 1.0f);
		monkeyMesh->Draw(colorProgramme);

		if(displayFramebuffers) 
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(width, 0, width, height);
			glScissor(width, 0, width, height);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(colorProgramme);
			glUniformMatrix4fv(glGetUniformLocation(colorProgramme, "mvpMatrix"), 1, GL_FALSE, glm::value_ptr(farViewProject * modelMonkey));
			glUniform4f(glGetUniformLocation(colorProgramme, "color"), 0.55f, 0.05f, 0.015f, 1.0f);
			monkeyMesh->Draw(colorProgramme);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		if(displayFramebuffers) 
		{
			glViewport(0, height, width, height);
			glScissor(0, height, width, height);
		}
		else {
			glViewport(0, 0, width * 2, height * 2);
			glScissor(0, 0, width * 2, height * 2);
		}
		
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(castProgramme);
		accum += 0.25f * deltaTime;
		glUniform1f(offsetUniform, 20.0f + abs(sin(accum)) * 300.0f);
		glUniformMatrix4fv (glGetUniformLocation(castProgramme, "mvpMatrix"), 1, GL_FALSE, glm::value_ptr(farViewProject * modelPlane));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, framebufferTexture1);
		glUniform1i(glGetUniformLocation(castProgramme, "texture_diffuse1"), 0); 
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, framebufferTexture2);
		glUniform1i(glGetUniformLocation(castProgramme, "texture_diffuse2"), 1); 
		for(size_t i = 0; i < planeMeshes->size(); i++)
		{
			planeMeshes->at(i).Draw(castProgramme);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();

		finishFrameTime = glfwGetTime();
		deltaTime = static_cast<float>(finishFrameTime - currentTime);
		currentTime = finishFrameTime;
		sprintf(title, "FPS: %4.4f", 1.0f / (float) deltaTime);
		glfwSetWindowTitle(window, title);
	}
	while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

	glDeleteFramebuffers(1, &fbo1);
	glDeleteFramebuffers(1, &fbo2);
	glfwTerminate();
	return 0;
}