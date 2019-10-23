#pragma once 

#include "RenderingEngine/Core/amodel.hpp"

struct ARay;
struct ATriangle;

class APointCloud {
    private:
        const AMesh& referenceMesh;
        float raysPerUnit;
        float pointsPerUnit;
        AMesh pointCloudMesh;
    public:
        APointCloud(const AMesh& referenceMesh, float raysPerUnit, float pointsPerUnit);
        ~APointCloud(void) = default;

        const AMesh& getMesh(void) const;
    private:
        AMesh generatePointCloud(void);
        bool getContactPoint(const std::vector<GLuint>& indices, const std::vector<AVertex>& vertices, const ARay& aray, glm::vec2& baryPosition, float& distance, ATriangle& atriangle, float biggerThan);
};