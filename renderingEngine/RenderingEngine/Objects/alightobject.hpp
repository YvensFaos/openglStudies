#pragma once

#include <GL/glew.h>

#include "../Core/alight.hpp"
#include "../Core/amodel.hpp"

///ALightObject is the union of a ALight and AMesh.
class ALightObject {
    private:
        ALight& alight;
        AModel alightModel;
        ALightUniform alightUniforms;

        GLuint shaderProgramme;
        GLuint lightIndex;
    private:
        static std::string defaultVertexShader;
        static std::string defaultGeometryShader;
        static std::string defaultFragmentShader;

        static GLuint lightObjectsVertexShader;
        static GLuint lightObjectsFragmentShader;
        static GLuint lightObjectsProgramme;

        static GLuint directionObjectsGeometryShader;
        static GLuint directionObjectsProgramme;

        static GLuint directionModelMatrixUniform;
        static GLuint directionVPMatrixUniform;
        static GLuint directionLightColorUniform;
        static GLuint directionDirectionUniform;
        static GLuint lightModelMatrixUniform;
        static GLuint lightVPMatrixUniform;
        static GLuint lightLightColorUniform;
    public:
        ALightObject(ALight& alight, GLuint shaderProgramme, GLuint lightIndex);
        ~ALightObject(void);

        void renderLightObject(glm::mat4 viewProjection);
        void setupUniforms(void);

        const ALight& getLight(void) const;
    public:
        static std::vector<ALightObject> GenerateALightObjectsFromLights(GLuint programme, std::vector<ALight>& lights);
    private:
        static void GenerateALightObjectDefaultProgramme(void);
        static bool CheckLightObjectsProgramme(void);
};