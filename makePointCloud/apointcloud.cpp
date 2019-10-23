#include "apointcloud.hpp"

#include "RenderingEngine/Core/aboundingbox.hpp"
#include "RenderingEngine/Math/ageometry.hpp"
#include "RenderingEngine/Utils/ahelper.hpp"
    
APointCloud::APointCloud(const AMesh& referenceMesh, float raysPerUnit, float pointsPerUnit) : referenceMesh(referenceMesh), raysPerUnit(raysPerUnit), pointsPerUnit(pointsPerUnit), pointCloudMesh(this->generatePointCloud()) { }

AMesh APointCloud::generatePointCloud(void) {
    ABoundingBox abb = referenceMesh.getBoundingBox();
    glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);

    float distanceX = abb.getMax().x - abb.getMin().x;
    float distanceY = abb.getMax().y - abb.getMin().y;
    float distanceZ = abb.getMax().z - abb.getMin().z;

    float step = 1.0 / this->raysPerUnit;
    float pointStep = 1.0 / this->pointsPerUnit;

    const std::vector<AVertex> vertices = referenceMesh.getVertices();
    const std::vector<GLuint> indices = referenceMesh.getIndices();

    ARay aray(abb.getMin(), direction);
    ARay fray(abb.getMin(), direction);

    float factor = 0.01f;

    ATriangle atriangle1;
    glm::vec2 baryPosition1;
    float distance1;

    ATriangle atriangle2;
    glm::vec2 baryPosition2;
    float distance2;

    std::vector<AVertex> pointCloudVertices;
    std::vector<unsigned int> pointCloudIndices;
    std::vector<ATexture> textures;

    AVertex vertex;
    GLuint index = -1;

    for(float originX = abb.getMax().x; originX >= abb.getMin().x; originX -= step) {
        aray.origin.x = originX;
        for(float originY = abb.getMax().y; originY >= abb.getMin().y; originY -= step) {
            aray.origin.y = originY;

            if(this->getContactPoint(indices, vertices, aray, baryPosition1, distance1, atriangle1, 0.00f)) {
                glm::vec3 hit = atriangle1.a + (baryPosition1.x * (atriangle1.b - atriangle1.a) + baryPosition1.y * (atriangle1.c - atriangle1.a));
                glm::vec4 hitColor = AHelper::generateRandomColor();

                
                fray.origin.x = hit.x + (fray.direction.x * factor);
                fray.origin.y = hit.y + (fray.direction.y * factor);
                fray.origin.z = hit.z + (fray.direction.z * factor);

                if(this->getContactPoint(indices, vertices, fray, baryPosition2, distance2, atriangle2, factor)) {
                    glm::vec3 hit2 = atriangle2.a + (baryPosition2.x * (atriangle2.b - atriangle2.a) + baryPosition2.y * (atriangle2.c - atriangle2.a));

                    vertex.Normal.x = hitColor.r;
                    vertex.Normal.y = hitColor.g;
                    vertex.Normal.z = hitColor.b;

                    vertex.Position.x = hit.x;
                    vertex.Position.y = hit.y;
                    vertex.Position.z = hit.z;

                    pointCloudVertices.push_back(vertex);
                    pointCloudIndices.push_back(++index);

                    for(float originZ = hit.z; originZ >= hit2.z; originZ -= pointStep) {
                        vertex.Position.x = hit.x;
                        vertex.Position.y = hit.y;
                        vertex.Position.z = originZ;

                        pointCloudVertices.push_back(vertex);
                        pointCloudIndices.push_back(++index);
                    }
                    
                    vertex.Position.x = hit2.x;
                    vertex.Position.y = hit2.y;
                    vertex.Position.z = hit2.z;

                    pointCloudVertices.push_back(vertex);
                    pointCloudIndices.push_back(++index);
                } 
            }
        }
    }

    return AMesh(pointCloudVertices, pointCloudIndices, textures);
}

bool APointCloud::getContactPoint(const std::vector<GLuint>& indices, const std::vector<AVertex>& vertices, const ARay& aray, glm::vec2& baryPosition, float& distance, ATriangle& atriangle, float biggerThan) {
    for(GLuint vertexIndex = 0; vertexIndex < vertices.size(); vertexIndex += 3) {
        atriangle.a = vertices[indices[vertexIndex    ]].Position;
        atriangle.b = vertices[indices[vertexIndex + 1]].Position;
        atriangle.c = vertices[indices[vertexIndex + 2]].Position;

        if(aray.TestARayATriangleIntersection(atriangle, baryPosition, distance) && fabs(distance) > biggerThan) {
            return true;
        }
    }

    return false;
}

const AMesh& APointCloud::getMesh(void) const {
    return this->pointCloudMesh;
}