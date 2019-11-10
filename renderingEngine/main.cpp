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

#include "RenderingEngine/Core/arenderer.hpp"
#include "RenderingEngine/Core/amodel.hpp"
#include "RenderingEngine/Core/acamera.hpp"
#include "RenderingEngine/Core/alight.hpp"
#include "RenderingEngine/Core/ashader.hpp"

#include "RenderingEngine/GraphicalTools/aframebuffer.hpp"
#include "RenderingEngine/GraphicalTools/askybox.hpp"

#include <luahandler.hpp>
#include <stb_image.h>

#include "RenderingEngine/Utils/amacrohelper.hpp"
#include "RenderingEngine/Utils/arenderquad.hpp"
#include "RenderingEngine/Utils/aluahelper.hpp"
#include "RenderingEngine/Utils/akeybind.hpp"

glm::mat4 skyView = glm::mat4(1.0);
float savedAcc = 0.0f;

float near_plane;
float far_plane;
float projectionDimension;

int width = 800;
int height = 600;

int main(int argc, char* argv[])
{
	bool paused = false;

	ARenderer arenderer(width, height, "Rendering Engine Demo Scene");	
	arenderer.changeClearColor(glm::vec4(0.02f, 0.04f, 0.25f, 0.0f));

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

	ASkybox askybox(std::vector<std::string>{
        "RenderingEngine/Resources/desertsky_ft.tga",
        "RenderingEngine/Resources/desertsky_bc.tga",
        "RenderingEngine/Resources/desertsky_up.tga",
        "RenderingEngine/Resources/desertsky_dn.tga",
        "RenderingEngine/Resources/desertsky_rt.tga",
        "RenderingEngine/Resources/desertsky_lf.tga"
    });

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

	GLfloat shadowWidth = 1024;
	GLfloat shadowHeight = 1024;
	ADepthbuffer adepthBuffer(shadowWidth, shadowHeight);

	std::vector<AModel> models = ALuaHelper::loadModelsFromTable("models", luaHandler);
	AModel plane = ALuaHelper::loadModelFromTable("plane", luaHandler);
	ALight alight = ALuaHelper::loadLightFromTable("light", luaHandler);

	ACamera& acamera = arenderer.getCamera();
	glm::mat4 projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, 0.1f, 1000.0f);
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	glm::mat4 view = acamera.getView();
	glm::mat4 viewProjectionMatrix = projection * view;
	glm::mat4 skyViewProjectionMatrix = projection * glm::mat4(glm::mat3(view));

	glm::vec3 lightPosition = 	alight.getPosition();
	glm::vec3 lightDirection = 	alight.getDirection();
	glm::vec3 lightUp = 		alight.getUp();
	glm::vec4 lightColor = 		alight.getColor();
	float lightIntensity = 		alight.getIntensity();
	bool lightDirectional = 	alight.getDirectional();

	near_plane = luaHandler.getGlobalNumber("nearPlane");
	far_plane = luaHandler.getGlobalNumber("farPlane");
	projectionDimension = luaHandler.getGlobalNumber("projectionDimension");
	glm::mat4 lightProjection = glm::ortho(-projectionDimension, projectionDimension, -projectionDimension, projectionDimension, near_plane, far_plane);
	glm::mat4 lightView = glm::lookAt(lightPosition, lightPosition + lightDirection, lightUp);
	glm::mat4 lightMatrix = lightProjection * lightView;

	arenderer.addKeybind(AKeyBind(GLFW_KEY_SPACE, [&paused](int action, int mods) {
		if(action == GLFW_PRESS || action == GLFW_REPEAT) {
			paused = !paused;
		}
	}));
	do
	{
		if(!paused) 
		{
			ALuaHelper::updateLight(luaHandler, alight, "updateLight", arenderer.getDeltaTime());
		}

		lightPosition = 	alight.getPosition();
		lightDirection = 	alight.getDirection();
		lightUp = 			alight.getUp();
		lightColor = 		alight.getColor();
		lightIntensity = 	alight.getIntensity();

		glm::mat4 lightProjection = glm::ortho(-projectionDimension, projectionDimension, -projectionDimension, projectionDimension, near_plane, far_plane);
		lightView = glm::lookAt(lightPosition, lightPosition + lightDirection, lightUp);
		lightMatrix = lightProjection * lightView;

		projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, 0.1f, 1000.0f);
		skyViewProjectionMatrix = projection * glm::mat4(glm::mat3(skyView));
		viewProjectionMatrix = projection * acamera.getView();

		arenderer.startFrame();

#pragma region SHADOW BUFFER
		glViewport(0, 0, shadowWidth, shadowHeight);
		adepthBuffer.bindBuffer();
		glClear(GL_DEPTH_BUFFER_BIT);

		glUseProgram(shadowProgramme);
		glUniformMatrix4fv (shadowLightMatrixUniform, 1, GL_FALSE, glm::value_ptr(lightMatrix));
		AModel::renderModelsInList(models, shadowModelMatrixUniform, shadowProgramme);
		glBindVertexArray(0);

		glUniformMatrix4fv (shadowModelMatrixUniform, 1, GL_FALSE, glm::value_ptr(plane.getModelMatrix()));
		plane.draw(shadowProgramme);
		glBindVertexArray(0);
#pragma endregion

#pragma region DEBUG QUAD
		glEnable(GL_SCISSOR_TEST);
		glViewport(0, height, width, height);
		glScissor (0, height, width,height);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.0f, 1.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		debugQuad.render(adepthBuffer.getFramebufferTexture());
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

		glBindTexture(GL_TEXTURE_2D, adepthBuffer.getFramebufferTexture());
		glUniform1i(shadowMapUniform, 0);
		AModel::renderModelsInList(models, modelMatrixUniform, shaderProgramme);

		glBindVertexArray(0);
		glUniformMatrix4fv (modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(plane.getModelMatrix()));
		plane.draw(shadowProgramme);
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

		glBindTexture(GL_TEXTURE_2D, adepthBuffer.getFramebufferTexture());
		glUniform1i(shadowMapUniform, 0);
		AModel::renderModelsInList(models, modelMatrixUniform, shaderProgramme);

		glBindVertexArray(0);
		glUniformMatrix4fv (modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(plane.getModelMatrix()));
		plane.draw(shaderProgramme);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);

		askybox.render(skyViewProjectionMatrix);

		glDisable(GL_SCISSOR_TEST);

		arenderer.finishFrame();
	}
	while(arenderer.isRunning());

	glfwTerminate();
	return 0;
}