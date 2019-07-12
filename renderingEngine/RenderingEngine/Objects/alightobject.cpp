#include "alightobject.hpp"

#include "../Core/ashader.hpp"

#include <glm/glm.hpp>

std::string ALightObject::defaultVertexShader = 
    "#version 400\n"
    "   layout (location = 0) in vec3 vertex;\n"
    "   uniform mat4 model;\n"
    "   uniform mat4 viewProjection;\n"
    "   void main()\n"
    "   {\n"
    "       gl_Position = viewProjection * model * vec4(vertex, 1.0);\n"
    "   }\n";

std::string ALightObject::defaultFragmentShader = 
    "#version 400\n"
    "   uniform vec4 lightColor;\n"
    "   out vec4 frag_colour;\n"
    "   void main()\n"
    "   {          \n"
    "       frag_colour = lightColor;\n"
    "   }\n";


GLuint ALightObject::lightObjectsVertexShader = -1;
GLuint ALightObject::lightObjectsFragmentShader = -1;
GLuint ALightObject::lightObjectsProgramme = -1;

std::string ALightObject::directionVertexShader = "";
std::string ALightObject::directionFragmentShader = "";
std::string ALightObject::directionGeometryShader = "";

GLuint ALightObject::directionObjectsVertexShader = -1;
GLuint ALightObject::directionObjectsFragmentShader = -1;
GLuint ALightObject::directionObjectsGeometryShader = -1;
GLuint ALightObject::directionObjectsProgramme = -1;

ALightObject::ALightObject(ALight* alight, GLuint shaderProgramme, GLuint lightIndex) 
    : alight(alight), shaderProgramme(shaderProgramme), lightIndex(lightIndex),
    alightUniforms(ALightUniform::loadALightUniformFromProgramme(shaderProgramme, lightIndex, alight))
{
    this->alightModel = new AModel("3DModels/cube.obj");
    this->alightModel->scale(glm::vec3(0.1, 0.1, 0.1));
}

ALightObject::~ALightObject(void) 
{ }

void ALightObject::GenerateALightObjectDefaultProgramme(void) {
    ALightObject::lightObjectsVertexShader = AShader::generateShader(ALightObject::defaultVertexShader, GL_VERTEX_SHADER);
	ALightObject::lightObjectsFragmentShader = AShader::generateShader(ALightObject::defaultFragmentShader, GL_FRAGMENT_SHADER);
    ALightObject::lightObjectsProgramme = AShader::generateProgram(ALightObject::lightObjectsVertexShader, ALightObject::lightObjectsFragmentShader);
}

bool ALightObject::CheckLightObjectsProgramme(void) {
    return lightObjectsProgramme == -1;
}