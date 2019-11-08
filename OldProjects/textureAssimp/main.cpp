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

	int width = 600;
	int height = 600;
	window = glfwCreateWindow(width, height, "Tutorial 01", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.02f, 0.04f, 0.25f, 0.0f);

	lua_State *lua = luaL_newstate();
	luaL_openlibs(lua);
	luaL_loadfile(lua, "config.lua");
    if (lua_pcall(lua, 0, 0, 0) != 0) {
        printf("Error: %s", lua_tostring(lua, -1));
        return -1;
    }

    std::string vertexShader = getString(lua, "vertexShader");
	std::string fragmentShader = getString(lua, "fragmentShader");

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	const char *v_str = vertexShader.c_str();
    glShaderSource(vs, 1, &v_str, NULL);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	const char *f_str = fragmentShader.c_str();
    glShaderSource(fs, 1, &f_str, NULL);
    glCompileShader(fs);

    GLuint shaderProgramme = glCreateProgram();
    glAttachShader(shaderProgramme, fs);
    glAttachShader(shaderProgramme, vs);
    glLinkProgram(shaderProgramme);

	GLint mvpMatrixUniform = glGetUniformLocation(shaderProgramme, "mvpMatrix");

	AModel amodel(getString(lua, "model"));

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);  
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f); 
	glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
	glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

	glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

	float fov = 45.0f;
	glm::mat4 projection = glm::perspective(glm::radians(fov), (float) width / (float) height, 0.1f, 100.0f);

	glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	glm::mat4 viewProjection = projection * view;
	glm::mat4 modelViewProject = viewProjection * model;
	double currentTime = glfwGetTime();
	double finishFrameTime = 0.0;
	float deltaTime = 0.0f;
	do{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		model = glm::rotate(model, glm::radians(10.0f) * deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv (mvpMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjection * model));
		amodel.Draw(shaderProgramme);
		
		glfwSwapBuffers(window);
		glfwPollEvents();

		finishFrameTime = glfwGetTime();
		deltaTime = static_cast<float>(finishFrameTime - currentTime);
		currentTime = finishFrameTime;
	}
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose(window) == 0 );

	glfwTerminate();
	return 0;
}