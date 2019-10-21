#pragma once 

#include <glm/glm.hpp>

class ABoundingBox {
    private:
        glm::vec3 min;
        glm::vec3 max;

    public:
        ABoundingBox(void);
        ABoundingBox(glm::vec3 min, glm::vec3 max);
        ABoundingBox(const ABoundingBox& anotherBoundingBox);
        ~ABoundingBox(void) = default;

        ABoundingBox& operator=(const ABoundingBox& anotherBoundingBox);

        glm::vec3 getMin() const;
        glm::vec3 getMax() const;
};