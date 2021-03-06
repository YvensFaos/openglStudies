#include "arenderer.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "acamera.hpp"

void ARenderer::staticKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
    ARenderer* arenderer = static_cast<ARenderer*>(glfwGetWindowUserPointer(window));
    arenderer->keyCallback(window, key, scancode, action, mods);
}
void ARenderer::staticMouseCallback(GLFWwindow* window, double xpos, double ypos) 
{
    ARenderer* arenderer = static_cast<ARenderer*>(glfwGetWindowUserPointer(window));
    arenderer->mouseCallback(window, xpos, ypos);
}
void ARenderer::staticMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) 
{
    ARenderer* arenderer = static_cast<ARenderer*>(glfwGetWindowUserPointer(window));
    arenderer->mouseButtonCallback(window, button, action, mods);
}

ARenderer::ARenderer(float width, float height, std::string title) : 
width(width), height(height), title(title), lastX(width / 2.0f), lastY(height / 2.0f),
firstMouse(true), mouseIsClickingLeft(false), shiftModPower(1.0f), deltaTime(0.0f), moveForce(20.0f), mouseSensitivity(5.0f),
accumulator(0.0), currentTime(0.0), finishFrameTime(0.0), clearColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f))
{
    initialize();
    acamera = new ACamera(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glfwSetWindowUserPointer(window, this);
}

ARenderer::~ARenderer(void)
{

}

void ARenderer::changeClearColor(glm::vec4 clearColor)
{
    this->clearColor.r = clearColor.r;
    this->clearColor.g = clearColor.g;
    this->clearColor.b = clearColor.b;
    this->clearColor.a = clearColor.a;
}

ACamera& ARenderer::getCamera(void) const
{
    return *acamera;
}

void ARenderer::startFrame(void) 
{
    glViewport(0, 0, width * 2, height * 2);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    currentTime = glfwGetTime();
	finishFrameTime = 0.0;
}

void ARenderer::finishFrame(void) 
{
    glfwSwapBuffers(window);
    glfwPollEvents();

    finishFrameTime = glfwGetTime();
    deltaTime = static_cast<float>(finishFrameTime - currentTime);
    currentTime = finishFrameTime;
    accumulator += deltaTime;
    sprintf(titleBuffer, "%s - FPS: %4.2f", title.c_str(), 1.0f / (float) deltaTime);
    glfwSetWindowTitle(window, titleBuffer);
}

float ARenderer::getDeltaTime(void) const 
{
    return deltaTime;
}

double ARenderer::getAccumulator(void) const 
{
    return accumulator;
}

bool ARenderer::isRunning(void) const
{
    return glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0;
}

void ARenderer::initialize(void)
{
    if(!glfwInit())
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
	if(window == NULL)
	{
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		return;
	}

	glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) 
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		return;
	}

	GLint MaxPatchVertices = 0;
	glGetIntegerv(GL_MAX_PATCH_VERTICES, &MaxPatchVertices);
	printf("Max supported patch vertices %d\n", MaxPatchVertices);

    glfwSetKeyCallback(window,         ARenderer::staticKeyCallback);
	glfwSetCursorPosCallback(window,   ARenderer::staticMouseCallback);
	glfwSetMouseButtonCallback(window, ARenderer::staticMouseButtonCallback);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glFrontFace(GL_CCW);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ARenderer::setCullFaces(bool activate) 
{
	if(activate) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
}

void ARenderer::addKeybind(AKeyBind akeyBind) 
{
	this->keysMap.try_emplace(akeyBind.getKey(), std::make_unique<AKeyBind>(akeyBind));
}

void ARenderer::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	for(auto it = this->keysMap.cbegin(); it != this->keysMap.cend(); it++) {
		if (key == (*it).first)
		{
			(*it).second->execute(action, mods);
		}
	}

    shiftModPower = 1.0f;
	if (mods == GLFW_MOD_SHIFT)
	{
		shiftModPower = 2.5f;
	} else if (mods == GLFW_MOD_CONTROL)
	{
		shiftModPower = 0.5f;
	}
    if ((key == GLFW_KEY_LEFT || key == GLFW_KEY_A) && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		acamera->MoveSideway(shiftModPower * -moveForce * deltaTime);
    }
	if ((key == GLFW_KEY_RIGHT || key == GLFW_KEY_D) && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		acamera->MoveSideway(shiftModPower * moveForce * deltaTime);
    }
	if ((key == GLFW_KEY_UP || key == GLFW_KEY_W) && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		acamera->MoveForward(shiftModPower * moveForce * deltaTime);
    }
	if ((key == GLFW_KEY_DOWN || key == GLFW_KEY_S) && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		acamera->MoveForward(shiftModPower *-moveForce * deltaTime);
    }
	if ((key == GLFW_KEY_R) && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		acamera->MoveUp(shiftModPower * moveForce * deltaTime);
    }
	if (( key == GLFW_KEY_F) && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		acamera->MoveUp(shiftModPower *-moveForce * deltaTime);
    }
	if (key == GLFW_KEY_Z && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		acamera->Zoom(5.0f);
    }
	if (key == GLFW_KEY_X && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		acamera->Zoom(-5.0f);
    }
	if (key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		acamera->RotateWithMouse(10, 0);
    }
	if (key == GLFW_KEY_E && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
		acamera->RotateWithMouse(-10, 0);
    }
	if(key == GLFW_KEY_P && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		printf("Printing Info: \n");
		printf("---------------\n");
		printf("Del: %9f\n", deltaTime);
		printf("Acc: %9f\n", accumulator);
		printf("---------------\n");
		printf("pos   = {%3.3f, %3.3f, %3.3f},\n", acamera->getPos().x, acamera->getPos().y, acamera->getPos().z);
		printf("dir   = {%3.3f, %3.3f, %3.3f},\n", acamera->getDir().x, acamera->getDir().y, acamera->getDir().z);
		printf("up    = {%3.3f, %3.3f, %3.3f},\n", acamera->getUp().x, acamera->getUp().y, acamera->getUp().z);
		printf("right = {%3.3f, %3.3f, %3.3f},\n", acamera->getRight().x, acamera->getRight().y, acamera->getRight().z);
		printf("angle = {%3.3f, %3.3f}\n", acamera->getAngles().x, acamera->getAngles().y);
		printf("---------------\n\n");
	}
}

void ARenderer::mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;
	
	if(mouseIsClickingLeft)
	{
		acamera->RotateWithMouse(xoffset / mouseSensitivity, yoffset / mouseSensitivity);
	}
}

void ARenderer::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    mouseIsClickingLeft = false;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		mouseIsClickingLeft = true;
	}
}
