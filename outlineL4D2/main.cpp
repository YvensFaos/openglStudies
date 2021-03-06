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
	ARenderer arenderer(width, height, "Edge Detection Scene");	
	arenderer.changeClearColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

	LuaHandler luaHandler;
	luaHandler.openFile("config.lua");
	
  	GLuint vs =  AShader::generateShader(luaHandler.getGlobalString("vertexShader"), GL_VERTEX_SHADER);
	GLuint fs =  AShader::generateShader(luaHandler.getGlobalString("fragmentShader"), GL_FRAGMENT_SHADER);

  	GLuint shaderProgramme = AShader::generateProgram(vs, fs);
	GLuint modelMatrixUniform = glGetUniformLocation(shaderProgramme, "model");
	GLuint vpMatrixUniform = glGetUniformLocation(shaderProgramme, "viewProjection");
	GLuint lightMatrixUniform = glGetUniformLocation(shaderProgramme, "lightViewProjection");

	GLuint bs =  AShader::generateShader(luaHandler.getGlobalString("singleColorFragment"), GL_FRAGMENT_SHADER);

  	GLuint blockoutProgramme = AShader::generateProgram(vs, bs);

	GLuint bmodelMatrixUniform = glGetUniformLocation(blockoutProgramme, "model");
	GLuint bvpMatrixUniform = glGetUniformLocation(blockoutProgramme, "viewProjection");
	GLuint blightMatrixUniform = glGetUniformLocation(blockoutProgramme, "lightViewProjection");

	GLuint    lightPositionUniform = glGetUniformLocation(shaderProgramme, "sceneLight.position");
	GLuint   lightDirectionUniform = glGetUniformLocation(shaderProgramme, "sceneLight.direction");
	GLuint       lightColorUniform = glGetUniformLocation(shaderProgramme, "sceneLight.color");
	GLuint   lightIntensityUniform = glGetUniformLocation(shaderProgramme, "sceneLight.intensity");
	GLuint lightDirectionalUniform = glGetUniformLocation(shaderProgramme, "sceneLight.directional");

	AFramebuffer blockoutBuffer(width * 2, height * 2);
	AFramebuffer regularImage(width * 2, height * 2);
	AFramebuffer finalAuraBuffer(width * 2, height * 2);

	ARenderQuad edgeQuad(luaHandler.getGlobalString("edgeFragmentShader"));
	AFramebuffer edgeBuffer(width * 2, height * 2);

	ARenderQuad dilationQuad(luaHandler.getGlobalString("dilationFragmentShader"));
	AFramebuffer dilationBuffer(width * 2, height * 2);

	ARenderQuad blurQuad(luaHandler.getGlobalString("blurFragmentShader"));
	AFramebuffer blurBuffer(width * 2, height * 2);

	ARenderQuad auraOnlyQuad(luaHandler.getGlobalString("auraOnlyFragment"));
	AFramebuffer auraBlurred(width * 2, height * 2);

	ARenderQuad stacked(luaHandler.getGlobalString("stackUpFrames"));

	std::vector<AModel> models = ALuaHelper::loadModelsFromTable("models", luaHandler);
	ALight alight = ALuaHelper::loadLightFromTable("light", luaHandler);

	ACamera& acamera = arenderer.getCamera();
	glm::mat4 projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	glm::mat4 view = acamera.getView();
	glm::mat4 viewProjectionMatrix = projection * view;

	glm::vec3 lightPosition = 	alight.getPosition();
	glm::vec3 lightDirection = 	alight.getDirection();
	glm::vec3 lightUp = 		alight.getUp();
	glm::vec4 lightColor = 		alight.getColor();
	float lightIntensity = 		alight.getIntensity();
	bool lightDirectional = 	alight.getDirectional();
	
	std::vector<GLuint> textures = {finalAuraBuffer.getFramebufferTexture(), regularImage.getFramebufferTexture()};

	std::vector<GLuint> texturesAuraOnly = {auraBlurred.getFramebufferTexture(), blockoutBuffer.getFramebufferTexture()};

	glActiveTexture(GL_TEXTURE0);
	glActiveTexture(GL_TEXTURE1);
	glDepthFunc(GL_LESS);
	do
	{
		arenderer.startFrame();
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		blockoutBuffer.bindBuffer();
		blockoutBuffer.setViewport();
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(blockoutProgramme);
		glUniformMatrix4fv(bvpMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
		AModel::renderModelsInList(models, bmodelMatrixUniform, blockoutProgramme);
		blockoutBuffer.unbindBuffer();

		regularImage.bindBuffer();
		regularImage.setViewport();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaderProgramme);
		glUniformMatrix4fv(vpMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
		glUniform3f(lightPositionUniform, lightPosition.x, lightPosition.y, lightPosition.z);
		glUniform3f(lightDirectionUniform, lightDirection.x, lightDirection.y, lightDirection.z);
		glUniform4f(lightColorUniform, lightColor.x, lightColor.y, lightColor.z, lightColor.w);
		glUniform1f(lightIntensityUniform, lightIntensity);
		glUniform1i(lightDirectionalUniform, lightDirectional);
		AModel::renderModelsInList(models, modelMatrixUniform, shaderProgramme);
		regularImage.unbindBuffer();

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

		dilationBuffer.bindBuffer();
		dilationBuffer.setViewport();
		glClear(GL_COLOR_BUFFER_BIT);
		edgeQuad.render(blockoutBuffer.getFramebufferTexture());
		dilationBuffer.unbindBuffer();

		blurBuffer.bindBuffer();
		blurBuffer.setViewport();
		glClear(GL_COLOR_BUFFER_BIT);
		dilationQuad.render(dilationBuffer.getFramebufferTexture());
		blurBuffer.unbindBuffer();

		auraBlurred.bindBuffer();
		auraBlurred.setViewport();
		glClear(GL_COLOR_BUFFER_BIT);
		blurQuad.render(blurBuffer.getFramebufferTexture());
		auraBlurred.unbindBuffer();

		finalAuraBuffer.bindBuffer();
		finalAuraBuffer.setViewport();
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(auraOnlyQuad.getProgramme());
		auraOnlyQuad.render(texturesAuraOnly);
		finalAuraBuffer.unbindBuffer();

		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(stacked.getProgramme());
		stacked.render(textures);

		arenderer.finishFrame();
	}
	while(arenderer.isRunning());

	glfwTerminate();
	return 0;
}