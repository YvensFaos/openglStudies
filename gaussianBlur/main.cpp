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

int main(void)
{
	int width = 800;
	int height = 600;
	ARenderer arenderer(width, height, "Empty Project Scene");	
	arenderer.changeClearColor(glm::vec4(0.02f, 0.02f, 0.02f, 0.0f));

	LuaHandler luaHandler;
	luaHandler.openFile("config.lua");
	
  	GLuint vs =  AShader::generateShader(luaHandler.getGlobalString("vertexShader"), GL_VERTEX_SHADER);
	GLuint fs =  AShader::generateShader(luaHandler.getGlobalString("fragmentShader"), GL_FRAGMENT_SHADER);

  	GLuint shaderProgramme = AShader::generateProgram(vs, fs);

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

	std::vector<AModel> models = ALuaHelper::loadModelsFromTable("models", luaHandler);
	ALight alight = ALuaHelper::loadLightFromTable("light", luaHandler);

	ACamera& acamera = arenderer.getCamera();
	ALuaHelper::setupCameraPosition("cameraPosition", acamera, luaHandler);
	glm::mat4 projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	glm::mat4 view = acamera.getView();
	glm::mat4 viewProjectionMatrix = projection * view;
	glm::mat4 skyViewProjectionMatrix = projection * glm::mat4(glm::mat3(view));
	glm::mat4 skyView = glm::mat4(1.0);

	glm::vec3 lightPosition = alight.getPosition();
	glm::vec3 lightDirection = alight.getDirection();
	glm::vec3 lightUp = alight.getUp();
	glm::vec4 lightColor = alight.getColor();
	float lightIntensity = alight.getIntensity();
	bool lightDirectional = alight.getDirectional();

	AFramebuffer framebuffer1(width * 2, height * 2);
	ARenderQuad renderQuad1(luaHandler.getGlobalString("gaussianBlur1"));
	AFramebuffer framebuffer2(width * 2, height * 2);
	ARenderQuad renderQuad2(luaHandler.getGlobalString("gaussianBlur2"));

	float sigma = luaHandler.getGlobalNumber("sigma");
	const double PI = std::atan(1.0) * 4.0;
	float twoSquaredSigma = 2 * (sigma * sigma);
	float twoPiSquaredSigma = static_cast<float>(PI) * twoSquaredSigma;
	float invertedRoot = 1.0 / sqrtf(twoPiSquaredSigma);
	std::function<float(float)> gaussianFunction = [invertedRoot, twoSquaredSigma](float value) { 
		return invertedRoot * exp(-1.0 * (value * value) / twoSquaredSigma);
	};

	float weight[5];
	std::string buffer[5];
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
		buffer[i] = "";
		buffer[i].append(uniformName);
		weightUniforms1[i] = glGetUniformLocation(renderQuad1.getProgramme(), buffer[i].c_str());
		weightUniforms2[i] = glGetUniformLocation(renderQuad2.getProgramme(), buffer[i].c_str());
		printf("GAUSSIAN WEIGHT [%d = %f].\n", i, weight[i]);
	}

	bool showSkybox = luaHandler.getGlobalBoolean("showSkybox");

	glActiveTexture(GL_TEXTURE0);
	do
	{
		projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
		view = acamera.getView();
		viewProjectionMatrix = projection * view;
		skyViewProjectionMatrix = projection * glm::mat4(glm::mat3(view));

		arenderer.startFrame();
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

		framebuffer1.bindBuffer();
		framebuffer1.setViewport();
		glClearColor(0.02f, 0.04f, 0.25f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glUseProgram(shaderProgramme);
		glUniformMatrix4fv(vpMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
		glUniform3f(lightPositionUniform, lightPosition.x, lightPosition.y, lightPosition.z);
		glUniform3f(lightDirectionUniform, lightDirection.x, lightDirection.y, lightDirection.z);
		glUniform4f(lightColorUniform, lightColor.x, lightColor.y, lightColor.z, lightColor.w);
		glUniform1f(lightIntensityUniform, lightIntensity);
		glUniform1i(lightDirectionalUniform, lightDirectional);

		AModel::renderModelsInList(models, modelMatrixUniform, shaderProgramme);

		if(showSkybox) {
			askybox.render(skyViewProjectionMatrix);
		}
		framebuffer1.unbindBuffer();

		framebuffer2.bindBuffer();
		framebuffer2.setViewport();
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glUseProgram(renderQuad1.getProgramme());
		for (unsigned int i = 0; i < 5; i++)
		{
			glUniform1f(weightUniforms1[i], weight[i]);
		}
		renderQuad1.render(framebuffer1.getFramebufferTexture(), false);
		framebuffer2.unbindBuffer();

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glUseProgram(renderQuad2.getProgramme());
		for (unsigned int i = 0; i < 5; i++)
		{
			glUniform1f(weightUniforms2[i], weight[i]);
		}
		renderQuad2.render(framebuffer2.getFramebufferTexture(), false);

		arenderer.finishFrame();
	}
	while(arenderer.isRunning());

	glfwTerminate();
	return 0;
}