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

#include "apointcloud.hpp"

int main(void)
{
	int width = 800;
	int height = 600;
	ARenderer arenderer(width, height, "Make Points Scene");	
	arenderer.changeClearColor(glm::vec4(0.02f, 0.02f, 0.02f, 0.0f));

	LuaHandler luaHandler;
	luaHandler.openFile("config.lua");
	
	GLuint vss =  AShader::generateShader(luaHandler.getGlobalString("simpleVertexShader"), GL_VERTEX_SHADER);
	GLuint fss =  AShader::generateShader(luaHandler.getGlobalString("simpleFragmentShader"), GL_FRAGMENT_SHADER);

	GLuint simpleShaderProgramme = AShader::generateProgram(vss, fss);

	ASkybox askybox(std::vector<std::string>{
        "../3DModels/desertsky_ft.tga",
        "../3DModels/desertsky_bc.tga",
        "../3DModels/desertsky_up.tga",
        "../3DModels/desertsky_dn.tga",
        "../3DModels/desertsky_rt.tga",
        "../3DModels/desertsky_lf.tga"
    });

	GLuint modelSimpleMatrixUniform = glGetUniformLocation(simpleShaderProgramme, "model");
	GLuint vpSimpleMatrixUniform = glGetUniformLocation(simpleShaderProgramme, "viewProjection");
	GLuint colorSimpleUniform = glGetUniformLocation(simpleShaderProgramme, "color");

	std::vector<AModel*> models = ALuaHelper::loadModelsFromTable("models", &luaHandler);
	float raysPerUnit = luaHandler.getGlobalNumber("raysPerUnit");
	float pointsPerUnit = luaHandler.getGlobalNumber("pointsPerUnit");
	APointCloud apointcloud(models[0]->getMeshAt(0), raysPerUnit, pointsPerUnit);
	const AMesh& apointCloudMesh = apointcloud.getMesh();

	ACamera& acamera = arenderer.getCamera();
	ALuaHelper::setupCameraPosition("cameraPosition", &acamera, &luaHandler);
	glm::mat4 projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	glm::mat4 view = acamera.getView();
	glm::mat4 viewProjectionMatrix = projection * view;
	glm::mat4 skyViewProjectionMatrix = projection * glm::mat4(glm::mat3(view));
	glm::mat4 skyView = glm::mat4(1.0);

	glm::mat4 pointModelMatrix = glm::mat4(1.0);
	glm::vec4 pointColor = glm::vec4(luaHandler.getGlobalNumber("rcolorR"), luaHandler.getGlobalNumber("rcolorG"), luaHandler.getGlobalNumber("rcolorB"), luaHandler.getGlobalNumber("rcolorA"));
	glActiveTexture(GL_TEXTURE0);
	do
	{
		projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
		view = acamera.getView();
		viewProjectionMatrix = projection * view;
		skyViewProjectionMatrix = projection * glm::mat4(glm::mat3(view));

		arenderer.startFrame();

		glViewport(0, 0, width * 2, height * 2);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.00f, 1.00f, 1.00f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(simpleShaderProgramme);
		glUniformMatrix4fv (modelSimpleMatrixUniform, 1, GL_FALSE, glm::value_ptr(pointModelMatrix));
		glUniformMatrix4fv(vpSimpleMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
		glUniform4f(colorSimpleUniform, pointColor.x, pointColor.y, pointColor.z, pointColor.w);
		apointCloudMesh.draw(simpleShaderProgramme, GL_POINTS);

		askybox.render(skyViewProjectionMatrix);

		arenderer.finishFrame();
	}
	while(arenderer.isRunning());

	glfwTerminate();
	return 0;
}