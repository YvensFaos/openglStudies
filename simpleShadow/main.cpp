// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

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
#include <luahandler.hpp>

#include <RenderingEngine/Core/arenderer.hpp>
#include <RenderingEngine/Core/amodel.hpp>
#include <RenderingEngine/Core/acamera.hpp>
#include <RenderingEngine/Core/alight.hpp>
#include <RenderingEngine/Core/ashader.hpp>
#include <RenderingEngine/GraphicalTools/aframebuffer.hpp>
#include <RenderingEngine/Objects/alightobject.hpp>
#include <RenderingEngine/Utils/amacrohelper.hpp>
#include <RenderingEngine/Utils/arenderquad.hpp>
#include <RenderingEngine/Utils/aluahelper.hpp>

#include <stb_image.h>

int main(void)
{
	int width = 800;
	int height = 600;
	ARenderer arenderer(width, height, "Simple Shadow Scene");	
	arenderer.changeClearColor(glm::vec4(0.02f, 0.02f, 0.02f, 0.0f));

	LuaHandler luaHandler;
	luaHandler.openFile("config.lua");
	
  	GLuint vs =  AShader::generateShader(luaHandler.getGlobalString("vertexShader"), GL_VERTEX_SHADER);
	GLuint fs =  AShader::generateShader(luaHandler.getGlobalString("fragmentShader"), GL_FRAGMENT_SHADER);
	GLuint shadowVs =  AShader::generateShader(luaHandler.getGlobalString("shadowVertexShader"), GL_VERTEX_SHADER);
	GLuint shadowFs =  AShader::generateShader(luaHandler.getGlobalString("shadowFragmentShader"), GL_FRAGMENT_SHADER);

  	GLuint shaderProgramme = AShader::generateProgram(vs, fs);
	GLuint shadowProgramme = AShader::generateProgram(shadowVs, shadowFs);

	GLuint modelMatrixUniform = glGetUniformLocation(shaderProgramme, "model");
	GLuint vpMatrixUniform = glGetUniformLocation(shaderProgramme, "viewProjection");
	GLuint lightMatrixUniform = glGetUniformLocation(shaderProgramme, "lightViewProjection");

	GLuint    lightPositionUniform = glGetUniformLocation(shaderProgramme, "sceneLight.position");
	GLuint   lightDirectionUniform = glGetUniformLocation(shaderProgramme, "sceneLight.direction");
	GLuint       lightColorUniform = glGetUniformLocation(shaderProgramme, "sceneLight.color");
	GLuint   lightIntensityUniform = glGetUniformLocation(shaderProgramme, "sceneLight.intensity");
	GLuint lightDirectionalUniform = glGetUniformLocation(shaderProgramme, "sceneLight.directional");
	GLuint 		  shadowMapUniform = glGetUniformLocation(shaderProgramme, "shadowMap");

	GLuint shadowModelMatrixUniform = glGetUniformLocation(shadowProgramme, "model");
	GLuint shadowLightMatrixUniform = glGetUniformLocation(shadowProgramme, "lightViewProjection");

	std::vector<AModel> models = ALuaHelper::loadModelsFromTable("models", luaHandler);
	ALight alight = ALuaHelper::loadLightFromTable("light", luaHandler);
	ALightObject alightObject(alight, shaderProgramme);
	glm::vec3 lightPosition = 	alight.getPosition();
	glm::vec3 lightDirection = 	alight.getDirection();
	glm::vec3 lightUp = 		alight.getUp();

	int shadowWidth =  luaHandler.getGlobalInteger("shadowWidth");
	int shadowHeight = luaHandler.getGlobalInteger("shadowHeight");
	ADepthbuffer adepthBuffer(shadowWidth, shadowHeight);

	ACamera& acamera = arenderer.getCamera();
	ALuaHelper::setupCameraPosition("cameraPosition", acamera, luaHandler);
	glm::mat4 projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	float near_plane = luaHandler.getGlobalNumber("nearPlane");
	float far_plane = luaHandler.getGlobalNumber("farPlane");
	float projectionDimension = luaHandler.getGlobalNumber("projectionDimension");
	glm::mat4 lightProjection = glm::ortho(-projectionDimension, projectionDimension, -projectionDimension, projectionDimension, near_plane, far_plane);
	glm::mat4 lightView = glm::lookAt(lightPosition, lightPosition + lightDirection, lightUp);
	glm::mat4 lightMatrix = lightProjection * lightView;

	glm::mat4 view = acamera.getView();
	glm::mat4 viewProjectionMatrix = projection * view;

	glActiveTexture(GL_TEXTURE0);
	do
	{
		projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
		view = acamera.getView();
		viewProjectionMatrix = projection * view;

		arenderer.startFrame();

		glViewport(0, 0, shadowWidth, shadowHeight);
		adepthBuffer.bindBuffer();
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shadowProgramme);
		glUniformMatrix4fv(shadowLightMatrixUniform, 1, GL_FALSE, glm::value_ptr(lightMatrix));
		AModel::renderModelsInList(models, shadowModelMatrixUniform, shadowProgramme);
		adepthBuffer.unbindBuffer();

		glViewport(0, 0, width * 2, height * 2);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.02f, 0.04f, 0.25f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		alightObject.renderLightObject(viewProjectionMatrix);

		glUseProgram(shaderProgramme);
		glUniformMatrix4fv(vpMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
		glUniformMatrix4fv(lightMatrixUniform, 1, GL_FALSE, glm::value_ptr(lightMatrix));
		alight.setupUniforms(lightPositionUniform, lightDirectionUniform, lightColorUniform, lightIntensityUniform, lightDirectionalUniform);

		glBindTexture(GL_TEXTURE_2D, adepthBuffer.getFramebufferTexture());
		glUniform1i(shadowMapUniform, 0);

		AModel::renderModelsInList(models, modelMatrixUniform, shaderProgramme, GL_TRIANGLES, false);

		arenderer.finishFrame();
	}
	while(arenderer.isRunning());

	glfwTerminate();
	return 0;
}