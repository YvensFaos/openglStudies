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
#include <RenderingEngine/Objects/atextureholder.hpp>

#include <perlinFunction.hpp>

#include <stb_image.h>

int main(void)
{
	int width = 800;
	int height = 600;
	ARenderer arenderer(width, height, "Terrain Generator Scene");	
	arenderer.changeClearColor(glm::vec4(0.02f, 0.02f, 0.02f, 0.0f));
	arenderer.setCullFaces(false);

	LuaHandler luaHandler;
	luaHandler.openFile("config.lua");
	
  	GLuint vs =  AShader::generateShader(luaHandler.getGlobalString("vertexShader"), GL_VERTEX_SHADER);
	GLuint fs =  AShader::generateShader(luaHandler.getGlobalString("fragmentShader"), GL_FRAGMENT_SHADER);

  	GLuint shaderProgramme = AShader::generateProgram(vs, fs);

	GLuint modelMatrixUniform = glGetUniformLocation(shaderProgramme, "model");
	GLuint vpMatrixUniform = glGetUniformLocation(shaderProgramme, "viewProjection");
	GLuint textureUniform0 = glGetUniformLocation(shaderProgramme, "textureUniform0");
	GLuint textureUniform1 = glGetUniformLocation(shaderProgramme, "textureUniform1");
	GLuint textureUniform2 = glGetUniformLocation(shaderProgramme, "textureUniform2");
	GLuint textureUniform3 = glGetUniformLocation(shaderProgramme, "textureUniform3");

	GLuint showOnlyNormalsUniform = glGetUniformLocation(shaderProgramme, "showOnlyNormals");
	GLuint objectsNumberPointLightsUniform = glGetUniformLocation(shaderProgramme, "numberPointLights");
	GLuint objectsNumberDirectionLightsUniform = glGetUniformLocation(shaderProgramme, "numberDirectionLights");

	std::vector<AModel> models = ALuaHelper::loadModelsFromTable("models", luaHandler);

	int showOnlyNormals = 1;

	arenderer.addKeybind(AKeyBind(GLFW_KEY_U, 
		[&models](int action, int mods) {
			models[0].rotate(glm::vec3(0,5,0));
		}
	));

	arenderer.addKeybind(AKeyBind(GLFW_KEY_I, 
		[&showOnlyNormals](int action, int mods) {
			if(action == GLFW_PRESS) {
				showOnlyNormals = (showOnlyNormals == 1) ? 0 : 1;
			}
		}
	));

	ACamera& acamera = arenderer.getCamera();
	ALuaHelper::setupCameraPosition("cameraPosition", acamera, luaHandler);
	glm::mat4 projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	ALight alight = ALuaHelper::loadLightFromTable("light", luaHandler);
	ALightObject alightObject(alight, shaderProgramme);

	glm::mat4 view = acamera.getView();
	glm::mat4 viewProjectionMatrix = projection * view;

	uint bwidth = 600;
	uint bheight = 600;
	float* buffer = new float[bwidth * bheight * 4];
	srand(6);

	PerlinNoise::generatePerlinNoise(bwidth, bheight, 16, 16, buffer);
	ATextureData atextureData1(bwidth, bheight, buffer);
	ATextureHolder   atexture1(atextureData1, GL_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);

	PerlinNoise::generatePerlinNoise(bwidth, bheight, 8, 8, buffer);
	ATextureData atextureData2(bwidth, bheight, buffer);
	ATextureHolder   atexture2(atextureData2, GL_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);

	PerlinNoise::generatePerlinNoise(bwidth, bheight, 4, 4, buffer);
	ATextureData atextureData3(bwidth, bheight, buffer);
	ATextureHolder   atexture3(atextureData3, GL_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);

	delete[] buffer;

	ATextureHolder mountains("rocky_512.jpg", "../3DModels");
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

		alightObject.renderLightObject(viewProjectionMatrix);

		glUseProgram(shaderProgramme);

		glUniformMatrix4fv(vpMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));

		glUniform1i(showOnlyNormalsUniform, showOnlyNormals);
		glUniform1i(objectsNumberPointLightsUniform, 0);
		glUniform1i(objectsNumberDirectionLightsUniform, 1);
		
		mountains.bindTexture(0);
		glUniform1i(textureUniform0, 0);
		atexture1.bindTexture(1);
		glUniform1i(textureUniform1, 1);
		atexture2.bindTexture(2);
		glUniform1i(textureUniform2, 2);
		atexture3.bindTexture(3);
		glUniform1i(textureUniform3, 3);

		alightObject.setupUniforms();
		AModel::renderModelsInList(models, modelMatrixUniform, shaderProgramme);

		arenderer.finishFrame();
		atexture3.unbindTexture(3);
		atexture2.unbindTexture(2);
		atexture1.unbindTexture(1);
		mountains.unbindTexture(0);
	}
	while(arenderer.isRunning());

	glfwTerminate();
	return 0;
}