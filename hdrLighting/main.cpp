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
#include <functional>
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

#include <RenderingEngine/Objects/alightobject.hpp>

#include <stb_image.h>

int main(void)
{
	int width = 800;
	int height = 600;
	ARenderer arenderer(width, height, "HDR Scene");	
	arenderer.changeClearColor(glm::vec4(0.02f, 0.02f, 0.02f, 0.0f));

	LuaHandler luaHandler;
	luaHandler.openFile("config.lua");
	
  	GLuint vs =  AShader::generateShader(luaHandler.getGlobalString("vertexShader"), GL_VERTEX_SHADER);
	GLuint fs =  AShader::generateShader(luaHandler.getGlobalString("multiLightFragmentShader"), GL_FRAGMENT_SHADER);

  	GLuint shaderProgramme = AShader::generateProgram(vs, fs);

	GLuint modelMatrixUniform = glGetUniformLocation(shaderProgramme, "model");
	GLuint objectsVMatrixUniform = glGetUniformLocation(shaderProgramme, "view");
	GLuint objectsPMatrixUniform = glGetUniformLocation(shaderProgramme, "projection");
	GLuint objectsNumberPointLightsUniform = glGetUniformLocation(shaderProgramme, "numberPointLights");
	GLuint objectsViewPositionUniform = glGetUniformLocation(shaderProgramme, "viewPosition");

	std::vector<ALight> lights = ALuaHelper::loadLightsFromTable("lights", luaHandler);
	std::vector<ALightObject> lightObjects = ALightObject::GenerateALightObjectsFromLights(shaderProgramme, lights);
	GLuint pointLightCount = 0;

	glm::vec3 pointLightPositions[10];
	for (unsigned int i = 0; i < lights.size(); i++)
	{
		if(!lights[i].getDirectional())
		{
			pointLightPositions[pointLightCount] = lights[i].getPosition();
			++pointLightCount;
		}
	}

	GLuint pointLightsPositionUniforms[10];
	char buffer[32];
	
	for (unsigned int i = 0; i < pointLightCount; i++)
	{
		sprintf(buffer, "pointLightsPositions[%d]", i);
		pointLightsPositionUniforms[i] = glGetUniformLocation(shaderProgramme, buffer);
	}

	std::vector<AModel*> models = ALuaHelper::loadModelsFromTable("models", &luaHandler);

	ACamera& acamera = arenderer.getCamera();
	ALuaHelper::setupCameraPosition("cameraPosition", &acamera, &luaHandler);
	glm::mat4 projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	glm::mat4 view = acamera.getView();
	glm::mat4 viewProjectionMatrix = projection * view;

	glm::vec3 cameraPosition = acamera.getPos();

	AFramebuffer highresFramebuffer(width * 2, height * 2, GL_RGB32F);
	ARenderQuad renderQuad(luaHandler.getGlobalString("hdrShader"));

	float sum = 0.0f;
	float lum = 0.0f;
	float logAve = 0.0f;
	unsigned int size = width * height * 4;
	GLfloat *texData = new GLfloat[size * 3];

	std::function<float(float, float, float)> calculateLuminance = [](float r, float g, float b) -> float {
		return 0.299 * r + 0.587 * g + 0.114 * b;
	};

	GLuint logAveUniform = glGetUniformLocation(renderQuad.getProgramme(), "logAve");
	GLuint exposureUniform = glGetUniformLocation(renderQuad.getProgramme(), "exposure");
	float exposure = luaHandler.getGlobalNumber("exposure");
	
	glActiveTexture(GL_TEXTURE0);
	do
	{
		projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
		view = acamera.getView();
		viewProjectionMatrix = projection * view;
		cameraPosition = acamera.getPos();
		
		arenderer.startFrame();

		highresFramebuffer.bindBuffer();
			glViewport(0, 0, width * 2, height * 2);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			for (unsigned int i = 0; i < lightObjects.size(); i++)
			{
				lightObjects[i].renderLightObject(viewProjectionMatrix);
			}
			
			glUseProgram(shaderProgramme);
			glUniformMatrix4fv(objectsVMatrixUniform, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(objectsPMatrixUniform, 1, GL_FALSE, glm::value_ptr(projection));
			glUniform3f(objectsViewPositionUniform, cameraPosition.x, cameraPosition.y, cameraPosition.z);
			glUniform1i(objectsNumberPointLightsUniform, pointLightCount);

			for (unsigned int i = 0; i < lightObjects.size(); i++)
			{
				lightObjects[i].setupUniforms();
			}
			for (unsigned int i = 0; i < pointLightCount; i++)
			{
				glUniform3f(pointLightsPositionUniforms[i], pointLightPositions[i].x, pointLightPositions[i].y, pointLightPositions[i].z);
			}

			AModel::renderModelsInList(&models, modelMatrixUniform, shaderProgramme);
		highresFramebuffer.unbindBuffer();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, highresFramebuffer.getFramebufferTexture()); 
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, texData);
		sum = 0.0f;
		for(unsigned int i = 0; i < size; i += 3) {
			lum = calculateLuminance(texData[i+0], texData[i+1], texData[i+2]);
			sum += logf(lum + 0.00001f); 
		}
		logAve = expf(sum / size);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width * 2, height * 2);
		glUseProgram(renderQuad.getProgramme());
		glUniform1f(logAveUniform, logAve);
		glUniform1f(exposureUniform, exposure);
		renderQuad.render(highresFramebuffer.getFramebufferTexture(), false);

		arenderer.finishFrame();
	}
	while(arenderer.isRunning());

	glfwTerminate();
	delete texData;
	return 0;
}