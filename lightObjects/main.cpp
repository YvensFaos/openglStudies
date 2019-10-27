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
#include <RenderingEngine/Objects/alightobject.hpp>
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
	
	GLuint cvs = AShader::generateShader(luaHandler.getGlobalString("completeVertexShader"), GL_VERTEX_SHADER);
	GLuint cfs = AShader::generateShader(luaHandler.getGlobalString("directionalLightFragmentShader"), GL_FRAGMENT_SHADER);

	GLuint objectsProgramme = AShader::generateProgram(cvs, cfs);
	GLuint objectsModelMatrixUniform = glGetUniformLocation(objectsProgramme, "model");
	GLuint objectsVMatrixUniform = glGetUniformLocation(objectsProgramme, "view");
	GLuint objectsPMatrixUniform = glGetUniformLocation(objectsProgramme, "projection");
	GLuint objectsLightMatrixUniform = glGetUniformLocation(objectsProgramme, "lightViewProjection");
	GLuint objectsNumberPointLightsUniform = glGetUniformLocation(objectsProgramme, "numberPointLights");
	GLuint objectsNumberDirectionLightsUniform = glGetUniformLocation(objectsProgramme, "numberDirectionLights");

	ACamera& acamera = arenderer.getCamera();
	ALuaHelper::setupCameraPosition("cameraPosition", &acamera, &luaHandler);
	glm::vec3 cameraPosition = acamera.getPos();
	glm::mat4 projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	glm::mat4 view = acamera.getView();
	glm::mat4 viewProjectionMatrix = projection * view;

	std::vector<ALight> lights = ALuaHelper::loadLightsFromTable("lights", luaHandler);
	auto lightsBegin = lights.begin();
	auto lightsEnd = lights.end();

	std::vector<ALightObject> lightObjects = ALightObject::GenerateALightObjectsFromLights(objectsProgramme, lights);
	GLuint directionalLightCount = 0;
	GLuint pointLightCount = 0;

	for (unsigned int i = 0; i < lights.size(); i++)
	{
		if(lights[i].getDirectional())
		{
			directionalLightCount++;
		}
		else
		{
			pointLightCount++;
		}
	}

	std::vector<AModel*> models = ALuaHelper::loadModelsFromTable("models", &luaHandler);

	glActiveTexture(GL_TEXTURE0);
	do
	{
		projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
		view = acamera.getView();
		viewProjectionMatrix = projection * view;
		
		arenderer.startFrame();

		glViewport(0, 0, width * 2, height * 2);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		for (unsigned int i = 0; i < lightObjects.size(); i++)
		{
			lightObjects[i].renderLightObject(viewProjectionMatrix);
		}
		
		glUseProgram(objectsProgramme);
		glUniformMatrix4fv(objectsVMatrixUniform, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(objectsPMatrixUniform, 1, GL_FALSE, glm::value_ptr(projection));
		glUniform1i(objectsNumberPointLightsUniform, pointLightCount);
		glUniform1i(objectsNumberDirectionLightsUniform, directionalLightCount);

		for (unsigned int i = 0; i < lightObjects.size(); i++)
		{
			lightObjects[i].setupUniforms();
		}

		AModel::renderModelsInList(&models, objectsModelMatrixUniform, objectsProgramme);

		arenderer.finishFrame();
	}
	while(arenderer.isRunning());

	glfwTerminate();
	return 0;
}