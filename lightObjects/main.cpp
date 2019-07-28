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
	ARenderer arenderer(width, height, "Light Objects");	
	arenderer.changeClearColor(glm::vec4(0.02f, 0.02f, 0.02f, 0.0f));

	LuaHandler luaHandler;
	luaHandler.openFile("config.lua");
	bool DEBUG = luaHandler.getGlobalBoolean("debug");
	
  	GLuint vs =  AShader::generateShader(luaHandler.getGlobalString("vertexShader"), GL_VERTEX_SHADER);
	GLuint gs =  AShader::generateShader(luaHandler.getGlobalString("geometryShader"), GL_GEOMETRY_SHADER);
	GLuint ls =  AShader::generateShader(luaHandler.getGlobalString("lightFragmentShader"), GL_FRAGMENT_SHADER);

	GLuint cvs = AShader::generateShader(luaHandler.getGlobalString("completeVertexShader"), GL_VERTEX_SHADER);
	GLuint cfs = AShader::generateShader(luaHandler.getGlobalString("directionalLightFragmentShader"), GL_FRAGMENT_SHADER);

	GLuint lightProgramme = AShader::generateProgram(vs, gs, ls);
	GLuint simpleLightProgramme = AShader::generateProgram(vs, ls);
	GLuint objectsProgramme = AShader::generateProgram(cvs, cfs);

	GLuint lightModelMatrixUniform = glGetUniformLocation(lightProgramme, "model");
	GLuint lightVMatrixUniform = glGetUniformLocation(lightProgramme, "view");
	GLuint lightPMatrixUniform = glGetUniformLocation(lightProgramme, "projection");
	GLuint lightLightColorUniform = glGetUniformLocation(lightProgramme, "lightColor");
	GLuint lightDirectionUniform = glGetUniformLocation(lightProgramme, "lightDirection");

	GLuint simpleLightModelMatrixUniform = glGetUniformLocation(simpleLightProgramme, "model");
	GLuint simpleLightVMatrixUniform = glGetUniformLocation(simpleLightProgramme, "view");
	GLuint simpleLightPMatrixUniform = glGetUniformLocation(simpleLightProgramme, "projection");
	GLuint simpleLightLightColorUniform = glGetUniformLocation(simpleLightProgramme, "lightColor");

	GLuint objectsModelMatrixUniform = glGetUniformLocation(objectsProgramme, "model");
	GLuint objectsVMatrixUniform = glGetUniformLocation(objectsProgramme, "view");
	GLuint objectsPMatrixUniform = glGetUniformLocation(objectsProgramme, "projection");
	GLuint objectsLightMatrixUniform = glGetUniformLocation(objectsProgramme, "lightViewProjection");
	GLuint objectsNumberPointLightsUniform = glGetUniformLocation(objectsProgramme, "numberPointLights");
	GLuint objectsNumberDirectionLightsUniform = glGetUniformLocation(objectsProgramme, "numberDirectionLights");

	ACamera* acamera = arenderer.getCamera();
	ALuaHelper::setupCameraPosition("cameraPosition", acamera, &luaHandler);
	glm::vec3 cameraPosition = acamera->getPos();
	glm::mat4 projection = glm::perspective(glm::radians(acamera->getZoom()), (float) width / (float) height, acamera->getNear(), acamera->getFar());
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	glm::mat4 view = acamera->getView();
	glm::mat4 viewProjectionMatrix = projection * view;

	std::vector<ALight*> lights = ALuaHelper::loadLightsFromTable("lights", &luaHandler);
	const int lightsSize = lights.size();
	std::vector<ALightUniform> directionalLightUniforms;
	std::vector<ALightUniform> pointLightUniforms;
	int directionalLightCount = 0;
	int pointLightCount = 0;
	bool isDirectional = false;
	ALight* lightPointer;

	for(unsigned int i = 0; i < lightsSize; i++) {
		lightPointer = lights[i];
		isDirectional = lightPointer->getDirectional();

		if(isDirectional) 
		{
			directionalLightUniforms.push_back(ALightUniform(
				ALightUniform::loadALightUniformFromProgramme(objectsProgramme, directionalLightCount++, lightPointer)));
		}
		else
		{
			pointLightUniforms.push_back(ALightUniform(
				ALightUniform::loadALightUniformFromProgramme(objectsProgramme, pointLightCount++, lightPointer)));
		}
	}

	AModel* alightModel = new AModel(luaHandler.getGlobalString("lightModel"));
    alightModel->scale(glm::vec3(0.1, 0.1, 0.1));

	std::vector<AModel*> models = ALuaHelper::loadModelsFromTable("models", &luaHandler);
	ALightUniform* alightUniformPointer;
	glm::vec3 lightPosition;
	glm::vec3 lightDirection;
	glm::vec3 lightUp;
	glm::vec4 lightColor;
	float lightIntensity;
	bool lightDirectional;

	glActiveTexture(GL_TEXTURE0);
	do
	{
		projection = glm::perspective(glm::radians(acamera->getZoom()), (float) width / (float) height, acamera->getNear(), acamera->getFar());
		view = acamera->getView();
		viewProjectionMatrix = projection * view;
		
		arenderer.startFrame();

		glViewport(0, 0, width * 2, height * 2);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.02f, 0.04f, 0.25f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Render Light Objects with Geometry Shader
		glUseProgram(lightProgramme);
		glUniformMatrix4fv(lightVMatrixUniform, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(lightPMatrixUniform, 1, GL_FALSE, glm::value_ptr(projection));
		for(unsigned int i = 0; i < lightsSize; i++) {
			lightPointer = lights[i];
			lightPosition = lightPointer->getPosition();
			lightDirection = lightPointer->getDirection();
			glUniform4f(lightLightColorUniform, 0.2f, 0.2f, 0.2f, 1.0f);
			glUniform3f(lightDirectionUniform, lightDirection.x, lightDirection.y, lightDirection.z);
			alightModel->setPosition(lightPosition);
			alightModel->renderModels(lightModelMatrixUniform, lightProgramme);
		}

		//Render Light Objects as Tetrahedrons
		glUseProgram(simpleLightProgramme);
		glUniformMatrix4fv(simpleLightVMatrixUniform, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(simpleLightPMatrixUniform, 1, GL_FALSE, glm::value_ptr(projection));
		for(unsigned int i = 0; i < lightsSize; i++) {
			lightPointer = lights[i];
			lightPosition = lightPointer->getPosition();
			lightColor = lightPointer->getColor();
			glUniform4f(simpleLightLightColorUniform, lightColor.x, lightColor.y, lightColor.z, lightColor.w);
			alightModel->setPosition(lightPosition);
			alightModel->renderModels(simpleLightModelMatrixUniform, simpleLightProgramme);
		}

		//Render Objects
		glUseProgram(objectsProgramme);
		glUniformMatrix4fv(objectsVMatrixUniform, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(objectsPMatrixUniform, 1, GL_FALSE, glm::value_ptr(projection));
		glUniform1i(objectsNumberPointLightsUniform, pointLightUniforms.size());
		glUniform1i(objectsNumberDirectionLightsUniform, directionalLightUniforms.size());
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
		AModel::renderModelsInList(&models, objectsModelMatrixUniform, objectsProgramme);

		// ALuaHelper::updateLight(&luaHandler, pointLightPointer, "updateLight", arenderer.getDeltaTime());
		// alightModel->setPosition(pointLightPointer->getPosition());

		arenderer.finishFrame();
	}
	while(arenderer.isRunning());

	glfwTerminate();
	return 0;
}