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
	ARenderer arenderer(width, height, "Bezier Tesselation Shader");
	arenderer.changeClearColor(glm::vec4(0.02f, 0.02f, 0.02f, 0.0f));

	LuaHandler luaHandler;
	luaHandler.openFile("config.lua");
	
	GLuint nvs =  AShader::generateShader(luaHandler.getGlobalString("normalVertexShader"), GL_VERTEX_SHADER);
	GLuint ngs =  AShader::generateShader(luaHandler.getGlobalString("normalGeometryShader"), GL_GEOMETRY_SHADER);
	GLuint nfs =  AShader::generateShader(luaHandler.getGlobalString("normalFragmentShader"), GL_FRAGMENT_SHADER);
	GLuint normalShaderProgramme = AShader::generateProgram(nvs, ngs, nfs);

  	GLuint vs =  AShader::generateShader(luaHandler.getGlobalString("vertexShader"), GL_VERTEX_SHADER);
	GLuint cs = AShader::generateShader(luaHandler.getGlobalString("controlTesselationShader"), GL_TESS_CONTROL_SHADER);
	GLuint es = AShader::generateShader(luaHandler.getGlobalString("evaluationTesselationShader"), GL_TESS_EVALUATION_SHADER);
	GLuint gs =  AShader::generateShader(luaHandler.getGlobalString("geometryShader"), GL_GEOMETRY_SHADER);
	GLuint fs =  AShader::generateShader(luaHandler.getGlobalString("fragmentShader"), GL_FRAGMENT_SHADER);

  	GLuint shaderProgramme = AShader::generateProgram(std::vector<GLuint>({vs, cs, es, gs, fs}));

	ASkybox askybox(std::vector<std::string>{
        "../3DModels/desertsky_ft.tga",
        "../3DModels/desertsky_bc.tga",
        "../3DModels/desertsky_up.tga",
        "../3DModels/desertsky_dn.tga",
        "../3DModels/desertsky_rt.tga",
        "../3DModels/desertsky_lf.tga"
    });

	GLuint 		modelMatrixUniform = glGetUniformLocation(shaderProgramme, "model");
	GLuint 		   vpMatrixUniform = glGetUniformLocation(shaderProgramme, "viewProjection");
	GLuint    lightPositionUniform = glGetUniformLocation(shaderProgramme, "sceneLight.position");
	GLuint   lightDirectionUniform = glGetUniformLocation(shaderProgramme, "sceneLight.direction");
	GLuint       lightColorUniform = glGetUniformLocation(shaderProgramme, "sceneLight.color");
	GLuint   lightIntensityUniform = glGetUniformLocation(shaderProgramme, "sceneLight.intensity");
	GLuint lightDirectionalUniform = glGetUniformLocation(shaderProgramme, "sceneLight.directional");
	GLuint      eyeWorldPosUniform = glGetUniformLocation(shaderProgramme, "eyeWorldPos");
	GLuint      minDistanceUniform = glGetUniformLocation(shaderProgramme, "minDistance");
	GLuint      maxDistanceUniform = glGetUniformLocation(shaderProgramme, "maxDistance");
	GLuint     maxTessLevelUniform = glGetUniformLocation(shaderProgramme, "maxTessLevel");

	GLuint 		modelMatrixNormalUniform = glGetUniformLocation(normalShaderProgramme, "model");
	GLuint 		   vpMatrixNormalUniform = glGetUniformLocation(normalShaderProgramme, "viewProjection");
	GLuint    lightPositionNormalUniform = glGetUniformLocation(normalShaderProgramme, "sceneLight.position");
	GLuint   lightDirectionNormalUniform = glGetUniformLocation(normalShaderProgramme, "sceneLight.direction");
	GLuint       lightColorNormalUniform = glGetUniformLocation(normalShaderProgramme, "sceneLight.color");
	GLuint   lightIntensityNormalUniform = glGetUniformLocation(normalShaderProgramme, "sceneLight.intensity");
	GLuint lightDirectionalNormalUniform = glGetUniformLocation(normalShaderProgramme, "sceneLight.directional");

	std::vector<AModel> models = ALuaHelper::loadModelsFromTable("models", luaHandler);
	ALight alight = ALuaHelper::loadLightFromTable("light", luaHandler);

	ACamera& acamera = arenderer.getCamera();
	ALuaHelper::setupCameraPosition("cameraPosition", acamera, luaHandler);
	glm::mat4 projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	glm::mat4 view = acamera.getView();
	glm::mat4 viewProjectionMatrix = projection * view;
	glm::mat4 skyViewProjectionMatrix = projection * glm::mat4(glm::mat3(view));
	glm::vec3 cameraPosition;

	float minDistance = luaHandler.getGlobalNumber("minDistance");
	float maxDistance = luaHandler.getGlobalNumber("maxDistance");
	float maxTessLevel = luaHandler.getGlobalNumber("maxTessLevel");

	bool tesselationOn = true;
	arenderer.addKeybind(APressKeyBind(GLFW_KEY_1, 
		[&tesselationOn](int action, int mods) {
			tesselationOn = !tesselationOn;
		}
	));

	glActiveTexture(GL_TEXTURE0);
	do
	{
		projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
		view = acamera.getView();
		viewProjectionMatrix = projection * view;
		skyViewProjectionMatrix = projection * glm::mat4(glm::mat3(view));
		cameraPosition = acamera.getPos();

		arenderer.startFrame();

		glViewport(0, 0, width * 2, height * 2);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.02f, 0.04f, 0.25f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if(tesselationOn) 
		{
			glUseProgram(shaderProgramme);
			glUniformMatrix4fv(vpMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
			glUniform3f(eyeWorldPosUniform, cameraPosition.x, cameraPosition.y, cameraPosition.z);
			glUniform1f(minDistanceUniform, minDistance);
			glUniform1f(maxDistanceUniform, maxDistance);
			glUniform1f(maxTessLevelUniform, maxTessLevel);
			alight.setupUniforms(lightPositionUniform, lightDirectionUniform, lightColorUniform, lightIntensityUniform, lightDirectionalUniform);

			glPatchParameteri(GL_PATCH_VERTICES, 3);
			AModel::renderPatchesModelsInList(models, modelMatrixUniform, shaderProgramme);
		} else {
			glUseProgram(normalShaderProgramme);
			glUniformMatrix4fv(vpMatrixNormalUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
			alight.setupUniforms(lightPositionNormalUniform, lightDirectionNormalUniform, lightColorNormalUniform, lightIntensityNormalUniform, lightDirectionalNormalUniform);

			AModel::renderModelsInList(models, modelMatrixNormalUniform, normalShaderProgramme);
		}

		askybox.render(skyViewProjectionMatrix);

		arenderer.finishFrame();
	}
	while(arenderer.isRunning());

	glfwTerminate();
	return 0;
}