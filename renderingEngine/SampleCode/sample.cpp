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
#include "../RenderingEngine/GraphicalTools/askybox.hpp"
#include "../RenderingEngine/Utils/amacrohelper.hpp"
#include "../RenderingEngine/Utils/arenderquad.hpp"
#include "../RenderingEngine/Utils/aluahelper.hpp"

#include <luahandler.hpp>
#include <stb_image.h>

int main(void)
{
    int width = 500;
    int height = 500;
	ARenderer arenderer(width, height, "Rendering Engine Sample Scene");	
	arenderer.changeClearColor(glm::vec4(0.2f, 0.0f, 0.0f, 1.0f));

	LuaHandler luaHandler;
	luaHandler.openFile("SampleCode/sample.lua");
	
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

	GLuint    lightPositionUniform = glGetUniformLocation(shaderProgramme, "sceneLight.position");
	GLuint   lightDirectionUniform = glGetUniformLocation(shaderProgramme, "sceneLight.direction");
	GLuint       lightColorUniform = glGetUniformLocation(shaderProgramme, "sceneLight.color");
	GLuint   lightIntensityUniform = glGetUniformLocation(shaderProgramme, "sceneLight.intensity");
	GLuint lightDirectionalUniform = glGetUniformLocation(shaderProgramme, "sceneLight.directional");

	std::vector<AModel> models = ALuaHelper::loadModelsFromTable("models", luaHandler);
	ALight alight = ALuaHelper::loadLightFromTable("light", luaHandler);

	ACamera acamera = arenderer.getCamera();
	glm::mat4 projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	glm::mat4 view = acamera.getView();
	glm::mat4 viewProjectionMatrix = projection * view;

	glm::mat4 skyViewProjectionMatrix = projection * glm::mat4(glm::mat3(glm::mat4(1.0)));
	do
	{
		arenderer.startFrame();

		glUseProgram(shaderProgramme);
		glUniformMatrix4fv(vpMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));

		alight.setupUniforms(lightPositionUniform, lightDirectionUniform, lightColorUniform, lightIntensityUniform, lightDirectionalUniform);
		AModel::renderModelsInList(models, modelMatrixUniform, shaderProgramme);

		askybox.render(skyViewProjectionMatrix);
		
		arenderer.finishFrame();
	}
	while(arenderer.isRunning());

	glfwTerminate();
	return 0;
}