#pragma once

#include "../Core/amesh.hpp"
#include "../Core/acamera.hpp"
#include <luahandler.hpp>

#include <glm/glm.hpp>
#include <vector>

class AFrustum {
    private:
        bool perspective;
        AMesh frustumMesh;

        glm::vec3 position;
        glm::vec3 direction;
        glm::vec3 up;

        glm::vec4 color;

        float aspectRatio;
        float fieldOfView;
        float nearPlane;
        float farPlane;
        float projectionDimension;

    private:
        static GLuint shaderProgramme;
        static std::string defaultVertexShader;
        static std::string defaultGeometryShader;
        static std::string defaultFragmentShader;

        static GLuint modelMatrixUniform;
        static GLuint vPMatrixUniform;
        static GLuint frustumColorUniform;
        static GLuint upUniform;
        static GLuint directionUniform;
        static GLuint nearPlaneUniform;
        static GLuint farPlaneUniform;
        static GLuint fieldOfViewUniform;
        static GLuint aspectRatioUniform;
        static GLuint perspectiveUniform;
        static GLuint projectionDimensionUniform;
    public:
        AFrustum(glm::vec3 position, glm::vec3 direction, glm::vec3 up, float projectionDimension, float nearPlane, float farPlane);
        AFrustum(glm::vec3 position, glm::vec3 direction, glm::vec3 up, float fieldOfView, float aspectRation, float nearPlane, float farPlane);
        AFrustum(ACamera& acamera, float projectionDimension, float nearPlane, float farPlane);
        AFrustum(ACamera& acamera, bool perspective, float aspectRatio, float nearPlane, float farPlane);
        ~AFrustum(void);

        void renderFrustum(glm::mat4 viewProjection) const;

        void setPosition(const glm::vec3 position);
        void setDirection(const glm::vec3 direction);
        void setUp(const glm::vec3 up);

        void setNearPlane(float nearPlane);
        void setFarPlane(float farPlane);
        void setProjectionDimension(float projectionDimension);

    private:
        void setupShaders(void);
};