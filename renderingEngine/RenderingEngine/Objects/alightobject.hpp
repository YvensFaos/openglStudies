#pragma once

#include <GL/glew.h>

#include "../Core/alight.hpp"
#include "../Core/amodel.hpp"

///ALightObject is the union of a ALight and AMesh.
///The shaderProgramme 
class ALightObject {
    private:
        ALight* alight;
        ALightUniform alightUniforms;
        AModel* alightModel;

        GLuint shaderProgramme;
        GLuint lightIndex;

        static std::string defaultVertexShader;
        static std::string defaultFragmentShader;
        static GLuint lightObjectsVertexShader;
        static GLuint lightObjectsFragmentShader;
        static GLuint lightObjectsProgramme;

        static std::string directionVertexShader;
        static std::string directionFragmentShader;
        static std::string directionGeometryShader;
        static GLuint directionObjectsVertexShader;
        static GLuint directionObjectsFragmentShader;
        static GLuint directionObjectsGeometryShader;
        static GLuint directionObjectsProgramme;
    public:
        ALightObject(ALight* alight, GLuint shaderProgramme, GLuint lightIndex);
        ~ALightObject(void);

    public:
        static void GenerateALightObjectDefaultProgramme(void);
    private:
        static bool CheckLightObjectsProgramme(void);
};