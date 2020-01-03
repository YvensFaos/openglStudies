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
	GLuint cs = AShader::generateShader(luaHandler.getGlobalString("controlTesselationShader"), GL_TESS_CONTROL_SHADER);
	GLuint es = AShader::generateShader(luaHandler.getGlobalString("evaluationTesselationShader"), GL_TESS_EVALUATION_SHADER);
	GLuint gs =  AShader::generateShader(luaHandler.getGlobalString("geometryShader"), GL_GEOMETRY_SHADER);
	GLuint fs =  AShader::generateShader(luaHandler.getGlobalString("fragmentShader"), GL_FRAGMENT_SHADER);

	GLuint wgs =  AShader::generateShader(luaHandler.getGlobalString("wireGeometryShader"), GL_GEOMETRY_SHADER);
	GLuint wfs =  AShader::generateShader(luaHandler.getGlobalString("wireFragmentShader"), GL_FRAGMENT_SHADER);

	GLuint shaderProgramme = AShader::generateProgram(std::vector<GLuint>({vs, cs, es, gs, fs}));

	GLuint modelMatrixUniform = glGetUniformLocation(shaderProgramme, "model");
	GLuint vpMatrixUniform = glGetUniformLocation(shaderProgramme, "viewProjection");
	GLuint textureUniform0 = glGetUniformLocation(shaderProgramme, "textureUniform0");
	GLuint textureUniform1 = glGetUniformLocation(shaderProgramme, "textureUniform1");
	GLuint textureUniform2 = glGetUniformLocation(shaderProgramme, "textureUniform2");
	GLuint textureUniform3 = glGetUniformLocation(shaderProgramme, "textureUniform3");
	GLuint objectsNumberPointLightsUniform = glGetUniformLocation(shaderProgramme, "numberPointLights");
	GLuint objectsNumberDirectionLightsUniform = glGetUniformLocation(shaderProgramme, "numberDirectionLights");
	GLuint maxTessLevelUniform = glGetUniformLocation(shaderProgramme, "maxTessLevel");

	GLuint wireShaderProgramme = AShader::generateProgram(std::vector<GLuint>({vs, cs, es, wgs, wfs}));

	GLuint wiremodelMatrixUniform = glGetUniformLocation(wireShaderProgramme, "model");
	GLuint wirevpMatrixUniform = glGetUniformLocation(wireShaderProgramme, "viewProjection");
	GLuint wiretextureUniform0 = glGetUniformLocation(wireShaderProgramme, "textureUniform0");
	GLuint wiretextureUniform1 = glGetUniformLocation(wireShaderProgramme, "textureUniform1");
	GLuint wiretextureUniform2 = glGetUniformLocation(wireShaderProgramme, "textureUniform2");
	GLuint wiretextureUniform3 = glGetUniformLocation(wireShaderProgramme, "textureUniform3");
	GLuint wireobjectsNumberPointLightsUniform = glGetUniformLocation(wireShaderProgramme, "numberPointLights");
	GLuint wireobjectsNumberDirectionLightsUniform = glGetUniformLocation(wireShaderProgramme, "numberDirectionLights");
	GLuint wiremaxTessLevelUniform = glGetUniformLocation(wireShaderProgramme, "maxTessLevel");
	GLuint wireColorUniform = glGetUniformLocation(wireShaderProgramme, "wireColor");

	std::vector<AModel> models = ALuaHelper::loadModelsFromTable("models", luaHandler);

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

	int TERRAIN = luaHandler.getGlobalInteger("terrainSize");
	int ELEVATN = luaHandler.getGlobalInteger("elevationSize");
	int DETAILS = luaHandler.getGlobalInteger("detailsSize");

	PerlinNoise::generatePerlinNoise(bwidth, bheight, DETAILS, DETAILS, buffer);
	ATextureData atextureData1(bwidth, bheight, buffer);
	ATextureHolder   atexture1(atextureData1, GL_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);

	PerlinNoise::generatePerlinNoise(bwidth, bheight, ELEVATN, ELEVATN, buffer);
	ATextureData atextureData2(bwidth, bheight, buffer);
	ATextureHolder   atexture2(atextureData2, GL_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);

	PerlinNoise::generatePerlinNoise(bwidth, bheight, TERRAIN, TERRAIN, buffer);
	ATextureData atextureData3(bwidth, bheight, buffer);
	ATextureHolder   atexture3(atextureData3, GL_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);

	delete[] buffer;

	ATextureHolder mountains("rocky_512.jpg", "../3DModels");

	float maxTessLevel = luaHandler.getGlobalNumber("maxTessLevel");

	glm::vec4 wireColor = ALuaHelper::readVec4FromTable("wireColor", luaHandler);

	bool tesselationOn = true;
	arenderer.addKeybind(APressKeyBind(GLFW_KEY_1, 
		[&tesselationOn](int action, int mods) {
			tesselationOn = !tesselationOn;
		}
	));

	arenderer.addKeybind(APressKeyBind(GLFW_KEY_2, 
		[&maxTessLevel](int action, int mods) {
			float value = mods == GLFW_MOD_SHIFT ? 2.0f : 1.0f;
			maxTessLevel = std::min(32.0f, maxTessLevel + value);
			printf("Max TesselationLevel = %2.2f.\n", maxTessLevel);
		}
	));
	arenderer.addKeybind(APressKeyBind(GLFW_KEY_3, 
		[&maxTessLevel](int action, int mods) {
			float value = mods == GLFW_MOD_SHIFT ? 2.0f : 1.0f;
			maxTessLevel = std::max(1.0f, maxTessLevel - value);
			printf("Max TesselationLevel = %2.2f.\n", maxTessLevel);
		}
	));

	do
	{
		projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
		view = acamera.getView();
		viewProjectionMatrix = projection * view;
		
		arenderer.startFrame();

		glViewport(0, 0, width * 2, height * 2);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(232.0f / 255.0f, 230.0f / 255.0f, 165.0f / 255.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		alightObject.renderLightObject(viewProjectionMatrix);

		if(tesselationOn) {
			glUseProgram(shaderProgramme);

			glUniformMatrix4fv(vpMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
			glUniform1i(objectsNumberPointLightsUniform, 0);
			glUniform1i(objectsNumberDirectionLightsUniform, 1);

			glUniform1f(maxTessLevelUniform, maxTessLevel);
			
			mountains.bindTexture(0);
			glUniform1i(textureUniform0, 0);
			atexture1.bindTexture(1);
			glUniform1i(textureUniform1, 1);
			atexture2.bindTexture(2);
			glUniform1i(textureUniform2, 2);
			atexture3.bindTexture(3);
			glUniform1i(textureUniform3, 3);

			alightObject.setupUniforms();
			AModel::renderPatchesModelsInList(models, modelMatrixUniform, shaderProgramme);

			atexture3.unbindTexture(3);
			atexture2.unbindTexture(2);
			atexture1.unbindTexture(1);
			mountains.unbindTexture(0);
		} else {
			glUseProgram(wireShaderProgramme);

			glUniformMatrix4fv(wirevpMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
			glUniform1i(wireobjectsNumberPointLightsUniform, 0);
			glUniform1i(wireobjectsNumberDirectionLightsUniform, 1);

			glUniform1f(wiremaxTessLevelUniform, maxTessLevel);
			
			mountains.bindTexture(0);
			glUniform1i(wiretextureUniform0, 0);
			atexture1.bindTexture(1);
			glUniform1i(wiretextureUniform1, 1);
			atexture2.bindTexture(2);
			glUniform1i(wiretextureUniform2, 2);
			atexture3.bindTexture(3);
			glUniform1i(wiretextureUniform3, 3);

			glUniform4f(wireColorUniform, wireColor.r, wireColor.g, wireColor.b, wireColor.a);

			alightObject.setupUniforms();
			AModel::renderPatchesModelsInList(models, wiremodelMatrixUniform, wireShaderProgramme);

			atexture3.unbindTexture(3);
			atexture2.unbindTexture(2);
			atexture1.unbindTexture(1);
			mountains.unbindTexture(0);
		}
		arenderer.finishFrame();
	}
	while(arenderer.isRunning());

	glfwTerminate();
	return 0;
}