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
#include <RenderingEngine/Utils/agaussianquad.hpp>

#include <RenderingEngine/Objects/alightobject.hpp>
#include <RenderingEngine/GraphicalTools/acubemap.hpp>

#include <stb_image.h>

int main(void)
{
	int width = 800;
	int height = 600;
	ARenderer arenderer(width, height, "Bloom + Point Shadow Scene");	
	arenderer.changeClearColor(glm::vec4(0.02f, 0.02f, 0.02f, 0.0f));

	LuaHandler luaHandler;
	luaHandler.openFile("config.lua");
	
  	GLuint vs =  AShader::generateShader(luaHandler.getGlobalString("vertexShader"), GL_VERTEX_SHADER);
	GLuint fs =  AShader::generateShader(luaHandler.getGlobalString("fragmentShader"), GL_FRAGMENT_SHADER);
  	GLuint shaderProgramme = AShader::generateProgram(vs, fs);

	GLuint shadowVs =  AShader::generateShader(luaHandler.getGlobalString("ashadowVertexShader"), GL_VERTEX_SHADER);
	GLuint shadowGs =  AShader::generateShader(luaHandler.getGlobalString("ashadowGeometryShader"), GL_GEOMETRY_SHADER);
	GLuint shadowFs =  AShader::generateShader(luaHandler.getGlobalString("ashadowFragmentShader"), GL_FRAGMENT_SHADER);
  	GLuint shadowProgramme = AShader::generateProgram(shadowVs, shadowGs, shadowFs);

	ARenderQuad adepthRenderQuad(luaHandler.getGlobalString("depthCubemapFragmentShader"));
	GLuint renderQuadProgramme = adepthRenderQuad.getProgramme();

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
	GLuint viewPositionUniform = glGetUniformLocation(shaderProgramme, "viewPosition");

	GLuint lightPositionUniform = glGetUniformLocation(shaderProgramme, "sceneLight.position");
	GLuint lightDirectionUniform = glGetUniformLocation(shaderProgramme, "sceneLight.direction");
	GLuint lightColorUniform = glGetUniformLocation(shaderProgramme, "sceneLight.color");
	GLuint lightIntensityUniform = glGetUniformLocation(shaderProgramme, "sceneLight.intensity");
	GLuint lightDirectionalUniform = glGetUniformLocation(shaderProgramme, "sceneLight.directional");
	GLuint lightSpecularPowerUniform = glGetUniformLocation(shaderProgramme, "sceneLight.specularPower");
	GLuint ambientLightColorUniform = glGetUniformLocation(shaderProgramme, "sceneAmbientLight.color");
	GLuint ambientLightIntensityUniform = glGetUniformLocation(shaderProgramme, "sceneAmbientLight.intensity");
	GLuint farPlaneUniform = glGetUniformLocation(shaderProgramme, "farPlane");
	GLuint depthMapUniform = glGetUniformLocation(shaderProgramme, "depthMap");

	GLuint shadowModelMatrixUniform = glGetUniformLocation(shadowProgramme, "model");
	GLuint shadowLightPositionUniform = glGetUniformLocation(shadowProgramme, "sceneLight.position");
	GLuint shadowLightDirectionUniform = glGetUniformLocation(shadowProgramme, "sceneLight.direction");
	GLuint shadowLightColorUniform = glGetUniformLocation(shadowProgramme, "sceneLight.color");
	GLuint shadowLightIntensityUniform = glGetUniformLocation(shadowProgramme, "sceneLight.intensity");
	GLuint shadowLightDirectionalUniform = glGetUniformLocation(shadowProgramme, "sceneLight.directional");
	GLuint shadowFarPlaneUniform = glGetUniformLocation(shadowProgramme, "farPlane");

	GLuint widthUniform = glGetUniformLocation(renderQuadProgramme, "width");
	GLuint heigthUniform = glGetUniformLocation(renderQuadProgramme, "height");

	GLuint shadowMatricesUniforms[6];
	char uniformName[64];
	std::string buffer;
	for(unsigned int i = 0; i < 6; i++) {
		sprintf(uniformName, "%s[%d]", "shadowMatrices", i);
		buffer = "";
		buffer.append(uniformName);
		shadowMatricesUniforms[i] = glGetUniformLocation(shadowProgramme, buffer.c_str());
	}

	std::vector<AModel> models = ALuaHelper::loadModelsFromTable("models", luaHandler);

	std::vector<ALight> lights = ALuaHelper::loadLightsFromTable("lights", luaHandler);
	std::vector<ALightObject> lightObjects = ALightObject::GenerateALightObjectsFromLights(shaderProgramme, lights);
	lightObjects[0].setupUpdateFunction(luaHandler, "updateLight");

	GLuint pointLightCount = 0;

	int shadowWidth = luaHandler.getGlobalInteger("shadowWidth");
	int shadowHeight = luaHandler.getGlobalInteger("shadowHeight");
	float shadowNear =luaHandler.getGlobalNumber("shadowNear");
	float shadowFar = luaHandler.getGlobalNumber("shadowFar");

	ADepthbuffer ashadowCubeMapFramebuffer(width * 2, height * 2);
	ADepthCubeMap adepthCubemap(ashadowCubeMapFramebuffer, shadowWidth, shadowHeight, GL_FLOAT);

	AFramebuffer shadowedSceneFramebuffer(width * 2, height * 2);
	shadowedSceneFramebuffer.setBufferShowFlag(0x02);
	ARenderQuad shadowedSceneRenderQuad;

	float downsample = luaHandler.getGlobalNumber("downsample");

	AFramebuffer thresholdSceneFramebuffer(width / downsample, height / downsample);
	thresholdSceneFramebuffer.setBufferShowFlag(0x04);
	ARenderQuad thresholdSceneRenderQuad(luaHandler.getGlobalString("thresholdShader"));

	float shadowAspect = (float)shadowWidth/(float)shadowHeight;
	glm::mat4 shadowProj = glm::perspective(glm::radians(luaHandler.getGlobalNumber("shadowFov")), shadowAspect, shadowNear, shadowFar);

	AAmbientLight ambientLight = ALuaHelper::loadAmbientLightFromTable("ambient", luaHandler);

	ALight& mainLight = lights[0];
	glm::vec3 mainLightPosition = mainLight.getPosition();

	adepthCubemap.setMatrixAt(0, shadowProj * glm::lookAt(mainLightPosition, mainLightPosition + glm::vec3( 1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	adepthCubemap.setMatrixAt(1, shadowProj * glm::lookAt(mainLightPosition, mainLightPosition + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	adepthCubemap.setMatrixAt(2, shadowProj * glm::lookAt(mainLightPosition, mainLightPosition + glm::vec3( 0.0, 1.0, 0.0), glm::vec3(0.0, 0.0,  1.0)));
	adepthCubemap.setMatrixAt(3, shadowProj * glm::lookAt(mainLightPosition, mainLightPosition + glm::vec3( 0.0,-1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
	adepthCubemap.setMatrixAt(4, shadowProj * glm::lookAt(mainLightPosition, mainLightPosition + glm::vec3( 0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
	adepthCubemap.setMatrixAt(5, shadowProj * glm::lookAt(mainLightPosition, mainLightPosition + glm::vec3( 0.0, 0.0,-1.0), glm::vec3(0.0, -1.0, 0.0)));

	AGaussianQuad agaussianQuad(width / downsample, height / downsample, 
		luaHandler.getGlobalNumber("gaussianSigma"), luaHandler.getGlobalInteger("gaussianWeightLength"), luaHandler.getGlobalInteger("gaussianIterations"));

	ARenderQuad combineRenderQuad(luaHandler.getGlobalString("combineShader"));
	std::vector<GLuint> combineTexturesVector = {shadowedSceneFramebuffer.getFramebufferTexture(), agaussianQuad.getGaussianBlurTexture()};

	ACamera& acamera = arenderer.getCamera();
	ALuaHelper::setupCameraPosition("cameraPosition", acamera, luaHandler);
	glm::mat4 projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	glm::mat4 view = acamera.getView();
	glm::mat4 viewProjectionMatrix = projection * view;
	glm::mat4 viewShadowMatrix = acamera.getView();
	glm::mat4 skyViewProjectionMatrix = projection * glm::mat4(glm::mat3(view));

	unsigned int showBuffer = luaHandler.getGlobalInteger("initialMask");

	arenderer.addKeybind(AKeyBind(GLFW_KEY_0, [&showBuffer](int action, int mods) { 
		printf("Print Current flag: %d\n", showBuffer);
	}));
	arenderer.addKeybind(AFlagTogglerKeyBind(GLFW_KEY_1, showBuffer, 0x01));
	arenderer.addKeybind(AFlagTogglerKeyBind(GLFW_KEY_2, showBuffer, 0x02));
	arenderer.addKeybind(AFlagTogglerKeyBind(GLFW_KEY_3, showBuffer, 0x04));
	ashadowCubeMapFramebuffer.setBufferShowFlag(0x02);

	glActiveTexture(GL_TEXTURE0);
	do
	{
		projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
		view = acamera.getView();
		viewProjectionMatrix = projection * view;
		skyViewProjectionMatrix = projection * glm::mat4(glm::mat3(view));

		arenderer.startFrame();
		
		ashadowCubeMapFramebuffer.bindBuffer();
			glViewport(0, 0, shadowWidth, shadowHeight); 
			glClear(GL_DEPTH_BUFFER_BIT);
			glUseProgram(shadowProgramme);
			
			glUniform1f(shadowFarPlaneUniform, shadowFar);
			adepthCubemap.setMatricesUniform(shadowMatricesUniforms);
			mainLight.setupUniforms(shadowLightPositionUniform, shadowLightDirectionUniform, shadowLightColorUniform, shadowLightIntensityUniform, shadowLightDirectionalUniform);

			AModel::renderModelsInList(models, shadowModelMatrixUniform, shadowProgramme);
		ashadowCubeMapFramebuffer.unbindBuffer();

		shadowedSceneFramebuffer.bindBuffer(showBuffer);
			glViewport(0, 0, width * 2, height * 2);
			glClearColor(0.02f, 0.04f, 0.25f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			for (unsigned int i = 0; i < lightObjects.size(); i++)
			{
				lightObjects[i].renderLightObject(viewProjectionMatrix);
			}
			glUseProgram(shaderProgramme);
			glUniformMatrix4fv(vpMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
			glUniform1f(farPlaneUniform, shadowFar);
			glUniform1i(depthMapUniform, 0);
			glUniform3f(viewPositionUniform, acamera.getPos().x, acamera.getPos().y, acamera.getPos().z);
			mainLight.setupUniforms(lightPositionUniform, lightDirectionUniform, lightColorUniform, lightIntensityUniform, lightDirectionalUniform, lightSpecularPowerUniform);
			ambientLight.setupUniforms(ambientLightColorUniform, ambientLightIntensityUniform);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, adepthCubemap.getCubeMap());
			AModel::renderModelsInList(models, modelMatrixUniform, shaderProgramme);
			askybox.render(skyViewProjectionMatrix);
		shadowedSceneFramebuffer.unbindBuffer();

		thresholdSceneFramebuffer.bindBuffer(showBuffer);
			glViewport(0, 0, width / downsample, height / downsample);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			thresholdSceneRenderQuad.render(shadowedSceneFramebuffer.getFramebufferTexture());
		thresholdSceneFramebuffer.unbindBuffer();

		agaussianQuad.render(thresholdSceneFramebuffer.getFramebufferTexture());

		glViewport(0, 0, width * 2, height * 2);
		glClearColor(0.02f, 0.04f, 0.25f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(combineRenderQuad.getProgramme());
		combineRenderQuad.render(combineTexturesVector);

		arenderer.finishFrame();

		for (unsigned int i = 0; i < lightObjects.size(); i++)
		{
			lightObjects[i].callUpdateFunction(luaHandler, arenderer);
		}
		mainLightPosition = mainLight.getPosition();
		adepthCubemap.setMatrixAt(0, shadowProj * glm::lookAt(mainLightPosition, mainLightPosition + glm::vec3( 1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		adepthCubemap.setMatrixAt(1, shadowProj * glm::lookAt(mainLightPosition, mainLightPosition + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		adepthCubemap.setMatrixAt(2, shadowProj * glm::lookAt(mainLightPosition, mainLightPosition + glm::vec3( 0.0, 1.0, 0.0), glm::vec3(0.0, 0.0,  1.0)));
		adepthCubemap.setMatrixAt(3, shadowProj * glm::lookAt(mainLightPosition, mainLightPosition + glm::vec3( 0.0,-1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
		adepthCubemap.setMatrixAt(4, shadowProj * glm::lookAt(mainLightPosition, mainLightPosition + glm::vec3( 0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
		adepthCubemap.setMatrixAt(5, shadowProj * glm::lookAt(mainLightPosition, mainLightPosition + glm::vec3( 0.0, 0.0,-1.0), glm::vec3(0.0, -1.0, 0.0)));
	}
	while(arenderer.isRunning());

	glfwTerminate();
	return 0;
}