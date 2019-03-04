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
#include <algorithm>
#include <vector>

#include "amodel.hpp"

#include <stb_image.h>

float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

std::string getString(lua_State* state, std::string variableName) {
	lua_getglobal(state, variableName.c_str());
    if(lua_isstring(state, -1)) {
        std::string value = lua_tostring(state, -1);
        lua_pop(state, 1);
        return value;
    }
    return "\0";
}

float getNumber(lua_State* state, std::string variableName) {
	lua_getglobal(state, variableName.c_str());
    if(lua_isnumber(state, -1)) {
        float value = lua_tonumber(state, -1);
        lua_pop(state, 1);
        return value;
    }
    return 0.0f;
}

float getThreshold(lua_State* state, float threshold, float deltaTime) {
	float newThreshold = 0.0f;

	lua_getglobal(state, "updateThreshold");
	lua_pushnumber(state, threshold);
    lua_pushnumber(state, deltaTime);
    lua_call(state, 2, 1);

	if(lua_isnumber(state, -1)) {
		newThreshold = lua_tonumber(state, -1);
		lua_pop(state, 1);
	} else {
		printf("Error reading updateThreshold method. Stack top: %d.", lua_gettop(state));
	}

	return newThreshold;
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

glm::mat4 skyView = glm::mat4(1.0);
float deltaTime = 0.0;
float moveForce = 40.0f;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		skyView = glm::rotate(skyView, glm::radians(-moveForce) * deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
    }
	if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		skyView = glm::rotate(skyView, glm::radians( moveForce) * deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
    }
	if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		skyView = glm::rotate(skyView, glm::radians(-moveForce) * deltaTime, glm::vec3(1.0f, 0.0f, 0.0f));
    }
	if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		skyView = glm::rotate(skyView, glm::radians( moveForce) * deltaTime, glm::vec3(1.0f, 0.0f, 0.0f));
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
	window = glfwCreateWindow(width, height, "Cubemap Reflection", NULL, NULL);
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
	glDepthFunc(GL_LESS);
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
	GLuint svs = generateShader(getString(lua, "skyboxVertexShader"), GL_VERTEX_SHADER);
	GLuint sfs = generateShader(getString(lua, "skyboxFragmentShader"), GL_FRAGMENT_SHADER);

    GLuint shaderProgramme = glCreateProgram();
    glAttachShader(shaderProgramme, vs);
    glAttachShader(shaderProgramme, fs);
    glLinkProgram(shaderProgramme);

	GLuint skyboxProgramme = glCreateProgram();
    glAttachShader(skyboxProgramme, svs);
    glAttachShader(skyboxProgramme, sfs);
    glLinkProgram(skyboxProgramme);

	GLuint cubemapTextureID;
	glGenTextures(1, &cubemapTextureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID);
	int skyboxWidth, skyboxHeight, nrChannels;
	unsigned char *data;

	std::vector<std::string> faces
    {
        "desertsky_ft.tga",
        "desertsky_bc.tga",
        "desertsky_up.tga",
        "desertsky_dn.tga",
        "desertsky_rt.tga",
        "desertsky_lf.tga"
    };

	for(GLuint i = 0; i < faces.size(); i++)
	{
		data = stbi_load(faces[i].c_str(), &skyboxWidth, &skyboxHeight, &nrChannels, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, skyboxWidth, skyboxHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	GLuint skyVpMatrixUniform = glGetUniformLocation(skyboxProgramme, "vpMatrix");
	GLuint skyboxUniform = glGetUniformLocation(skyboxProgramme, "skybox");

	GLuint modelMatrixUniform = glGetUniformLocation(shaderProgramme, "model");
	GLuint vpMatrixUniform = glGetUniformLocation(shaderProgramme, "viewProjection");
	GLuint cameraPosUniform = glGetUniformLocation(shaderProgramme, "cameraPos");
	GLuint modelSkyboxUniform = glGetUniformLocation(shaderProgramme, "skybox");
	GLuint refractiveIndexUniform = glGetUniformLocation(shaderProgramme, "refractiveIndex");

	AModel model(getString(lua, "model"));

	unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);  
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f); 
	glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
	glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);
	glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
	skyView = glm::lookAt(cameraPos, cameraTarget, cameraUp);

	float fov = 45.0f;
	glm::mat4 projection = glm::perspective(glm::radians(fov), (float) width / (float) height, 0.1f, 100.0f);
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	glm::mat4 viewProjectionMatrix = projection * view;
	glm::mat4 skyViewProjectionMatrix = projection * glm::mat4(glm::mat3(view));

	double currentTime = glfwGetTime();
	double finishFrameTime = 0.0;
	deltaTime = 0.0f;
	char title[128];

	glm::vec3 baseColor = glm::vec3(0.75f, 0.75f, 0.75f);
	do{
		modelMatrix = glm::rotate(modelMatrix, glm::radians(10.0f) * deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));		
		skyViewProjectionMatrix = projection * glm::mat4(glm::mat3(skyView));

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shaderProgramme);
		glUniformMatrix4fv (modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv (vpMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
		glUniform3f(cameraPosUniform, cameraPos.x, cameraPos.y, cameraPos.z);
		glUniform1f(refractiveIndexUniform, getNumber(lua, "refractiveIndex"));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID);
		glUniform1i(modelSkyboxUniform, 0);
		std::for_each(model.getMeshes()->begin(), model.getMeshes()->end(), [shaderProgramme](AMesh &mesh){ 
			mesh.Draw(shaderProgramme);
		});
		glBindVertexArray(0);

		glDepthFunc(GL_LEQUAL); 
		glUseProgram(skyboxProgramme);
		glUniformMatrix4fv (skyVpMatrixUniform, 1, GL_FALSE, glm::value_ptr(skyViewProjectionMatrix));

		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID);
		glUniform1i(skyboxUniform, 0);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); 

		glfwSwapBuffers(window);
		glfwPollEvents();

		finishFrameTime = glfwGetTime();
		deltaTime = static_cast<float>(finishFrameTime - currentTime);
		currentTime = finishFrameTime;
		sprintf(title, "FPS: %4.2f", 1.0f / (float) deltaTime);
		glfwSetWindowTitle(window, title);
	}
	while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

	glfwTerminate();
	lua_close(lua);
	return 0;
}