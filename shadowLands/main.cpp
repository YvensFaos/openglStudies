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
#include <RenderingEngine/Utils/amacrohelper.hpp>
#include <RenderingEngine/Utils/aluahelper.hpp>
#include <RenderingEngine/Objects/alightobject.hpp>
#include <RenderingEngine/Objects/atextureholder.hpp>
#include <RenderingEngine/GraphicalTools/aframebuffer.hpp>
#include <RenderingEngine/Utils/arenderquad.hpp>
#include <RenderingEngine/Utils/afrustum.hpp>

//#include "afrustum.hpp"

#include <perlinFunction.hpp>

#include <stb_image.h>

int main(void)
{
	int width = 800;
	int height = 600;
	ARenderer arenderer(width, height, "Shadow Lands");
	arenderer.changeClearColor(glm::vec4(0.02f, 0.02f, 0.02f, 0.0f));
	arenderer.setCullFaces(false);

	LuaHandler luaHandler;
	luaHandler.openFile("config.lua");
	
  	GLuint vs =  AShader::generateShader(luaHandler.getGlobalString("vertexShader"), GL_VERTEX_SHADER);
	GLuint cs = AShader::generateShader(luaHandler.getGlobalString("controlTesselationShader"), GL_TESS_CONTROL_SHADER);
	GLuint es = AShader::generateShader(luaHandler.getGlobalString("evaluationTesselationShader"), GL_TESS_EVALUATION_SHADER);
	GLuint gs =  AShader::generateShader(luaHandler.getGlobalString("geometryShader"), GL_GEOMETRY_SHADER);
	GLuint fs =  AShader::generateShader(luaHandler.getGlobalString("fragmentShader"), GL_FRAGMENT_SHADER);

	GLuint shaderProgramme = AShader::generateProgram(std::vector<GLuint>({vs, cs, es, gs, fs}));

	GLuint modelMatrixUniform = glGetUniformLocation(shaderProgramme, "model");
	GLuint vpMatrixUniform = glGetUniformLocation(shaderProgramme, "viewProjection");
	GLuint lightViewProjectionUniform = glGetUniformLocation(shaderProgramme, "lightViewProjection");
	GLuint textureUniform0 = glGetUniformLocation(shaderProgramme, "textureUniform0");
	GLuint textureUniform1 = glGetUniformLocation(shaderProgramme, "textureUniform1");
	GLuint textureUniform2 = glGetUniformLocation(shaderProgramme, "textureUniform2");
	GLuint textureUniform3 = glGetUniformLocation(shaderProgramme, "textureUniform3");
	GLuint objectsNumberPointLightsUniform = glGetUniformLocation(shaderProgramme, "numberPointLights");
	GLuint objectsNumberDirectionLightsUniform = glGetUniformLocation(shaderProgramme, "numberDirectionLights");
	GLuint maxTessLevelUniform = glGetUniformLocation(shaderProgramme, "maxTessLevel");
	GLuint shadowMapUniform = glGetUniformLocation(shaderProgramme, "shadowMap");
	GLuint adjustHeightUniform = glGetUniformLocation(shaderProgramme, "adjustHeight");

	GLuint     ambientColorUniform = glGetUniformLocation(shaderProgramme, "sceneAmbientLight.color");
	GLuint ambientIntensityUniform = glGetUniformLocation(shaderProgramme, "sceneAmbientLight.intensity");

	GLuint ses = AShader::generateShader(luaHandler.getGlobalString("shadowEvaluationTesselationShader"), GL_TESS_EVALUATION_SHADER);
	GLuint sgs =  AShader::generateShader(luaHandler.getGlobalString("shadowGeometryShader"), GL_GEOMETRY_SHADER);
	GLuint sfs = AShader::generateShader(luaHandler.getGlobalString("shadowFragmentShader"), GL_FRAGMENT_SHADER);

	GLuint shadowProgramme = AShader::generateProgram(std::vector<GLuint>({vs, cs, ses, sgs, sfs}));

	GLuint shadowmodelMatrixUniform = glGetUniformLocation(shadowProgramme, "model");
	GLuint shadowvpMatrixUniform = glGetUniformLocation(shadowProgramme, "viewProjection");
	GLuint shadowtextureUniform0 = glGetUniformLocation(shadowProgramme, "textureUniform0");
	GLuint shadowtextureUniform1 = glGetUniformLocation(shadowProgramme, "textureUniform1");
	GLuint shadowtextureUniform2 = glGetUniformLocation(shadowProgramme, "textureUniform2");
	GLuint shadowtextureUniform3 = glGetUniformLocation(shadowProgramme, "textureUniform3");
	GLuint shadowobjectsNumberPointLightsUniform = glGetUniformLocation(shadowProgramme, "numberPointLights");
	GLuint shadowobjectsNumberDirectionLightsUniform = glGetUniformLocation(shadowProgramme, "numberDirectionLights");
	GLuint shadowmaxTessLevelUniform = glGetUniformLocation(shadowProgramme, "maxTessLevel");
	GLuint shadowFarPlaneUniform = glGetUniformLocation(shadowProgramme, "farPlane");
	GLuint shadowAdjustHeightUniform = glGetUniformLocation(shadowProgramme, "adjustHeight");

	GLuint wgs =  AShader::generateShader(luaHandler.getGlobalString("wireGeometryShader"), GL_GEOMETRY_SHADER);
	GLuint wfs =  AShader::generateShader(luaHandler.getGlobalString("wireFragmentShader"), GL_FRAGMENT_SHADER);

	GLuint wireShaderProgramme = AShader::generateProgram(std::vector<GLuint>({vs, cs, ses, wgs, wfs}));

	GLuint wiremodelMatrixUniform = glGetUniformLocation(wireShaderProgramme, "model");
	GLuint wirevpMatrixUniform = glGetUniformLocation(wireShaderProgramme, "viewProjection");
	GLuint wiretextureUniform0 = glGetUniformLocation(wireShaderProgramme, "textureUniform0");
	GLuint wiretextureUniform1 = glGetUniformLocation(wireShaderProgramme, "textureUniform1");
	GLuint wiretextureUniform2 = glGetUniformLocation(wireShaderProgramme, "textureUniform2");
	GLuint wiretextureUniform3 = glGetUniformLocation(wireShaderProgramme, "textureUniform3");
	GLuint wireobjectsNumberPointLightsUniform = glGetUniformLocation(wireShaderProgramme, "numberPointLights");
	GLuint wireobjectsNumberDirectionLightsUniform = glGetUniformLocation(wireShaderProgramme, "numberDirectionLights");
	GLuint wiremaxTessLevelUniform = glGetUniformLocation(wireShaderProgramme, "maxTessLevel");
	GLuint wireColorUniform = glGetUniformLocation(wireShaderProgramme, "wireColor");
	GLuint wireadjustHeightUniform = glGetUniformLocation(wireShaderProgramme, "adjustHeight");

	std::vector<AModel> models = ALuaHelper::loadModelsFromTable("models", luaHandler);

	ACamera& acamera = arenderer.getCamera();
	ALuaHelper::setupCameraPosition("cameraPosition", acamera, luaHandler);
	glm::mat4 projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	AAmbientLight aambientLight = ALuaHelper::loadAmbientLightFromTable("ambient", luaHandler);
	glm::vec4 ambientLightColor = aambientLight.getColor();
	float ambientLightIntensity = aambientLight.getIntensity();

	ALight alight = ALuaHelper::loadLightFromTable("light", luaHandler);
	ALightObject alightObject(alight, shaderProgramme);

	glm::mat4 view = acamera.getView();
	glm::mat4 viewProjectionMatrix = projection * view;

	uint bwidth = luaHandler.getGlobalInteger("bwidth");
	uint bheight = luaHandler.getGlobalInteger("bheight");
	float* buffer = new float[bwidth * bheight * 4];

	int TERRAIN = luaHandler.getGlobalInteger("terrainSize");
	int ELEVATN = luaHandler.getGlobalInteger("elevationSize");
	int DETAILS = luaHandler.getGlobalInteger("detailsSize");

	GLfloat shadowWidth = luaHandler.getGlobalInteger("shadowWidth");
	GLfloat shadowHeight = luaHandler.getGlobalInteger("shadowHeight");
	ADepthbuffer adepthBuffer(shadowWidth, shadowHeight);
	ADepthbuffer adepthBuffer2(shadowWidth, shadowHeight);
	ARenderQuad debugQuad(luaHandler.getGlobalString("debugQuadFragShader"));
	ARenderQuad combineQuad(luaHandler.getGlobalString("combineQuadFragShader"));
	ADepthbuffer combineBuffer(shadowWidth, shadowHeight);

	std::vector<GLuint> texturesCombineQuad = {adepthBuffer.getFramebufferTexture(), adepthBuffer2.getFramebufferTexture()};

	glm::vec3 alightPosition = alight.getPosition();
	glm::vec3 alightDirection = alight.getDirection();
	glm::vec3 alightUp = alight.getUp();

	float nearPlane = luaHandler.getGlobalNumber("nearPlane");
	float farPlane = luaHandler.getGlobalNumber("farPlane");
	float projectionDimension = luaHandler.getGlobalNumber("projectionDimension");

	glm::mat4 lightProjection = glm::ortho(-projectionDimension, projectionDimension, -projectionDimension, projectionDimension, nearPlane, farPlane);
	glm::mat4 lightView = glm::lookAt(alightPosition, alightPosition + alightDirection, alightUp);
	glm::mat4 lightMatrix = lightProjection * lightView;

	srand(luaHandler.getGlobalInteger("seed"));
	PerlinNoise::generatePerlinNoise(bwidth, bheight, DETAILS, DETAILS, buffer);
	ATextureData atextureData1(bwidth, bheight, buffer);
	ATextureHolder   atexture1(atextureData1, GL_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);

	PerlinNoise::generatePerlinNoise(bwidth, bheight, ELEVATN, ELEVATN, buffer);
	ATextureData atextureData2(bwidth, bheight, buffer);
	ATextureHolder   atexture2(atextureData2, GL_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);

	PerlinNoise::generatePerlinNoise(bwidth, bheight, TERRAIN, TERRAIN, buffer);
	ATextureData atextureData3(bwidth, bheight, buffer);
	ATextureHolder   atexture3(atextureData3, GL_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);

	delete[] buffer;

	ATextureHolder mountains("rocky_512.jpg", "../3DModels");

	float maxTessLevel = luaHandler.getGlobalNumber("maxTessLevel");
	float maxTessLevelShadow = luaHandler.getGlobalNumber("maxTessLevelShadow");

	glm::vec4 wireColor = ALuaHelper::readVec4FromTable("wireColor", luaHandler);

	float adjustHeight = luaHandler.getGlobalNumber("adjustHeight");

	bool tesselationOn = true;
	bool showDepthMap = true;
	bool pauseUpdate = true;
	
	arenderer.addKeybind(APressKeyBind(GLFW_KEY_1, 
		[&tesselationOn](int action, int mods) {
			tesselationOn = !tesselationOn;
		}
	));
	arenderer.addKeybind(APressKeyBind(GLFW_KEY_2, 
		[&maxTessLevel](int action, int mods) {
			float value = mods == GLFW_MOD_SHIFT ? -1.0f : 1.0f;
			maxTessLevel = std::min(32.0f, std::max(1.0f, maxTessLevel + value));
			printf("Max TesselationLevel = %2.2f.\n", maxTessLevel);
		}
	));
	arenderer.addKeybind(APressKeyBind(GLFW_KEY_3, 
		[&maxTessLevelShadow](int action, int mods) {
			float value = mods == GLFW_MOD_SHIFT ? -1.0f : 1.0f;
			maxTessLevelShadow = std::min(32.0f, std::max(1.0f, maxTessLevelShadow + value));
			printf("Max TesselationLevel Shadow = %2.2f.\n", maxTessLevelShadow);
		}
	));
	arenderer.addKeybind(APressKeyBind(GLFW_KEY_4, 
		[&nearPlane](int action, int mods) {
			float value = mods == GLFW_MOD_SHIFT ? -0.25f : 0.25f;
			nearPlane = nearPlane + value;
			printf("Near Plane = %2.2f.\n", nearPlane);
		}
	));
	arenderer.addKeybind(APressKeyBind(GLFW_KEY_5, 
		[&farPlane](int action, int mods) {
			float value = mods == GLFW_MOD_SHIFT ? -0.25f : 0.25f;
			farPlane = farPlane + value;
			printf("Far Plane = %2.2f.\n", farPlane);
		}
	));
	arenderer.addKeybind(APressKeyBind(GLFW_KEY_6, 
		[&projectionDimension](int action, int mods) {
			float value = mods == GLFW_MOD_SHIFT ? -0.125f : 0.125f;
			projectionDimension = projectionDimension + value;
			printf("Projection Dimension = %2.2f.\n", projectionDimension);
		}
	));
	arenderer.addKeybind(APressKeyBind(GLFW_KEY_7, 
		[&adjustHeight](int action, int mods) {
			float value = mods == GLFW_MOD_SHIFT ? -0.125f : 0.125f;
			adjustHeight = adjustHeight + value;
			printf("Adjust Height = %2.2f.\n", adjustHeight);
		}
	));
	arenderer.addKeybind(APressKeyBind(GLFW_KEY_9, 
		[&pauseUpdate](int action, int mods) {
			pauseUpdate = !pauseUpdate;
		}
	));
	arenderer.addKeybind(APressKeyBind(GLFW_KEY_0, 
		[&showDepthMap](int action, int mods) {
			showDepthMap = !showDepthMap;
		}
	));

	//Move towards DIRECTION
	arenderer.addKeybind(APressKeyBind(GLFW_KEY_I, 
		[&alight](int action, int mods) {
			alight.setPosition(alight.getPosition() + alight.getDirection() * 0.25f);
			alight.log();
		}
	));
	arenderer.addKeybind(APressKeyBind(GLFW_KEY_K, 
		[&alight](int action, int mods) {
			alight.setPosition(alight.getPosition() - alight.getDirection() * 0.25f);
			alight.log();
		}
	));
	//Move towards RIGHT
	arenderer.addKeybind(APressKeyBind(GLFW_KEY_J, 
		[&alight](int action, int mods) {
			glm::vec3 r = glm::cross(alight.getUp(), alight.getDirection());
			alight.setPosition(alight.getPosition() + r * 0.25f);
			alight.log();
		}
	));
	arenderer.addKeybind(APressKeyBind(GLFW_KEY_L, 
		[&alight](int action, int mods) {
			glm::vec3 r = glm::cross(alight.getUp(), alight.getDirection());
			alight.setPosition(alight.getPosition() - r * 0.25f);
			alight.log();
		}
	));

	//Move towards Up
	arenderer.addKeybind(APressKeyBind(GLFW_KEY_Y, 
		[&alight](int action, int mods) {
			alight.setPosition(alight.getPosition() + alight.getUp() * 0.25f);
			alight.log();
		}
	));
	arenderer.addKeybind(APressKeyBind(GLFW_KEY_H, 
		[&alight](int action, int mods) {
			alight.setPosition(alight.getPosition() - alight.getUp() * 0.25f);
			alight.log();
		}
	));



	//Rotate Direction around Up
	arenderer.addKeybind(AKeyBind(GLFW_KEY_U, 
		[&alight](int action, int mods) {
			glm::vec3 d = glm::rotate(alight.getDirection(), glm::radians(2.0f), alight.getUp());
			alight.setDirection(d);
			alight.log();
		}
	));
	arenderer.addKeybind(AKeyBind(GLFW_KEY_O, 
		[&alight](int action, int mods) {
			glm::vec3 d = glm::rotate(alight.getDirection(),glm::radians(-2.0f), alight.getUp());
			alight.setDirection(d);
			alight.log();
		}
	));
	//Rotation Direction around R
	arenderer.addKeybind(AKeyBind(GLFW_KEY_T, 
		[&alight](int action, int mods) {
			glm::vec3 r = glm::cross(alight.getUp(), alight.getDirection());
			glm::vec3 u = glm::rotate(alight.getDirection(), glm::radians(2.0f), r);
			alight.setDirection(u);
			alight.log();
		}
	));
	arenderer.addKeybind(AKeyBind(GLFW_KEY_G, 
		[&alight](int action, int mods) {
			glm::vec3 r = glm::cross(alight.getUp(), alight.getDirection());
			glm::vec3 u = glm::rotate(alight.getDirection(), glm::radians(-2.0f), r);
			alight.setDirection(u);
			alight.log();
		}
	));

	AFrustum frustum(alightPosition, alightDirection, alightUp, projectionDimension, nearPlane, farPlane);

	do
	{
		projection = glm::perspective(glm::radians(acamera.getZoom()), (float) width / (float) height, acamera.getNear(), acamera.getFar());
		view = acamera.getView();
		viewProjectionMatrix = projection * view;

		alightPosition = alight.getPosition();
		alightDirection = alight.getDirection();
		alightUp = alight.getUp();
		lightProjection = glm::ortho(-projectionDimension, projectionDimension, -projectionDimension, projectionDimension, nearPlane, farPlane);
		lightView = glm::lookAt(alightPosition, alightPosition + alightDirection, alightUp);
		lightMatrix = lightProjection * lightView;

		frustum.setPosition(alightPosition);
		frustum.setDirection(alightDirection);
		frustum.setUp(alightUp);
		frustum.setNearPlane(nearPlane);
		frustum.setFarPlane(farPlane);
		frustum.setProjectionDimension(projectionDimension);
		
		arenderer.startFrame();

		arenderer.setCullFaces(true);
		glViewport(0, 0, shadowWidth, shadowHeight);
		adepthBuffer.bindBuffer();
		glClear(GL_DEPTH_BUFFER_BIT);
			glUseProgram(shadowProgramme);
			glUniformMatrix4fv(shadowvpMatrixUniform, 1, GL_FALSE, glm::value_ptr(lightMatrix));
			glUniform1i(shadowobjectsNumberPointLightsUniform, 0);
			glUniform1i(shadowobjectsNumberDirectionLightsUniform, 1);
			glUniform1f(shadowmaxTessLevelUniform, maxTessLevelShadow);
			glUniform1f(shadowFarPlaneUniform, farPlane);
			glUniform1f(shadowAdjustHeightUniform, adjustHeight);
			
			mountains.bindTexture(0);
			glUniform1i(shadowtextureUniform0, 0);
			atexture1.bindTexture(1);
			glUniform1i(shadowtextureUniform1, 1);
			atexture2.bindTexture(2);
			glUniform1i(shadowtextureUniform2, 2);
			atexture3.bindTexture(3);
			glUniform1i(shadowtextureUniform3, 3);

			alightObject.setupUniforms();
			AModel::renderPatchesModelsInList(models, shadowmodelMatrixUniform, shadowProgramme);

			atexture3.unbindTexture(3);
			atexture2.unbindTexture(2);
			atexture1.unbindTexture(1);
		adepthBuffer.unbindBuffer();
		

		glCullFace(GL_FRONT);
		glViewport(0, 0, shadowWidth, shadowHeight);
		adepthBuffer2.bindBuffer();
		glClear(GL_DEPTH_BUFFER_BIT);
			glUseProgram(shadowProgramme);
			glUniformMatrix4fv(shadowvpMatrixUniform, 1, GL_FALSE, glm::value_ptr(lightMatrix));
			glUniform1i(shadowobjectsNumberPointLightsUniform, 0);
			glUniform1i(shadowobjectsNumberDirectionLightsUniform, 1);
			glUniform1f(shadowmaxTessLevelUniform, maxTessLevelShadow);
			glUniform1f(shadowFarPlaneUniform, farPlane);
			glUniform1f(shadowAdjustHeightUniform, adjustHeight);
			
			mountains.bindTexture(0);
			glUniform1i(shadowtextureUniform0, 0);
			atexture1.bindTexture(1);
			glUniform1i(shadowtextureUniform1, 1);
			atexture2.bindTexture(2);
			glUniform1i(shadowtextureUniform2, 2);
			atexture3.bindTexture(3);
			glUniform1i(shadowtextureUniform3, 3);

			alightObject.setupUniforms();
			AModel::renderPatchesModelsInList(models, shadowmodelMatrixUniform, shadowProgramme);

			atexture3.unbindTexture(3);
			atexture2.unbindTexture(2);
			atexture1.unbindTexture(1);
		adepthBuffer2.unbindBuffer();
		glCullFace(GL_BACK);
		arenderer.setCullFaces(false);

		combineBuffer.bindBuffer();
		//try to render it only the backface and add it to the non backface
		glViewport(0, 0, shadowWidth, shadowHeight);
		glClearColor(0.0f / 255.0f, 0.0f / 255.0f, 165.0f / 255.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glUseProgram(combineQuad.getProgramme());
			combineQuad.render(texturesCombineQuad);
		combineBuffer.unbindBuffer();

		glViewport(0, 0, width * 2, height * 2);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(232.0f / 255.0f, 230.0f / 255.0f, 165.0f / 255.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		alightObject.renderLightObject(viewProjectionMatrix);

		if(tesselationOn) {
			glUseProgram(shaderProgramme);

			glUniformMatrix4fv(vpMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
			glUniformMatrix4fv(lightViewProjectionUniform, 1, GL_FALSE, glm::value_ptr(lightMatrix));
			glUniform1i(objectsNumberPointLightsUniform, 0);
			glUniform1i(objectsNumberDirectionLightsUniform, 1);

			glUniform1f(maxTessLevelUniform, maxTessLevel);
			glUniform1f(adjustHeightUniform, adjustHeight);

			glUniform4f(ambientColorUniform, ambientLightColor.x, ambientLightColor.y, ambientLightColor.z, ambientLightColor.w);
			glUniform1f(ambientIntensityUniform, ambientLightIntensity);
			
			mountains.bindTexture(0);
			glUniform1i(textureUniform0, 0);
			atexture1.bindTexture(1);
			glUniform1i(textureUniform1, 1);
			atexture2.bindTexture(2);
			glUniform1i(textureUniform2, 2);
			atexture3.bindTexture(3);
			glUniform1i(textureUniform3, 3);

 			glActiveTexture(GL_TEXTURE0 + 4);
			glBindTexture(GL_TEXTURE_2D, combineBuffer.getFramebufferTexture());
			glUniform1i(shadowMapUniform, 4);

			alightObject.setupUniforms();
			AModel::renderPatchesModelsInList(models, modelMatrixUniform, shaderProgramme);

			atexture3.unbindTexture(3);
			atexture2.unbindTexture(2);
			atexture1.unbindTexture(1);
			mountains.unbindTexture(0);
		} else {
			glUseProgram(wireShaderProgramme);

			glUniformMatrix4fv(wirevpMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
			glUniform1i(wireobjectsNumberPointLightsUniform, 0);
			glUniform1i(wireobjectsNumberDirectionLightsUniform, 1);

			glUniform1f(wiremaxTessLevelUniform, maxTessLevel);
			glUniform1f(wireadjustHeightUniform, adjustHeight);
			
			mountains.bindTexture(0);
			glUniform1i(wiretextureUniform0, 0);
			atexture1.bindTexture(1);
			glUniform1i(wiretextureUniform1, 1);
			atexture2.bindTexture(2);
			glUniform1i(wiretextureUniform2, 2);
			atexture3.bindTexture(3);
			glUniform1i(wiretextureUniform3, 3);

			glUniform4f(wireColorUniform, wireColor.r, wireColor.g, wireColor.b, wireColor.a);

			alightObject.setupUniforms();
			AModel::renderPatchesModelsInList(models, wiremodelMatrixUniform, wireShaderProgramme);

			atexture3.unbindTexture(3);
			atexture2.unbindTexture(2);
			atexture1.unbindTexture(1);
			mountains.unbindTexture(0);
		}

		frustum.renderFrustum(viewProjectionMatrix);

		if(showDepthMap) {
			glEnable(GL_SCISSOR_TEST);
			glViewport(0, 0, 320, 320);
			glScissor(0, 0, 320, 320);
			glClearColor(0.0f / 255.0f, 0.0f / 255.0f, 165.0f / 255.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				debugQuad.render(adepthBuffer.getFramebufferTexture());

			glViewport(0, 320 + 10, 320, 320);
			glScissor(0, 320 + 10, 320, 320);
			glClearColor(0.0f / 255.0f, 0.0f / 255.0f, 165.0f / 255.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				debugQuad.render(adepthBuffer2.getFramebufferTexture());

			glViewport(0, 640 + 20, 320, 320);
			glScissor(0, 640 + 20, 320, 320);
			glClearColor(0.0f / 255.0f, 0.0f / 255.0f, 165.0f / 255.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glUseProgram(combineQuad.getProgramme());
				combineQuad.render(texturesCombineQuad);

			//combineQuad

			glDisable(GL_SCISSOR_TEST);
		}

		arenderer.finishFrame();

		if(!pauseUpdate) {
			ALuaHelper::updateLight(luaHandler, alight, "updateLight", arenderer.getDeltaTime());
		}
	}
	while(arenderer.isRunning());

	glfwTerminate();
	return 0;
}