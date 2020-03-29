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
	highresFramebuffer.setBufferShowFlag(0x02);
	ARenderQuad renderQuad(luaHandler.getGlobalString("hdrShader"));

	ARenderQuad hdrGammaQuad(luaHandler.getGlobalString("hdrGammaCorrection"));	
	AFramebuffer normalExposureBuffer(width * 2, height * 2, GL_RGB32F);
	normalExposureBuffer.setBufferShowFlag(0x04);
	
	float downsample = luaHandler.getGlobalNumber("downsample");

	AFramebuffer   highExposureBuffer(width / downsample, height / downsample, GL_RGB32F);
	highExposureBuffer.setBufferShowFlag(0x08);

	AFramebuffer thresholdBuffer(width / downsample, height / downsample, GL_RGB32F);
	ARenderQuad thresholdRenderQuad(luaHandler.getGlobalString("thresholdShader"));
	thresholdBuffer.setBufferShowFlag(0x10);

	AFramebuffer blur1Framebuffer(width / downsample, height / downsample, GL_RGB16F, GL_RGB, GL_FLOAT);
	ARenderQuad blur1RenderQuad(luaHandler.getGlobalString("gaussianBlur1"));
	blur1Framebuffer.setBufferShowFlag(0x20);

	AFramebuffer blur2Framebuffer(width / downsample, height / downsample, GL_RGB16F, GL_RGB, GL_FLOAT);
	ARenderQuad blur2RenderQuad(luaHandler.getGlobalString("gaussianBlur2"));
	blur2Framebuffer.setBufferShowFlag(0x40);

	GLuint samplers[2];
    glGenSamplers(2, samplers);
    GLuint linearSampler = samplers[0];
    GLuint nearestSampler = samplers[1];
    GLfloat border[] = {0.0f,0.0f,0.0f,0.0f};
    glSamplerParameteri(nearestSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(nearestSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(nearestSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(nearestSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameterfv(nearestSampler, GL_TEXTURE_BORDER_COLOR, border);

	glSamplerParameteri(linearSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(linearSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(linearSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(linearSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameterfv(linearSampler, GL_TEXTURE_BORDER_COLOR, border);
	
	glBindSampler(0, nearestSampler);
    glBindSampler(1, nearestSampler);

	ARenderQuad combineRenderQuad(luaHandler.getGlobalString("combineShader"));

	float sigma = luaHandler.getGlobalNumber("sigma");
	float sigma2 = (sigma * sigma);
	float coeff = 1.0 / glm::two_pi<float>() * sigma2;
	std::function<float(float)> gaussianFunction = [coeff, sigma2](float value) { 
		return (float) (coeff * exp(-(value*value) / (2.0 * sigma2)));
	};

	int GAUSSIAN_WEIGHT = luaHandler.getGlobalInteger("gaussianWeight");

	float* weight = new float[GAUSSIAN_WEIGHT];
	std::string* bufferStrings = new std::string[GAUSSIAN_WEIGHT];
	float gaussianSum = 0.0f;

	GLuint* weightUniforms1 = new GLuint[GAUSSIAN_WEIGHT];
	GLuint* weightUniforms2 = new GLuint[GAUSSIAN_WEIGHT];

	char uniformName[20];
	for (unsigned int i = 0; i < GAUSSIAN_WEIGHT; i++)
	{
		weight[i] = gaussianFunction(static_cast<float>(i));
		gaussianSum += i == 0 ? weight[i] : 2 * weight[i];
	}

	for (unsigned int i = 0; i < GAUSSIAN_WEIGHT; i++)
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

	GLuint texture1Uniform = glGetUniformLocation(combineRenderQuad.getProgramme(), "texture1");
	GLuint texture2Uniform = glGetUniformLocation(combineRenderQuad.getProgramme(), "texture2");
	GLuint bloomFactorUniform = glGetUniformLocation(combineRenderQuad.getProgramme(), "bloomFactor");
	float bloomFactor = luaHandler.getGlobalNumber("bloomFactor");

	std::vector<GLuint> texturesVector = {normalExposureBuffer.getFramebufferTexture(), blur2Framebuffer.getFramebufferTexture()};

	arenderer.addKeybind(APressKeyBind(GLFW_KEY_G, [&normalGamma](int action, int mods) { 
		if(mods == GLFW_MOD_SHIFT) { normalGamma += 0.1f; } else { normalGamma -= 0.1f; }
		printf(" normalGamma > %f\n", normalGamma);
	}, GLFW_PRESS | GLFW_REPEAT));

	arenderer.addKeybind(APressKeyBind(GLFW_KEY_H, [&highGamma](int action, int mods) { 
		if(mods == GLFW_MOD_SHIFT) { highGamma += 0.1f; } else { highGamma -= 0.1f; }
		printf(" highGamma > %f\n", highGamma);
	}, GLFW_PRESS | GLFW_REPEAT));

	arenderer.addKeybind(APressKeyBind(GLFW_KEY_B, [&highExposure](int action, int mods) { 
		if(mods == GLFW_MOD_SHIFT) { highExposure += 0.1f; } else { highExposure -= 0.1f; }
		printf(" highExposure > %f\n", highExposure);
	}, GLFW_PRESS | GLFW_REPEAT));

	arenderer.addKeybind(APressKeyBind(GLFW_KEY_N, [&normalExposure](int action, int mods) { 
		if(mods == GLFW_MOD_SHIFT) { normalExposure += 0.1f; } else { normalExposure -= 0.1f; }
		printf(" normalExposure > %f\n", normalExposure);
	}, GLFW_PRESS | GLFW_REPEAT));

	unsigned int showBuffer = luaHandler.getGlobalInteger("initialMask"); //0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80;

	arenderer.addKeybind(AKeyBind(GLFW_KEY_0, [&showBuffer](int action, int mods) { 
		printf("Print Current flag: %d\n", showBuffer);
	}));
	arenderer.addKeybind(AFlagTogglerKeyBind(GLFW_KEY_1, showBuffer, 0x01));
	arenderer.addKeybind(AFlagTogglerKeyBind(GLFW_KEY_2, showBuffer, 0x02));
	arenderer.addKeybind(AFlagTogglerKeyBind(GLFW_KEY_3, showBuffer, 0x04));
	arenderer.addKeybind(AFlagTogglerKeyBind(GLFW_KEY_4, showBuffer, 0x08));
	arenderer.addKeybind(AFlagTogglerKeyBind(GLFW_KEY_5, showBuffer, 0x10));
	arenderer.addKeybind(AFlagTogglerKeyBind(GLFW_KEY_6, showBuffer, 0x20));
	arenderer.addKeybind(AFlagTogglerKeyBind(GLFW_KEY_7, showBuffer, 0x40));
	
	unsigned int gaussianBlurPasses = luaHandler.getGlobalInteger("gaussianBlurPasses");

	float averageFPS = 0.0f;
	int frameCount = 0;
	glActiveTexture(GL_TEXTURE0);
	glActiveTexture(GL_TEXTURE1);
	do
	{
		projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
		view = acamera.getView();
		viewProjectionMatrix = projection * view;
		cameraPosition = acamera.getPos();
		
		arenderer.startFrame();

		glEnable(GL_DEPTH_TEST);

		highresFramebuffer.bindBuffer(showBuffer);
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

		glDisable(GL_DEPTH_TEST);

		normalExposureBuffer.bindBuffer(showBuffer);
			glViewport(0, 0, width * 2, height * 2);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glUseProgram(hdrGammaQuad.getProgramme());
			glBindSampler(0, nearestSampler);
			glBindSampler(1, nearestSampler);
			glUniform1f(gammaUniform, normalGamma);
			glUniform1f(hdrExposureUniform, normalExposure);
			hdrGammaQuad.render(highresFramebuffer.getFramebufferTexture(), false);
		normalExposureBuffer.unbindBuffer();

		highExposureBuffer.bindBuffer(showBuffer);
			glViewport(0, 0, width / downsample, height / downsample);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glUseProgram(hdrGammaQuad.getProgramme());
			glBindSampler(0, nearestSampler);
			glBindSampler(1, nearestSampler);
			glUniform1f(gammaUniform, highGamma);
			glUniform1f(hdrExposureUniform, highExposure);
			hdrGammaQuad.render(highresFramebuffer.getFramebufferTexture(), false);
		highExposureBuffer.unbindBuffer();

		thresholdBuffer.bindBuffer(showBuffer);
			glViewport(0, 0, width / downsample, height / downsample);
			glClear(GL_COLOR_BUFFER_BIT);
			glUseProgram(thresholdRenderQuad.getProgramme());
			glBindSampler(0, linearSampler);
			glBindSampler(1, linearSampler);
			thresholdRenderQuad.render(highExposureBuffer.getFramebufferTexture(), false);
		thresholdBuffer.unbindBuffer();		

		for(unsigned int i = 0; i < gaussianBlurPasses; i++) {
			
			blur1Framebuffer.bindBuffer(showBuffer);
				glViewport(0, 0, width / downsample, height / downsample);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glUseProgram(blur1RenderQuad.getProgramme());
				glBindSampler(0, nearestSampler);
				glBindSampler(1, nearestSampler);
				for (unsigned int i = 0; i < GAUSSIAN_WEIGHT; i++)
				{
					glUniform1f(weightUniforms1[i], weight[i]);
				}

				if(i == 0) {
					blur1RenderQuad.render(thresholdBuffer.getFramebufferTexture(), false);
				} else {
					blur1RenderQuad.render(blur2Framebuffer.getFramebufferTexture(), false);
				}

			blur1Framebuffer.unbindBuffer();

			blur2Framebuffer.bindBuffer(showBuffer);
				glViewport(0, 0, width / downsample, height / downsample);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glUseProgram(blur2RenderQuad.getProgramme());
				glBindSampler(0, nearestSampler);
				glBindSampler(1, nearestSampler);
				for (unsigned int i = 0; i < GAUSSIAN_WEIGHT; i++)
				{
					glUniform1f(weightUniforms1[i], weight[i]);
				}

				blur2RenderQuad.render(blur1Framebuffer.getFramebufferTexture(), false);
			 blur2Framebuffer.unbindBuffer();
		}

		if(showBuffer & 0x01) {
			glViewport(0, 0, width * 2, height * 2);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glUseProgram(combineRenderQuad.getProgramme());
			glBindSampler(0, nearestSampler);
			glBindSampler(1, linearSampler);
			glUniform1f(bloomFactorUniform, bloomFactor);
			combineRenderQuad.render(texturesVector);
			glBindSampler(0, nearestSampler);
			glBindSampler(1, nearestSampler);
		}
		arenderer.finishFrame();
		averageFPS += arenderer.getDeltaTime();
		++frameCount;
	}
	while(arenderer.isRunning());

	glfwTerminate();
	delete[] weight;
	delete[] bufferStrings;
	delete[] weightUniforms1;
	delete[] weightUniforms2;

	printf("Average Time Spent: %4.6f.\r\n", averageFPS / static_cast<float>(frameCount));
	printf("Average FPS:        %4.6f.\r\n", 1.0f / (averageFPS / static_cast<float>(frameCount)));


	return 0;
}