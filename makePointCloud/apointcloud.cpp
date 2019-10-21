#include "apointcloud.hpp"

#include "RenderingEngine/Core/aboundingbox.hpp"
#include "RenderingEngine/Math/ageometry.hpp"
    
APointCloud::APointCloud(const AMesh& referenceMesh, float density) : referenceMesh(referenceMesh), density(density), pointCloudMesh(this->generatePointCloud()) { }

AMesh APointCloud::generatePointCloud(void) {
    ABoundingBox abb;// = referenceMesh.getBoundingBox();
    glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);

    float distanceX = abb.getMax().x - abb.getMin().x;
    float distanceY = abb.getMax().y - abb.getMin().y;

    float step = 1.0 / this->density;

    const std::vector<AVertex> vertices;// = referenceMesh.getVertices();
    const std::vector<GLuint> indices;// = referenceMesh.getIndices();

    ARay aray(abb.getMin(), direction);
    ARay fray(abb.getMin(), direction);
    ATriangle atriangle;

    glm::vec2 baryPosition1;
    float distance1;

    glm::vec2 baryPosition2;
    float distance2;

    std::vector<AVertex> pointCloudVertices;
    std::vector<unsigned int> pointCloudIndices;
    std::vector<ATexture> textures;

    AVertex vertex;
    GLuint index = -1;
    for(float originX = abb.getMin().x; originX <= distanceX; originX += step) {
        aray.origin.x = originX;
        for(float originY = abb.getMin().y; originY <= distanceY; originY += step) {
            aray.origin.y = originY;
            if(this->getContactPoint(indices, vertices, aray, baryPosition1, distance1)) {
                fray.origin = aray.origin +  direction * (1.0f - baryPosition1.x - baryPosition1.y);
                fray.direction = direction;
                if(this->getContactPoint(indices, vertices, fray, baryPosition2, distance2)) {
                    //Calculate the points by going inside the area
                    for(float originZ = fray.origin.z; originZ <= distance2; originZ += step) {
                        vertex.Position = fray.origin + glm::vec3(0.0f, 0.0f, originZ);
                        pointCloudVertices.push_back(vertex);
                        pointCloudIndices.push_back(++index);
                    }
                }
            }
        }
    }

    return AMesh(pointCloudVertices, pointCloudIndices, textures);
}

bool APointCloud::getContactPoint(const std::vector<GLuint>& indices, const std::vector<AVertex>& vertices, const ARay& aray, glm::vec2& baryPosition, float& distance) {
    ATriangle atriangle;
    for(GLuint vertexIndex = 0; vertexIndex < vertices.size(); vertexIndex += 3) {
        atriangle.a = vertices[indices[vertexIndex    ]].Position;
        atriangle.b = vertices[indices[vertexIndex + 1]].Position;
        atriangle.c = vertices[indices[vertexIndex + 2]].Position;

        if(aray.TestARayATriangleIntersection(atriangle, baryPosition, distance)) {
            return true;
        }
    }

    return false;
}