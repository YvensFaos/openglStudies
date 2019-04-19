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
#include "acamera.hpp"
#include "alight.hpp"
#include "ashader.hpp"

#include <luahandler.hpp>
#include <stb_image.h>

#include "askybox.hpp"
#include "Utils/arenderquad.hpp"

#define printMatrix(matrix) {int _i = -1; \
							printf("%2.2f %2.2f %2.2f %2.2f\n", matrix[++_i][0], matrix[_i][1], matrix[_i][2], matrix[_i][3]); \
							printf("%2.2f %2.2f %2.2f %2.2f\n", matrix[++_i][0], matrix[_i][1], matrix[_i][2], matrix[_i][3]); \
							printf("%2.2f %2.2f %2.2f %2.2f\n", matrix[++_i][0], matrix[_i][1], matrix[_i][2], matrix[_i][3]); \
							printf("%2.2f %2.2f %2.2f %2.2f\n", matrix[++_i][0], matrix[_i][1], matrix[_i][2], matrix[_i][3]); \
							printf("----- ----- ----- -----\n");}

void updateLightWithLua(LuaHandler* luaHandler, ALight* alight, float deltaTime, float accumulator) 
{
	luaHandler->getFunction("updateLight");
	luaHandler->pushNumber(deltaTime);
	luaHandler->pushNumber(accumulator);
	glm::vec3 position = alight->getPosition();
	luaHandler->pushNumber(position.x);
	luaHandler->pushNumber(position.y);
	luaHandler->pushNumber(position.z);
	glm::vec3 direction = alight->getDirection();
	luaHandler->pushNumber(direction.x);
	luaHandler->pushNumber(direction.y);
	luaHandler->pushNumber(direction.z);
	glm::vec3 up = alight->getUp();
	luaHandler->pushNumber(up.x);
	luaHandler->pushNumber(up.y);
	luaHandler->pushNumber(up.z);
	glm::vec4 color = alight->getColor();
	luaHandler->pushNumber(color.x);
	luaHandler->pushNumber(color.y);
	luaHandler->pushNumber(color.z);
	luaHandler->pushNumber(color.w);
	luaHandler->pushNumber(alight->getIntensity());

	luaHandler->callFunctionFromStack(16, 14);

	position.x = luaHandler->popNumber();
	position.y = luaHandler->popNumber();
	position.z = luaHandler->popNumber();
	alight->setPosition(position);
	direction.x = luaHandler->popNumber();
	direction.y = luaHandler->popNumber();
	direction.z = luaHandler->popNumber();
	alight->setDirection(direction);
	up.x = luaHandler->popNumber();
	up.y = luaHandler->popNumber();
	up.z = luaHandler->popNumber();
	alight->setUp(up);
	color.x = luaHandler->popNumber();
	color.y = luaHandler->popNumber();
	color.z = luaHandler->popNumber();
	color.w = luaHandler->popNumber();
	alight->setColor(color);
	alight->setIntensity(luaHandler->popNumber());
}

glm::mat4 skyView = glm::mat4(1.0);
float deltaTime = 0.0f;
float moveForce = 20.0f;
double accumulator = 0.0f;
float savedAcc = 0.0f;
bool paused = false;
float near_plane;
float far_plane;
float projectionDimension;
ACamera acamera(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f));

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	float shiftModPower = 1.0f;
	if (mods == GLFW_MOD_SHIFT)
	{
		shiftModPower = 2.5f;
	} else if (mods == GLFW_MOD_CONTROL)
	{
		shiftModPower = 0.5f;
	}
    if ((key == GLFW_KEY_LEFT || key == GLFW_KEY_A) && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		acamera.MoveSideway(shiftModPower * -moveForce * deltaTime);
    }
	if ((key == GLFW_KEY_RIGHT || key == GLFW_KEY_D) && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		acamera.MoveSideway(shiftModPower * moveForce * deltaTime);
    }
	if ((key == GLFW_KEY_UP || key == GLFW_KEY_W) && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		acamera.MoveForward(shiftModPower * moveForce * deltaTime);
    }
	if ((key == GLFW_KEY_DOWN || key == GLFW_KEY_S) && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		acamera.MoveForward(shiftModPower *-moveForce * deltaTime);
    }
	if (key == GLFW_KEY_Z && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		acamera.Zoom(5.0f);
    }
	if (key == GLFW_KEY_X && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		acamera.Zoom(-5.0f);
    }
	if (key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		acamera.RotateWithMouse(10, 0);
    }
	if (key == GLFW_KEY_E && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		acamera.RotateWithMouse(-10, 0);
    }

	if (key == GLFW_KEY_1 && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		near_plane += 0.1;
    }
	if (key == GLFW_KEY_2 && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		near_plane -= 0.1;
    }
	if (key == GLFW_KEY_3 && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		far_plane += 0.1;
    }
	if (key == GLFW_KEY_4 && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		far_plane -= 0.1;
    }
	if (key == GLFW_KEY_5 && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		projectionDimension += 1.25;
    }
	if (key == GLFW_KEY_6 && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		projectionDimension -= 1.25;
    }
	if (key == GLFW_KEY_P && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		printf("Printing Info: \n");
		printf("---------------\n");
		printf("Acc: %9f\n", accumulator);
		printf("---------------\n");
		printf("Near: %9f\n", near_plane);
		printf("Far:  %9f\n",  far_plane);
		printf("Proj: %9f\n",  projectionDimension);
		printf("---------------\n");
    }
	if (key == GLFW_KEY_SPACE && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
			if(paused) {
				accumulator = savedAcc;
				paused = false;
			} else {
				savedAcc = accumulator;
				paused = true;
			}
    }
}

int width = 800;
int height = 600;
float lastX = width / 2.0f;
float lastY = height / 2.0f;
float mouseSensitivity = 5.0;
bool firstMouse = true;
bool mouseIsClickingLeft = false;

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;
	
	if(mouseIsClickingLeft)
	{
		acamera.RotateWithMouse(xoffset / mouseSensitivity, yoffset / mouseSensitivity);
	}
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	mouseIsClickingLeft = false;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		mouseIsClickingLeft = true;
	}
}

int main(void)
{
	if(!glfwInit())
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

	window = glfwCreateWindow(width, height, "Shadow Mapping", NULL, NULL);
	if( window == NULL ) 
	{
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);

	if (glewInit() != GLEW_OK) 
	{
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
	glEnable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.02f, 0.04f, 0.25f, 0.0f);

	LuaHandler luaHandler;
	luaHandler.openFile("config.lua");
	
  GLuint vs =  AShader::generateShader(luaHandler.getGlobalString("vertexShader"), GL_VERTEX_SHADER);
	GLuint fs =  AShader::generateShader(luaHandler.getGlobalString("fragmentShader"), GL_FRAGMENT_SHADER);
	GLuint svs = AShader::generateShader(luaHandler.getGlobalString("skyboxVertexShader"), GL_VERTEX_SHADER);
	GLuint sfs = AShader::generateShader(luaHandler.getGlobalString("skyboxFragmentShader"), GL_FRAGMENT_SHADER);
	GLuint hvs = AShader::generateShader(luaHandler.getGlobalString("shadowVertexShader"), GL_VERTEX_SHADER);
	GLuint hfs = AShader::generateShader(luaHandler.getGlobalString("shadowFragmentShader"), GL_FRAGMENT_SHADER);

  GLuint shaderProgramme = AShader::generateProgram(vs, fs);
	GLuint skyboxProgramme  = AShader::generateProgram(svs, sfs);
	GLuint shadowProgramme = AShader::generateProgram(hvs, hfs);

	std::vector<std::string> faces{
        "desertsky_ft.tga",
        "desertsky_bc.tga",
        "desertsky_up.tga",
        "desertsky_dn.tga",
        "desertsky_rt.tga",
        "desertsky_lf.tga"
    };
	ASkybox askybox(faces);

	GLuint skyVpMatrixUniform = glGetUniformLocation(skyboxProgramme, "vpMatrix");
	GLuint skyboxUniform = glGetUniformLocation(skyboxProgramme, "skybox");

	GLuint modelMatrixUniform = glGetUniformLocation(shaderProgramme, "model");
	GLuint vpMatrixUniform = glGetUniformLocation(shaderProgramme, "viewProjection");
	GLuint lightMatrixUniform = glGetUniformLocation(shaderProgramme, "lightViewProjection");
	GLuint shadowMapUniform = glGetUniformLocation(shaderProgramme, "shadowMap");

	GLuint    lightPositionUniform = glGetUniformLocation(shaderProgramme, "sceneLight.position");
	GLuint   lightDirectionUniform = glGetUniformLocation(shaderProgramme, "sceneLight.direction");
	GLuint       lightColorUniform = glGetUniformLocation(shaderProgramme, "sceneLight.color");
	GLuint   lightIntensityUniform = glGetUniformLocation(shaderProgramme, "sceneLight.intensity");
	GLuint lightDirectionalUniform = glGetUniformLocation(shaderProgramme, "sceneLight.directional");

	GLuint shadowModelMatrixUniform = glGetUniformLocation(shadowProgramme, "model");
	GLuint shadowLightMatrixUniform = glGetUniformLocation(shadowProgramme, "lightViewProjection");

	ARenderQuad debugQuad(luaHandler.getGlobalString("debugQuadVertexShader"), luaHandler.getGlobalString("debugQuadFragShader"));

	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	int modelsCount = luaHandler.getGlobalInteger("modelsCount");
	luaHandler.loadTable("models");
	std::vector<AModel> models;
	glm::vec3 translateTo;
	glm::vec3 scaleTo;
	glm::vec3 rotateTo;
	for(int i = 1; i <= modelsCount; i++)
	{
		luaHandler.getTableFromTable(i);
		AModel model = AModel(luaHandler.getStringFromTable("file"));

		luaHandler.getTableFromTable("pos");
		translateTo.x = luaHandler.getNumberFromTable(1);
		translateTo.y = luaHandler.getNumberFromTable(2);
		translateTo.z = luaHandler.getNumberFromTable(3);
		model.translate(translateTo);
		luaHandler.popTable();

		luaHandler.getTableFromTable("rot");
		rotateTo.x = luaHandler.getNumberFromTable(1);
		rotateTo.y = luaHandler.getNumberFromTable(2);
		rotateTo.z = luaHandler.getNumberFromTable(3);
		model.rotate(rotateTo);
		luaHandler.popTable();

		luaHandler.getTableFromTable("sca");
		scaleTo.x = luaHandler.getNumberFromTable(1);
		scaleTo.y = luaHandler.getNumberFromTable(2);
		scaleTo.z = luaHandler.getNumberFromTable(3);
		model.scale(scaleTo);
		luaHandler.popTable();

		models.emplace_back(model);
		luaHandler.popTable();
	}
	luaHandler.popTable();

	luaHandler.loadTable("plane");	
	AModel plane = AModel(luaHandler.getStringFromTable("file"));
	luaHandler.getTableFromTable("pos");
	translateTo.x = luaHandler.getNumberFromTable(1);
	translateTo.y = luaHandler.getNumberFromTable(2);
	translateTo.z = luaHandler.getNumberFromTable(3);
	plane.translate(translateTo);
	luaHandler.popTable();

	luaHandler.getTableFromTable("rot");
	rotateTo.x = luaHandler.getNumberFromTable(1);
	rotateTo.y = luaHandler.getNumberFromTable(2);
	rotateTo.z = luaHandler.getNumberFromTable(3);
	plane.rotate(rotateTo);
	luaHandler.popTable();

	luaHandler.getTableFromTable("sca");
	scaleTo.x = luaHandler.getNumberFromTable(1);
	scaleTo.y = luaHandler.getNumberFromTable(2);
	scaleTo.z = luaHandler.getNumberFromTable(3);
	plane.scale(scaleTo);
	luaHandler.popTable();
	luaHandler.popTable();

	glm::vec3 lightPositionValue;
	glm::vec3 lightDirectionValue;
	glm::vec3 lightUpValue;
	glm::vec4 lightColorValue;
	float lightIntensityValue;
	bool lightDirectionalValue;

	luaHandler.loadTable("light");
	luaHandler.getTableFromTable("pos");
	lightPositionValue.x = luaHandler.getNumberFromTable(1);
	lightPositionValue.y = luaHandler.getNumberFromTable(2);
	lightPositionValue.z = luaHandler.getNumberFromTable(3);
	luaHandler.popTable();

	luaHandler.getTableFromTable("dir");
	lightDirectionValue.x = luaHandler.getNumberFromTable(1);
	lightDirectionValue.y = luaHandler.getNumberFromTable(2);
	lightDirectionValue.z = luaHandler.getNumberFromTable(3);
	luaHandler.popTable();

	luaHandler.getTableFromTable("up");
	lightUpValue.x = luaHandler.getNumberFromTable(1);
	lightUpValue.y = luaHandler.getNumberFromTable(2);
	lightUpValue.z = luaHandler.getNumberFromTable(3);
	luaHandler.popTable();

	luaHandler.getTableFromTable("col");
	lightColorValue.x = luaHandler.getNumberFromTable(1);
	lightColorValue.y = luaHandler.getNumberFromTable(2);
	lightColorValue.z = luaHandler.getNumberFromTable(3);
	lightColorValue.w = luaHandler.getNumberFromTable(4);
	luaHandler.popTable();

	lightIntensityValue = luaHandler.getNumberFromTable("intensity");
	lightDirectionalValue = luaHandler.getBoolFromTable("directional");
	luaHandler.popTable();

	ALight alight(lightPositionValue, lightDirectionValue, lightColorValue, lightIntensityValue, lightDirectionalValue);
	alight.setUp(lightUpValue);

	glm::mat4 projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, 0.1f, 1000.0f);
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	glm::mat4 view = acamera.getView();
	glm::mat4 viewProjectionMatrix = projection * view;
	glm::mat4 skyViewProjectionMatrix = projection * glm::mat4(glm::mat3(view));

	double currentTime = glfwGetTime();
	double finishFrameTime = 0.0;
	deltaTime = 0.0f;
	char title[128];

	const std::vector<AMesh>* pointer;
	glm::vec3 lightPosition = alight.getPosition();
	glm::vec3 lightDirection = alight.getDirection();
	glm::vec3 lightUp = alight.getUp();
	glm::vec4 lightColor = alight.getColor();
	float lightIntensity = alight.getIntensity();
	bool lightDirectional = alight.getDirectional();

	near_plane = luaHandler.getGlobalNumber("nearPlane");
	far_plane = luaHandler.getGlobalNumber("farPlane");
	projectionDimension = luaHandler.getGlobalNumber("projectionDimension");
	glm::mat4 lightProjection = glm::ortho(-projectionDimension, projectionDimension, -projectionDimension, projectionDimension, near_plane, far_plane);
	glm::mat4 lightView = glm::lookAt(lightPosition, lightPosition + lightDirection, lightUp);
	glm::mat4 lightMatrix = lightProjection * lightView;

	glActiveTexture(GL_TEXTURE0);
	do
	{
		if(!paused) 
		{
			updateLightWithLua(&luaHandler, &alight, deltaTime, accumulator);
		}

		lightPosition = alight.getPosition();
		lightDirection = alight.getDirection();
		lightUp = alight.getUp();
		lightColor = alight.getColor();
		lightIntensity = alight.getIntensity();

		glm::mat4 lightProjection = glm::ortho(-projectionDimension, projectionDimension, -projectionDimension, projectionDimension, near_plane, far_plane);
		lightView = glm::lookAt(lightPosition, lightPosition + lightDirection, lightUp);
		lightMatrix = lightProjection * lightView;

		projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, 0.1f, 1000.0f);
		skyViewProjectionMatrix = projection * glm::mat4(glm::mat3(skyView));
		viewProjectionMatrix = projection * acamera.getView();

#pragma region SHADOW BUFFER
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		glUseProgram(shadowProgramme);
		glUniformMatrix4fv (shadowLightMatrixUniform, 1, GL_FALSE, glm::value_ptr(lightMatrix));

		for(auto model : models) 
		{
			pointer = model.getMeshes();

			auto begin = pointer->begin();
			auto end = pointer->end();
			auto modelMatrix = model.getModelMatrix();
			std::for_each(begin, end, [shadowModelMatrixUniform, modelMatrix, shadowProgramme](AMesh mesh)
			{
				glUniformMatrix4fv (shadowModelMatrixUniform, 1, GL_FALSE, glm::value_ptr(modelMatrix));
				mesh.Draw(shadowProgramme);
			});
		}
		glBindVertexArray(0);

		glUniformMatrix4fv (shadowModelMatrixUniform, 1, GL_FALSE, glm::value_ptr(plane.getModelMatrix()));
		plane.Draw(shadowProgramme);
		glBindVertexArray(0);
#pragma endregion

#pragma region DEBUG QUAD
		glEnable(GL_SCISSOR_TEST);
		glViewport(0, height, width, height);
		glScissor (0, height, width,height);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.0f, 1.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		debugQuad.render(depthMap);
#pragma endregion

#pragma region LIGHT CAMERA
		glViewport(width, height, width, height);
		glScissor (width, height, width,height);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shaderProgramme);
		
		glUniformMatrix4fv(vpMatrixUniform, 1, GL_FALSE, glm::value_ptr(lightMatrix));
		glUniformMatrix4fv(lightMatrixUniform, 1, GL_FALSE, glm::value_ptr(lightMatrix));
		glUniform3f(lightPositionUniform, lightPosition.x, lightPosition.y, lightPosition.z);
		glUniform3f(lightDirectionUniform, lightDirection.x, lightDirection.y, lightDirection.z);
		glUniform4f(lightColorUniform, lightColor.x, lightColor.y, lightColor.z, lightColor.w);
		glUniform1f(lightIntensityUniform, lightIntensity);
		glUniform1i(lightDirectionalUniform, lightDirectional);

		glBindTexture(GL_TEXTURE_2D, depthMap);
		glUniform1i(shadowMapUniform, 0);
		
		for(auto model : models) 
		{
			pointer = model.getMeshes();

			auto begin = pointer->begin();
			auto end = pointer->end();
			auto modelMatrix = model.getModelMatrix();
			std::for_each(begin, end, [modelMatrixUniform, modelMatrix, shaderProgramme](AMesh mesh)
			{
				glUniformMatrix4fv (modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(modelMatrix));
				mesh.Draw(shaderProgramme);
			});
		}

		glBindVertexArray(0);
		glUniformMatrix4fv (modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(plane.getModelMatrix()));
		plane.Draw(shaderProgramme);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
#pragma endregion

		glViewport(width, 0, width, height);
		glScissor(width, 0, width,height);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.02f, 0.04f, 0.25f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shaderProgramme);
		
		glUniformMatrix4fv(vpMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
		glUniformMatrix4fv(lightMatrixUniform, 1, GL_FALSE, glm::value_ptr(lightMatrix));
		glUniform3f(lightPositionUniform, lightPosition.x, lightPosition.y, lightPosition.z);
		glUniform3f(lightDirectionUniform, lightDirection.x, lightDirection.y, lightDirection.z);
		glUniform4f(lightColorUniform, lightColor.x, lightColor.y, lightColor.z, lightColor.w);
		glUniform1f(lightIntensityUniform, lightIntensity);
		glUniform1i(lightDirectionalUniform, lightDirectional);

		glBindTexture(GL_TEXTURE_2D, depthMap);
		glUniform1i(shadowMapUniform, 0);
		
		for(auto model : models) 
		{
			pointer = model.getMeshes();

			auto begin = pointer->begin();
			auto end = pointer->end();
			auto modelMatrix = model.getModelMatrix();
			std::for_each(begin, end, [modelMatrixUniform, modelMatrix, shaderProgramme](AMesh mesh)
			{
				glUniformMatrix4fv (modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(modelMatrix));
				mesh.Draw(shaderProgramme);
			});
		}

		glBindVertexArray(0);
		glUniformMatrix4fv (modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(plane.getModelMatrix()));
		plane.Draw(shaderProgramme);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glUseProgram(skyboxProgramme);
		glUniformMatrix4fv (skyVpMatrixUniform, 1, GL_FALSE, glm::value_ptr(skyViewProjectionMatrix));
		glUniform1i(skyboxUniform, 0);
		
		askybox.render();

		glfwSwapBuffers(window);
		glfwPollEvents();

		glDisable(GL_SCISSOR_TEST);

		finishFrameTime = glfwGetTime();
		deltaTime = static_cast<float>(finishFrameTime - currentTime);
		currentTime = finishFrameTime;
		accumulator += deltaTime;
		sprintf(title, "FPS: %4.2f", 1.0f / (float) deltaTime);
		printf("FPS: %4.2f\n", 1.0f / (float) deltaTime);
		glfwSetWindowTitle(window, title);
	}
	while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

	glfwTerminate();
	return 0;
}