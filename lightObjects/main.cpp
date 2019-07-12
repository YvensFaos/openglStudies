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
#include <RenderingEngine/Utils/anormaldebugger.hpp>

#include <stb_image.h>

int main(void)
{
	int width = 800;
	int height = 600;
	ARenderer arenderer(width, height, "Light Objects");	
	arenderer.changeClearColor(glm::vec4(0.02f, 0.02f, 0.02f, 0.0f));

	LuaHandler luaHandler;
	luaHandler.openFile("config.lua");
	bool DEBUG = luaHandler.getGlobalBoolean("debug");
	
  	GLuint vs =  AShader::generateShader(luaHandler.getGlobalString("vertexShader"), GL_VERTEX_SHADER);
	GLuint gs =  AShader::generateShader(luaHandler.getGlobalString("geometryShader"), GL_GEOMETRY_SHADER);
	GLuint ls =  AShader::generateShader(luaHandler.getGlobalString("lightFragmentShader"), GL_FRAGMENT_SHADER);

	GLuint lightProgramme = AShader::generateProgram(vs, gs, ls);

	GLuint lightModelMatrixUniform = glGetUniformLocation(lightProgramme, "model");
	GLuint lightVMatrixUniform = glGetUniformLocation(lightProgramme, "view");
	GLuint lightPMatrixUniform = glGetUniformLocation(lightProgramme, "projection");
	GLuint lightLightColorUniform = glGetUniformLocation(lightProgramme, "lightColor");
	GLuint lightDirectionUniform = glGetUniformLocation(lightProgramme, "lightDirection");

	ACamera* acamera = arenderer.getCamera();
	ALuaHelper::setupCameraPosition("cameraPosition", acamera, &luaHandler);
	glm::vec3 cameraPosition = acamera->getPos();
	glm::mat4 projection = glm::perspective(glm::radians(acamera->getZoom()), (float) width / (float) height, acamera->getNear(), acamera->getFar());
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	glm::mat4 view = acamera->getView();
	glm::mat4 viewProjectionMatrix = projection * view;

	ALight* pointLightPointer = ALuaHelper::loadLightFromTable("pointLight", &luaHandler);
	AModel* alightModel = new AModel(luaHandler.getGlobalString("lightModel"));
    alightModel->scale(glm::vec3(0.1, 0.1, 0.1));
	glm::vec4 lightColor;
	glm::vec3 lightDirection;

	glActiveTexture(GL_TEXTURE0);
	do
	{
		projection = glm::perspective(glm::radians(acamera->getZoom()), (float) width / (float) height, acamera->getNear(), acamera->getFar());
		view = acamera->getView();
		viewProjectionMatrix = projection * view;
		
		arenderer.startFrame();

		glViewport(0, 0, width * 2, height * 2);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.02f, 0.04f, 0.25f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(lightProgramme);
		glUniformMatrix4fv(lightVMatrixUniform, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(lightPMatrixUniform, 1, GL_FALSE, glm::value_ptr(projection));
		lightColor = pointLightPointer->getColor();
		lightDirection = pointLightPointer->getDirection();
		glUniform4f(lightLightColorUniform, lightColor.x, lightColor.y, lightColor.z, lightColor.w);
		glUniform3f(lightDirectionUniform, lightDirection.x, lightDirection.y, lightDirection.z);
		alightModel->renderModels(lightModelMatrixUniform, lightProgramme);

		ALuaHelper::updateLight(&luaHandler, pointLightPointer, "updateLight", arenderer.getDeltaTime());
		alightModel->setPosition(pointLightPointer->getPosition());

		arenderer.finishFrame();
	}
	while(arenderer.isRunning());

	glfwTerminate();
	return 0;
}