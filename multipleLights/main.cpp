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
#include <RenderingEngine/Utils/anormaldebugger.hpp>

#include <stb_image.h>

int main(void)
{
	int width = 800;
	int height = 600;
	ARenderer arenderer(width, height, "Multiple Lights");	
	arenderer.changeClearColor(glm::vec4(0.02f, 0.02f, 0.02f, 0.0f));

	LuaHandler luaHandler;
	luaHandler.openFile("config.lua");
	bool DEBUG = luaHandler.getGlobalBoolean("debug");
	
  	GLuint vs =  AShader::generateShader(luaHandler.getGlobalString("vertexShader"), GL_VERTEX_SHADER);
	GLuint fs =  AShader::generateShader(luaHandler.getGlobalString("fragmentShader"), GL_FRAGMENT_SHADER);
	GLuint ls =  AShader::generateShader(luaHandler.getGlobalString("lightFragmentShader"), GL_FRAGMENT_SHADER);

  	GLuint shaderProgramme = AShader::generateProgram(vs, fs);
	GLuint lightProgramme = AShader::generateProgram(vs, ls);

	ASkybox askybox(std::vector<std::string>{
        "../3DModels/desertsky_ft.tga",
        "../3DModels/desertsky_bc.tga",
        "../3DModels/desertsky_up.tga",
        "../3DModels/desertsky_dn.tga",
        "../3DModels/desertsky_rt.tga",
        "../3DModels/desertsky_lf.tga"
    });

	GLuint modelMatrixUniform = glGetUniformLocation(shaderProgramme, "model");
	GLuint vMatrixUniform = glGetUniformLocation(shaderProgramme, "view");
	GLuint pMatrixUniform = glGetUniformLocation(shaderProgramme, "projection");
	GLuint lightMatrixUniform = glGetUniformLocation(shaderProgramme, "lightViewProjection");
	GLuint numberPointLightsUniform = glGetUniformLocation(shaderProgramme, "numberPointLights");
	GLuint numberDirectionLightsUniform = glGetUniformLocation(shaderProgramme, "numberDirectionLights");

	GLuint lightModelMatrixUniform = glGetUniformLocation(lightProgramme, "model");
	GLuint lightVMatrixUniform = glGetUniformLocation(lightProgramme, "view");
	GLuint lightPMatrixUniform = glGetUniformLocation(lightProgramme, "projection");
	GLuint lightLightColorUniform = glGetUniformLocation(lightProgramme, "lightColor");

	std::vector<ALight*> lights = ALuaHelper::loadLightsFromTable("lights", &luaHandler);
	const int lightsSize = lights.size();
	std::vector<ALightUniform> directionalLightUniforms;
	std::vector<ALightUniform> pointLightUniforms;

	int directionalLightCount = 0;
	int pointLightCount = 0;
	ALight* lightPointer;

	GLuint    lightPositionUniform = -1;
	GLuint   lightDirectionUniform = -1;
	GLuint       lightColorUniform = -1;
	GLuint   lightIntensityUniform = -1;
	GLuint lightDirectionalUniform = -1;
	char uniformName[64];
	bool isDirectional = false;
	std::string buffer;

	ALight* pointLightPointer;
	for(unsigned int i = 0; i < lightsSize; i++) {
		lightPointer = lights[i];
		isDirectional = lightPointer->getDirectional();

		sprintf(uniformName, "%s[%d].", isDirectional ? 
			"directionalLights" : "pointLights", isDirectional ? pointLightCount++ : directionalLightCount++);

		buffer = "";
		buffer.append(uniformName);
		lightPositionUniform    = glGetUniformLocation(shaderProgramme, buffer.append("position").c_str());
		buffer = "";
		buffer.append(uniformName);
		lightDirectionUniform   = glGetUniformLocation(shaderProgramme, buffer.append("direction").c_str());
		buffer = "";
		buffer.append(uniformName);
		lightColorUniform       = glGetUniformLocation(shaderProgramme, buffer.append("color").c_str());
		buffer = "";
		buffer.append(uniformName);
		lightIntensityUniform   = glGetUniformLocation(shaderProgramme, buffer.append("intensity").c_str());
		buffer = "";
		buffer.append(uniformName);
		lightDirectionalUniform = glGetUniformLocation(shaderProgramme, buffer.append("directional").c_str());

		if(isDirectional) 
		{
			directionalLightUniforms.push_back(ALightUniform(lightPositionUniform, lightDirectionUniform, lightColorUniform, lightIntensityUniform, lightDirectionalUniform));
		}
		else
		{
			pointLightUniforms.push_back(ALightUniform(lightPositionUniform, lightDirectionUniform, lightColorUniform, lightIntensityUniform, lightDirectionalUniform));
			pointLightPointer = lights[i];
		}
	}

	std::vector<AModel*> models = ALuaHelper::loadModelsFromTable("models", &luaHandler);
	AModel* lightObject = ALuaHelper::loadModelFromTable("lightObject", &luaHandler);

	ACamera* acamera = arenderer.getCamera();
	ALuaHelper::setupCameraPosition("cameraPosition", acamera, &luaHandler);
	glm::vec3 cameraPosition = acamera->getPos();
	glm::mat4 projection = glm::perspective(glm::radians(acamera->getZoom()), (float) width / (float) height, acamera->getNear(), acamera->getFar());
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	glm::mat4 view = acamera->getView();
	glm::mat4 viewProjectionMatrix = projection * view;
	glm::mat4 skyViewProjectionMatrix = projection * glm::mat4(glm::mat3(view));
	glm::mat4 skyView = glm::mat4(1.0);

	glm::vec3 lightPosition;
	glm::vec3 lightDirection;
	glm::vec3 lightUp;
	glm::vec4 lightColor;
	float lightIntensity;
	bool lightDirectional;

	ANormalDebugger normalDebugger;
	GLuint anormalDebuggerModelUniform = normalDebugger.getModelUniformLocation();
	GLuint anormalDebuggerProgramme = normalDebugger.getProgramme();

	ALightUniform* alightUniformPointer;
	glActiveTexture(GL_TEXTURE0);
	do
	{
		projection = glm::perspective(glm::radians(acamera->getZoom()), (float) width / (float) height, acamera->getNear(), acamera->getFar());
		view = acamera->getView();
		viewProjectionMatrix = projection * view;
		skyViewProjectionMatrix = projection * glm::mat4(glm::mat3(view));
		
		arenderer.startFrame();

		glViewport(0, 0, width * 2, height * 2);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.02f, 0.04f, 0.25f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaderProgramme);

		glUniformMatrix4fv(vMatrixUniform, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(pMatrixUniform, 1, GL_FALSE, glm::value_ptr(projection));
		glUniform1i(numberPointLightsUniform, pointLightCount);
		glUniform1i(numberDirectionLightsUniform, directionalLightCount);

		directionalLightCount = 0;
		pointLightCount = 0;
		for(unsigned int i = 0; i < lightsSize; i++) {
			lightPointer = lights[i];
			isDirectional = lightPointer->getDirectional();

			if(isDirectional) {
				alightUniformPointer = &directionalLightUniforms[directionalLightCount++];
			} else {
				alightUniformPointer = &pointLightUniforms[pointLightCount++];
			}

			lightPosition = lightPointer->getPosition();
			lightDirection = lightPointer->getDirection();
			lightUp = lightPointer->getUp();
			lightColor = lightPointer->getColor();
			lightIntensity = lightPointer->getIntensity();

			glUniform3f(alightUniformPointer->lightPositionUniform, lightPosition.x, lightPosition.y, lightPosition.z);
			glUniform3f(alightUniformPointer->lightDirectionUniform, lightDirection.x, lightDirection.y, lightDirection.z);
			glUniform4f(alightUniformPointer->lightColorUniform, lightColor.x, lightColor.y, lightColor.z, lightColor.w);
			glUniform1f(alightUniformPointer->lightIntensityUniform, lightIntensity);
			glUniform1i(alightUniformPointer->lightDirectionalUniform, isDirectional);
		}
		
		AModel::renderModelsInList(&models, modelMatrixUniform, shaderProgramme);

		if(DEBUG) 
		{
			normalDebugger.setupForRendering(viewProjectionMatrix);
			AModel::renderModelsInList(&models, anormalDebuggerModelUniform, anormalDebuggerProgramme);
		}

		if(pointLightPointer) {
			glUseProgram(lightProgramme);
			glUniformMatrix4fv(lightVMatrixUniform, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(lightPMatrixUniform, 1, GL_FALSE, glm::value_ptr(projection));
			lightColor = pointLightPointer->getColor();
			glUniform4f(lightLightColorUniform, lightColor.x, lightColor.y, lightColor.z, lightColor.w);
			lightObject->renderModels(lightModelMatrixUniform, lightProgramme);

			ALuaHelper::updateLight(&luaHandler, pointLightPointer, "updateLight", arenderer.getDeltaTime());
			lightObject->setPosition(pointLightPointer->getPosition());
		}

		askybox.render(skyViewProjectionMatrix);
		arenderer.finishFrame();
	}
	while(arenderer.isRunning());

	glfwTerminate();
	return 0;
}