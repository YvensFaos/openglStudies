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

float moveLight(LuaHandler* luaHandler, ALight* light, float deltaTime)
{
	luaHandler->getFunction("moveY");
	luaHandler->pushNumber(light->getPosition().y);
	luaHandler->pushNumber(deltaTime);
	luaHandler->callFunctionFromStack(2, 1);
	return luaHandler->popNumber();
}

int main(void)
{
	int width = 800;
	int height = 600;
	ARenderer arenderer(width, height, "Full Light Scene");	
	arenderer.changeClearColor(glm::vec4(0.02f, 0.02f, 0.02f, 0.0f));

	LuaHandler luaHandler;
	luaHandler.openFile("config.lua");
	
  GLuint vs =  AShader::generateShader(luaHandler.getGlobalString("vertexShader"), GL_VERTEX_SHADER);
	GLuint fs =  AShader::generateShader(luaHandler.getGlobalString("fragmentShader"), GL_FRAGMENT_SHADER);
	GLuint ls =  AShader::generateShader(luaHandler.getGlobalString("lightFragmentShader"), GL_FRAGMENT_SHADER);

  GLuint shaderProgramme = AShader::generateProgram(vs, fs);
	GLuint lightProgramme = AShader::generateProgram(vs, ls);

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
	GLuint viewPositionUniform = glGetUniformLocation(shaderProgramme, "viewPosition");
	GLuint vlightPositionUniform = glGetUniformLocation(shaderProgramme, "lightPosition");

	GLuint     ambientColorUniform = glGetUniformLocation(shaderProgramme, "sceneAmbientLight.color");
	GLuint ambientIntensityUniform = glGetUniformLocation(shaderProgramme, "sceneAmbientLight.intensity");

	GLuint      lightPositionUniform = glGetUniformLocation(shaderProgramme, "sceneLight.position");
	GLuint     lightDirectionUniform = glGetUniformLocation(shaderProgramme, "sceneLight.direction");
	GLuint         lightColorUniform = glGetUniformLocation(shaderProgramme, "sceneLight.color");
	GLuint     lightIntensityUniform = glGetUniformLocation(shaderProgramme, "sceneLight.intensity");
	GLuint   lightDirectionalUniform = glGetUniformLocation(shaderProgramme, "sceneLight.directional");
	GLuint lightSpecularPowerUniform = glGetUniformLocation(shaderProgramme, "sceneLight.specularPower");

	GLuint coloredModelMatrixUniform = glGetUniformLocation(lightProgramme, "model");
	GLuint coloredVpMatrixUniform = glGetUniformLocation(lightProgramme, "viewProjection");
	GLuint coloredLightUniform = glGetUniformLocation(lightProgramme, "lightColor");

	std::vector<AModel*> models = ALuaHelper::loadModelsFromTable("models", &luaHandler);
	AModel* lightObject = ALuaHelper::loadModelFromTable("lightObject", &luaHandler);
	ALight* alight = ALuaHelper::loadLightFromTable("light", &luaHandler);

	ACamera* acamera = arenderer.getCamera();
	ALuaHelper::setupCameraPosition("cameraPosition", acamera, &luaHandler);
	glm::vec3 cameraPosition = acamera->getPos();
	glm::mat4 projection = glm::perspective(glm::radians(acamera->getZoom()), (float) width / (float) height, acamera->getNear(), acamera->getFar());

	glm::mat4 view = acamera->getView();
	glm::mat4 viewProjectionMatrix = projection * view;
	glm::mat4 skyViewProjectionMatrix = projection * glm::mat4(glm::mat3(view));

	glm::vec3 lightPosition = alight->getPosition();
	glm::vec3 lightDirection = alight->getDirection();
	glm::vec3 lightUp = alight->getUp();
	glm::vec4 lightColor = alight->getColor();
	float lightIntensity = alight->getIntensity();
	bool lightDirectional = alight->getDirectional();

	lightObject->setPosition(lightPosition);

	AAmbientLight* aambientLight = ALuaHelper::loadAmbientLightFromTable("ambient", &luaHandler);
	glm::vec4 ambientLightColor = aambientLight->getColor();
	float ambientLightIntensity = aambientLight->getIntensity();

	do
	{
		arenderer.startFrame();

		projection = glm::perspective(glm::radians(acamera->getZoom()), (float) width / (float) height, acamera->getNear(), acamera->getFar());
		view = acamera->getView();
		viewProjectionMatrix = projection * view;
		skyViewProjectionMatrix = projection * glm::mat4(glm::mat3(view));

		glUseProgram(shaderProgramme);
		glUniformMatrix4fv(vpMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
		glUniform3f(viewPositionUniform, cameraPosition.x, cameraPosition.y, cameraPosition.z);
		glUniform4f(ambientColorUniform, ambientLightColor.x, ambientLightColor.y, ambientLightColor.z, ambientLightColor.w);
		glUniform1f(ambientIntensityUniform, ambientLightIntensity);

		glUniform3f(vlightPositionUniform, lightPosition.x, lightPosition.y, lightPosition.z);
		glUniform3f(lightPositionUniform, lightPosition.x, lightPosition.y, lightPosition.z);
		glUniform3f(lightDirectionUniform, lightDirection.x, lightDirection.y, lightDirection.z);
		glUniform4f(lightColorUniform, lightColor.x, lightColor.y, lightColor.z, lightColor.w);
		glUniform1f(lightIntensityUniform, lightIntensity);
		glUniform1i(lightDirectionalUniform, lightDirectional);
		
		AModel::renderModelsInList(&models, modelMatrixUniform, shaderProgramme);

		glUseProgram(lightProgramme);
		glUniformMatrix4fv(coloredVpMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
		glUniform4f(coloredLightUniform, lightColor.x, lightColor.y, lightColor.z, lightColor.w);
		lightObject->renderModels(coloredModelMatrixUniform, lightProgramme);

		askybox.render(skyViewProjectionMatrix);

		arenderer.finishFrame();

		lightPosition.y = moveLight(&luaHandler, alight, arenderer.getDeltaTime());
		alight->setPosition(lightPosition);
		lightObject->setPosition(lightPosition);
	}
	while(arenderer.isRunning());

	glfwTerminate();
	return 0;
}