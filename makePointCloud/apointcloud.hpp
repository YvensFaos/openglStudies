#pragma once 

#include "RenderingEngine/Core/amodel.hpp"

struct ARay;

class APointCloud {
    private:
        const AMesh& referenceMesh;
        
        AMesh pointCloudMesh;
        float density;
    public:
        APointCloud(const AMesh& referenceMesh, float density);
        ~APointCloud(void) = default;

    private:
        AMesh generatePointCloud(void);
        bool getContactPoint(const std::vector<GLuint>& indices, const std::vector<AVertex>& vertices, const ARay& aray, glm::vec2& baryPosition, float& distance);
};