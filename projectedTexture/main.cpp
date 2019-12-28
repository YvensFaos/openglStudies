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
#include <RenderingEngine/Objects/atextureholder.hpp>

#include <stb_image.h>

int main(void)
{
	int width = 800;
	int height = 600;
	ARenderer arenderer(width, height, "Projected Texture");	
	arenderer.changeClearColor(glm::vec4(0.02f, 0.02f, 0.02f, 0.0f));

	LuaHandler luaHandler;
	luaHandler.openFile("config.lua");
	
  	GLuint vs =  AShader::generateShader(luaHandler.getGlobalString("vertexShader"), GL_VERTEX_SHADER);
	GLuint fs =  AShader::generateShader(luaHandler.getGlobalString("fragmentShader"), GL_FRAGMENT_SHADER);

  	GLuint shaderProgramme = AShader::generateProgram(vs, fs);

	GLuint modelMatrixUniform = glGetUniformLocation(shaderProgramme, "model");
	GLuint vMatrixUniform = glGetUniformLocation(shaderProgramme, "view");
	GLuint pMatrixUniform = glGetUniformLocation(shaderProgramme, "projection");
	GLuint projectedMatrixUniform = glGetUniformLocation(shaderProgramme, "projected");
	
	GLuint lightMatrixUniform = glGetUniformLocation(shaderProgramme, "lightViewProjection");

	GLuint    lightPositionUniform = glGetUniformLocation(shaderProgramme, "sceneLight.position");
	GLuint   lightDirectionUniform = glGetUniformLocation(shaderProgramme, "sceneLight.direction");
	GLuint       lightColorUniform = glGetUniformLocation(shaderProgramme, "sceneLight.color");
	GLuint   lightIntensityUniform = glGetUniformLocation(shaderProgramme, "sceneLight.intensity");
	GLuint lightDirectionalUniform = glGetUniformLocation(shaderProgramme, "sceneLight.directional");
	GLuint projectedTextureUniform = glGetUniformLocation(shaderProgramme, "projectedTexture");

	std::vector<AModel> models = ALuaHelper::loadModelsFromTable("models", luaHandler);
	ALight alight = ALuaHelper::loadLightFromTable("light", luaHandler);

	std::string texturePath = luaHandler.getGlobalString("texturePath");
	std::string textureDirectory = luaHandler.getGlobalString("textureDirectory");

	ATextureHolder projectedTexture(texturePath.c_str(), textureDirectory, false, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);

	ACamera& acamera = arenderer.getCamera();
	ALuaHelper::setupCameraPosition("cameraPosition", acamera, luaHandler);
	glm::mat4 projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	glm::vec3 projPos = ALuaHelper::readVec3FromTable("projPos", luaHandler);
	glm::vec3 projAt  = ALuaHelper::readVec3FromTable("projAt", luaHandler);
	glm::vec3 projUp  = ALuaHelper::readVec3FromTable("projUp", luaHandler);

	glm::vec3 projSca = ALuaHelper::readVec3FromTable("projSca", luaHandler);
	glm::vec3 projTra = ALuaHelper::readVec3FromTable("projTra", luaHandler);

	glm::mat4 projView = glm::lookAt(projPos, projAt, projUp); 
	glm::mat4 projProj = glm::perspective(30.0f, 1.0f, 0.2f, 1000.0f);
	glm::mat4 projScaleTransform = glm::translate(projTra) * glm::scale(projSca);
	glm::mat4 projectedMatrix = projScaleTransform * projProj * projView;

	glm::mat4 view = acamera.getView();
	glm::mat4 viewProjectionMatrix = projection * view;

	glActiveTexture(GL_TEXTURE0);
	do
	{
		projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
		view = acamera.getView();
		viewProjectionMatrix = projection * view;

		arenderer.startFrame();

		glViewport(0, 0, width * 2, height * 2);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.02f, 0.04f, 0.25f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaderProgramme);

		projectedTexture.bindTexture(0);
		glUniform1i(projectedTextureUniform, 0);
		glUniformMatrix4fv(vMatrixUniform, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(pMatrixUniform, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(projectedMatrixUniform, 1, GL_FALSE, glm::value_ptr(projectedMatrix));
		alight.setupUniforms(lightPositionUniform, lightDirectionUniform, lightColorUniform, lightIntensityUniform, lightDirectionalUniform);
		AModel::renderModelsInList(models, modelMatrixUniform, shaderProgramme, GL_TRIANGLES, false);

		arenderer.finishFrame();
	}
	while(arenderer.isRunning());

	glfwTerminate();
	return 0;
}