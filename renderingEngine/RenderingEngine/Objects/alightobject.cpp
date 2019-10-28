#include "alightobject.hpp"

#include "../Core/ashader.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

std::string ALightObject::defaultVertexShader = 
    "#version 400\n"
    "   layout (location = 0) in vec3 vertex;\n"
    "   out vectorOut {\n"
    "       vec4 lposition;\n"
    "   } vectorOut;\n"
    "   uniform mat4 model;\n"
    "   uniform mat4 viewProjection;\n"
    "   void main()\n"
    "   {\n"
    "       vectorOut.lposition = viewProjection * model * vec4(vertex, 1.0);\n"            
    "       gl_Position = viewProjection * model * vec4(vertex, 1.0);\n"
    "   }\n";

std::string ALightObject::defaultGeometryShader = 
    "#version 400\n"
    "    layout (triangles) in;\n"
    "    layout (line_strip, max_vertices = 10) out;\n"
    "    in vectorOut {\n"
    "        vec4 lposition;\n"
    "    } vectorIn[];\n"
    "    uniform vec3 lightDirection;\n"
    "    const float MAGNITUDE = 0.2;\n"
    "    const float ARROW_HEAD_SIZE = 0.4;\n"
    "    void main()\n"
    "    {\n"
    "        vec3 dir1 = (vec3(vectorIn[1].lposition - vectorIn[0].lposition));\n"
    "        float hl1 = length(dir1) / 2.0;\n"
    "        dir1 = normalize(dir1);\n"
    "        vec3 dir2 = (vec3(vectorIn[2].lposition - vectorIn[0].lposition));\n"
    "        float hl2 = length(dir2) / 2.0;\n"
    "        dir2 = normalize(dir2);\n"
    "        vec4 midp = vectorIn[0].lposition + vec4(dir1*hl1 + dir2*hl2, 0.0);\n"
    "        gl_Position = midp;\n"
    "        EmitVertex();\n"
    "        vec3 normalizedLightDirection = normalize(lightDirection);\n"
    "        vec3 direction = vec3(midp) - normalizedLightDirection;\n"
    "        direction = normalize(direction);\n"
    "        vec4 finp = midp + vec4(normalizedLightDirection, 0) * MAGNITUDE;\n"
    "        gl_Position = finp;\n"
    "        EmitVertex();\n"
    "        gl_Position = finp + vec4(cross(vec3(1, 0, 0), direction), 0) * MAGNITUDE * ARROW_HEAD_SIZE;\n"
    "        EmitVertex();\n"
    "        gl_Position = finp + vec4(cross(vec3(-1, 0, 0), direction), 0) * MAGNITUDE * ARROW_HEAD_SIZE;\n"
    "        EmitVertex();\n"
    "        gl_Position = finp;\n"
    "        EmitVertex();\n"
    "        gl_Position = finp + vec4(cross(vec3(0, 1, 0), direction), 0) * MAGNITUDE * ARROW_HEAD_SIZE;\n"
    "        EmitVertex();\n"
    "        gl_Position = finp + vec4(cross(vec3(0, -1, 0), direction), 0) * MAGNITUDE * ARROW_HEAD_SIZE;\n"
    "        EmitVertex();\n"
    "        gl_Position = finp;\n"
    "        EmitVertex();\n"
    "        gl_Position = finp + vec4(cross(vec3(0, 0, 1), direction), 0) * MAGNITUDE * ARROW_HEAD_SIZE;\n"
    "        EmitVertex();\n"
    "        gl_Position = finp + vec4(cross(vec3(0, 0, -1), direction), 0) * MAGNITUDE * ARROW_HEAD_SIZE;\n"
    "        EmitVertex();\n"
    "        EndPrimitive();\n"
    "    }\n";

std::string ALightObject::defaultFragmentShader = 
    "#version 400\n"
    "   uniform vec4 lightColor;\n"
    "   out vec4 frag_colour;\n"
    "   void main()\n"
    "   {\n"
    "       frag_colour = lightColor;\n"
    "   }\n";

GLuint ALightObject::lightObjectsVertexShader = -1;
GLuint ALightObject::lightObjectsFragmentShader = -1;
GLuint ALightObject::lightObjectsProgramme = -1;

GLuint ALightObject::directionObjectsGeometryShader = -1;
GLuint ALightObject::directionObjectsProgramme = -1;

GLuint ALightObject::lightModelMatrixUniform = -1;
GLuint ALightObject::lightVPMatrixUniform = -1;
GLuint ALightObject::lightLightColorUniform = -1;

GLuint ALightObject::directionModelMatrixUniform = -1;
GLuint ALightObject::directionVPMatrixUniform = -1;
GLuint ALightObject::directionLightColorUniform = -1;
GLuint ALightObject::directionDirectionUniform = -1;

ALightObject::ALightObject(ALight& alight, GLuint shaderProgramme, GLuint lightIndex) 
    : alight(alight), alightModel("../3DModels/tetrahedron.obj"), shaderProgramme(shaderProgramme), lightIndex(lightIndex),
    alightUniforms(ALightUniform::loadALightUniformFromProgramme(shaderProgramme, lightIndex, alight))
{
    ALightObject::GenerateALightObjectDefaultProgramme();
    this->alightModel.scale(glm::vec3(0.1, 0.1, 0.1));
}

ALightObject::~ALightObject(void) 
{ }

void ALightObject::renderLightObject(glm::mat4 viewProjection)
{
    alightModel.setPosition(alight.getPosition());

    glUseProgram(ALightObject::directionObjectsProgramme);
    {
        glUniformMatrix4fv(ALightObject::directionVPMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjection));
        glm::vec4 lightColor = alight.getColor();
        glUniform4f(ALightObject::directionLightColorUniform, 0.8f, 0.8f, 0.8f, 1.0f);
        glm::vec3 lightDirection = alight.getDirection();
        glUniform3f(ALightObject::directionDirectionUniform, lightDirection.x, lightDirection.y, lightDirection.z);
        alightModel.renderModels(ALightObject::directionModelMatrixUniform, ALightObject::directionObjectsProgramme);
    }
    glUseProgram(ALightObject::lightObjectsProgramme);
    {
        glUniformMatrix4fv(ALightObject::lightVPMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjection));
        glm::vec4 lightColor = alight.getColor();
        glUniform4f(ALightObject::lightLightColorUniform, lightColor.x, lightColor.y, lightColor.z, lightColor.w);
        alightModel.renderModels(ALightObject::lightModelMatrixUniform, ALightObject::lightObjectsProgramme);
    }
}

const ALight& ALightObject::getLight(void) const
{
    return this->alight;
}

void ALightObject::setupUniforms(void)
{
    glm::vec3 lightPosition = alight.getPosition();
    glm::vec3 lightDirection = alight.getDirection();
    glm::vec3 lightUp = alight.getUp();
    glm::vec4 lightColor = alight.getColor();
    float lightIntensity = alight.getIntensity();
    bool isDirectional = alight.getDirectional();

    glUniform3f(alightUniforms.lightPositionUniform, lightPosition.x, lightPosition.y, lightPosition.z);
    glUniform3f(alightUniforms.lightDirectionUniform, lightDirection.x, lightDirection.y, lightDirection.z);
    glUniform4f(alightUniforms.lightColorUniform, lightColor.x, lightColor.y, lightColor.z, lightColor.w);
    glUniform1f(alightUniforms.lightIntensityUniform, lightIntensity);
    glUniform1i(alightUniforms.lightDirectionalUniform, isDirectional);
}

// Static methods

bool ALightObject::CheckLightObjectsProgramme(void) 
{
    return ALightObject::lightObjectsProgramme == -1;
}

void ALightObject::GenerateALightObjectDefaultProgramme(void) 
{
    if(ALightObject::CheckLightObjectsProgramme())
    {
        ALightObject::lightObjectsVertexShader = AShader::generateShader(ALightObject::defaultVertexShader, GL_VERTEX_SHADER);
        ALightObject::lightObjectsFragmentShader = AShader::generateShader(ALightObject::defaultFragmentShader, GL_FRAGMENT_SHADER);
        ALightObject::directionObjectsGeometryShader = AShader::generateShader(ALightObject::defaultGeometryShader, GL_GEOMETRY_SHADER);
        
        ALightObject::directionObjectsProgramme = AShader::generateProgram(ALightObject::lightObjectsVertexShader, ALightObject::directionObjectsGeometryShader, ALightObject::lightObjectsFragmentShader);
	    
        ALightObject::directionModelMatrixUniform = glGetUniformLocation(ALightObject::directionObjectsProgramme, "model");
	    ALightObject::directionVPMatrixUniform = glGetUniformLocation(ALightObject::directionObjectsProgramme, "viewProjection");
	    ALightObject::directionLightColorUniform = glGetUniformLocation(ALightObject::directionObjectsProgramme, "lightColor");
	    ALightObject::directionDirectionUniform = glGetUniformLocation(ALightObject::directionObjectsProgramme, "lightDirection");

        ALightObject::lightObjectsProgramme = AShader::generateProgram(ALightObject::lightObjectsVertexShader, ALightObject::lightObjectsFragmentShader);
        
        ALightObject::lightModelMatrixUniform = glGetUniformLocation(ALightObject::lightObjectsProgramme, "model");
	    ALightObject::lightVPMatrixUniform = glGetUniformLocation(ALightObject::lightObjectsProgramme, "viewProjection");
	    ALightObject::lightLightColorUniform = glGetUniformLocation(ALightObject::lightObjectsProgramme, "lightColor");
    }
}

std::vector<ALightObject> ALightObject::GenerateALightObjectsFromLights(GLuint programme, std::vector<ALight>& lights)
{
    std::vector<ALightObject> lightObjects;
    GLuint directionalLightCount = 0;
    GLuint pointLightCount = 0;

    bool isDirectional;
    for(unsigned int i = 0; i < lights.size(); i++) 
    {
        if(lights[i].getDirectional()) 
        {
            lightObjects.push_back(ALightObject(lights[i], programme, directionalLightCount++));
        }
        else 
        {
            lightObjects.push_back(ALightObject(lights[i], programme, pointLightCount++));
        }
    }

    return lightObjects;
}