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
#include <RenderingEngine/Utils/amacrohelper.hpp>
#include <RenderingEngine/Utils/aluahelper.hpp>
#include <RenderingEngine/Objects/alightobject.hpp>

#include <perlinFunction.hpp>

#include "atexture.hpp"

#include <stb_image.h>

int main(void)
{
	int width = 800;
	int height = 600;
	ARenderer arenderer(width, height, "Terrain Generator Scene");	
	arenderer.changeClearColor(glm::vec4(0.02f, 0.02f, 0.02f, 0.0f));

	LuaHandler luaHandler;
	luaHandler.openFile("config.lua");
	
  	GLuint vs =  AShader::generateShader(luaHandler.getGlobalString("vertexShader"), GL_VERTEX_SHADER);
	GLuint fs =  AShader::generateShader(luaHandler.getGlobalString("fragmentShader"), GL_FRAGMENT_SHADER);

  	GLuint shaderProgramme = AShader::generateProgram(vs, fs);

	GLuint modelMatrixUniform = glGetUniformLocation(shaderProgramme, "model");
	GLuint vpMatrixUniform = glGetUniformLocation(shaderProgramme, "viewProjection");
	GLuint textureUniform1 = glGetUniformLocation(shaderProgramme, "textureUniform1");
	GLuint textureUniform2 = glGetUniformLocation(shaderProgramme, "textureUniform2");
	GLuint textureUniform3 = glGetUniformLocation(shaderProgramme, "textureUniform3");
	GLuint timeUniform = glGetUniformLocation(shaderProgramme, "time");

	std::vector<AModel*> models = ALuaHelper::loadModelsFromTable("models", &luaHandler);

	ACamera& acamera = arenderer.getCamera();
	ALuaHelper::setupCameraPosition("cameraPosition", &acamera, &luaHandler);
	glm::mat4 projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	glm::mat4 view = acamera.getView();
	glm::mat4 viewProjectionMatrix = projection * view;

	ATextureData atextureData1(600, 600);
	ATextureData atextureData2(600, 600);
	ATextureData atextureData3(600, 600);
	PerlinNoise::generatePerlinNoise(atextureData1.width, atextureData1.height, 16, 16, atextureData1.data);
	PerlinNoise::generatePerlinNoise(atextureData2.width, atextureData2.height,  8,  8, atextureData2.data);
	PerlinNoise::generatePerlinNoise(atextureData3.width, atextureData3.height,  4,  4, atextureData3.data);

	ATextureHolder atexture1(atextureData1);
	ATextureHolder atexture2(atextureData2);
	ATextureHolder atexture3(atextureData3);
	do
	{
		projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
		view = acamera.getView();
		viewProjectionMatrix = projection * view;
		
		arenderer.startFrame();

		glViewport(0, 0, width * 2, height * 2);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.4f, 0.04f, 0.25f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaderProgramme);

		glUniformMatrix4fv(vpMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
		glUniform1f(timeUniform, arenderer.getAccumulator());
		
		atexture1.bindTexture(1);
		glUniform1i(textureUniform1, 1);
		atexture2.bindTexture(2);
		glUniform1i(textureUniform2, 2);
		atexture3.bindTexture(3);
		glUniform1i(textureUniform3, 3);
		
		AModel::renderModelsInList(&models, modelMatrixUniform, shaderProgramme);

		arenderer.finishFrame();
		atexture1.unbindTexture(3);
		atexture1.unbindTexture(2);
		atexture1.unbindTexture(1);
	}
	while(arenderer.isRunning());

	glfwTerminate();
	return 0;
}