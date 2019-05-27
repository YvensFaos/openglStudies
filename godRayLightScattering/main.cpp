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
#include <RenderingEngine/GraphicalTools/askybox.hpp>
#include <RenderingEngine/Utils/amacrohelper.hpp>
#include <RenderingEngine/Utils/arenderquad.hpp>
#include <RenderingEngine/Utils/aluahelper.hpp>

#include <stb_image.h>

glm::vec3 getCurrentPosition(LuaHandler* luaHandler, float accumulator, glm::vec3 initialPosition)
{
	luaHandler->getFunction("lightMovementFunction");
	luaHandler->pushNumber(accumulator);
	luaHandler->pushNumber(initialPosition.x);
	luaHandler->pushNumber(initialPosition.y);
	luaHandler->pushNumber(initialPosition.z);

	luaHandler->callFunctionFromStack(4, 3);

	glm::vec3 position;
	position.x = luaHandler->popNumber();
	position.y = luaHandler->popNumber();
	position.z = luaHandler->popNumber();

	return position;
}

int main(void)
{
	int width = 800;
	int height = 600;
	ARenderer arenderer(width, height, "Light Scattering Scene");	
	arenderer.changeClearColor(glm::vec4(0.02f, 0.04f, 0.25f, 0.0f));

	LuaHandler luaHandler;
	luaHandler.openFile("config.lua");
	
  GLuint vs =  AShader::generateShader(luaHandler.getGlobalString("vertexShader"), GL_VERTEX_SHADER);
	GLuint fs =  AShader::generateShader(luaHandler.getGlobalString("fragmentShader"), GL_FRAGMENT_SHADER);
	GLuint lfs =  AShader::generateShader(luaHandler.getGlobalString("lightFragmentShader"), GL_FRAGMENT_SHADER);
	GLuint lcfs =  AShader::generateShader(luaHandler.getGlobalString("lightScatteringShader"), GL_FRAGMENT_SHADER);

  GLuint shaderProgramme = AShader::generateProgram(vs, fs);
	GLuint lightboxProgramme  = AShader::generateProgram(vs, lfs);
	
	ASkybox askybox(std::vector<std::string>{
        "../3DModels/desertsky_ft.tga",
        "../3DModels/desertsky_bc.tga",
        "../3DModels/desertsky_up.tga",
        "../3DModels/desertsky_dn.tga",
        "../3DModels/desertsky_rt.tga",
        "../3DModels/desertsky_lf.tga"
    });

	GLuint modelMatrixUniform = glGetUniformLocation(shaderProgramme, "model");
	GLuint vpMatrixUniform = glGetUniformLocation(shaderProgramme, "viewProjection");
	GLuint lightMatrixUniform = glGetUniformLocation(shaderProgramme, "lightViewProjection");

	GLuint    lightPositionUniform = glGetUniformLocation(shaderProgramme, "sceneLight.position");
	GLuint   lightDirectionUniform = glGetUniformLocation(shaderProgramme, "sceneLight.direction");
	GLuint       lightColorUniform = glGetUniformLocation(shaderProgramme, "sceneLight.color");
	GLuint   lightIntensityUniform = glGetUniformLocation(shaderProgramme, "sceneLight.intensity");
	GLuint lightDirectionalUniform = glGetUniformLocation(shaderProgramme, "sceneLight.directional");

	GLuint lightModelMatrixUniform = glGetUniformLocation(lightboxProgramme, "model");
	GLuint lightVpMatrixUniform = glGetUniformLocation(lightboxProgramme, "viewProjection");
	GLuint lightColorBoxUniform = glGetUniformLocation(lightboxProgramme, "lightColor");

	std::vector<AModel*> models = ALuaHelper::loadModelsFromTable("models", &luaHandler);
	AModel* lightObject = ALuaHelper::loadModelFromTable("lightObject", &luaHandler);
	ALight* alight = ALuaHelper::loadLightFromTable("light", &luaHandler);

	ACamera* acamera = arenderer.getCamera();
	glm::mat4 projection = glm::perspective(glm::radians(acamera->getZoom()), (float) width / (float) height, acamera->getNear(), acamera->getFar());
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	glm::mat4 view = acamera->getView();
	glm::mat4 viewProjectionMatrix = projection * view;
	glm::mat4 skyViewProjectionMatrix = projection * glm::mat4(glm::mat3(view));

	glm::vec4 lPos = lightObject->getPosition();
	glm::vec3 lInitialPosition(lPos);

	glm::vec4 inScene = viewProjectionMatrix * lPos;
	inScene.x = (inScene.x / inScene.w);
	inScene.y = (inScene.y / inScene.w);

	glm::vec3 lightPosition = alight->getPosition();
	glm::vec3 lightDirection = alight->getDirection();
	glm::vec3 lightUp = alight->getUp();
	glm::vec4 lightColor = alight->getColor();
	float lightIntensity = alight->getIntensity();
	bool lightDirectional = alight->getDirectional();

	AFramebuffer aframebuffer      (width, height);
	AFramebuffer anotherFramebuffer(width, height);
	AFramebuffer scatterFramebuffer(width, height);

	ARenderQuad debugQuad(luaHandler.getGlobalString("lightScatteringShader"));
	GLuint lightScatteringProgramme = debugQuad.getProgramme();

	GLuint lsExposure = glGetUniformLocation(lightScatteringProgramme, "exposure");
	GLuint lsDecay = glGetUniformLocation(lightScatteringProgramme, "decay");
	GLuint lsDensity = glGetUniformLocation(lightScatteringProgramme, "density");
	GLuint lsWeight = glGetUniformLocation(lightScatteringProgramme, "weight");
	GLuint lsLightPositionOnScreen = glGetUniformLocation(lightScatteringProgramme, "lightPositionOnScreen");
	GLuint lsTextureUniform = glGetUniformLocation(lightScatteringProgramme, "textureUniform");

	float Exposure = luaHandler.getGlobalNumber("Exposure");
	float Decay = luaHandler.getGlobalNumber("Decay");
	float Density = luaHandler.getGlobalNumber("Density");
	float Weight = luaHandler.getGlobalNumber("Weight");
	float lightMovement = luaHandler.getGlobalNumber("LightMovement");
	float lightSpeed = luaHandler.getGlobalNumber("LightSpeed");

	ARenderQuad finalRenderQuad(luaHandler.getGlobalString("mixFragmentShader"));
	GLuint mixProgramme = finalRenderQuad.getProgramme();
	std::vector<GLuint> textures;
	textures.push_back(anotherFramebuffer.getFramebufferTexture());
	textures.push_back(scatterFramebuffer.getFramebufferTexture());
	
	do
	{
		arenderer.startFrame();
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		aframebuffer.bindBuffer();
			glViewport(0, 0, width, height);
			glClearColor(0.05f, 0.05f, 0.05f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(lightboxProgramme);
			glUniformMatrix4fv(lightVpMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
			glUniform4f(lightColorBoxUniform, 0.0, 0.0, 0.0, 1.0);
			AModel::renderModelsInList(&models, lightModelMatrixUniform, lightboxProgramme);
			glUniform4f(lightColorBoxUniform, lightColor.x, lightColor.y, lightColor.z, lightColor.w);
			lightObject->renderModels(lightModelMatrixUniform, lightboxProgramme);
			glBindVertexArray(0);
		aframebuffer.unbindBuffer();

		scatterFramebuffer.bindBuffer();
			glViewport(0, 0, width, height);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(lightScatteringProgramme);
			glUniform1f(lsExposure, Exposure);
			glUniform1f(lsDecay, Decay);
			glUniform1f(lsDensity, Density);
			glUniform1f(lsWeight, Weight);
			glUniform2f(lsLightPositionOnScreen, inScene.x, inScene.y);
			glUniform1i(lsTextureUniform, 0);

			debugQuad.render(aframebuffer.getFramebufferTexture(), false);
		scatterFramebuffer.unbindBuffer();

		anotherFramebuffer.bindBuffer();
			glViewport(0, 0, width, height);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(shaderProgramme);
			glUniformMatrix4fv(vpMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
			glUniform3f(lightPositionUniform, lightPosition.x, lightPosition.y, lightPosition.z);
			glUniform3f(lightDirectionUniform, lightDirection.x, lightDirection.y, lightDirection.z);
			glUniform4f(lightColorUniform, lightColor.x, lightColor.y, lightColor.z, lightColor.w);
			glUniform1f(lightIntensityUniform, lightIntensity);
			glUniform1i(lightDirectionalUniform, lightDirectional);

			AModel::renderModelsInList(&models, modelMatrixUniform, shaderProgramme);

			glUseProgram(lightboxProgramme);
			glUniformMatrix4fv(lightVpMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
			glUniform4f(lightColorBoxUniform, lightColor.x, lightColor.y, lightColor.z, lightColor.w);
			lightObject->renderModels(lightModelMatrixUniform, lightboxProgramme);

			askybox.render(skyViewProjectionMatrix);
		anotherFramebuffer.unbindBuffer();

		glViewport(0, 0, width * 2, height * 2);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(mixProgramme);
		finalRenderQuad.render(textures);

		arenderer.finishFrame();

		lightObject->setPosition(getCurrentPosition(&luaHandler, arenderer.getAccumulator(), lInitialPosition));
		lPos = lightObject->getPosition();
		inScene = viewProjectionMatrix * lPos;
		inScene.x = (inScene.x / inScene.w);
		inScene.y = (inScene.y / inScene.w);
	}
	while(arenderer.isRunning());

	glfwTerminate();
	return 0;
}