#pragma once

#include <GL/glew.h>

#include "../Core/alight.hpp"
#include "../Core/amodel.hpp"

class LuaHandler;
class ARenderer;

///ALightObject is the union of a ALight and AMesh.
class ALightObject {
    private:
        ALight& alight;
        AModel alightModel;
        ALightUniform alightUniforms;

        GLuint shaderProgramme;
        GLuint lightIndex;

        bool hasUpdateFunction;
        std::string updateFunction;
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
        ALightObject(ALight& alight, GLuint shaderProgramme, GLuint lightIndex = 0);
        ~ALightObject(void);

        void renderLightObject(glm::mat4 viewProjection);
        void setupUniforms(void);
        void callUpdateFunction(LuaHandler& handler, const ARenderer& arenderer);

        const ALight& getLight(void) const;
    public:
        bool setupUpdateFunction(LuaHandler& handler, const std::string functionName);
    public:
        static std::vector<ALightObject> GenerateALightObjectsFromLights(GLuint programme, std::vector<ALight>& lights);
    private:
        static void GenerateALightObjectDefaultProgramme(void);
        static bool CheckLightObjectsProgramme(void);
};