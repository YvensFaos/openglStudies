#include "anormaldebugger.hpp"

#include <GL/glew.h>

#include "../Core/ashader.hpp"
#include "luahandler.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

std::string ANormalDebugger::defaultVertexShader = 
    "#version 400\n"
    "   layout (location = 0) in vec3 vertex;\n"
    "   layout (location = 1) in vec3 normal;\n"
    "   out vectorOut {\n"
    "       vec4 vposition;\n"
    "       vec3 vnormal;\n"
    "   } vectorOut;\n"
    "   uniform mat4 model;\n"
    "   uniform mat4 viewProjection;\n"
    "   void main()\n"
    "   {\n"
    "       vectorOut.vposition = viewProjection * model * vec4(vertex, 1.0);\n"
    "       vectorOut.vnormal = normal; //mat3(transpose(inverse(model))) * \n"
    "       gl_Position = vectorOut.vposition;\n"
    "   };\n";

std::string ANormalDebugger::defaultGeometryShader = 
    "#version 400\n"
    "   layout (triangles) in;\n"
    "   layout (line_strip, max_vertices = 2) out;\n"
    "   in vectorOut {\n"
    "       vec4 vposition;\n"
    "       vec3 vnormal;\n"
    "   } vectorIn[];\n"
    "   const float MAGNITUDE = 0.2;\n"
    "   void main()\n"
    "   {\n"
    "       vec3 dir1 = (vec3(vectorIn[1].vposition - vectorIn[0].vposition));\n"
    "       float hl1 = length(dir1) / 2.0;\n"
    "       dir1 = normalize(dir1);\n"
    "       vec3 dir2 = (vec3(vectorIn[2].vposition - vectorIn[0].vposition));\n"
    "       float hl2 = length(dir2) / 2.0;\n"
    "       dir2 = normalize(dir2);\n"
    "       vec3 nor1 = normalize(cross(dir2, dir1));\n"
    "       vec4 midp = vectorIn[0].vposition;\n"
    "       if(hl1 > hl2)\n"
    "       {\n"
    "           midp = midp + vec4(dir1*hl1, 0);\n"
    "       }\n"
    "       else\n"
    "       {\n"
    "           midp = midp + vec4(dir2*hl2, 0);\n"
    "       }\n"
    "       gl_Position = midp;\n"
    "       EmitVertex();\n"
    "       gl_Position = midp + vec4(nor1, 0) * MAGNITUDE;\n"
    "       EmitVertex();\n"
    "       EndPrimitive();\n"
    "   }\n";

std::string ANormalDebugger::defaultFragmentShader = 
    "#version 400\n"
    "   uniform vec4 normalColor;\n"
    "   out vec4 frag_colour;\n"
    "   void main()\n"
    "   {\n"
    "       frag_colour = normalColor;\n"
    "   }\n";

ANormalDebugger::ANormalDebugger(void)
{
    initialize(ANormalDebugger::defaultVertexShader, ANormalDebugger::defaultGeometryShader, ANormalDebugger::defaultFragmentShader);
}

ANormalDebugger::ANormalDebugger(std::string vertexShaderText, std::string geometryShaderText, std::string fragmentShaderText)
{
    initialize(vertexShaderText, geometryShaderText, fragmentShaderText);
}

ANormalDebugger::~ANormalDebugger(void)
{

}

void ANormalDebugger::setupForRendering(glm::mat4 viewProjectionMatrix) const
{
    glUseProgram(programme);
    glUniformMatrix4fv(viewProjectionMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
    glUniform4f(normalColorUniform, normalColor.x, normalColor.y, normalColor.z, normalColor.w);
}

const GLuint ANormalDebugger::getModelUniformLocation(void) const
{
    return modelUniform;
}

const glm::vec4 ANormalDebugger::getNormalColor(void) const
{
    return normalColor;
}

void ANormalDebugger::setNormalColor(const glm::vec4 normalColor)
{
    this->normalColor.x = normalColor.x;
    this->normalColor.y = normalColor.y;
    this->normalColor.z = normalColor.z;
    this->normalColor.w = normalColor.w;
}

void ANormalDebugger::initialize(std::string vertexShaderText, std::string geometryShaderText, std::string fragmentShaderText)
{
    vertexShader =  AShader::generateShader(vertexShaderText, GL_VERTEX_SHADER);
	geometryShader =  AShader::generateShader(geometryShaderText, GL_GEOMETRY_SHADER);
	fragmentShader =  AShader::generateShader(fragmentShaderText, GL_FRAGMENT_SHADER);

    programme = AShader::generateProgram(vertexShader, geometryShader, fragmentShader);

    modelUniform = glGetUniformLocation(programme, "model");
    viewProjectionMatrixUniform = glGetUniformLocation(programme, "viewProjection");
    normalColorUniform = glGetUniformLocation(programme, "normalColor");

    normalColor = glm::vec4(1,1,0,1);
    
}