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
	ARenderer arenderer(width, height, "Deferred Rendering Scene");	
	arenderer.changeClearColor(glm::vec4(1.0f, 0.02f, 0.02f, 0.0f));

	LuaHandler luaHandler;
	luaHandler.openFile("config.lua");
	
  	GLuint vs =  AShader::generateShader(luaHandler.getGlobalString("vertexShader"), GL_VERTEX_SHADER);
	GLuint fs =  AShader::generateShader(luaHandler.getGlobalString("fragmentShader"), GL_FRAGMENT_SHADER);

  	GLuint shaderProgramme = AShader::generateProgram(vs, fs);

	GLuint modelMatrixUniform = glGetUniformLocation(shaderProgramme, "model");
	GLuint vpMatrixUniform = glGetUniformLocation(shaderProgramme, "viewProjection");

	std::vector<AModel> models = ALuaHelper::loadModelsFromTable("models", luaHandler);
	std::vector<ALight> alights = ALuaHelper::loadLightsFromTable("lights", luaHandler);
	GLuint lightCount = alights.size();

	ACamera& acamera = arenderer.getCamera();
	ALuaHelper::setupCameraPosition("cameraPosition", acamera, luaHandler);
	glm::mat4 projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	glm::mat4 view = acamera.getView();
	glm::mat4 viewProjectionMatrix = projection * view;
	glm::mat4 skyViewProjectionMatrix = projection * glm::mat4(glm::mat3(view));

	AGBuffer gbuffer(width * 2, height * 2);
	ARenderQuad renderQuad(luaHandler.getGlobalString("gFragmentShader"));
	
	std::vector<GLuint> gtexturesVector;
	gtexturesVector.push_back(gbuffer.getPositionTextureID());
	gtexturesVector.push_back(gbuffer.getNormalTextureID());
	gtexturesVector.push_back(gbuffer.getColorTextureID());

	GLuint gbufferProgramme = renderQuad.getProgramme();
	std::vector<ALightUniform> lightUniforms;

	for (size_t i = 0; i < lightCount; i++)
	{
		lightUniforms.push_back(ALightUniform::loadALightUniformFromProgrammeWithName(gbufferProgramme, i, alights[i], "lights"));
	}

	do
	{
		projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
		view = acamera.getView();
		viewProjectionMatrix = projection * view;
		skyViewProjectionMatrix = projection * glm::mat4(glm::mat3(view));

		arenderer.startFrame();

		gbuffer.bindBuffer();
			gbuffer.setViewport();
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glUseProgram(shaderProgramme);
			glUniformMatrix4fv(vpMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
			AModel::renderModelsInList(models, modelMatrixUniform, shaderProgramme);
		gbuffer.unbindBuffer();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width * 2, height * 2);
		glClearColor(0.02f, 0.04f, 0.25f, 0.0f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(renderQuad.getProgramme());

		for (size_t i = 0; i < lightCount; i++)
		{
			alights[i].setupUniforms(lightUniforms[i]);
		}

		renderQuad.render(gtexturesVector);

		arenderer.finishFrame();
	}
	while(arenderer.isRunning());

	glfwTerminate();
	return 0;
}