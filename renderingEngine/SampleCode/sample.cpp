#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

#include <lua.hpp>
#include <string>
#include <algorithm>
#include <vector>

#include "../RenderingEngine/Core/arenderer.hpp"
#include "../RenderingEngine/Core/amodel.hpp"
#include "../RenderingEngine/Core/acamera.hpp"
#include "../RenderingEngine/Core/alight.hpp"
#include "../RenderingEngine/Core/ashader.hpp"
#include "../RenderingEngine/GraphicalTools/aframebuffer.hpp"
#include "../RenderingEngine/GraphicalTools/askybox.hpp"
#include "../RenderingEngine/Utils/amacrohelper.hpp"
#include "../RenderingEngine/Utils/arenderquad.hpp"
#include "../RenderingEngine/Utils/aluahelper.hpp"

#include <luahandler.hpp>
#include <stb_image.h>

int main(void)
{
    int width = 800;
    int height = 800;
	ARenderer arenderer(width, height, "Rendering Engine Sample Scene");	
	arenderer.changeClearColor(glm::vec4(0.2f, 0.0f, 0.0f, 1.0f));

	LuaHandler luaHandler;
	luaHandler.openFile("SampleCode/sample.lua");
	
    GLuint vs =  AShader::generateShader(luaHandler.getGlobalString("vertexShader"), GL_VERTEX_SHADER);
	GLuint fs =  AShader::generateShader(luaHandler.getGlobalString("fragmentShader"), GL_FRAGMENT_SHADER);
	GLuint svs = AShader::generateShader(luaHandler.getGlobalString("skyboxVertexShader"), GL_VERTEX_SHADER);
	GLuint sfs = AShader::generateShader(luaHandler.getGlobalString("skyboxFragmentShader"), GL_FRAGMENT_SHADER);
	GLuint hvs = AShader::generateShader(luaHandler.getGlobalString("shadowVertexShader"), GL_VERTEX_SHADER);
	GLuint hfs = AShader::generateShader(luaHandler.getGlobalString("shadowFragmentShader"), GL_FRAGMENT_SHADER);

    GLuint shaderProgramme = AShader::generateProgram(vs, fs);
	GLuint skyboxProgramme  = AShader::generateProgram(svs, sfs);
	GLuint shadowProgramme = AShader::generateProgram(hvs, hfs);

	ASkybox askybox(std::vector<std::string>{
        "RenderingEngine/Resources/desertsky_ft.tga",
        "RenderingEngine/Resources/desertsky_bc.tga",
        "RenderingEngine/Resources/desertsky_up.tga",
        "RenderingEngine/Resources/desertsky_dn.tga",
        "RenderingEngine/Resources/desertsky_rt.tga",
        "RenderingEngine/Resources/desertsky_lf.tga"
    });

	GLuint skyVpMatrixUniform = glGetUniformLocation(skyboxProgramme, "vpMatrix");
	GLuint skyboxUniform = glGetUniformLocation(skyboxProgramme, "skybox");
	GLuint modelMatrixUniform = glGetUniformLocation(shaderProgramme, "model");
	GLuint vpMatrixUniform = glGetUniformLocation(shaderProgramme, "viewProjection");
	GLuint lightMatrixUniform = glGetUniformLocation(shaderProgramme, "lightViewProjection");
	GLuint shadowMapUniform = glGetUniformLocation(shaderProgramme, "shadowMap");

	GLuint    lightPositionUniform = glGetUniformLocation(shaderProgramme, "sceneLight.position");
	GLuint   lightDirectionUniform = glGetUniformLocation(shaderProgramme, "sceneLight.direction");
	GLuint       lightColorUniform = glGetUniformLocation(shaderProgramme, "sceneLight.color");
	GLuint   lightIntensityUniform = glGetUniformLocation(shaderProgramme, "sceneLight.intensity");
	GLuint lightDirectionalUniform = glGetUniformLocation(shaderProgramme, "sceneLight.directional");

	GLuint shadowModelMatrixUniform = glGetUniformLocation(shadowProgramme, "model");
	GLuint shadowLightMatrixUniform = glGetUniformLocation(shadowProgramme, "lightViewProjection");

	std::vector<AModel*> models = ALuaHelper::loadModelsFromTable("models", &luaHandler);
	ALight* alight = ALuaHelper::loadLightFromTable("light", &luaHandler);

	ACamera* acamera = arenderer.getCamera();
	glm::mat4 projection = glm::perspective(glm::radians(acamera->getZoom()), (float) width / (float) height, acamera->getNear(), acamera->getFar());
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	glm::mat4 view = acamera->getView();
	glm::mat4 viewProjectionMatrix = projection * view;

	glm::vec3 lightPosition = alight->getPosition();
	glm::vec3 lightDirection = alight->getDirection();
	glm::vec3 lightUp = alight->getUp();
	glm::vec4 lightColor = alight->getColor();
	float lightIntensity = alight->getIntensity();
	bool lightDirectional = alight->getDirectional();

	float nearPlane = luaHandler.getGlobalNumber("nearPlane");
	float farPlane = luaHandler.getGlobalNumber("farPlane");
	float projectionDimension = luaHandler.getGlobalNumber("projectionDimension");
	glm::mat4 lightProjection = glm::ortho(-projectionDimension, projectionDimension, -projectionDimension, projectionDimension, nearPlane, farPlane);
	glm::mat4 lightView = glm::lookAt(lightPosition, lightPosition + lightDirection, lightUp);
	glm::mat4 lightMatrix = lightProjection * lightView;
    glm::mat4 skyView = glm::mat4(1.0);

	glActiveTexture(GL_TEXTURE0);
	lightPosition = alight->getPosition();
	lightDirection = alight->getDirection();
	lightUp = alight->getUp();
	lightColor = alight->getColor();
	lightIntensity = alight->getIntensity();

	glm::mat4 skyViewProjectionMatrix = projection * glm::mat4(glm::mat3(skyView));
	do
	{
		arenderer.startFrame();

		glUseProgram(shaderProgramme);
		glUniformMatrix4fv(vpMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
		glUniformMatrix4fv(lightMatrixUniform, 1, GL_FALSE, glm::value_ptr(lightMatrix));
		glUniform3f(lightPositionUniform, lightPosition.x, lightPosition.y, lightPosition.z);
		glUniform3f(lightDirectionUniform, lightDirection.x, lightDirection.y, lightDirection.z);
		glUniform4f(lightColorUniform, lightColor.x, lightColor.y, lightColor.z, lightColor.w);
		glUniform1f(lightIntensityUniform, lightIntensity);
		glUniform1i(lightDirectionalUniform, lightDirectional);
		AModel::renderModelsInList(&models, modelMatrixUniform, shaderProgramme);

		glUseProgram(skyboxProgramme);
		glUniformMatrix4fv (skyVpMatrixUniform, 1, GL_FALSE, glm::value_ptr(skyViewProjectionMatrix));
		glUniform1i(skyboxUniform, 0);
		askybox.render();
		
		arenderer.finishFrame();
	}
	while(arenderer.isRunning());

	glfwTerminate();
	return 0;
}