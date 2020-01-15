#include "afrustum.hpp"

#include "../Core/ashader.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

std::string AFrustum::defaultVertexShader = 
    "#version 410 core\n"
    "   layout (location = 0) in vec3 vertex;\n"
    "   out vectorOut {\n"
    "       vec3 vposition;\n"
    "   } vectorOut;\n"
    "   uniform mat4 model;\n"
    "   void main()\n"
    "   {\n"
    "       vectorOut.vposition = vec3(model * vec4(vertex, 1.0));\n"
    "       gl_Position = vec4(vectorOut.vposition, 1.0);\n"
    "   }";

std::string AFrustum::defaultGeometryShader = 
    "#version 410 core\n"
    "    layout(points) in;\n"
    "    layout(line_strip, max_vertices = 16) out;\n"
    "    in vectorOut {\n"
    "        vec3 vposition;\n"
    "    } vectorIn;\n"
    "    uniform vec3 up;\n"
    "    uniform vec3 direction;\n"
    "    uniform float nearPlane;\n"
    "    uniform float farPlane;\n"
    "    uniform float fieldOfView;\n"
    "    uniform float aspectRatio;\n"
    "    uniform float projectionDimension;\n"
    "    uniform bool perspective;\n"
    "    uniform mat4 viewProjection;\n"
    "    void main() {        \n"
    "        vec3 initialPoint = vec3(gl_in[0].gl_Position);\n"
    "        vec3 dVector = normalize(direction);\n"
    "        vec3 vVector = normalize(cross(dVector, up));\n"
    "        vec3 hVector = normalize(cross(dVector, vVector));\n"
    "        vec3 nearPoint = vec3(0.0f);\n"
    "        vec3 farPoint = vec3(0.0f);\n"
    "        float nearDistanceH = 0.0f;\n"
    "        float nearDistanceV = 0.0f;\n"
    "        float farDistanceH = 0.0f;\n"
    "        float farDistanceV = 0.0f;\n"
    "        if(perspective) {\n"
    "            nearPoint = initialPoint + dVector * nearPlane;\n" 
    "            nearDistanceH = tan(radians(fieldOfView / 2.0)) * nearPlane;\n" 
    "            nearDistanceV = nearDistanceH * aspectRatio;\n" 
    "            \n" 
    "            farPoint = initialPoint + dVector * farPlane;\n" 
    "            farDistanceH  = tan(radians(fieldOfView / 2.0)) * farPlane;\n" 
    "            farDistanceV = farDistanceH * aspectRatio;\n" 
    "        } else {\n"
    "            nearPoint = initialPoint + dVector * nearPlane;\n" 
    "            nearDistanceH = projectionDimension;\n" 
    "            nearDistanceV = projectionDimension;\n" 
    "            \n" 
    "            farPoint = initialPoint + dVector * farPlane;\n" 
    "            farDistanceH = projectionDimension;\n" 
    "            farDistanceV = projectionDimension;\n" 
    "        }\n"
    "        gl_Position = viewProjection * vec4(nearPoint + nearDistanceH * hVector + nearDistanceV * vVector, gl_in[0].gl_Position.w);\n"
    "        EmitVertex();\n"
    "        gl_Position = viewProjection * vec4(nearPoint + nearDistanceH * hVector - nearDistanceV * vVector, gl_in[0].gl_Position.w);\n"
    "        EmitVertex();\n"
    "        gl_Position = viewProjection * vec4(nearPoint - nearDistanceH * hVector - nearDistanceV * vVector, gl_in[0].gl_Position.w);\n"
    "        EmitVertex();\n"
    "        gl_Position = viewProjection * vec4(nearPoint - nearDistanceH * hVector + nearDistanceV * vVector, gl_in[0].gl_Position.w);\n"
    "        EmitVertex();\n"
    "        gl_Position = viewProjection * vec4(nearPoint + nearDistanceH * hVector + nearDistanceV * vVector, gl_in[0].gl_Position.w);\n"
    "        EmitVertex();\n"
    "        gl_Position = viewProjection * vec4(farPoint + farDistanceH * hVector + farDistanceV * vVector, gl_in[0].gl_Position.w);\n"
    "        EmitVertex();\n"
    "        gl_Position = viewProjection * vec4(farPoint + farDistanceH * hVector - farDistanceV * vVector, gl_in[0].gl_Position.w);\n"
    "        EmitVertex();\n"
    "        gl_Position = viewProjection * vec4(farPoint - farDistanceH * hVector - farDistanceV * vVector, gl_in[0].gl_Position.w);\n"
    "        EmitVertex();\n"
    "        gl_Position = viewProjection * vec4(farPoint - farDistanceH * hVector + farDistanceV * vVector, gl_in[0].gl_Position.w);\n"
    "        EmitVertex();\n"
    "        gl_Position = viewProjection * vec4(farPoint + farDistanceH * hVector + farDistanceV * vVector, gl_in[0].gl_Position.w);\n"
    "        EmitVertex();\n"
    "        gl_Position = viewProjection * vec4(farPoint - farDistanceH * hVector + farDistanceV * vVector, gl_in[0].gl_Position.w);\n"
    "        EmitVertex();\n"
    "        gl_Position = viewProjection * vec4(nearPoint - nearDistanceH * hVector + nearDistanceV * vVector, gl_in[0].gl_Position.w);\n"
    "        EmitVertex();\n"
    "        gl_Position = viewProjection * vec4(nearPoint - nearDistanceH * hVector - nearDistanceV * vVector, gl_in[0].gl_Position.w);\n"
    "        EmitVertex();\n"
    "        gl_Position = viewProjection * vec4(farPoint - farDistanceH * hVector - farDistanceV * vVector, gl_in[0].gl_Position.w);\n"
    "        EmitVertex();\n"
    "        gl_Position = viewProjection * vec4(farPoint + farDistanceH * hVector - farDistanceV * vVector, gl_in[0].gl_Position.w);\n"
    "        EmitVertex();\n"
    "        gl_Position = viewProjection * vec4(nearPoint + nearDistanceH * hVector - nearDistanceV * vVector, gl_in[0].gl_Position.w);\n"
    "        EmitVertex();\n"
    "        EndPrimitive();\n"
    "    }\n"
    "";

std::string AFrustum::defaultFragmentShader = 
    "#version 410 core"
    "   uniform vec4 frustumColor;"
    "   out vec4 frag_colour;"
    "   void main() {   "
    "       frag_colour = frustumColor;"
    "   }"
    "";

GLuint AFrustum::shaderProgramme = -1;
GLuint AFrustum::modelMatrixUniform = -1;
GLuint AFrustum::vPMatrixUniform = -1;
GLuint AFrustum::frustumColorUniform = -1;
GLuint AFrustum::upUniform = -1;
GLuint AFrustum::directionUniform = -1;
GLuint AFrustum::nearPlaneUniform = -1;
GLuint AFrustum::farPlaneUniform = -1;
GLuint AFrustum::fieldOfViewUniform = -1;
GLuint AFrustum::aspectRatioUniform = -1;
GLuint AFrustum::perspectiveUniform = -1;
GLuint AFrustum::projectionDimensionUniform = -1;

AFrustum::AFrustum(glm::vec3 position, glm::vec3 direction, glm::vec3 up, float projectionDimension, float nearPlane, float farPlane) 
: position(position), direction(direction), up(up), projectionDimension(projectionDimension), 
  fieldOfView(0), aspectRatio(0), nearPlane(nearPlane), farPlane(farPlane), perspective(false),
  frustumMesh({AMesh::generateVertex(glm::vec3(0.0), direction)}, {0}, std::vector<ATexture>() )
{ 
    this->setupShaders();
}

AFrustum::AFrustum(glm::vec3 position, glm::vec3 direction, glm::vec3 up, float fieldOfView, float aspectRatio, float nearPlane, float farPlane) 
: position(position), direction(direction), up(up), projectionDimension(0), 
  fieldOfView(fieldOfView), aspectRatio(aspectRatio), nearPlane(nearPlane), farPlane(farPlane), perspective(true),
  frustumMesh({AMesh::generateVertex(glm::vec3(0.0), direction)}, {0}, std::vector<ATexture>() )
{ 
    this->setupShaders();
}

AFrustum::AFrustum(ACamera& acamera, float projectionDimension, float nearPlane, float farPlane)
: position(acamera.getPos()), direction(acamera.getDir()), up(acamera.getUp()), projectionDimension(projectionDimension), 
  fieldOfView(acamera.getZoom()), aspectRatio(0), nearPlane(nearPlane), farPlane(farPlane), perspective(false),
  frustumMesh({AMesh::generateVertex(glm::vec3(0.0), acamera.getDir())}, {0}, std::vector<ATexture>() )
{ 
    this->setupShaders();
}

AFrustum::AFrustum(ACamera& acamera, bool perspective, float aspectRatio, float nearPlane, float farPlane) 
: position(acamera.getPos()), direction(acamera.getDir()), up(acamera.getUp()), projectionDimension(0), 
  fieldOfView(acamera.getZoom()), aspectRatio(aspectRatio), nearPlane(nearPlane), farPlane(farPlane), perspective(true),
  frustumMesh({AMesh::generateVertex(glm::vec3(0.0), acamera.getDir())}, {0}, std::vector<ATexture>() )
{ 
    this->setupShaders();
}

AFrustum::~AFrustum(void) { 
    //TODO
}

void AFrustum::renderFrustum(glm::mat4 viewProjection) const {
    glUseProgram(AFrustum::shaderProgramme);

    glUniformMatrix4fv(AFrustum::modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(glm::translate(glm::mat4(1.0), this->position)));
    glUniformMatrix4fv(AFrustum::vPMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjection));
    glUniform4f(AFrustum::frustumColorUniform, 0.0f, 0.05f, 0.9f, 1.0f);

    glUniform3f(AFrustum::upUniform, this->up.x, this->up.y, this->up.z);
    glUniform3f(AFrustum::directionUniform, this->direction.x, this->direction.y, this->direction.z);

    glUniform1f(AFrustum::nearPlaneUniform, this->nearPlane);
    glUniform1f(AFrustum::farPlaneUniform, this->farPlane);
    glUniform1f(AFrustum::fieldOfViewUniform, this->fieldOfView);
    glUniform1f(AFrustum::aspectRatioUniform, this->aspectRatio);
    glUniform1f(AFrustum::projectionDimensionUniform, this->projectionDimension);

    glUniform1i(AFrustum::perspectiveUniform, this->perspective ? 1 : 0);

    this->frustumMesh.draw(AFrustum::shaderProgramme, GL_POINTS, false);
}

void AFrustum::setPosition(const glm::vec3 position) {
    this->position.x = position.x;
    this->position.y = position.y;
    this->position.z = position.z;
}

void AFrustum::setDirection(const glm::vec3 direction) {
    this->direction.x = direction.x;
    this->direction.y = direction.y;
    this->direction.z = direction.z;
}

void AFrustum::setUp(const glm::vec3 up) {
    this->up.x = up.x;
    this->up.y = up.y;
    this->up.z = up.z;
}

void AFrustum::setNearPlane(float nearPlane) {
    this->nearPlane = nearPlane;
}

void AFrustum::setFarPlane(float farPlane) {
    this->farPlane = farPlane;
}

void AFrustum::setProjectionDimension(float projectionDimension) {
    this->projectionDimension = projectionDimension;
}

void AFrustum::setupShaders(void) {
    if(AFrustum::shaderProgramme == -1)
    {
        GLuint vs =  AShader::generateShader(AFrustum::defaultVertexShader, GL_VERTEX_SHADER);
	    GLuint gs =  AShader::generateShader(AFrustum::defaultGeometryShader, GL_GEOMETRY_SHADER);
	    GLuint fs =  AShader::generateShader(AFrustum::defaultFragmentShader, GL_FRAGMENT_SHADER);

	    AFrustum::shaderProgramme = AShader::generateProgram(vs, gs, fs);

        AFrustum::modelMatrixUniform = glGetUniformLocation(AFrustum::shaderProgramme, "model");
        AFrustum::vPMatrixUniform = glGetUniformLocation(AFrustum::shaderProgramme, "viewProjection");
        AFrustum::frustumColorUniform = glGetUniformLocation(AFrustum::shaderProgramme, "frustumColor");
        AFrustum::upUniform = glGetUniformLocation(AFrustum::shaderProgramme, "up");
        AFrustum::directionUniform = glGetUniformLocation(AFrustum::shaderProgramme, "direction");
        AFrustum::nearPlaneUniform = glGetUniformLocation(AFrustum::shaderProgramme, "nearPlane");
        AFrustum::farPlaneUniform = glGetUniformLocation(AFrustum::shaderProgramme, "farPlane");
        AFrustum::fieldOfViewUniform = glGetUniformLocation(AFrustum::shaderProgramme, "fieldOfView");
        AFrustum::aspectRatioUniform = glGetUniformLocation(AFrustum::shaderProgramme, "aspectRatio");
        AFrustum::perspectiveUniform = glGetUniformLocation(AFrustum::shaderProgramme, "perspective");
        AFrustum::projectionDimensionUniform = glGetUniformLocation(AFrustum::shaderProgramme, "projectionDimension");
    }
}