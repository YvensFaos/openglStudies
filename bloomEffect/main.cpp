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
	ARenderer arenderer(width, height, "Bloom Scene");	
	arenderer.changeClearColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));

	LuaHandler luaHandler;
	luaHandler.openFile("config.lua");
	
  	GLuint vs = AShader::generateShader(luaHandler.getGlobalString("vertexShader"), GL_VERTEX_SHADER);
	GLuint fs = AShader::generateShader(luaHandler.getGlobalString("multiLightFragmentShader"), GL_FRAGMENT_SHADER);

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

	std::vector<AModel> models = ALuaHelper::loadModelsFromTable("models", luaHandler);

	ACamera& acamera = arenderer.getCamera();
	ALuaHelper::setupCameraPosition("cameraPosition", acamera, luaHandler);
	glm::mat4 projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	glm::mat4 view = acamera.getView();
	glm::mat4 viewProjectionMatrix = projection * view;

	glm::vec3 cameraPosition = acamera.getPos();	

	AFramebuffer highresFramebuffer(width * 2, height * 2, GL_RGB32F);
	ARenderQuad renderQuad(luaHandler.getGlobalString("hdrShader"));

	ARenderQuad hdrGammaQuad(luaHandler.getGlobalString("hdrGammaCorrection"));	
	AFramebuffer normalExposureBuffer(width * 2, height * 2, GL_RGB32F);
	AFramebuffer   highExposureBuffer(width * 2, height * 2, GL_RGB32F);

	float downsample = luaHandler.getGlobalNumber("downsample");

	AFramebuffer thresholdBuffer(width / downsample, height / downsample, GL_RGB32F);
	ARenderQuad thresholdRenderQuad(luaHandler.getGlobalString("thresholdShader"));

	AFramebuffer blur1Framebuffer(width / downsample, height / downsample);
	ARenderQuad blur1RenderQuad(luaHandler.getGlobalString("gaussianBlur1"));
	AFramebuffer blur2Framebuffer(width / downsample, height / downsample);
	ARenderQuad blur2RenderQuad(luaHandler.getGlobalString("gaussianBlur2"));

	ARenderQuad combineRenderQuad(luaHandler.getGlobalString("combineShader"));

	float sum = 0.0f;
	float lum = 0.0f;
	float logAve = 0.0f;
	unsigned int size = width * height * 4;
	GLfloat *texData = new GLfloat[size * 3];

	std::function<float(float, float, float)> calculateLuminance = [](float r, float g, float b) -> float {
		return 0.299 * r + 0.587 * g + 0.114 * b;
	};

	float sigma = luaHandler.getGlobalNumber("sigma");
	const double PI = std::atan(1.0) * 4.0;
	float twoSquaredSigma = 2 * (sigma * sigma);
	float twoPiSquaredSigma = static_cast<float>(PI) * twoSquaredSigma;
	float invertedRoot = 1.0 / sqrtf(twoPiSquaredSigma);
	std::function<float(float)> gaussianFunction = [invertedRoot, twoSquaredSigma](float value) { 
		return invertedRoot * exp(-1.0 * (value * value) / twoSquaredSigma);
	};

	float weight[5];
	std::string bufferStrings[5];
	float gaussianSum = 0.0f;

	GLuint weightUniforms1[5];
	GLuint weightUniforms2[5];

	char uniformName[20];
	for (unsigned int i = 0; i < 5; i++)
	{
		weight[i] = gaussianFunction(static_cast<float>(i));
		gaussianSum += i == 0 ? weight[i] : 2 * weight[i];
	}

	for (unsigned int i = 0; i < 5; i++)
	{
		weight[i] /= gaussianSum;
		sprintf(uniformName, "%s[%d]", "gaussianWeight", i);
		bufferStrings[i] = "";
		bufferStrings[i].append(uniformName);
		weightUniforms1[i] = glGetUniformLocation(blur1RenderQuad.getProgramme(), bufferStrings[i].c_str());
		weightUniforms2[i] = glGetUniformLocation(blur1RenderQuad.getProgramme(), bufferStrings[i].c_str());
		printf("GAUSSIAN WEIGHT [%d = %f].\n", i, weight[i]);
	}

	GLuint gammaUniform = glGetUniformLocation(hdrGammaQuad.getProgramme(), "gamma");
	GLuint hdrExposureUniform = glGetUniformLocation(hdrGammaQuad.getProgramme(), "exposure");

	float normalGamma = luaHandler.getGlobalNumber("normalGamma");
	float normalExposure = luaHandler.getGlobalNumber("normalExposure");

	float highGamma = luaHandler.getGlobalNumber("highGamma");
	float highExposure = luaHandler.getGlobalNumber("highExposure");

	GLuint logAveUniform = glGetUniformLocation(renderQuad.getProgramme(), "logAve");
	GLuint exposureUniform = glGetUniformLocation(renderQuad.getProgramme(), "exposure");

	GLuint thresholdUniform = glGetUniformLocation(thresholdRenderQuad.getProgramme(), "threshold");
	float threshold = luaHandler.getGlobalNumber("threshold");

	GLuint texture1Uniform = glGetUniformLocation(combineRenderQuad.getProgramme(), "texture1");
	GLuint texture2Uniform = glGetUniformLocation(combineRenderQuad.getProgramme(), "texture2");

	std::vector<GLuint> texturesVector = {normalExposureBuffer.getFramebufferTexture(), blur2Framebuffer.getFramebufferTexture()};

	arenderer.addKeybind(AKeyBind(GLFW_KEY_G, [&normalGamma](int action, int mods) { 
		if(mods == GLFW_MOD_SHIFT) {
			if(action == GLFW_PRESS || action == GLFW_REPEAT) {
				normalGamma += 0.1f;
			}
		} else {
			if(action == GLFW_PRESS || action == GLFW_REPEAT) {
				normalGamma -= 0.1f;
			}
		}
		printf(" normalGamma > %f\n", normalGamma);
	}));

	arenderer.addKeybind(AKeyBind(GLFW_KEY_H, [&highGamma](int action, int mods) { 
		if(mods == GLFW_MOD_SHIFT) {
			if(action == GLFW_PRESS || action == GLFW_REPEAT) {
				highGamma += 0.1f;
			}
		} else {
			if(action == GLFW_PRESS || action == GLFW_REPEAT) {
				highGamma -= 0.1f;
			}
		}
		printf(" highGamma > %f\n", highGamma);
	}));

	arenderer.addKeybind(AKeyBind(GLFW_KEY_B, [&highExposure](int action, int mods) { 
		if(mods == GLFW_MOD_SHIFT) {
			if(action == GLFW_PRESS || action == GLFW_REPEAT) {
				highExposure += 0.01f;
			}
		} else {
			if(action == GLFW_PRESS || action == GLFW_REPEAT) {
				highExposure -= 0.01f;
			}
		}
		printf(" highExposure > %f\n", highExposure);
	}));

	arenderer.addKeybind(AKeyBind(GLFW_KEY_N, [&normalExposure](int action, int mods) { 
		if(mods == GLFW_MOD_SHIFT) {
			if(action == GLFW_PRESS || action == GLFW_REPEAT) {
				normalExposure += 0.01f;
			}
		} else {
			if(action == GLFW_PRESS || action == GLFW_REPEAT) {
				normalExposure -= 0.01f;
			}
		}
		printf(" normalExposure > %f\n", normalExposure);
	}));

	arenderer.addKeybind(AKeyBind(GLFW_KEY_M, [&threshold](int action, int mods) { 
		if(mods == GLFW_MOD_SHIFT) {
			if(action == GLFW_PRESS || action == GLFW_REPEAT) {
				threshold += 0.01f;
			}
		} else {
			if(action == GLFW_PRESS || action == GLFW_REPEAT) {
				threshold -= 0.01f;
			}
		}
		printf(" threshold > %f\n", threshold);
	}));
	
	glActiveTexture(GL_TEXTURE0);
	glActiveTexture(GL_TEXTURE1);
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
			AModel::renderModelsInList(models, modelMatrixUniform, shaderProgramme);
		highresFramebuffer.unbindBuffer();

		normalExposureBuffer.bindBuffer();
			glViewport(0, 0, width * 2, height * 2);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glUseProgram(hdrGammaQuad.getProgramme());
			glUniform1f(gammaUniform, normalGamma);
			glUniform1f(hdrExposureUniform, normalExposure);
			hdrGammaQuad.render(highresFramebuffer.getFramebufferTexture(), false);
		normalExposureBuffer.unbindBuffer();

		highExposureBuffer.bindBuffer();
			glViewport(0, 0, width * 2, height * 2);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glUseProgram(hdrGammaQuad.getProgramme());
			glUniform1f(gammaUniform, highGamma);
			glUniform1f(hdrExposureUniform, highExposure);
			hdrGammaQuad.render(highresFramebuffer.getFramebufferTexture(), false);
		highExposureBuffer.unbindBuffer();

		thresholdBuffer.bindBuffer();
			glViewport(0, 0, width / downsample, height / downsample);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glUseProgram(thresholdRenderQuad.getProgramme());
			glUniform1f(thresholdUniform, threshold);
			thresholdRenderQuad.render(highExposureBuffer.getFramebufferTexture(), false);
		thresholdBuffer.unbindBuffer();

		blur1Framebuffer.bindBuffer();
			glViewport(0, 0, width / downsample, height / downsample);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glUseProgram(blur1RenderQuad.getProgramme());
			for (unsigned int i = 0; i < 5; i++)
			{
				glUniform1f(weightUniforms1[i], weight[i]);
			}
			glUniform1f(thresholdUniform, threshold);
			blur1RenderQuad.render(thresholdBuffer.getFramebufferTexture(), false);
		blur1Framebuffer.unbindBuffer();

		blur2Framebuffer.bindBuffer();
			glViewport(0, 0, width / downsample, height / downsample);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glUseProgram(blur2RenderQuad.getProgramme());
			for (unsigned int i = 0; i < 5; i++)
			{
				glUniform1f(weightUniforms1[i], weight[i]);
			}
			glUniform1f(thresholdUniform, threshold);
			blur2RenderQuad.render(blur1Framebuffer.getFramebufferTexture(), false);
		blur2Framebuffer.unbindBuffer();

		glViewport(0, 0, width * 2, height * 2);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(combineRenderQuad.getProgramme());
		combineRenderQuad.render(texturesVector);
		arenderer.finishFrame();
	}
	while(arenderer.isRunning());

	glfwTerminate();
	delete[] texData;
	return 0;
}